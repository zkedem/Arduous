/*
	AVR_BIOS
	Based on ChaN's Stand-alone MMC boot loader (https://github.com/zevero/avr_boot)
*/

//const char filename[13] ="FIRMWARE.BIN\0"; 	// EDIT FILENAME HERE
#include <avr/wdt.h> //Watchdog
// The following code is recommended in http://avr-libc.nongnu.org/user-manual/group__avr__watchdog.html but is disabled for now because avr_boot doesn't currently do anything with mcusr_mirror so for now we will only reset MCUSR and disable WDT.
//uint8_t mcusr_mirror __attribute__ ((section (".noinit")));void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));void get_mcusr(void){mcusr_mirror = MCUSR;MCUSR = 0;wdt_disable();}
void disable_watchdog(void) __attribute__((naked)) __attribute__((section(".init3")));
void disable_watchdog(void)
{
#if defined(MCUCSR)
	MCUCSR = ~(_BV(WDRF));	//Some MCUs require the watchdog reset flag to be cleared before WDT can be disabled. & operation is skipped to spare few bytes as bits in MCUSR can only be cleared.
#else
	MCUSR = ~(_BV(WDRF));	//Some MCUs require the watchdog reset flag to be cleared before WDT can be disabled. & operation is skipped to spare few bytes as bits in MCUSR can only be cleared.
#endif
	wdt_disable();	//immediately disable watchdog in case it was running in the application to avoid perpetual reset loop
}
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <string.h>
#include "pff/src/pff.h"
#include "bios_config.h"
#include "eeprom_util.h"


#if BOOT_ADR > 0xFFFF
  #define PGM_READ_BYTE(x) pgm_read_byte_far(x)
#else
  #define PGM_READ_BYTE(x) pgm_read_byte(x)
#endif

#if USE_UART
  #include "uart/minio.h"
#endif

FATFS Fatfs;					// Petit-FatFs work area 
BYTE Buff[SPM_PAGESIZE];	        	// Page data buffer 

/*void pre_main(void) __attribute__ ((naked)) __attribute__ ((section (".init8")));
void pre_main(void)
{
	__asm__ __volatile__ (
	"	rjmp	1f\n"
	"	rjmp	doFlash\n"
	"1:\n"
	);
}*/

static void do_spm(uint16_t address, uint8_t command, uint16_t data)
{
	// Do spm stuff
	__asm__ __volatile__ (
		"    movw  r0, %3\n"
		"    out %0, %1\n"
		"    spm\n"
		"    clr  r1\n"
		:
		: "i" (_SFR_IO_ADDR(__SPM_REG)),
		  "r" ((uint8_t)command),
		  "z" ((uint16_t)address),
		  "r" ((uint16_t)data)
		: "r0"
	);

	// wait for spm to complete
	//   it doesn't have much sense for __BOOT_PAGE_FILL,
	//   but it doesn't hurt and saves some bytes on 'if'
	boot_spm_busy_wait();
	#if defined(RWWSRE)
		// this 'if' condition should be: (command == __BOOT_PAGE_WRITE || command == __BOOT_PAGE_ERASE)...
		// but it's tweaked a little assuming that in every command we are interested in here, there
		// must be also SELFPRGEN set. If we skip checking this bit, we save here 4B
		if ((command & (_BV(PGWRT)|_BV(PGERS))) && (data == 0)) {
			// Reenable read access to flash
			boot_rww_enable();
		}
	#endif
}

/*static uint8_t pagecmp(const DWORD fa, uint8_t buff[SPM_PAGESIZE])
{
	UINT i;
	uint8_t b_flash,b_buff;
	for (i = 0; i < SPM_PAGESIZE; i++) {
		b_flash = PGM_READ_BYTE(fa + i);
		b_buff = buff[i];
		if (b_flash != b_buff)
			return 1;
	}
	return 0;
}*/

//do_spm(fa, __BOOT_PAGE_ERASE, 0);

void doFlash() {
	DWORD fa;	/* Flash address */
	WORD pa;	/* Address within page */
	WORD data;
	UINT br;	/* Bytes read */
	
	for (fa = 0; fa < BOOT_ADR; fa += SPM_PAGESIZE) {	/* Update all application pages */
		//memset(Buff, 0xFF, SPM_PAGESIZE);
		do_spm(fa, __BOOT_PAGE_ERASE, 0);
		for (pa = 0; pa < SPM_PAGESIZE; pa += sizeof(WORD)) {
			pf_read(&data, sizeof(WORD), &br);
			do_spm(fa + pa, __BOOT_PAGE_FILL, data);
		}
		do_spm(fa, __BOOT_PAGE_WRITE, 0);
		//if (pagecmp(fa, Buff)) {		/* Only flash if page is changed */
		//	flash_erase(fa);		/* Erase a page */
		//	flash_write(fa, Buff);		/* Write it if the data is available */		
		//}
	}
}

int checkFile(char *filename) {
	uint8_t fresult;
	fresult = pf_mount(&Fatfs);	/* Initialize file system */

	if (fresult != FR_OK) { /* File System could not be mounted */
		puts_P(PSTR("Please insert MMC/SD card."));
		return 0;
	}

	fresult = pf_open(filename);

	if (fresult != FR_OK) { /* File could not be opened */
		puts_P(PSTR("Could not open file."));
		return 0;
	}
	doFlash();
	return 1;
}

int main(void)
{
	int fstatus;
	char filename[13];
	struct bios_config current;
	UART_init();
	puts_P(PSTR("AVR_BIOS"));
	eeprom_read_block(&current, (void*) 0, sizeof(struct bios_config));
	current.do_spm = do_spm;
	current.spm_pagesize = SPM_PAGESIZE;
	current.ram_limit = XRAMEND;
	if (memcmp_E(&current, (void*) 0, sizeof(struct bios_config)) != 0) {
		puts_P(PSTR("Updating configuration..."));
		eeprom_write_block(&current, (void*) 0, sizeof(struct bios_config));
	}
	while (1) {
		fstatus = 0;
		putchar('?');
		if (*gets_i(filename, 13))
			fstatus = checkFile(filename);
		if (fstatus && pgm_read_word(0) != 0xFFFF)
			__asm__("jmp 0x00");
	}
}
