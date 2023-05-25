#ifndef SERVICES_H_
#define SERVICES_H_

#ifdef __AVR__
	#include <avr/interrupt.h>
	#define serve_syscall INT0_vect
	void serve_syscall(void) __attribute__((__signal__, __INTR_ATTRS));
#else
	void serve_syscall(void);
#endif

char *_getenv(const char *name);
int setenv(const char *envname, const char *envval, int overwrite);


#endif /* SERVICES_H_ */