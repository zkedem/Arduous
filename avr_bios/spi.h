/*
 * Header for SPI interface.
 */

#ifndef SPI_H_
#define SPI_H_

#include "pff/src/integer.h"

void init_spi(void);
void dly_100us(void);
void select(void);
void deselect(void);
BYTE rcv_spi(void);
void xmit_spi(BYTE d);

#endif /* SPI_H_ */