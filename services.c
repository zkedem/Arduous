#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syscall.h"
#include "headers/syscall_info.h"
#include "services.h"

struct env_list {
	char *statement;
	struct env_list *next;
};

struct syscall_info_t syscall_info;
static struct env_list *envlh = NULL;

char *_getenv(const char *name) {
	static char *value = NULL;
	free(value);
	struct env_list *envlp = envlh;
	if (envlp == NULL)
		return NULL;
	size_t eq_index;
	while ((strncmp(name, envlp->statement, (eq_index = strcspn(envlp->statement, "="))) != 0) || (envlp->next != NULL))
		envlp = envlp->next;
	value = malloc((eq_index + 1) * sizeof(char));
	strncpy(value, envlp->statement, eq_index);
	value[eq_index] = '\0';
	return value;
}

int setenv(const char *envname, const char *envval, int overwrite)
{
	if (strchr(envname, (int) '=') != NULL)
		return -1;
	struct env_list *envlp1 = envlh;
	struct env_list *envlp2 = NULL;
	size_t eq_index;
	if (envlh == NULL) {
		envlh = malloc(sizeof(struct env_list));
		envlh->next = NULL;
		envlp2 = envlh;
	} else {
		while ((strncmp(envname, envlp1->statement, (eq_index = strcspn(envlp1->statement, "="))) != 0) || (envlp1->next != NULL))
			envlp1 = envlp1->next;
		if ((strncmp(envname, envlp1->statement, eq_index) == 0) && overwrite) {
			free(envlp1->statement);
			envlp2 = envlp1;
		} else if (envlp1->next == NULL) {
			envlp1->next = malloc(sizeof(struct env_list));
			envlp1->next->next = NULL;
			envlp2 = envlp1->next;
		}
	}
	if (envlp2 != NULL) {
		envlp2->statement = (char*) malloc((strlen(envname) + strlen(envval) + 2) * sizeof(char));
		strcpy(envlp2->statement, envname);
		strcat(envlp2->statement, "=");
		strcat(envlp2->statement, envval);
	}
	return 0;
}

void serve_syscall(void)
{
	switch (syscall_info.syscall_number) {
	case 0:
		*((char*) syscall_info.return_value) = getchar();
		break;
	case 1:
		*((char**) syscall_info.return_value) = gets(va_arg(syscall_info.args, char*));
		break;
	case 2:
		*((int*) syscall_info.return_value) = putchar(va_arg(syscall_info.args, int));
		break;
	case 3:
		*((int*) syscall_info.return_value) = puts(va_arg(syscall_info.args, char*));
		break;
	case 4:
		*((int*) syscall_info.return_value) = vsprintf(va_arg(syscall_info.args, char*), va_arg(syscall_info.args, char*), va_arg(syscall_info.args, va_list));
	case 5:
		*((void**) syscall_info.return_value) = malloc(va_arg(syscall_info.args, size_t));
		break;
	case 6:
		free(va_arg(syscall_info.args, void*));
		break;
	case 7:
		*((char**) syscall_info.return_value) = _getenv(va_arg(syscall_info.args, char*));
		break;
	case 8:
		*((int*) syscall_info.return_value) = setenv(va_arg(syscall_info.args, char*), va_arg(syscall_info.args, char*), va_arg(syscall_info.args, int));
		break;
	default:
		break;
	}
	va_end(syscall_info.args);
	__ack();
}