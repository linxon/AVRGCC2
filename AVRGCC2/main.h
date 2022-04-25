/*
 * main.h
 *
 * Created: 21.01.2022 21:40:29
 *  Author: linxon
 */


#ifndef _MAIN_H_
#define _MAIN_H_

#include "typedef.h"

// Модули
//#include "usart.h"
//#include "eeprom.h"
#include "rs485.h"
//#include "ldm200.h"
#include "tm1637.h"

#define LED_ON()		(GPIO_B.port |=  _BV(PB1))
#define LED_OFF()		(GPIO_B.port &= ~_BV(PB1))

typedef volatile struct {
	byte edit_mode		: 1;
	byte clock_updated	: 1;
	byte min_entered	: 1;
	byte hour_entered	: 1;
	byte led_en			: 1;
} MAIN_CYCLE_FLAGS_t;

void init_me(void);
void blink_led(void);

#endif /* _MAIN_H_ */
