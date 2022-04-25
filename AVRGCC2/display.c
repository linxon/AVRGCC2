#include "display.h"

LCD_CONFIG_t lcd_cfg = {
	.twi = {
		.addr = 0x27, // адрес по умолчанию
		.freq = 0x48, // 100kHz = 16000000 / (16 + 2 * 72 * 1)
	},

	.cols = 16,
	.rows = 2,

	// Конфигурация для 1602 и 1604:
	//		0x00 - адрес 1-й строки дисплея
	//		0x40 - адрес 2-й строки
	//		0x14 - 3-й строки
	//		0x54 - 4-й строки
	.row_addr_table = { 0x00, 0x40, 0x14, 0x54 }, // TODO: добавить поддержку 1601

	// настройки дисплея по умолчанию
	.display_ctrl_reg = LCD_DISPLAY_CTRL_MASK
			| (1 << LCD_DB2)  // включаем дисплей
			| (1 << LCD_DB1)  // включаем курсор
			| (1 << LCD_DB0), // включаем мигание курсора
	.function_set_reg = LCD_FUNCTION_SET_MASK
			| LCD_4BIT_BUS_MASK
			| LCD_2LINE_MASK
			| LCD_5X8DOTS_MASK,
	.entry_mode_reg = LCD_ENTRY_MODE_MASK
			| ~(1 << LCD_DB0)
			| (1 << LCD_DB1),

	._blacklight = LCD_BACKLIGHT // используется в основной команде при отправке
};

void lcd_init(byte addr, uint8_t cols, uint8_t rows) {
	lcd_cfg.twi.addr = addr;
	lcd_cfg.cols = cols;
	lcd_cfg.rows = rows;

	twi_init(&lcd_cfg.twi);

	_delay_ms(50); // дадим дисплею прочухаться после первого включения

	// для установки режима передачи нужно отправить особый сигнал
	_twi_lcd_write(0x03 << LCD_DB4);
	_delay_us(4500);
	_twi_lcd_write(0x03 << LCD_DB4);
	_delay_us(4500);
	_twi_lcd_write(0x03 << LCD_DB4);
	_delay_us(150);

	_twi_lcd_write(0x02 << LCD_DB4); // ставим 4bit bus режим

	lcd_send(lcd_cfg.function_set_reg, LCD_RS_CMD_MASK);
	lcd_send(lcd_cfg.display_ctrl_reg, LCD_RS_CMD_MASK);
	lcd_send(lcd_cfg.entry_mode_reg,   LCD_RS_CMD_MASK);

	lcd_clear();
	lcd_return_home();
}

void _twi_lcd_strobe(byte data) {
	twi_data_write(data | LCD_EN_MASK, TWI_WRITE_APPEND);
	_delay_us(1);

	twi_data_write(data & ~LCD_EN_MASK, TWI_WRITE_NOAPPEND);
	_delay_us(50);
}

void _twi_lcd_write(byte data) {
	twi_data_write(data | lcd_cfg._blacklight, TWI_WRITE_APPEND);
	_twi_lcd_strobe(data | lcd_cfg._blacklight);
}

void lcd_send(byte data, byte mode) {
	_twi_lcd_write((data & 0xF0) | mode);
	_twi_lcd_write(((data << LCD_DB4) & 0xF0) | mode);
}

// очистка экрана
void lcd_clear(void) {
	lcd_send(LCD_CLEAR_MASK, LCD_RS_CMD_MASK);
	_delay_ms(2);
}

// возврат в начальное положение курсора (данные в DDRAM при этом сохраняются)
void lcd_return_home(void) {
	lcd_send(LCD_RET_HOME_MASK, LCD_RS_CMD_MASK);
	_delay_ms(2);
}

// выбор режима ввода
void lcd_entry_mode(LCD_ENTRY_MODE_t mode) {
	if (mode == LCD_ENTRY_MODE_INC)
		lcd_cfg.function_set_reg |= (1 << LCD_DB1);

	if (mode == LCD_ENTRY_MODE_DEC)
		lcd_cfg.function_set_reg &= ~(1 << LCD_DB1);

	if (mode == LCD_ENTRY_MODE_SHIFT_LEFT)
		lcd_cfg.function_set_reg &= ~(1 << LCD_DB0);

	if (mode == LCD_ENTRY_MODE_SHIFT_RIGHT)
		lcd_cfg.function_set_reg |= (1 << LCD_DB0);

	lcd_send(lcd_cfg.function_set_reg, LCD_RS_CMD_MASK);
}

void lcd_display_setup(LCD_DISPLAY_t mode) {
	switch (mode) {
		case LCD_DISPLAY_ON:
			lcd_cfg.display_ctrl_reg |= (1 << LCD_DB2);
			break;

		case LCD_DISPLAY_OFF:
			lcd_cfg.display_ctrl_reg &= ~(1 << LCD_DB2);
			break;

		case LCD_BACKLIGHT_ON:
			lcd_cfg._blacklight = (1 << LCD_DB3);
			break;

		case LCD_BACKLIGHT_OFF:
			lcd_cfg._blacklight = ~(1 << LCD_DB3);
			break;

		case LCD_CURSOR_ON:
			lcd_cfg.display_ctrl_reg |= (1 << LCD_DB1);
			break;

		case LCD_CURSOR_OFF:
			lcd_cfg.display_ctrl_reg &= ~(1 << LCD_DB1);
			break;

		case LCD_CURSOR_BLINK_ON:
			lcd_cfg.display_ctrl_reg |= (1 << LCD_DB0);
			break;

		case LCD_CURSOR_BLINK_OFF:
			lcd_cfg.display_ctrl_reg &= ~(1 << LCD_DB0);
			break;

		default:
			break;
	}

	lcd_send(lcd_cfg.display_ctrl_reg, LCD_RS_CMD_MASK);
}

void lcd_cursor_setup(LCD_CURSOR_t mode) {
	switch (mode) {
		case LCD_CURSOR_SHIFT_LEFT:
			lcd_cfg.cursor_shift_reg &= ~(1 << LCD_DB2) | ~(1 << LCD_DB3);
			break;

		case LCD_CURSOR_SHIFT_RIGHT:
			lcd_cfg.cursor_shift_reg &= (1 << LCD_DB2) | ~(1 << LCD_DB3);
			break;

		default:
			break;
	}

	lcd_send(lcd_cfg.cursor_shift_reg, LCD_RS_CMD_MASK);
}

void lcd_cursor(uint8_t col, uint8_t row) {
	// курсор не должен уходить за пределы формата экрана
	if (row > lcd_cfg.rows)
		row = lcd_cfg.rows;
	if (col > lcd_cfg.cols)
		col = lcd_cfg.cols;

	lcd_send(LCD_DDRAM_MASK | ((col -1) + lcd_cfg.row_addr_table[(row -1)]), LCD_RS_CMD_MASK);
}

void lcd_print(const char *s) {
	for (uint8_t i = 0; s[i] != '\0'; i++) // i < strlen(str)
		lcd_send(s[i], LCD_RS_DATA_MASK);
}

void lcd_set_val(const char *v, uint8_t reserve, uint8_t col, uint8_t row) {
	// сбрасываем прежнее значение на экране
	lcd_cursor(col, row);
	for (uint8_t i = 0; i < reserve; i++)
		lcd_send(0x20, LCD_RS_DATA_MASK);

	lcd_cursor(col, row);
	lcd_print(v);
}

// чуть позже нужно добавить поддержку меню
//void display_menu(display_menu_opts_t menu) {
//	;
//}


