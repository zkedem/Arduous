#ifndef SYSCALL_INFO_H_
#define SYSCALL_INFO_H_

#include <stdarg.h>

struct syscall_info_t {
	int syscall_number;
	void *return_value;
	va_list args;
};


#endif /* SYSCALL_INFO_H_ */