/*
 * This is a minimal set of I/O functions like those in stdio.h
 * As such, this library is called "minio".
 * This is the implementation file, minio.c
 */

#include "minio.h"
#define 	UCSRA 	UCSR0A
#define 	UCSRB 	UCSR0B
#define 	UCSRC 	UCSR0C
#define 	UBRRH 	UBRR0H
#define 	UBRRL 	UBRR0L
#define 	UDRE 	UDRE0
#define 	UDR 	UDR0
#define 	RXC 	RXC0

#if BOOT_ADR > 0xFFFF
  #define PGM_READ_BYTE(x) pgm_read_byte_far(x)
#else
  #define PGM_READ_BYTE(x) pgm_read_byte(x)
#endif

// Just enable the UART Tx and set baud rate for 38400 on 3.6864MHz (STK500)

void UART_init(void)
{
	UBRRL = (uint8_t)(USE_UART & 0xFF); // SEE HERE: http://wormfood.net/avrbaudcalc.php
	UBRRH = (uint8_t)(USE_UART >> 8);
    UCSRB |= (1 << RXEN0) | (1 << TXEN0);
}

// The classic Tx one character routine
int putchar(int character)
{
    while (!(UCSRA & (1 << UDRE)));
    UDR = (unsigned char) character;
	return character;
}

int getchar()
{
	while (!(UCSRA & (1 << RXC)));
	return UDR;
}

char *gets_i(char *buf, int n)
{
	char ch;
	int i = 0;
	while (1) {
		ch = getchar();
		if (ch == '\r') {
			buf[i] = 0;
			break;
		} else if (ch == '\n') {
			continue;
		} else if ((ch == '\x7F') && (i != 0)) {
			i--;
			putchar(ch);
		} else if ((ch != '\x7F') && (i != n - 1)) {
			buf[i] = ch;
			i++;
			putchar(ch);
		}
	}
	putchar('\r');
	putchar('\n');
	return buf;
}

// Print string from program memory.
int puts_P(const char *str)
{
    char c;
    do {
        c = PGM_READ_BYTE(str++);
        if (c) {
            putchar(c);
        }
    } while (c != 0);
	putchar('\r');
	putchar('\n');
	return 1;
}

// Print string normally.
int puts(const char *str)
{
	char c;
	do {
		c = *(str++);
		if (c) {
			putchar(c);
		}
	} while (c != 0);
	putchar('\r');
	putchar('\n');
	return 1;
}
