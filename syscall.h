#ifdef __AVR__
	#include <avr/interrupt.h>
    #define MUST_ACK
	#define MUST_SETUP_INT
    #ifdef __AVR_ATmega328P__
		#define INT_TRIGGER INT0
        #define INT_PORT PORTD
        #define INT_PIN PORTD2
		#define INT_DIR_PORT DDRD
		#define INT_DIR_PIN DDD2
    #endif
    #define __syscall() INT_PORT &= ~(1 << INT_PIN)
#endif

#ifdef MUST_ACK
    #ifdef __AVR__
        #define __ack() INT_PORT |= (1 << INT_PIN)
    #endif
#else
    #define __ack()
#endif

#ifdef MUST_SETUP_INT
	#ifdef __AVR__
		#define __setup_int() \
			sei(); \
			EIMSK |= (1 << INT_TRIGGER); \
			INT_DIR_PORT |= _BV(INT_DIR_PIN); \
			__ack()
	#endif
#else
	#define __setup_int()
#endif