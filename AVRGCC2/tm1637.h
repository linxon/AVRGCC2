/*
 * tm1637.h
 *
 * Created: 08.04.2022 1:11:44
 *  Author: linxon
 */


#ifndef TM1637_H_
#define TM1637_H_

#include "typedef.h"
#include "functions.h"

#define TM1637_MAX_SEG_SUPPORT	4
#define TM1637_BRIGHNESS		3 // 0-7

#define TM1637_GPIO				GPIO_C
#define TM1637_DIO_PIN			PC3
#define TM1637_CLK_PIN			PC4

#define TM1637_REG_B0			(0)
#define TM1637_REG_B1			(1)
#define TM1637_REG_B2			(2)
#define TM1637_REG_B3			(3)
#define TM1637_REG_B4			(4)
#define TM1637_REG_B5			(5)
#define TM1637_REG_B6			(6)
#define TM1637_REG_B7			(7)

#define TM1637_REG_DATA_SET		0x40
#define TM1637_REG_ADDR_SET		0xC0
#define TM1637_REG_DISPL_SET	0x80

#define TM1637_ADDR_FIXED		(1 << TM1637_REG_B2)
#define TM1637_ADDR_AUTO		(0 << TM1637_REG_B2)
#define TM1637_DISPLAY_ON		(1 << TM1637_REG_B3)
#define TM1637_DISPLAY_OFF		(0 << TM1637_REG_B3)

#define TM1637_MASK_ZERO		0x3F
#define TM1637_MASK_ONE			0x06
#define TM1637_MASK_TWO			0x5B
#define TM1637_MASK_THREE		0x4F
#define TM1637_MASK_FOUR		0x66
#define TM1637_MASK_FIVE		0x6D
#define TM1637_MASK_SIX			0x7D
#define TM1637_MASK_SEVEN		0x07
#define TM1637_MASK_EIGHT		0x7F
#define TM1637_MASK_NINE		0x6F
#define TM1637_MASK_COLON		0x80
#define TM1637_MASK_MINUS		0x40
#define TM1637_MASK_UNDERLINE	0x08

#define TM1637_DIO_2_HIGH()		(TM1637_GPIO.port |=  _BV(TM1637_DIO_PIN))
#define TM1637_DIO_2_LOW()		(TM1637_GPIO.port &= ~_BV(TM1637_DIO_PIN))
#define TM1637_CLK_2_HIGH()		(TM1637_GPIO.port |=  _BV(TM1637_CLK_PIN))
#define TM1637_CLK_2_LOW()		(TM1637_GPIO.port &= ~_BV(TM1637_CLK_PIN))

#define TM1637_INIT()			(				\
	TM1637_GPIO.ddr |= _BV(TM1637_DIO_PIN),		\
	TM1637_GPIO.ddr |= _BV(TM1637_CLK_PIN),		\
	TM1637_GPIO.port |= _BV(TM1637_DIO_PIN),	\
	TM1637_GPIO.port |= _BV(TM1637_CLK_PIN)		\
)

#define _TM1637_START()			(				\
	TM1637_DIO_2_LOW(),							\
	TM1637_CLK_2_LOW()							\
)

#define _TM1637_STOP()			(				\
	TM1637_DIO_2_LOW(),							\
	TM1637_CLK_2_HIGH(),						\
	TM1637_DIO_2_HIGH()							\
)

#define TM1637_BUFF_SET_NUM_8(n, p)		({		\
	my_itoa_8(n, _tm1637_str_buff);				\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_NUM_16(n, p)	({		\
	my_itoa_16(n, _tm1637_str_buff);			\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_NUM_32(n, p)	({		\
	my_itoa_32(n, _tm1637_str_buff);			\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_NUM_U8(n, p)	({		\
	my_itoa_u8(n, _tm1637_str_buff);			\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_NUM_U16(n, p)	({		\
	my_itoa_u16(n, _tm1637_str_buff);			\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_NUM_U32(n, p)	({		\
	my_itoa_u32(n, _tm1637_str_buff);			\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_HEX_U8(n, p)	({		\
	my_itohex_u8(n, _tm1637_str_buff);			\
	my_strrev(_tm1637_str_buff);				\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_HEX_U16(n, p)	({		\
	my_itohex_u16(n, _tm1637_str_buff);			\
	my_strrev(_tm1637_str_buff);				\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_HEX_U32(n, p)	({		\
	my_itohex_u32(n, _tm1637_str_buff);			\
	my_strrev(_tm1637_str_buff);				\
	tm1637_buff_set_str(_tm1637_str_buff, p);	\
})

#define TM1637_BUFF_SET_STRREV(s, p)	({		\
	char *_strrev_buff = s;						\
	my_strrev(_strrev_buff);					\
	tm1637_buff_set_str(_strrev_buff, p);		\
})

#define TM1637_BUFF_SET_CURSOR(p)		({		\
	tm1637_buff_set(TM1637_MASK_UNDERLINE, p);	\
})

#define TM1637_BUFF_SET_MINUS(p)		({		\
	tm1637_buff_set(TM1637_MASK_MINUS, p);		\
})

#define TM1637_ARGMOD_SHIFT_POS_L(v, start, end)	(	\
	((v) <= conv_to_999(start - end))					\
		? (end) + (((start) - count_999((v))) - (end))	\
		: (end)											\
)

#define TM1637_ARGMOD_SKIP_POS(curr_p, skip_p)	(		\
	(curr_p == skip_p)? curr_p+1: curr_p				\
)

typedef struct tm1637_digital_led {
	const char sign;
	const byte mask;
} TM1637_DIGITAL_LED_t[];

extern char _tm1637_str_buff[];

byte tm1637_get_mask(char);

extern void tm1637_buff_clear(void);
extern void tm1637_buff_fill(char);
extern void tm1637_buff_set(byte, uint8_t);
extern void tm1637_buff_set_sign(char, uint8_t);
extern void tm1637_buff_set_str(const char *, uint8_t);

extern void tm1637_display(void);
extern void tm1637_send(byte);

#endif /* TM1637_H_ */
