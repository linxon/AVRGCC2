/*
 * ldm200.c
 *
 * Created: 09.03.2022 20:39:06
 *  Author: linxon
 */

/*
		Небольшая библиотека для использования поразрядных, динамических индикаторов LDM200


		Принцип работы:

		Перед тем, как начать работать с индикаторами нужно проинициализировать порты, через
		которые будет происходить передача импульсов и данных.

		Пины:
			LOAD - смещение данных
			D_IN - данные
			CLK  - синхронизация, стробб (загрузка данных из сдвигового регистра в основной и вывод на экран)

		Порядок записи данных:
			1) Пишем один бит в определенную ячейку регистра
			2) Сдвигаем его с помощью короткого импульса на LOAD пине
			3) Снова пункт 1 до тех пор, пока не будут переданы все 8-мь бит
			4) Строббируем получившиеся данные с помощью короткого импульса на CLK (вывод на экран)


		Частота обновления рекомендуется не ниже - 60Hz * <количество сегментов>
		иначе мерцания становятся видимы гразу. Когда данные меняются только в одном
		или сразу в двух регистрах, то <количество сегментов> можно уменьшить.
*/


#include "ldm200.h"


//  --6--
// |     |
// 3     7
// |     |
//  --1--
// |     |
// 2     4
// |     |
//  --0--   .5
// для наглядности лучше будем работать с байтами
static const LDM200_DIGITAL_LED_t led_mask_d PROGMEM = {
	// цифры
	{ '0', 0b11011101 }, { '1', 0b10010000 },
	{ '2', 0b11000111 }, { '3', 0b11010011 },
	{ '4', 0b10011010 }, { '5', 0b01011011 },
	{ '6', 0b01011111 }, { '7', 0b11010000 },
	{ '8', 0b11011111 }, { '9', 0b11011011 },

	// буквы и символы (только в режиме upper case)
	{ 'A', 0b11011110 },
	{ 'B', 0b00011111 },
	{ 'C', 0b00000111 },
	{ 'D', 0b10010111 },
	{ 'E', 0b01001111 },
	{ 'F', 0b01001110 },
	{ 'H', 0b00011110 },
	{ 'R', 0b00000110 },
	{ 'L', 0b00001101 },
	{ 'O', 0b00010111 },
	{ 'Q', 0b00110111 },
	{ 'P', 0b11001110 },
	{ 'T', 0b00001111 },
	{ 'U', 0b00010101 },
	{ 'N', 0b00010110 },
	{ '-', 0b00000010 },
	{ '_', 0b00000001 },
	{ ' ', 0b00000000 }
};

static byte _buff[LDM200_MAX_SEG_SUPPORT];
char _ldm200_str_buff[LDM200_MAX_SEG_SUPPORT+1];

byte ldm200_get_mask(char sign) {
	register uint8_t i = 0;

	for (; i < ARRAY_SIZE(led_mask_d); ++i) {
		if (my_toupper(sign) == pgm_read_byte(&led_mask_d[i].sign))
			return pgm_read_byte(&led_mask_d[i].mask);
	}

	return EMPTY_BYTE;
}

void ldm200_buff_clear(void) {
	register uint8_t i = 0;

	for (; i < LDM200_MAX_SEG_SUPPORT; ++i)
		_buff[i] = EMPTY_BYTE;
}

// функция-заполняшка, которая умеет заполнять данными пустые разряды индикатора
void ldm200_buff_fill(char sign) {
	register uint8_t i = 0;

	for (; i < LDM200_MAX_SEG_SUPPORT; ++i) {
		if (_buff[i] == EMPTY_BYTE)
			ldm200_buff_set_sign(sign, i+1, FALSE);
	}
}

void ldm200_buff_set(byte data, uint8_t pos) {
	pos = (pos > LDM200_MAX_SEG_SUPPORT)? 1: pos;
	_buff[pos-1] |= data;
}

void ldm200_buff_set_sign(char sign, uint8_t pos, bool doten) {
	pos = (pos > LDM200_MAX_SEG_SUPPORT)? 1: pos;
	_buff[pos-1] = ldm200_get_mask(sign) | (doten? 0x20: 0);
}

void ldm200_buff_set_str(const char *s) {
	register uint8_t i = 0;
	register uint8_t pos = 0;

	for (; s[i] != '\0'; ++i) {
		pos++;

		if (s[i] == '.') {
			pos--;
			continue;
		}

		ldm200_buff_set_sign(s[i], pos, (s[i+1] == '.'? TRUE: FALSE));
	}
}

// Основная функция для отображения информации на экране
void ldm200_display(void) {
	register uint8_t i = LDM200_MAX_SEG_SUPPORT;
	register uint8_t j = 0;

	// <число разрядов> * 8 (6 * 8 = 48 битов)
	for (; i > 0; --i) {

		for (; j < 8; ++j) {
			// маску нужно инвертировать (т.е. вместо 1 подавать 0 - общий анод)
			if (_buff[i-1] & _BV(7 - j))
				LDM200_GPIO.port &= ~_BV(LDM200_D_IN_PIN);
			else
				LDM200_GPIO.port |=  _BV(LDM200_D_IN_PIN);

			LDM200_PULSE_CLK(); // стробируем данные
		}

		j = 0;
	}

	LDM200_PULSE_LOAD(); // загружаем биты в основные регистры на вывод
}
