#include <avr/io.h>
#include <avr/pgmspace.h>

/*
 * This is a minimal set of I/O functions like those in stdio.h
 * As such, this library is called "minio".
 * This is the header file, minio.h
 */

// Just enable the UART Tx and set baud rate for 38400 on 3.6864MHz (STK500)
void UART_init(void);
// The classic Tx one character routine
int putchar(int character);
// Receive a character from the UART
int getchar();
// Receive entire line from UART (interactive)
char *gets_i(char *buf, int n);
// classic Tx a C-string routine (string in PROGMEM)
int puts_P(const char *str);
// Alternative version of puts_P()
int puts(const char *str);
