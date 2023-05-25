#include <syscall.h>
#include <stdarg.h>
#include <headers/syscall_info.h>

extern struct syscall_info_t *syscall_info_p;

void issue_syscall(int syscall_number, void *return_value, ...)
{
	syscall_info_p->syscall_number = syscall_number;
	syscall_info_p->return_value = return_value;
	va_start(syscall_info_p->args, return_value);
	__syscall();
}
