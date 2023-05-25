#include <stdint.h>
#include <avr/eeprom.h>

struct bios_config {
	// Public
	void (*do_spm)(uint16_t, uint8_t, uint16_t); // do_spm(address, command, data);
	uint32_t spm_pagesize; // Size of flash memory page for do_spm function (SPM_PAGESIZE).
	uint16_t ram_limit; // Highest RAM address/total RAM present on system (XRAMEND).
	// Private
	uint16_t console_device; // 16-bit address of console device on ISA bus, for interactive configuration and diagnostics.
	uint16_t console_type; // Integer identifying the console type (internal UART, video card/keyboard, etc.).
	uint16_t boot_device; // 16-bit address of boot drive on ISA bus.
	uint16_t boot_type; // Integer identifying the boot drive type (SD card, CDROM, ATA hard drive, etc.).
};
