#include "i2c.h"

static TWI_CONF_t *twi_conf;
TWI_STAT_t twi_stat;

void twi_init(TWI_CONF_t *c) {
	twi_conf = c;

	TWI_GPIO.ddr |= _BV(TWI_SCL_PIN) | _BV(TWI_SDA_PIN);

	switch(twi_conf->presclr) {
		case TWI_PRESCLR_VALUE_4:
			TWSR |=  _BV(TWPS0);
			TWSR &= ~_BV(TWPS1);

			break;

		case TWI_PRESCLR_VALUE_16:
			TWSR &= ~_BV(TWPS0);
			TWSR |=  _BV(TWPS1);

			break;

		case TWI_PRESCLR_VALUE_64:
			TWSR |= _BV(TWPS0);
			TWSR |= _BV(TWPS1);

			break;

		default:
			twi_conf->presclr = TWI_PRESCLR_VALUE_1;
			TWSR &= ~_BV(TWPS0);
			TWSR &= ~_BV(TWPS1);

			break;
	}

	TWBR = TWI_TWBR_CALC(twi_conf->freq, twi_conf->presclr);

#ifdef DEBUG_TWI
	printf_P(PSTR("TWI conf:\n\tAddr: 0x%X\n\tFreq: 0x%X\n\n"),
		twi_conf->addr, twi_conf->freq);
#endif
}

// имеется бага, при которой МК зависает во время чтения, когда
// возвращаемое значения ниже 0x10 хз почему
twi_data_t twi_data_read(void) {
	twi_data_t result = EMPTY_BYTE;
	twi_stat.err = 0;

	if(_twi_send_start() == ERROR)
		twi_stat.err = TWI_STAT_ERR_START_FAILED;

	if(_twi_send_addr(TW_READ) == ERROR && !twi_stat.err)
		twi_stat.err = TWI_STAT_ERR_SEND_ADDR;
	else {
		TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
		loop_until_bit_is_set(TWCR, TWINT);

		if(TW_STATUS != TW_MR_DATA_ACK && !twi_stat.err)
			twi_stat.err = TWI_STAT_ERR_WR_DATA;

		result = TWDR;
	}

	if(!twi_stat.err)
		twi_stat.res = TWI_STAT_OK;

	_twi_send_stop();

	return result;
}

void twi_data_write(twi_data_t data, byte mode) {
	twi_stat.err = 0;
	twi_stat.res = 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		twi_stat.res = _twi_send_start();
		if (twi_stat.res == ERROR)
			twi_stat.err = TWI_STAT_ERR_START_FAILED;

		twi_stat.res = _twi_send_addr(TW_WRITE);
		if (twi_stat.res == ERROR && !twi_stat.err)
			twi_stat.err = TWI_STAT_ERR_SEND_ADDR;
		else {
			TWDR = data;
			TWCR = _BV(TWEN) | _BV(TWINT);
			loop_until_bit_is_set(TWCR, TWINT);

			if (TW_STATUS != TW_MT_DATA_ACK && !twi_stat.err)
				twi_stat.err = TWI_STAT_ERR_WR_DATA;
		}

		if (!twi_stat.err)
			twi_stat.res = TWI_STAT_OK;

		if (mode == TWI_WRITE_NOAPPEND)
			_twi_send_stop();
	}
}

bool _twi_send_start(void) {
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
	loop_until_bit_is_set(TWCR, TWINT);

	_delay_us(2);

	if (TW_STATUS != TW_START)
		return ERROR;

	return SUCCESS;
}

bool _twi_send_addr(byte rw_mode) {
	TWDR = (twi_conf->addr << TWDR1) | (rw_mode << TWDR0);

	TWCR = _BV(TWEN) | _BV(TWINT);
	loop_until_bit_is_set(TWCR, TWINT);

	if (TW_STATUS != (rw_mode? TW_MR_SLA_ACK: TW_MT_SLA_ACK))
		return ERROR;

	return SUCCESS;
}

bool _twi_send_stop(void) {
	_delay_us(2);
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);

	_delay_us(10);

	return SUCCESS;
}



