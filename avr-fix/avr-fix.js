#!/usr/bin/env node

const process = require('process');
const fs = require('fs');
const path = require('path');
const vsctm = require('vscode-textmate');
const oniguruma = require('vscode-oniguruma');

if (process.argv.length < 3) {
    console.error('No file name given.');
    process.exit(1);
}

/**
 * Utility to read a file as a promise
 */
function readFile(path) {
    return new Promise((resolve, reject) => {
        fs.readFile(path, (error, data) => error ? reject(error) : resolve(data));
    });
}

const wasmBin = fs.readFileSync(path.join(__dirname, './node_modules/vscode-oniguruma/release/onig.wasm')).buffer;
let oldSource = [];
let newSource = [];
//readFile('C:/Users/CollegeBYOD/Documents/Atmel Studio/7.0/Sample/Sample/main.s').then(data => text = data.toString().split(/[\r\n]+/));
readFile(process.argv[2]).then(data => {
    oldSource = data.toString().split(/[\r\n]+/);
    newSource = data.toString().split(/[\r\n]+/);
});

const vscodeOnigurumaLib = oniguruma.loadWASM(wasmBin).then(() => {
    return {
        createOnigScanner(patterns) { return new oniguruma.OnigScanner(patterns); },
        createOnigString(s) { return new oniguruma.OnigString(s); }
    };
});

// Create a registry that can create a grammar from a scope name.
const registry = new vsctm.Registry({
    onigLib: vscodeOnigurumaLib,
    loadGrammar: (scopeName) => {
        if (scopeName === 'source.avrasm') {
            // https://github.com/textmate/javascript.tmbundle/blob/master/Syntaxes/JavaScript.plist
            return readFile(path.join(__dirname, './avrasm.tmLanguage')).then(data => vsctm.parseRawGrammar(data.toString()));
        }
        console.log(`Unknown scope name: ${scopeName}`);
        return null;
    }
});

// Load the JavaScript grammar and any other grammars included by it async.
var dataSymbols = [];
var bssSymbols = [];
var newCode = [];
var isSectionDirective = false;
var isCommDirective = false;
var collectDataSymbols = false;
var collectBssSymbols = false;
var doPatch = false;
var ldiEncountered = false;
var ldsEncountered = false;
var stsEncountered = false;
var isData = false;
var isBss = false;
var currentReg = '';
var currentFunction = '';
var currentSymbol = '';
var nextToken = -1;

registry.loadGrammar('source.avrasm').then(async function(grammar) {
    let ruleStack = vsctm.INITIAL;
    for (let i = 0; i < oldSource.length; i++) {
        const line = oldSource[i];
        const lineTokens = grammar.tokenizeLine(line, ruleStack);
        for (let j = 0; j < lineTokens.tokens.length; j++) {
            const token = lineTokens.tokens[j];
            if (
                token.scopes.includes('meta.preprocessor.directive.avrasm')
                && line.substring(token.startIndex, token.endIndex) == '.section'
            ) {
                isSectionDirective = true;
            } else if (
                token.scopes.includes('meta.preprocessor.directive.avrasm')
                && line.substring(token.startIndex, token.endIndex) == '.comm'
            ) {
                isCommDirective = true;
            }
            if (isSectionDirective) {
                if (
                    (
                        token.scopes.includes('meta.preprocessor.directive.avrasm')
                        && line.substring(token.startIndex, token.endIndex) == '.data'
                    )
                    || (token.scopes.includes('rodata.directive.avrasm'))
                ) {
                    isSectionDirective = false;
                    collectDataSymbols = true;
                    collectBssSymbols = false;
                }
                if (
                    token.scopes.includes('meta.preprocessor.directive.avrasm')
                    && line.substring(token.startIndex, token.endIndex) == '.bss'
                ) {
                    isSectionDirective = false;
                    collectDataSymbols = false;
                    collectBssSymbols = true;
                }
                if (
                    token.scopes.includes('meta.preprocessor.directive.avrasm')
                    && line.substring(token.startIndex, token.endIndex) == '.text'
                ) {
                    isSectionDirective = false;
                    collectDataSymbols = false;
                    collectBssSymbols = false;
                    doPatch = true;
                }
                if (
                    token.scopes.includes('meta.preprocessor.directive.avrasm')
                    && line.substring(token.startIndex, token.endIndex) != '.text'
                ) {
                    doPatch = false;
                }
            }
            if (
                isCommDirective
                && !token.scopes.includes('meta.preprocessor.directive.avrasm')
                && token.scopes.includes('generic.avrasm')
            ) {
                dataSymbols.push(line.substring(token.startIndex, token.endIndex));
                isCommDirective = false;
            }
            if (
                collectDataSymbols
                && token.scopes.includes('entity.name.class.label.avrasm')
            ) {
                dataSymbols.push(line.substring(token.startIndex, token.endIndex - 1));
            }
            if (
                collectBssSymbols
                && token.scopes.includes('entity.name.class.label.avrasm')
            ) {
                bssSymbols.push(line.substring(token.startIndex, token.endIndex - 1));
            }
            if (doPatch) {
                if (
                    token.scopes.includes('keyword.instruction.avrasm')
                    && line.substring(token.startIndex, token.endIndex) == 'ldi'
                ) {
                    ldiEncountered = true;
                    nextToken = j + 2;
                }
                if (
                    token.scopes.includes('keyword.instruction.avrasm')
                    && line.substring(token.startIndex, token.endIndex) == 'lds'
                ) {
                    ldsEncountered = true;
                    nextToken = j + 2;
                }
                if (
                    token.scopes.includes('keyword.instruction.avrasm')
                    && line.substring(token.startIndex, token.endIndex) == 'sts'
                ) {
                    stsEncountered = true;
                    nextToken = j + 2;
                }
                if (ldiEncountered && j == nextToken) {
                    if (token.scopes.includes('variable.language.registers.avrasm')) {
                        currentReg = line.substring(token.startIndex, token.endIndex);
                        nextToken = j + 2;
                    }
                    if (token.scopes.includes('entity.name.function.avrasm')) {
                        currentFunction = line.substring(token.startIndex, token.endIndex);
                        nextToken = j + 2;
                    }
                    if (token.scopes.includes('generic.avrasm')) {
                        if (dataSymbols.includes(line.substring(token.startIndex, token.endIndex))) {
                            if (currentFunction == 'lo8') {
                                newSource.splice(newSource.indexOf(line) + 1, 0, '\tadd ' + currentReg + ',r2');
                            }
                            if (currentFunction == 'hi8') {
                                newSource.splice(newSource.indexOf(line) + 1, 0, '\tadc ' + currentReg + ',r3');
                            }
                        }
                        if (bssSymbols.includes(line.substring(token.startIndex, token.endIndex))) {
                            if (currentFunction == 'lo8') {
                                newSource.splice(newSource.indexOf(line) + 1, 0, '\tadd ' + currentReg + ',r4');
                            }
                            if (currentFunction == 'hi8') {
                                newSource.splice(newSource.indexOf(line) + 1, 0, '\tadd ' + currentReg + ',r5');
                            }
                        }
                    }
                    // End ldi procedure upon reaching end of line:
                    if (j == lineTokens.tokens.length - 1) {
                        newCode = [];
                        currentReg = '';
                        currentFunction = '';
                        ldiEncountered = false;
                        nextToken = -1;
                    }
                }
                if (ldsEncountered && j == nextToken) {
                    if (token.scopes.includes('variable.language.registers.avrasm')) {
                        currentReg = line.substring(token.startIndex, token.endIndex);
                        nextToken = j + 2;
                    } else {
                        if (dataSymbols.includes(line.substring(token.startIndex, token.endIndex))) {
                            currentSymbol = line.substring(token.startIndex, line.length);
                            isData = true;
                            isBss = false;
                        }
                        if (bssSymbols.includes(line.substring(token.startIndex, token.endIndex))) {
                            currentSymbol = line.substring(token.startIndex, line.length);
                            isData = false;
                            isBss = true;
                        }
                    }
                    if (j == lineTokens.tokens.length - 1) {
                        newCode.push('\tpush r30');
                        newCode.push('\tpush r31');
                        newCode.push('\tldi r30,lo8(' + currentSymbol + ')');
                        newCode.push('\tldi r31,hi8(' + currentSymbol + ')');
                        if (isData) {
                            newCode.push('\tadd r30,r2');
                            newCode.push('\tadc r31,r3');
                        } else if (isBss) {
                            newCode.push('\tadd r30,r4');
                            newCode.push('\tadc r31,r5');
                        }
                        newCode.push('\tld ' + currentReg + ',Z');
                        newCode.push('\tpop r31');
                        newCode.push('\tpop r30');
                        newSource.splice(newSource.indexOf(oldSource[i]), 1, ...newCode);
                        newCode = [];
                        currentReg = '';
                        currentFunction = '';
                        currentSymbol = '';
                        ldsEncountered = false;
                        isData = false;
                        isBss = false;
                        nextToken = -1;
                    }
                }
                if (stsEncountered && j == nextToken) {
                    if (token.scopes.includes('generic.avrasm')) {
                        if (dataSymbols.includes(line.substring(token.startIndex, token.endIndex))) {
                            currentSymbol = line.substring(token.startIndex, line.indexOf(','));
                            isData = true;
                            isBss = false;
                        }
                        if (bssSymbols.includes(line.substring(token.startIndex, token.endIndex))) {
                            currentSymbol = line.substring(token.startIndex, line.indexOf(','));
                            isData = false;
                            isBss = true;
                        }
                        for (let k = 0; k < lineTokens.tokens.length; k++) {
                            if (lineTokens.tokens[k].scopes.includes('comma.avrasm')) {
                                nextToken = k + 1;
                            }
                        }
                    } else {
                        if (token.scopes.includes('variable.language.registers.avrasm')) {
                            currentReg = line.substring(token.startIndex, token.endIndex);
                        }
                    }
                    if (j == lineTokens.tokens.length - 1) {
                        newCode.push('\tpush r30');
                        newCode.push('\tpush r31');
                        newCode.push('\tldi r30,lo8(' + currentSymbol + ')');
                        newCode.push('\tldi r31,hi8(' + currentSymbol + ')');
                        if (isData) {
                            newCode.push('\tadd r30,r2');
                            newCode.push('\tadc r31,r3');
                        } else if (isBss) {
                            newCode.push('\tadd r30,r4');
                            newCode.push('\tadc r31,r5');
                        }
                        newCode.push('\tst Z,' + currentReg);
                        newCode.push('\tpop r31');
                        newCode.push('\tpop r30');
                        newSource.splice(newSource.indexOf(oldSource[i]), 1, ...newCode);
                        newCode = [];
                        currentReg = '';
                        currentFunction = '';
                        currentSymbol = '';
                        stsEncountered = false;
                        isData = false;
                        isBss = false;
                        nextToken = -1;
                    }
                }
            }
        }
        ruleStack = lineTokens.ruleStack;
    }
    fs.access(process.argv[2] + '.og', err => {
        if (err) {
            fs.copyFile(process.argv[2], process.argv[2] + '.og', () => {
                fs.writeFileSync(process.argv[2], newSource.join('\n'));
            });
        } else {
            console.error('Already patched.');
            process.exit(2);
        }
    });
});