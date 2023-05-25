#include <stdint.h>

#ifndef BIOS_SPM_H_
#define BIOS_SPM_H_


void (*get_do_spm(void))(uint16_t address, uint8_t command, uint16_t data);
uint32_t get_spm_pagesize(void);
uint16_t get_ram_limit(void);


#endif /* BIOS_SPM_H_ */