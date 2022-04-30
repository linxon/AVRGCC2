#ifndef _EEPROM_H_
#define _EEPROM_H_ 1

//#include <util/parity.h>
#include "typedef.h"

#define EEPROM_BUFF_IS_NOT_READY		0
#define EEPROM_BUFF_IS_READY			1

#define EEPROM_COMMIT_READ				0
#define EEPROM_COMMIT_WRITE				1
#define EEPROM_COMMIT_WR_IF_NOT_INIT	2

#define EEPROM_BUFF_EMPTY_IDX			-1
#define EEPROM_BUFF_SIZE				8 // (8 * 2) - 1 = 15 свободных ¤чеек при включенном прерывании (максимум 127)

#define EEPROM_ENABLE_INTERRUPT()		(EECR |= _BV(EERIE))
#define EEPROM_DISABLE_INTERRUPT()		(EECR &= ~_BV(EERIE))

typedef byte *eeprom_data_ptr_t;
typedef byte eeprom_rw_mode_t;

typedef struct eeprom_buff_t {
	uint16_t addr;
	eeprom_data_ptr_t data;
	size_t r_size; // реальный, динамически измен¤емый размер данных с учетом записи буфера
} EEPROM_BUFF_t[EEPROM_BUFF_SIZE];

typedef struct eeprom_stat_t {
	volatile int8_t buff_idx;
	volatile int8_t buff_idx_free;
	volatile int8_t buff_pointer;
} EEPROM_STAT_t;

extern void EEPROM_set_addr(uint16_t);
extern uint16_t EEPROM_get_addr(void);
extern byte EEPROM_read_byte(uint16_t);
extern void EEPROM_write_byte(uint16_t, byte);
extern void EEPROM_format(uint16_t, uint16_t);
extern void EEPROM_read(uint16_t, void *, size_t);
extern void EEPROM_write(uint16_t, void *, size_t);
extern void EEPROM_commit(void *, size_t, eeprom_rw_mode_t);
extern void EEPROM_ISR_handler(void);

void _EEPROM_buff_append(uint16_t, void *, size_t);
void _EEPROM_buff_insert(uint16_t, void *);
void _EEPROM_read_block(uint16_t, void *, size_t);
void _EEPROM_write_block(uint16_t, void *, size_t);

#endif /* _EEPROM_H_ */
