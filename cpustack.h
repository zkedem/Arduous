#ifdef __AVR__
	#define __save_stack_pointer() \
		__asm__ __volatile__( \
			"in r6,__SP_L__\n\t" \
			"in r7,__SP_H__\n\t" \
		)

	#define __return_immediately() \
		__asm__ __volatile__( \
			"out __SP_L__,r6\n\t" \
			"out __SP_H__,r7\n\t" \
			"ret\n\t" \
		)
	register void *__stack_pointer __asm__("r6");
#endif