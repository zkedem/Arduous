#define F_CPU 16000000L
#define BAUD 9600

#include <avr/io.h>
#include <stdio.h>

#include <util/setbaud.h>
#include <avr/sfr_defs.h>

int uart_putchar(char c, FILE *stream);

int uart_getchar(FILE *stream);

char *gets_i(char *buf, int n);

void init_serial(void);