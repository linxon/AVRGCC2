/*
 * main.h
 *
 * Created: 07.04.2022 21:01:15
 *  Author: linxon
 */


#ifndef _I2C_H_
#define _I2C_H_

#include <util/twi.h>
#include "typedef.h"

#define TWI_GPIO				GPIO_C
#define TWI_SDA_PIN				PC4
#define TWI_SCL_PIN				PC5

#define TWDR0					0
#define TWDR1					1
#define TWDR2					2
#define TWDR3					3
#define TWDR4					4
#define TWDR5					5
#define TWDR6					6
#define TWDR7					7

#define TWI_PRESCLR_VALUE_1		1
#define TWI_PRESCLR_VALUE_4		4
#define TWI_PRESCLR_VALUE_16	16
#define TWI_PRESCLR_VALUE_64	64

#define TWI_WRITE_APPEND		1
#define TWI_WRITE_NOAPPEND		0

#define TWI_STAT_OK						0
#define TWI_STAT_ERR_START_FAILED		1
#define TWI_STAT_ERR_SEND_ADDR			2
#define TWI_STAT_ERR_STOP_FAILED		3
#define TWI_STAT_ERR_WR_DATA			4

// обратная формула: 100kHz = 16000000 / (16 + 2 * 72 * 1)
// пример: 72 = ( ((16000000 / 1) / 100000) - 16 ) / 2
// пример: 12 = ( ((16000000 / 4) / 100000) - 16 ) / 2
#define TWI_TWBR_CALC(freq, prsclr)		((((F_CPU / (prsclr)) / (freq)) - 16) / 2)
#define TWI_TWBR_TO_FREQ(twbr, prsclr)	(F_CPU / (16 + 2 * (twbr) * (prsclr)))

typedef byte twi_data_t;

typedef struct {
	uint8_t res;
	uint8_t err;
} TWI_STAT_t;

typedef struct twi_conf_t {
	uint32_t freq;
	uint8_t presclr;
	uint8_t addr;
} TWI_CONF_t;

extern TWI_STAT_t twi_stat;

extern void twi_init(TWI_CONF_t *);
extern twi_data_t twi_data_read(void);
extern void twi_data_write(twi_data_t, byte);

bool _twi_send_start(void);
bool _twi_send_addr(byte);
bool _twi_send_stop(void);

#endif /* _I2C_H_ */
