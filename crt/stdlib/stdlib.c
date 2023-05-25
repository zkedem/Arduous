#include <headers/stdlib.h>
#include <stdio.h>
#include <cpustack.h>
#include <service_request.h>

void abort(void)
{
	exit(1);
}

void exit(int __status)
{
	// TODO: implement atexit()
	_Exit(__status);
}

void _Exit(int status)
{
	char errorlvl_value[((sizeof(int) / 2) * 3 + sizeof(int)) + 2];
	sprintf(errorlvl_value, "%i", status);
	setenv("errorlvl", errorlvl_value, 1);
	__return_immediately();
}

char *getenv(const char *name)
{
	char *value;
	issue_syscall(6, &value, name);
	return value;
}

int setenv(const char *envname, const char *envval, int overwrite)
{
	int result;
	issue_syscall(7, &result, envname, envval, overwrite);
	return result;
}