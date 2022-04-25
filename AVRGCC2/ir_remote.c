/*
 * ir_remote.c
 *
 * Created: 22.01.2022 12:51:12
 *  Author: linxon
 */


/*

	Формат фрейма NEC стандарта содержит несколько полей данных:
		1) Стартовый фрейм: AGC Pulse (9ms) + Long Pulse (4.5ms) = 13.5ms
		   \                                                   /
			\                   Leader code                   /

		2) 8bit Адрес источника + 8bit адрес в инверсном виде = 16bit
		   \                  /   \                         /
		    \   LSB -> MSB   /     \       LSB -> MSB      /

		3 8bit Команда от источника + 8bit команда в инверсном виде = 16bit
*/

#include "ir_remote.h"

static IR_DATA_t *_ir_d;

volatile uint32_t ir_raw_data;

static volatile uint8_t event_type;
static volatile uint8_t data_idx = 0;
static volatile uint16_t t_counter = 0;

void ir_init(struct ir_data_t *ir_data) {
	_ir_d = ir_data;

	// Настраиваем пин на вход и отключаем pull-up резистор (имеется на плате)
	_ir_d->cfg.gpio_ptr->ddr  &= ~_BV(_ir_d->cfg.source_pin);
	_ir_d->cfg.gpio_ptr->port &= ~_BV(_ir_d->cfg.source_pin);

	_ir_d->callback.setup_timer();
	_ir_d->callback.timer_intrr_disable();

	if (_ir_d->cfg.polarity == IR_CFG_POLARITY_ACTIVE_LOW)
		_ir_d->callback.pulse_mode(IR_PULSE_FALLING_EDGE);
	else
		_ir_d->callback.pulse_mode(IR_PULSE_RISING_EDGE);

	_ir_d->_entered  = IR_DATA_IS_NOT_ENTERED;
	_ir_d->_repeated = IR_DATA_IS_NOT_REPEATED;
}

void ir_ISR_timer_counter(void) {
	++t_counter;

	if (t_counter == IR_TIMER_COMP_TIMEOUT) {
		event_type = IR_EVENT_IDLE;

		t_counter = 0;
		_ir_d->_repeated = IR_DATA_IS_NOT_REPEATED;
		_ir_d->callback.timer_intrr_disable();
	}
}

void ir_ISR_handler(void) {
	register bool pin_state = ir_read_pulse();

	switch (_ir_d->cfg.frame_type) {
		case IR_CFG_FRAME_TYPE_NEC:
			ir_NEC_process(pin_state);

			break;

		/*
		case // TODO: добавить другие протоколы
			break;
		*/

		default:
			break;
	}
}

void ir_NEC_process(bool pin_state) {

	// принимаем данные
	if (event_type == IR_EVENT_DATA) {

		if (data_idx < 32) {

			// 0 - 1.2ms
			// 1 - 2.2ms
			ir_raw_data |= ((uint32_t) IR_TIMER_MIN_MAX_V(t_counter, 21, 23) << data_idx++);
			t_counter = 0;

			if (data_idx == 32) {
				event_type = IR_EVENT_IDLE;

				_ir_d->_entered = IR_DATA_IS_ENTERED;

				data_idx = 0;
			}
		}

		return;
	}

	// начинаем обрабатывать сигнал
	if (event_type == IR_EVENT_IDLE && pin_state == HIGH) {
		event_type = IR_EVENT_AGC_PULSE;

		_ir_d->callback.pulse_mode(IR_PULSE_TOGGLE);
		_ir_d->callback.timer_intrr_enable();
		t_counter = 0;

		return;
	}

	if (event_type == IR_EVENT_AGC_PULSE
			&& IR_TIMER_MIN_MAX_V(t_counter, 90, 93)  // это если таймер действительно отсчитал 9.0 - 9.2ms
			&& pin_state == LOW)                      // и pin_state равен LOW
	{

		event_type = IR_EVENT_LONG_PAUSE;

		if (_ir_d->cfg.polarity == IR_CFG_POLARITY_ACTIVE_LOW)
			_ir_d->callback.pulse_mode(IR_PULSE_FALLING_EDGE);
		else
			_ir_d->callback.pulse_mode(IR_PULSE_RISING_EDGE);

		return; // тогда ждем Long/Short pause сигналы
	}

	if (event_type == IR_EVENT_LONG_PAUSE
			&& IR_TIMER_MIN_MAX_V(t_counter, 110, 138) // работаем только с Repeat code и Leader code (11.0 - 13.7ms)
			&& pin_state == HIGH)
	{

		if (IR_TIMER_MIN_MAX_V(t_counter, 110, 114)) { // AGC Pulse + Short pause = 11.1ms (+-)
			event_type = IR_EVENT_IDLE;
			_ir_d->_repeated = IR_DATA_IS_REPEATED;

			return; // значит пришел Repeat code - пропускаем
		}

		// сбрасываем параметры перед приемом новых данных
		_ir_d->_entered  = IR_DATA_IS_NOT_ENTERED;
		_ir_d->_repeated = IR_DATA_IS_NOT_REPEATED;
		event_type = IR_EVENT_DATA;
		t_counter = 0;

		return; // или все таки пришел стартовый бит - Leader code. Ждем следующих сигналов (данные)
	}
}

int8_t ir_NEC_decode(byte *addr, byte *cmd) {
	if (!ir_raw_data)
		return ERROR;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		if (_ir_d->_entered != IR_DATA_IS_ENTERED)
			return ERROR;

		*addr = ir_NEC_check_d(IR_NEC_GET_ADDR_FIELD(ir_raw_data));
		*cmd  = ir_NEC_check_d(IR_NEC_GET_COMM_FIELD(ir_raw_data));

		if (_ir_d->_repeated == IR_DATA_IS_NOT_REPEATED)
			ir_raw_clean();
	}

	return SUCCESS;
}

inline byte ir_NEC_check_d(uint16_t raw_d) {
	register byte invr_d = ~IR_NEC_GET_INVR_FIELD(raw_d);

	if (invr_d == (byte) raw_d)
		return (byte) raw_d;

	return EMPTY_BYTE;
}

//int8_t ir_RC_5_decode(uint16_t *addr, uint16_t *cmd) {
//	return SUCCESS;
//}

inline bool ir_read_pulse(void) {
	register bool pin_state = (_ir_d->cfg.gpio_ptr->pin & _BV(_ir_d->cfg.source_pin));

	if (_ir_d->cfg.polarity == IR_CFG_POLARITY_ACTIVE_LOW)
		return pin_state? LOW: HIGH;
	else
		return pin_state? HIGH: LOW;
}

inline void ir_raw_clean(void) {
	_ir_d->_entered = IR_DATA_IS_NOT_ENTERED;
	ir_raw_data = 0;
}

inline void _ir_pulse_mode_by(PULSE_EDGE_MODE_t mode) {
	ACSR &= ~_BV(ACD); // включаем компаратор
	ACSR |= _BV(ACIE); // включаем прерывание по ANA_COMP_vect

	switch (mode) {
		case IR_PULSE_FALLING_EDGE:
			ACSR |= _BV(ACIS1);
			ACSR &= ~_BV(ACIS0);

			return;

		case IR_PULSE_RISING_EDGE:
			ACSR |= _BV(ACIS1);
			ACSR |= _BV(ACIS0);

			return;

		case IR_PULSE_TOGGLE:
			ACSR &= ~_BV(ACIS1);
			ACSR &= ~_BV(ACIS0);

			return;

		default:
			return;
	}
}

inline void _ir_setup_timer0(void) {
	TCCR0 |= _BV(WGM01); // включить CTC режим
	TCCR0 |= _BV(CS00);  // используем clk/1 режим тактирования
	OCR0 = 200; // 4 000 000 / 1 / <200> / 20 = 1000Hz (нам нужно получить 1ms)
}

inline void _ir_timer_intrr_enable(void) {
	TIMSK |=  _BV(OCIE0);
}

inline void _ir_timer_intrr_disable(void) {
	TIMSK &= ~_BV(OCIE0);
}
