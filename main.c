#include <string.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "syscall.h"
#include "uart.h"
#include "bios_spm.h"
#include "load.h"
#include "ff15/source/ff.h"
#include "ff15/source/diskio.h"
#include "services.h"
#include "headers/syscall_info.h"
#include "cpustack.h"
#include <stdlib.h>

extern char *__malloc_heap_start;
extern char *__malloc_heap_end;
extern struct syscall_info_t syscall_info;

void (*do_spm)(uint16_t, uint8_t, uint16_t);
uint32_t spm_pagesize;
uint16_t ram_limit;
static FATFS filesystem;

void mount(void)
{
	FRESULT res = f_mount(&filesystem, "0:", 0);
	if (res != FR_OK) {
		puts("Mount error.");
	}
}

void unmount(void)
{
	FRESULT res = f_unmount("0:");
	if (res != FR_OK) {
		puts("Unmount error.");
	}
}

void list(char *path)
{
	DIR dir;
	UINT BytesWritten;
	char string[128];
	
	FRESULT res = f_opendir(&dir, path);
	
	if (res == FR_OK) {
		while (1) {
			FILINFO fno;
			
			res = f_readdir(&dir, &fno);
			
			// Could not read directory
			if ((res != FR_OK) || (fno.fname[0] == 0))
				break;
			
			sprintf(string, "%c%c%c%c %10d %s/%s",
			((fno.fattrib & AM_DIR) ? 'D' : '-'),
			((fno.fattrib & AM_RDO) ? 'R' : '-'),
			((fno.fattrib & AM_SYS) ? 'S' : '-'),
			((fno.fattrib & AM_HID) ? 'H' : '-'),
			(int)fno.fsize, path, fno.fname);
			
			puts(string);
		}
	} else {
		puts("Could not open directory.");
	}
}

/*EntryPoint load_program(char *path)
{
	
}*/

/*int run(EntryPoint other_main)
{
	return other_main();
}*/

int main(void)
{
	__setup_int();
	__save_stack_pointer();
	char command[128];
	char *parse;
	init_serial();
	puts("TestOS version 1.0");
	do_spm = get_do_spm();
	spm_pagesize = get_spm_pagesize();
	ram_limit = get_ram_limit();
	printf("Address of do_spm(): %p\n", do_spm);
	printf("Flash page size: %i bytes\n", spm_pagesize);
	printf("Total RAM: %i bytes\n", ram_limit);
	printf("Heap start: %i bytes\n", __malloc_heap_start);
	printf("Heap end: %i bytes\n", __malloc_heap_end);
	printf("Stack pointer: %i\n", __stack_pointer);
	void *init_malloc = malloc(__malloc_heap_start - __malloc_heap_end);
	free(init_malloc);
	disk_initialize(0);
	int retval;
	entry_point other_main;
	//uint16_t address;
	uint8_t value_at;
	while (1) {
		putchar('.');
		gets_i(command, 128);
		parse = strtok(command, " ");
		if (strcmp(parse, "mount") == 0) {
			mount();
		} else if (strcmp(parse, "unmount") == 0) {
			unmount();
		} else if (strcmp(parse, "list") == 0) {
			parse = strtok(NULL, " ");
			list(parse);
		} else if (strcmp(parse, "load") == 0) {
			parse = strtok(NULL, " ");
			other_main = load(parse);
			printf("Program loaded at address %p.\n", other_main);
		} else if (strcmp(parse, "run") == 0) {
			parse = strtok(NULL, " ");
			retval = -1;
			sscanf(parse, "%x", &other_main);
			if (other_main != NULL) {
				int argc = 0;
				char **argv = malloc((argc + 4) * sizeof(char*));
				argv[argc + 0] = NULL;
				argv[argc + 1] = get_data_section(other_main);
				argv[argc + 2] = get_bss_section(other_main);
				argv[argc + 3] = (char*) &syscall_info;
				other_main(argc, argv);
			}
			printf("Return: %s\n", _getenv("errorlvl"));
		} else if (strcmp(parse, "inspect") == 0) {
			parse = strtok(NULL, " ");
			sscanf(parse, "%x", &other_main);
			memcpy_P(&value_at, other_main, 1);
			printf("%x\n", value_at);
		} else {
			puts("Invalid command.");
		}
	}
	return 0;
}
