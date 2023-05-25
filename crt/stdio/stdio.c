/*
 * stdio.c
 *
 * Created: 5/16/2023 5:55:22 PM
 *  Author: CollegeBYOD
 */

#include <headers/stdio.h>
#include <service_request.h>
#include <stdarg.h>

int sprintf(char *__s, const char *__fmt, ...)
{
	int result;
	va_list args;
	va_start(args, __fmt);
	issue_syscall(4, &result, __s, __fmt, args);
	return result;
}

int vsprintf(char *__s, const char *__fmt, va_list ap)
{
	int result;
	issue_syscall(4, &result, __s, __fmt, ap);
	return result;
}