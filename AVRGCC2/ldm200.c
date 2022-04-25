/*
 * ldm200.c
 *
 * Created: 09.03.2022 20:39:06
 *  Author: linxon
 */

/*
		��������� ���������� ��� ������������� �����������, ������������ ����������� LDM200


		������� ������:

		����� ���, ��� ������ �������� � ������������ ����� ������������������� �����, �����
		������� ����� ����������� �������� ��������� � ������.

		����:
			LOAD - �������� ������
			D_IN - ������
			CLK  - �������������, ������ (�������� ������ �� ���������� �������� � �������� � ����� �� �����)

		������� ������ ������:
			1) ����� ���� ��� � ������������ ������ ��������
			2) �������� ��� � ������� ��������� �������� �� LOAD ����
			3) ����� ����� 1 �� ��� ���, ���� �� ����� �������� ��� 8-�� ���
			4) ����������� ������������ ������ � ������� ��������� �������� �� CLK (����� �� �����)


		������� ���������� ������������� �� ���� - 60Hz * <���������� ���������>
		����� �������� ���������� ������ �����. ����� ������ �������� ������ � �����
		��� ����� � ���� ���������, �� <���������� ���������> ����� ���������.
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
// ��� ����������� ����� ����� �������� � �������
static const LDM200_DIGITAL_LED_t led_mask_d PROGMEM = {
	// �����
	{ '0', 0b11011101 }, { '1', 0b10010000 },
	{ '2', 0b11000111 }, { '3', 0b11010011 },
	{ '4', 0b10011010 }, { '5', 0b01011011 },
	{ '6', 0b01011111 }, { '7', 0b11010000 },
	{ '8', 0b11011111 }, { '9', 0b11011011 },

	// ����� � ������� (������ � ������ upper case)
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

// �������-����������, ������� ����� ��������� ������� ������ ������� ����������
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

// �������� ������� ��� ����������� ���������� �� ������
void ldm200_display(void) {
	register uint8_t i = LDM200_MAX_SEG_SUPPORT;
	register uint8_t j = 0;

	// <����� ��������> * 8 (6 * 8 = 48 �����)
	for (; i > 0; --i) {

		for (; j < 8; ++j) {
			// ����� ����� ������������� (�.�. ������ 1 �������� 0 - ����� ����)
			if (_buff[i-1] & _BV(7 - j))
				LDM200_GPIO.port &= ~_BV(LDM200_D_IN_PIN);
			else
				LDM200_GPIO.port |=  _BV(LDM200_D_IN_PIN);

			LDM200_PULSE_CLK(); // ���������� ������
		}

		j = 0;
	}

	LDM200_PULSE_LOAD(); // ��������� ���� � �������� �������� �� �����
}
