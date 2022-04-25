/*
 * ldm200.h
 *
 * Created: 09.03.2022 20:39:16
 *  Author: linxon
 */


#ifndef LDM200_H_
#define LDM200_H_

#include "typedef.h"
#include "functions.h"

// LDM206 + LDM206 = 12
#define LDM200_MAX_SEG_SUPPORT	6 // LDM205 = 5; LDM206 = 6; LDM208 = 8; ...

#define LDM200_GPIO				GPIO_D
#define LDM200_LOAD_PIN			PD5 // LOAD (SH_CP; сдвиг)
#define LDM200_D_IN_PIN			PD6 // D_IN (DS; данные)
#define LDM200_CLK_PIN			PD7 // CLK (ST_CP; защелка)

#define LDM200_INIT()			(				\
	LDM200_GPIO.ddr  |=  _BV(LDM200_LOAD_PIN),	\
	LDM200_GPIO.port &= ~_BV(LDM200_LOAD_PIN),	\
	LDM200_GPIO.ddr  |=  _BV(LDM200_D_IN_PIN),	\
	LDM200_GPIO.port &= ~_BV(LDM200_D_IN_PIN),	\
	LDM200_GPIO.ddr  |=  _BV(LDM200_CLK_PIN),	\
	LDM200_GPIO.port &= ~_BV(LDM200_CLK_PIN)	\
)

#define LDM200_PULSE_CLK()		(				\
	LDM200_GPIO.port |=  _BV(LDM200_CLK_PIN),	\
	LDM200_GPIO.port &= ~_BV(LDM200_CLK_PIN)	\
)

#define LDM200_PULSE_LOAD()		(				\
	LDM200_GPIO.port |=  _BV(LDM200_LOAD_PIN),	\
	LDM200_GPIO.port &= ~_BV(LDM200_LOAD_PIN)	\
)

#define LDM200_BUFF_SET_NUM_8(n)		({		\
	my_itoa_8(n, _ldm200_str_buff);				\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

#define LDM200_BUFF_SET_NUM_16(n)	({			\
	my_itoa_16(n, _ldm200_str_buff);			\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

#define LDM200_BUFF_SET_NUM_32(n)	({			\
	my_itoa_32(n, _ldm200_str_buff);			\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

#define LDM200_BUFF_SET_NUM_U8(n)	({			\
	my_itoa_u8(n, _ldm200_str_buff);			\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

#define LDM200_BUFF_SET_NUM_U16(n)	({			\
	my_itoa_u16(n, _ldm200_str_buff);			\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

#define LDM200_BUFF_SET_NUM_U32(n)	({			\
	my_itoa_u32(n, _ldm200_str_buff);			\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

#define LDM200_BUFF_SET_HEX_U8(n)	({			\
	my_itohex_u8(n, _ldm200_str_buff);			\
	my_strrev(_ldm200_str_buff);				\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

#define LDM200_BUFF_SET_HEX_U16(n)	({			\
	my_itohex_u16(n, _ldm200_str_buff);			\
	my_strrev(_ldm200_str_buff);				\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

#define LDM200_BUFF_SET_HEX_U32(n)	({			\
	my_itohex_u32(n, _ldm200_str_buff);			\
	my_strrev(_ldm200_str_buff);				\
	ldm200_buff_set_str(_ldm200_str_buff);		\
})

typedef struct ldm200_digital_led {
	const char sign;
	const byte mask;
} LDM200_DIGITAL_LED_t[];

// можно объявить этот массив внутри каждого макроса LDM200_BUFF_SET_*, но дешевле
// иметь один внешний массив для всех
extern char _ldm200_str_buff[];

byte ldm200_get_mask(char);

extern void ldm200_buff_clear(void);
extern void ldm200_buff_fill(char);
extern void ldm200_buff_set(byte, uint8_t);
extern void ldm200_buff_set_sign(char, uint8_t, bool);
extern void ldm200_buff_set_str(const char *);

extern void ldm200_display(void);

#endif /* LDM200_H_ */
