/*
 * ir_remote.h
 *
 * Created: 22.01.2022 12:51:21
 *  Author: linxon
 */


#ifndef IR_REMOTE_H_
#define IR_REMOTE_H_

#include "typedef.h"

#define IR_CFG_FRAME_TYPE_NEC			0
#define IR_CFG_FRAME_TYPE_RC_5			1
// TODO: добавить поддержку других форматов
//#define IR_FRAME_TYPE_RC_6	2
//#define IR_FRAME_TYPE_SIRC	3
//#define IR_FRAME_TYPE_JVC		4
#define IR_CFG_POLARITY_ACTIVE_LOW		0
#define IR_CFG_POLARITY_ACTIVE_HIGH		1

#define IR_DATA_IS_NOT_ENTERED			0
#define IR_DATA_IS_ENTERED				1
#define IR_DATA_IS_NOT_REPEATED			0
#define IR_DATA_IS_REPEATED				1

#define IR_EVENT_IDLE					0
#define IR_EVENT_AGC_PULSE				1
#define IR_EVENT_LONG_PAUSE				2
#define IR_EVENT_DATA					3

#define IR_TIMER_COMP_TIMEOUT			2160 // (1080*2) 108.0ms - timeout
#define IR_TIMER_MIN_MAX_V(v, min, max)	(((v) >= ((min)*2) && (v) <= ((max)*2))? 1: 0)

#define IR_NEC_GET_ADDR_FIELD(v)		((v) >> 0)
#define IR_NEC_GET_COMM_FIELD(v)		((v) >> 16)
#define IR_NEC_GET_INVR_FIELD(v)		((v) >> 8)

typedef enum {
	IR_PULSE_FALLING_EDGE,
	IR_PULSE_RISING_EDGE,
	IR_PULSE_TOGGLE
} PULSE_EDGE_MODE_t;

typedef struct ir_cfg_t {
	uint8_t frame_type;
	bool polarity;
	uint8_t source_pin;
	GPIO_t *gpio_ptr;
} IR_CFG_t;

typedef struct ir_callback_t {
	void (*pulse_mode)(PULSE_EDGE_MODE_t mode);
	void (*setup_timer)(void);
	void (*timer_intrr_enable)(void);
	void (*timer_intrr_disable)(void);
} IR_CALLBACK_t;

typedef struct ir_data_t {
	struct ir_cfg_t cfg;
	struct ir_callback_t callback;

	uint8_t addr;
	uint8_t command;

	volatile bool _entered;
	volatile bool _repeated;
} IR_DATA_t;

extern volatile uint32_t ir_raw_data;

extern void ir_init(struct ir_data_t *);
extern void ir_ISR_timer_counter(void);
extern void ir_ISR_handler(void);
extern int8_t ir_NEC_decode(byte *, byte *);
extern void ir_raw_clean(void);
//extern int8_t ir_RC_5_decode(uint16_t *addr, uint16_t *cmd);

void ir_NEC_process(bool);
byte ir_NEC_check_d(uint16_t);
bool ir_read_pulse(void);

// callback
extern void _ir_pulse_mode_by(PULSE_EDGE_MODE_t);
extern void _ir_setup_timer0(void);
extern void _ir_timer_intrr_enable(void);
extern void _ir_timer_intrr_disable(void);

#endif /* IR_REMOTE_H_ */
