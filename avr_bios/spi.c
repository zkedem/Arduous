#include <inttypes.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include "spi.h"
#include "spi_pins.h"

#define _SFR_IO_ADDR(sfr) (sfr)

#define	DDR_CS	_SFR_IO_ADDR(SD_CS_DDR), SD_CS_BIT
#define	PORT_CS	_SFR_IO_ADDR(SD_CS_PORT), SD_CS_BIT

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define bit_of(var, position) (((var) >> (position)) & 1)
#define invoke(macro, ...) macro(__VA_ARGS__)

void init_spi(void)
{
	invoke(sbi, DDR_CS);
	invoke(sbi, DDR_DI);
	invoke(sbi, DDR_CK);
	invoke(sbi, PORT_DO);
}

void dly_100us(void)
{
	_delay_us(100);
}

void select(void)
{
	deselect();
	invoke(cbi, PORT_CS);
	xmit_spi(0xFF);
}

void deselect(void)
{
	invoke(sbi, PORT_CS);
}

BYTE rcv_spi(void)
{
	xmit_spi(0xFF);
	// Implicit return value.
}

void xmit_spi(BYTE d)
{
	for (uint8_t i = 8; i > 0; i--) {
		if (bit_of(d, 7))
			invoke(sbi, PORT_DI);
		else
			invoke(cbi, PORT_DI);
		d <<= 1;
		if (invoke(bit_of, PIN_DO))
			d++;
		invoke(sbi, PORT_CK);
		invoke(cbi, PORT_CK);
	}
}