#include <avr/io.h>
#include <avr/eeprom.h>
#include "bios_config.h"
#include "bios_spm.h"

static struct bios_config current;

void init_info(void) __attribute__((constructor));
void init_info(void)
{
	eeprom_read_block(&current, (void*) 0, sizeof(struct bios_config));
}

void (*get_do_spm(void))(uint16_t, uint8_t, uint16_t)
{
	return current.do_spm;
}

uint32_t get_spm_pagesize(void)
{
	return current.spm_pagesize;
}

uint16_t get_ram_limit(void)
{
	return current.ram_limit;
}