/*
 * main.c
 *
 * Created: 21.01.2022 21:40:29
 *  Author: linxon
 */


#include "main.h"

static RS485_CFG_t rs485_cfg;
static RS485_TX_t  rs485_tx;
static RS485_RX_t  rs485_rx;

static int16_t a_num;

static volatile uint16_t	mseconds;
static volatile int8_t		seconds = 0;
static volatile int8_t		minutes = 0;
static volatile int8_t		hours   = 0;

MAIN_CYCLE_FLAGS_t m_flags = { // флаговый автомат
	.edit_mode		= FALSE,
	.clock_updated	= FALSE,
	.min_entered	= FALSE,
	.led_en			= FALSE
};

ISR(USART_RXC_vect)  { rs485_ISR_reader(); }
ISR(USART_UDRE_vect) { rs485_ISR_writer(); }

ISR(TIMER2_COMP_vect) {
	mseconds++;

	if (mseconds == 2000) {
		seconds++;
		mseconds = 0;

		m_flags.clock_updated = TRUE;
	}

	if (seconds >= 60) {
		minutes++;
		seconds = 0;
	}

	if (minutes >= 60) {
		hours++;
		minutes = 0;
	}

	if (hours >= 24)
		hours = minutes = seconds = 0;
}

void main(void) {
	init_me();

	do {

		if (m_flags.clock_updated) {
			tm1637_buff_clear();

			if (hours >= 0)
				TM1637_BUFF_SET_NUM_8(hours, TM1637_ARGMOD_SHIFT_POS_L(hours, 2, 1));

			if (minutes >= 0)
				TM1637_BUFF_SET_NUM_8(minutes, TM1637_ARGMOD_SHIFT_POS_L(minutes, 4, 3));

			if (m_flags.edit_mode)
				tm1637_buff_fill('-');
			else
				tm1637_buff_fill('0');

			if ((seconds+1) % 2 || m_flags.edit_mode)
				tm1637_buff_set(TM1637_MASK_COLON, 2);

			tm1637_display();

			// сбрасываем до следующего тика
			m_flags.clock_updated = FALSE;
		}

		// Slave режим
		if (rs485_cfg.mode == RS485_CFG_SLAVE_MODE) {
			if (rs485_receive_d(&rs485_rx) != RS485_STAT_OK) // ждем данных с пульта
				continue;

			if (IS_NUMERICAL(rs485_rx.data) != -1) {
				if (a_num >= 0)
					a_num = a_num * 10 + rs485_rx.data; // сдвиг для положительных чисел
				else
					a_num = a_num * 10 - rs485_rx.data; // сдвиг для отрицательных чисел
			}

			switch (rs485_rx.data) {

				case 0x46: // стрелка влево (пульт)
					a_num = (a_num - (a_num % 10)) / 10;

					break;

				case 0x47: // стрелка вправо
					a_num = a_num * 10;

					break;

				case 0x19: // стрелка вверх
					++a_num;

					break;

				case 0x1D: // стрелка вниз
					--a_num;

					break;

				case 0x0A: // кнопка Enter
					a_num = 0;

					if (m_flags.edit_mode) {
						if (m_flags.min_entered && !m_flags.hour_entered) {
							m_flags.hour_entered = TRUE; // введены часы
							m_flags.edit_mode = FALSE;

							TIMSK |= _BV(OCIE2);  // включаем таймер времени
						}

						if (!m_flags.min_entered)
							m_flags.min_entered = TRUE; // введены минуты

					} else {
						hours = minutes = seconds = -1; // сбрасываем время

						m_flags.clock_updated = TRUE;
						m_flags.edit_mode	= TRUE;

						TIMSK &= ~_BV(OCIE2);
					}

					m_flags.led_en = TRUE;

					break;

				default:
					break;
			}

			if (m_flags.edit_mode) {
				if (!m_flags.min_entered) {
					a_num = (a_num < 60 && a_num > 0)? a_num: 0;
					minutes = (uint8_t) a_num;
				}

				if (m_flags.min_entered && !m_flags.hour_entered) {
					a_num = (a_num < 24 && a_num > 0)? a_num: 0;
					hours = (uint8_t) a_num;
				}

				if (m_flags.min_entered && m_flags.hour_entered)
					m_flags.min_entered = m_flags.hour_entered = FALSE;
			}

			m_flags.clock_updated	= TRUE;
		}

		rs485_rx.addr = rs485_rx.data = rs485_rx.crc16 = 0;
		rs485_tx.addr = rs485_tx.data = rs485_tx.crc16 = 0;

		blink_led();

	} while(1);
}

void init_me(void) {
	cli();
	wdt_disable();
	ACSR |= _BV(ACD);

	// Сигнальные пины (На реле)
	//GPIO_C.ddr  = 0b00111111;
	//GPIO_C.port = 0x0;

	rs485_cfg.mode				= RS485_CFG_SLAVE_MODE;
	rs485_cfg.device_addr		= 0x02;
	rs485_cfg.slave_addr[0]		= 0x01;  // мастер устройство с IR датчиком
	//rs485_cfg.slave_addr[1]	= 0x03;
	//rs485_cfg.slave_addr[2]	= 0x04;

	rs485_init(&rs485_cfg);

	// пин LED индикатора
	GPIO_B.ddr |= _BV(PB1);
	LED_ON();

//	LDM200_INIT();
	TM1637_INIT();

	TCCR2 |= _BV(CS21) | _BV(WGM21);
	TIMSK |= _BV(OCIE2);
	OCR2 = 250;

	_delay_ms(100);
	sei();
}

void blink_led(void) {
	if (!m_flags.led_en)
		return;

	for (uint8_t i = 0; i < 2; ++i) {
		LED_OFF();
		my_delay_ms(30);
		LED_ON();
		my_delay_ms(30);
	}

	m_flags.led_en = FALSE;
}
