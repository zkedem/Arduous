#include "uart.h"

int uart_putchar(char c, FILE *stream) 
{
    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return(0);
}

int uart_getchar(FILE *stream) 
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

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

void init_serial(void) 
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
        
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
    stdout = &uart_output;
    stdin  = &uart_input;
}