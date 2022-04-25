#ifndef _DISPLAY_H_
#define _DISPLAY_H_				1

#include "typedef.h"
#include "i2c.h"

// регистр данных
#define LCD_DB0					0
#define LCD_DB1					1
#define LCD_DB2					2
#define LCD_DB3					3
#define LCD_DB4					4
#define LCD_DB5					5
#define LCD_DB6					6
#define LCD_DB7					7

// регистр команд (entry mode)
#define LCD_CLEAR_MASK			0x01
#define LCD_RET_HOME_MASK		0x02
#define LCD_ENTRY_MODE_MASK		0x04
#define LCD_DISPLAY_CTRL_MASK	0x08
#define LCD_CURSOR_SHIFT_MASK	0x10
#define LCD_FUNCTION_SET_MASK	0x20
#define LCD_CGRAM_MASK			0x40
#define LCD_DDRAM_MASK			0x80

// функции (function set)
#define LCD_8BIT_BUS_MASK		0x10
#define LCD_4BIT_BUS_MASK		0x00
#define LCD_2LINE_MASK			0x08
#define LCD_1LINE_MASK			0x00
#define LCD_5X10DOTS_MASK		0x04
#define LCD_5X8DOTS_MASK		0x00

// управление дисплеем
#define LCD_DISPLAY_CURSOR_ON	0x02
#define LCD_DISPLAY_CUR_BLINK   0x01

// команды управления
#define LCD_RS_CMD_MASK			0x00 // регистр команд
#define LCD_RS_DATA_MASK		0x01 // регистр данных
#define LCD_RW_MASK				0x02
#define LCD_EN_MASK				0x04

#define LCD_BACKLIGHT			0x08

// настройки ввода
typedef enum {
	LCD_ENTRY_MODE_INC,
	LCD_ENTRY_MODE_DEC,
	LCD_ENTRY_MODE_SHIFT_LEFT,
	LCD_ENTRY_MODE_SHIFT_RIGHT // Когда
} LCD_ENTRY_MODE_t; // __attribute__((packed))

// настройки дисплея
typedef enum {
	LCD_DISPLAY_ON,
	LCD_DISPLAY_OFF,
	LCD_BACKLIGHT_ON,
	LCD_BACKLIGHT_OFF,
	LCD_DISPLAY_MOVE_LEFT,
	LCD_DISPLAY_MODE_RIGHT,
	LCD_CURSOR_ON,
	LCD_CURSOR_OFF,
	LCD_CURSOR_BLINK_ON,
	LCD_CURSOR_BLINK_OFF
} LCD_DISPLAY_t;

typedef enum {
	LCD_CURSOR_SHIFT_LEFT,
	LCD_CURSOR_SHIFT_RIGHT
} LCD_CURSOR_t;

typedef struct lcd_conf_t {
	struct twi_conf_t twi;

	// настройки разрешения экрана
	uint8_t cols;
	uint8_t rows;
	byte row_addr_table[4];

	// флаговые регистры
	byte display_ctrl_reg;
	byte cursor_shift_reg;
	byte function_set_reg;
	byte entry_mode_reg;

	byte _blacklight;
} LCD_CONFIG_t;

//typedef enum {
//	LCD_DISPLAY_SHOW_MAIN
//} display_menu_opts_t;

extern LCD_CONFIG_t lcd_cfg;

extern void lcd_init(byte addr, uint8_t cols, uint8_t rows);
extern void lcd_send(byte data, byte mode);
extern void lcd_clear(void);
extern void lcd_return_home(void);
extern void lcd_entry_mode(LCD_ENTRY_MODE_t mode);
extern void lcd_cursor_setup(LCD_CURSOR_t mode);
extern void lcd_cursor(uint8_t col, uint8_t row);
extern void lcd_display_setup(LCD_DISPLAY_t mode);
extern void lcd_print(const char *s);
extern void lcd_set_val(const char *v, uint8_t reserve, uint8_t col, uint8_t row);

//void display_menu(display_menu_opts_t menu);

void _twi_lcd_write(byte data);
void _twi_lcd_strobe(byte data);

#endif /* _DISPLAY_H_ */
