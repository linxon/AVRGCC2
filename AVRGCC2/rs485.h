/*
 * rs485.h
 *
 * Created: 22.01.2022 12:27:17
 *  Author: linxon
 */


#ifndef RS485_H_
#define RS485_H_

#include "typedef.h"
#include "functions.h"
#include "usart.h"
#include "timer.h"

#define RS485_GPIO							GPIO_D
#define RS485_DE_RE_PIN						PD2
#define RS485_MAX_SLAVE_ADDR_IDX			32

#define RS485_RW_MODE_FREE					0
#define RS485_RW_MODE_READ					1
#define RS485_RW_MODE_WRITE					2

#define RS485_STAT_ACK_PACKET				0xAA
#define RS485_STAT_NOACK_PACKET				0x55
#define RS485_STAT_REPEAT_PACKET			0x99
#define RS485_STAT_STOP_PACKET				0x66

#if USART_BAUDRATE == 9600 // для других скоростей нужны другие тайминги!
#define RS485_RW_TIMEOUT_ATTEMPT_COUNT		50 // 50 * 5ms = 250ms (50 попыток по 5ms каждый)
#define RS485_RW_TIMEOUT_DELAY_MS			5  // 4.20ms - минимум при передаче 5-ти байтов на 9600 бод
#define RS485_PROTO_TIMEOUT_ATTEMPT_COUNT	10 // 10 * 50us = 500us (10 попыток по 500us+250ms каждый)
#define RS485_PROTO_TIMEOUT_DELAY_US		50
#else
#warning "BAUDRATE ERR"
#endif

#define RS485_DRIVER_ENABLE()				(RS485_GPIO.port |=  _BV(RS485_DE_RE_PIN))
#define RS485_RECEIVER_ENABLE()				(RS485_GPIO.port &= ~_BV(RS485_DE_RE_PIN))

typedef byte *rs485_data_ptr;

typedef enum {
	RS485_CFG_MASTER_MODE,
	RS485_CFG_SLAVE_MODE
} RS485_CFG_MODE_t;

typedef enum {
	RS485_STAT_ERR_NOT_ME,
	RS485_STAT_ERR_CHECKSUM,
	RS485_STAT_OK
} RS485_STAT_t;

typedef struct rs485_cfg_t {
	bool mode;
	byte device_addr;
	byte slave_addr[RS485_MAX_SLAVE_ADDR_IDX];
} RS485_CFG_t;

typedef volatile struct rs485_buff_t {
	rs485_data_ptr d_ptr;
	uint8_t size;
} RS485_TX_BUFF_t, RS485_RX_BUFF_t;

typedef struct rs485_data_t {
	byte addr;		    // адрес Slave/Master устройства
	uint16_t data;      // данные, передаваемые устройству
	uint16_t crc16;		// хеш-сумма
} RS485_TX_t, RS485_RX_t;

extern void rs485_init(struct rs485_cfg_t *);
extern void rs485_ISR_reader(void);
extern void rs485_ISR_writer(void);
extern void rs485_read(rs485_data_ptr, uint8_t);
extern void rs485_write(rs485_data_ptr, uint8_t);
extern RS485_STAT_t rs485_receive_d(struct rs485_data_t *);
extern RS485_STAT_t rs485_send_d(uint8_t, struct rs485_data_t *);
extern void rs485_default_mode(void);
extern void rs485_switch_mode(RS485_CFG_MODE_t);
extern void rs485_send_proto(uint8_t, uint8_t);
extern byte rs485_listen_proto(void);

//void _buff_append(rs485_data_ptr data, uint8_t size);

#endif /* RS485_H_ */
