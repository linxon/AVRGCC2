/*
 * TM1637.c
 *
 * Created: 08.04.2022 1:11:14
 *  Author: linxon
 */


#include "tm1637.h"

static const TM1637_DIGITAL_LED_t led_mask_d PROGMEM = {
	{ '0', TM1637_MASK_ZERO },	{ '1', TM1637_MASK_ONE },
	{ '2', TM1637_MASK_TWO },	{ '3', TM1637_MASK_THREE },
	{ '4', TM1637_MASK_FOUR },	{ '5', TM1637_MASK_FIVE },
	{ '6', TM1637_MASK_SIX },	{ '7', TM1637_MASK_SEVEN },
	{ '8', TM1637_MASK_EIGHT },	{ '9', TM1637_MASK_NINE },

	{ ':', TM1637_MASK_COLON },
	{ '-', TM1637_MASK_MINUS },
	{ '_', TM1637_MASK_UNDERLINE }
};

static byte _buff[TM1637_MAX_SEG_SUPPORT];
char _tm1637_str_buff[TM1637_MAX_SEG_SUPPORT+1];

byte tm1637_get_mask(char sign) {
	register uint8_t i = 0;

	for (; i < ARRAY_SIZE(led_mask_d); ++i) {
		if (my_toupper(sign) == pgm_read_byte(&led_mask_d[i].sign))
			return pgm_read_byte(&led_mask_d[i].mask);
	}

	return EMPTY_BYTE;
}

void tm1637_buff_clear(void) {
	register uint8_t i = 0;

	for (; i < TM1637_MAX_SEG_SUPPORT; ++i)
		_buff[i] = EMPTY_BYTE;
}

void tm1637_buff_fill(char sign) {
	register uint8_t i = 0;

	for (; i < TM1637_MAX_SEG_SUPPORT; ++i) {
		if (_buff[i] == EMPTY_BYTE)
			tm1637_buff_set_sign(sign, i+1);
	}
}

void tm1637_buff_set(byte data, uint8_t pos) {
	pos = (pos > TM1637_MAX_SEG_SUPPORT)? 1: pos;
	_buff[pos-1] |= data;
}

void tm1637_buff_set_sign(char sign, uint8_t pos) {
	pos = (pos > TM1637_MAX_SEG_SUPPORT)? 1: pos;
	_buff[pos-1] = tm1637_get_mask(sign);
}

void tm1637_buff_set_str(const char *s, uint8_t pos) {
	register uint8_t i = 0;

	for (; s[i] != '\0'; ++i, ++pos)
		tm1637_buff_set_sign(s[i], pos);
}

void tm1637_display(void) {
	register uint8_t i = 0;

	_TM1637_START();
	tm1637_send(TM1637_REG_DATA_SET | TM1637_ADDR_FIXED);
	_TM1637_STOP();

	for (; i <= TM1637_MAX_SEG_SUPPORT; ++i) {
		_TM1637_START();

		tm1637_send(TM1637_REG_ADDR_SET + i);
		tm1637_send(_buff[i]);

		_TM1637_STOP();
	}

	_TM1637_START();
	tm1637_send(TM1637_REG_DISPL_SET | TM1637_DISPLAY_ON | TM1637_BRIGHNESS);
	_TM1637_STOP();
}

void tm1637_send(byte data) {
	register uint8_t i = 0;

	for (; i < 8; ++i) {
		if (data & _BV(i))
			TM1637_DIO_2_HIGH();
		else
			TM1637_DIO_2_LOW();

		TM1637_CLK_2_HIGH();
		TM1637_CLK_2_LOW();
	}

	TM1637_CLK_2_LOW();
	TM1637_CLK_2_HIGH();
	TM1637_CLK_2_LOW();
}
