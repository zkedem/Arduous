#include <stdint.h>
#include <avr/eeprom.h>
#include "eeprom_util.h"

/*
 * Adapted from https://opensource.apple.com/source/tcl/tcl-3.1/tcl/compat/memcmp.c
 * s1 is in RAM, s2 is in EEPROM.
 */
int memcmp_E(const void *s1, const void *s2, size_t len)
{
	uint8_t u1, u2;
	for (; len--; s1++, s2++) {
		u1 = * (uint8_t*) s1;
		u2 = eeprom_read_byte((uint8_t*) s2);
		if (u1 != u2)
			return (u1 - u2);
	}
	return 0;
}