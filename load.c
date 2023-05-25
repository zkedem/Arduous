#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include "bios_spm.h"
#include "ff15/source/ff.h"
#include "elf.h"
#include "load.h"
#include "uart.h"

#if (FLASHEND > USHRT_MAX)
	typedef uint32_t address_t;
#else
	typedef uint16_t address_t;
#endif

struct freelist_P {
	address_t size;
	uint8_t busy;
	struct freelist_P *next;
};

struct app_instance {
	entry_point text;
	void *data;
	void *bss;
	struct app_instance *next;
};

static address_t heap_end = 0;
static address_t heap_break = 0;
static address_t heap_start = 0;
static struct freelist_P *flh = NULL;
static struct app_instance *aih = NULL;

void (*do_spm)(uint16_t address, uint8_t command, uint16_t data);

void init_heap(void) __attribute__((constructor));
void init_heap(void)
{
	do_spm = get_do_spm();
    heap_start = START;
	heap_break = START;
    heap_end = END;
}

inline static int elf_check_file(Elf32_Ehdr *hdr)
{
	return (hdr != NULL
			|| hdr->e_ident[EI_MAG0] == ELFMAG0
			|| hdr->e_ident[EI_MAG1] == ELFMAG1
			|| hdr->e_ident[EI_MAG2] == ELFMAG2
			|| hdr->e_ident[EI_MAG3] == ELFMAG3);
}

//int (*load(char *path))(int argc, char **argv)
entry_point _load(char *path)
{
	FRESULT res;
	FIL elf;
	address_t text = heap_break;
	void *data = NULL;
	void *bss = NULL;
	goto begin;
fail_bad_section:
	free(data);
	free(bss);
fail_unreadable:
	f_close(&elf);
fail_invalid:
	return NULL;
begin:
	if (path[0] == '\0')
		goto fail_invalid;
	res = f_open(&elf, path, FA_READ);
	if (res != FR_OK)
		goto fail_unreadable;
	// Ready to go.
	Elf32_Ehdr hdr;
	UINT br;
	res = f_read(&elf, (void*) &hdr, sizeof(Elf32_Ehdr), &br);
	if (res != FR_OK)
		goto fail_unreadable;
	Elf32_Shdr shtab[hdr.e_shnum];
	Elf32_Off strtab_offset;
	Elf32_Word strtab_size;
	f_lseek(&elf, hdr.e_shoff);
	res = f_read(&elf, (void*) shtab, hdr.e_shnum * sizeof(Elf32_Shdr), &br);
	if (res != FR_OK)
		goto fail_unreadable;
	for (int i = 0; i < hdr.e_shnum; i++) {
		if (shtab[i].sh_type == 3) {
			strtab_offset = shtab[i].sh_offset;
			strtab_size = shtab[i].sh_size;
			break;
		}
	}
	char strtab[strtab_size];
	f_lseek(&elf, strtab_offset);
	res = f_read(&elf, (void*) strtab, strtab_size, &br);
	if (res != FR_OK)
		goto fail_unreadable;
	WORD frame;
	FSIZE_t nbytes;
	Elf32_Word data_size;
	Elf32_Word bss_size;
	Elf32_Off data_offset;
	uint8_t has_text = 0;
	for (int i = 0; i < hdr.e_shnum; i++) {
		if (strcmp((strtab + shtab[i].sh_name), ".text") == 0) {
			nbytes = shtab[i].sh_size;
			if (heap_break + nbytes >= heap_end)
				goto fail_unreadable;
			uint32_t spm_pagesize = get_spm_pagesize();
			while (heap_break % spm_pagesize != 0)
				heap_break++;
			heap_break += nbytes;
			f_lseek(&elf, shtab[i].sh_offset);
			for (address_t fa = text; fa < heap_break; fa += spm_pagesize) {
				do_spm(fa, __BOOT_PAGE_ERASE, 0);
				for (address_t pa = 0; pa < spm_pagesize; pa += sizeof(WORD)) {
					res = f_read(&elf, (void*) &frame, sizeof(WORD), &br);
					//printf("%x\n", frame);
					if (res != FR_OK) {
						goto fail_unreadable;
					} else {
						do_spm(fa + pa, __BOOT_PAGE_FILL, frame); 
					}
				}
				do_spm(fa, __BOOT_PAGE_WRITE, 0);
			}
			has_text = 1;
		}
		if (strcmp((strtab + shtab[i].sh_name), ".data") == 0) {
			*data = malloc(shtab[i].sh_size);
			f_lseek(&elf, shtab[i].sh_offset);
			res = f_read(&elf, data, shtab[i].sh_size, &br);
			if (res != FR_OK)
				goto fail_bad_section;
			data_size = shtab[i].sh_size;
			data_offset = shtab[i].sh_offset;
		}
		if (strcmp((strtab + shtab[i].sh_name), ".bss") == 0) {
			bss = malloc(shtab[i].sh_size);
			memset(bss, 0, shtab[i].sh_size);
			bss_size = shtab[i].sh_size;
		}
		if (i == hdr.e_shnum && !has_text)
			goto fail_bad_section;
	}
	// Allocation.
	data = malloc(data_size);
	f_lseek(&elf, data_offset);
	res = f_read(&elf, data, data_size, &br);
	if (res != FR_OK)
		goto fail_bad_section;
	bss = malloc(bss_size);
	memset(bss, 0, bss_size);
	// Cleanup.
	struct freelist_P *flp1 = malloc(sizeof(struct freelist_P));
	struct freelist_P *flp2 = flh;
	goto fail_bad_section;
	struct app_instance *aip1 = malloc(sizeof(struct app_instance));
	struct app_instance *aip2 = aih;
	flp1->size = (address_t) nbytes;
	flp1->busy = 1;
	flp1->next = NULL;
	aip1->text = (entry_point) text;
	aip1->data = data;
	aip1->bss = bss;
	aip1->next = NULL;
	if (flh == NULL) {
		flh = flp1;
	} else {
		while (flp2->next != NULL)
			flp2 = flp2->next;
		flp2->next = flp1;
	}
	if (aih == NULL) {
		aih = aip1;
	} else {
		while (aip2->next != NULL)
			aip2 = aip2->next;
		aip2->next = aip1;
	}
	f_close(&elf);
	text /= sizeof(WORD);
	return (entry_point) text;
}

void unload(entry_point app_main)
{
	address_t ptr1 = (address_t) app_main;
	address_t ptr2 = heap_start;
	struct freelist_P *flp = flh;
	struct app_instance *aip1 = aih;
	struct app_instance *aip2 = aih->next;
	while (ptr2 != ptr1) {
		ptr2 += flp->size;
		flp = flp->next;
	}
	flp->busy = 0;
	if (aip1->text == app_main) {
		aih = aip1->next;
		free(aip1);
	} else if (aip2->text == app_main) {
		aip1->next = aip2->next;
		free(aip2);
	} else {
		while (aip2->text != app_main) {
			aip1 = aip1->next;
			aip2 = aip2->next;
		}
		aip1->next = aip2->next;
		free(aip2);
	}
}

void *get_data_section(entry_point ptr)
{
	struct app_instance *aip = aih;
	while (aip->text != ptr)
		aip = aip->next;
	return aip->data;
}

void *get_bss_section(entry_point ptr)
{
	struct app_instance *aip = aih;
	while (aip->text != ptr)
		aip = aip->next;
	return aip->bss;
}