#include <headers/stdlib.h>
#include <headers/syscall_info.h>
#include <cpustack.h>

register void *__data_section __asm__("r2");
register void *__bss_section __asm__("r4");
struct syscall_info_t *syscall_info_p;

extern int main(int argc, char **argv);

void _start(int argc, char **argv)
{
	__save_stack_pointer();
	__data_section = (void*) argv[argc + 1];
	__bss_section = (void*) argv[argc + 2];
	syscall_info_p = (struct syscall_info_t*) argv[argc + 3];
	int status = main(argc, argv);
	exit(status);
}