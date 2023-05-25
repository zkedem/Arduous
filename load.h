#ifndef LOAD_H
#define LOAD_H

#include <avr/io.h>
#include <avr/boot.h> // Or similar
#include <stdint.h>

#define START 18000
#define END 57343

typedef void (*entry_point)(int, char**);

entry_point load(char *path);
void unload(entry_point ptr);
void *get_data_section(entry_point ptr);
void *get_bss_section(entry_point ptr);

#endif // LOAD_H
