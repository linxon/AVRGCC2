/*
 * rs485.c
 *
 * Created: 22.01.2022 12:27:06
 *  Author: linxon
 */

#include "rs485.h"

static RS485_CFG_t *_rs485_cfg;

static RS485_TX_BUFF_t _rs485_tx_buff;
static RS485_RX_BUFF_t _rs485_rx_buff;
static RS485_TX_t _rs485_proto_tx;
static RS485_RX_t _rs485_proto_rx;

static byte rw_stat = RS485_RW_MODE_FREE;

static TIMER_COUNTER_t t_listen_proto = {
	.count = RS485_PROTO_TIMEOUT_ATTEMPT_COUNT,
	.div = RS485_PROTO_TIMEOUT_DELAY_US };

static TIMER_COUNTER_t t_mode_switch  = {
	.count = RS485_RW_TIMEOUT_ATTEMPT_COUNT,
	.div = RS485_RW_TIMEOUT_DELAY_MS };

static TIMER_COUNTER_t t_rw_timeout	  = {
	.count = RS485_RW_TIMEOUT_ATTEMPT_COUNT,
	.div = RS485_RW_TIMEOUT_DELAY_MS };

void rs485_init(struct rs485_cfg_t *cfg) {
	_rs485_cfg = cfg;

	usart_init();

	USART_RXC_INTRR_DISABLE();
	USART_UDRE_INTRR_DISABLE();

	RS485_GPIO.ddr |= _BV(RS485_DE_RE_PIN);
	rs485_default_mode();
}

void rs485_ISR_reader(void) {

	_rs485_rx_buff.size--;
	*(_rs485_rx_buff.d_ptr++) = USART_DATA_REG;

	if (_rs485_rx_buff.size == 0) {
		rw_stat = RS485_RW_MODE_FREE;
		rs485_default_mode();

		USART_RXC_INTRR_DISABLE();
	}
}

void rs485_ISR_writer(void) {
	/*

	1) Заполняем буфер и включаем прерывание по опустошению UDR - UDRE
	2) Начинает работать прерывание USART_UDRE_vect
	3) В обработчике прерывания вычитаем один байт из size и пытаемся записать данные в UDR
	4) В обработчике прерывания проверяем общее количество оставшихся байтов. Если не 0, то переходим к пункту 3
	5) выходим из обработчика прерывания и ждем следующего

	*/

	_rs485_tx_buff.size--;
	USART_DATA_REG = *(_rs485_tx_buff.d_ptr++);
	USART_CLEAR_TXC_FLAG();

	if (_rs485_tx_buff.size == 0) {
		rs485_default_mode();
		rw_stat = RS485_RW_MODE_FREE;

		USART_UDRE_INTRR_DISABLE(); // данное прерывание при включении будет вызываться постоянно, если стоит флаг UDRE!
	}
}

void rs485_read(rs485_data_ptr data, uint8_t size) {

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_rs485_rx_buff.d_ptr = data;
		_rs485_rx_buff.size = size;
	}

	while (timer_loop_begin(&t_rw_timeout)) {

		if (rw_stat == RS485_RW_MODE_FREE) {
			rw_stat = RS485_RW_MODE_READ;

			if (_rs485_cfg->mode == RS485_CFG_MASTER_MODE)
				RS485_RECEIVER_ENABLE(); // разрешаем принимать данные мастеру

			USART_RXC_INTRR_ENABLE();

			timer_reset_counter();
			continue;
		}

		else if (_rs485_rx_buff.size == 0) {
			timer_reset_counter();
			break; // ack
		}

		if (!timer_loop_until_is_set(TIMER_DELAY_MODE_MS)) {
			rw_stat = RS485_RW_MODE_FREE;

			_rs485_rx_buff.d_ptr = NULL;
			_rs485_rx_buff.size = 0;

			rs485_default_mode();
			return; // timeout!
		}
	}
}

void rs485_write(rs485_data_ptr data, uint8_t size) {

	// ждем, пока не закончит принимать/передавать данные
	while (timer_loop_begin(&t_rw_timeout)) {

		if ((_rs485_tx_buff.size == 0)
				&& rw_stat == RS485_RW_MODE_FREE)
		{
			rw_stat = RS485_RW_MODE_WRITE;

			if (_rs485_cfg->mode == RS485_CFG_SLAVE_MODE)
				RS485_DRIVER_ENABLE();

			timer_reset_counter();
			break;
		}

		if (!timer_loop_until_is_set(TIMER_DELAY_MODE_MS)) {
			_rs485_tx_buff.d_ptr = NULL;
			_rs485_tx_buff.size = 0;

			rs485_default_mode();
			return; // timeout!
		}
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_rs485_tx_buff.d_ptr = data;
		_rs485_tx_buff.size = size;

		USART_UDRE_INTRR_ENABLE();
	}
}

RS485_STAT_t rs485_receive_d(struct rs485_data_t *rx_d) {
	rs485_read((rs485_data_ptr) rx_d, sizeof(*rx_d));

	if (rx_d->addr != _rs485_cfg->device_addr)
		return RS485_STAT_ERR_NOT_ME;

	if (rx_d->crc16 != crc_chk((byte *) &rx_d->data, sizeof(rx_d->data)))
		return RS485_STAT_ERR_CHECKSUM;

	return RS485_STAT_OK;
}

RS485_STAT_t rs485_send_d(uint8_t slave_addr_idx, struct rs485_data_t* tx_d) {

	tx_d->addr = _rs485_cfg->slave_addr[slave_addr_idx];
	tx_d->crc16 = crc_chk((byte *) &tx_d->data, sizeof(tx_d->data));
	rs485_write((rs485_data_ptr) tx_d, sizeof(*tx_d));

	return RS485_STAT_OK;
}

void rs485_default_mode(void) {

	// нужно подождать, пока все данные будут переданы и уже тогда переключать на режим приема/передачи
	while (timer_loop_begin(&t_mode_switch)) {
		if (rw_stat == RS485_RW_MODE_FREE || USART_IS_TX_COMPLETE()) {
			timer_reset_counter();
			break;
		}

		timer_loop_until_is_set(TIMER_DELAY_MODE_MS);
	}

	if (_rs485_cfg->mode == RS485_CFG_MASTER_MODE)
		RS485_DRIVER_ENABLE();
	else
		RS485_RECEIVER_ENABLE();
}

void rs485_switch_mode(RS485_CFG_MODE_t m) {
	_rs485_cfg->mode = m;
	rs485_default_mode();
}

void rs485_send_proto(uint8_t slave_addr_idx, uint8_t proto_d) {
	_rs485_proto_tx.data = proto_d;
	rs485_send_d(slave_addr_idx, &_rs485_proto_tx);
}

byte rs485_listen_proto(void) {

	while (timer_loop_begin(&t_listen_proto)) {

		if (rs485_receive_d(&_rs485_proto_rx) == RS485_STAT_OK) {
			timer_reset_counter();
			break;
		}

		if (timer_loop_until_is_set(TIMER_DELAY_MODE_US))
			return ERROR; // timeout!
	}

	return (byte) _rs485_proto_rx.data;
}
