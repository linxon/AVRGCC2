#include "eeprom.h"

static bool _eeprom_ready = EEPROM_BUFF_IS_READY;

static uint16_t eeprom_addr;
static EEPROM_BUFF_t _eeprom_buff;
static EEPROM_STAT_t _eeprom_stat = {
	.buff_idx      = EEPROM_BUFF_EMPTY_IDX,
	.buff_idx_free = EEPROM_BUFF_EMPTY_IDX,
	.buff_pointer  = EEPROM_BUFF_EMPTY_IDX
};

inline void EEPROM_set_addr(uint16_t addr) {
	eeprom_addr = addr > 0? addr: 0x0000;
}

inline uint16_t EEPROM_get_addr(void) {
	return eeprom_addr;
}

byte EEPROM_read_byte(uint16_t addr) {
	while (EECR & (1 << EEWE));

	EEAR = addr;
	EECR = _BV(EERE);

	return EEDR;
}

void EEPROM_write_byte(uint16_t addr, byte value) {
	EEAR = addr;
	EEDR = value;
	EECR |= _BV(EEMWE);
	EECR |= _BV(EEWE);

	while (EECR & (1 << EEWE));
}

void EEPROM_format(uint16_t start, uint16_t end) {
	uint16_t i;

	for (i = start; i <= end; ++i)
		EEPROM_write_byte(i, 0xFF);
}

void EEPROM_read(uint16_t addr, void *value, size_t size) {
	eeprom_data_ptr_t data_ptr = (eeprom_data_ptr_t) value;
	_EEPROM_read_block(addr, data_ptr, size);
}

void EEPROM_write(uint16_t addr, void *value, size_t size) {
	eeprom_data_ptr_t data_ptr = (eeprom_data_ptr_t) value;

	if (IS_SREG_I_ENABLED())
		_EEPROM_buff_append(addr, data_ptr, size); // режим записи со включенным прерыванием
	else
		_EEPROM_write_block(addr, data_ptr, size); // иначе пишем в лоб
}

void EEPROM_commit(void *value, size_t size, eeprom_rw_mode_t mode) {
	eeprom_data_ptr_t data_ptr = (eeprom_data_ptr_t) value;
	//eeprom_data_ptr_t tmp_data = NULL;

	switch (mode) {
		case EEPROM_COMMIT_READ:
			EEPROM_read(eeprom_addr, data_ptr, size);

			break;

		case EEPROM_COMMIT_WRITE:
			EEPROM_write(eeprom_addr, data_ptr, size);

			break;

		case EEPROM_COMMIT_WR_IF_NOT_INIT:
			//_EEPROM_read_block(eeprom_addr, tmp_data, size);
			/* TODO: ƒоработать!
			if (memcmp(tmp_data, data_ptr, size) == 0) {
				*data_ptr = *tmp_data;
			} else
				EEPROM_write(eeprom_addr, data_ptr, size);
			*/

			break;

		default:
			break;
	}

	EEPROM_set_addr(eeprom_addr + size);
}

// Ёта штука будет обрабатывать буфер данных во врем¤ вызова прерывани¤ EE_RDY_vect
void EEPROM_ISR_handler(void) {
	_eeprom_ready = EEPROM_BUFF_IS_READY;

	if (_eeprom_stat.buff_pointer > _eeprom_stat.buff_idx) {

		// остались ли еще какие-нибудь данные в буфере?
		for (register int8_t i = 0; i < _eeprom_stat.buff_idx; ++i) {
			if (_eeprom_buff[i].r_size > 0) {
				_eeprom_stat.buff_pointer = i;

				return; // фиксируем указатель на ¤чейку и выходим. ∆дем вызова перывани¤ EE_RDY_vect
			}
		}

		// буфер пуст - останавливаем работу
		_eeprom_stat.buff_idx      = EEPROM_BUFF_EMPTY_IDX;
		_eeprom_stat.buff_idx_free = EEPROM_BUFF_EMPTY_IDX;
		_eeprom_stat.buff_pointer  = EEPROM_BUFF_EMPTY_IDX;

		EEPROM_DISABLE_INTERRUPT();

		return;

	} else if (_eeprom_stat.buff_pointer == EEPROM_BUFF_EMPTY_IDX)
		_eeprom_stat.buff_pointer++;

	uint16_t			*addr	  = &_eeprom_buff[_eeprom_stat.buff_pointer].addr;
	eeprom_data_ptr_t	*data_ptr = &_eeprom_buff[_eeprom_stat.buff_pointer].data;
	size_t				*size	  = &_eeprom_buff[_eeprom_stat.buff_pointer].r_size;

#ifdef DEBUG_EEPROM // DEBUG_EEPROM
	printf_P(PSTR("IDX: %d\n"),      _eeprom_stat.buff_idx);
	printf_P(PSTR("WORK IDX: %d\n"), _eeprom_stat.buff_pointer);
	printf_P(PSTR("ADDR: 0x%X\n\n"), *addr);
	printf_P(PSTR("SIZE: %d\n"),     *size);
#endif

	if (*size == 0) {

		// фиксируем освободившуюс¤ ¤чейку (пусть _EEPROM_buff_append подхватит ее номер и заполнит данными)
		if (_eeprom_stat.buff_idx_free == EEPROM_BUFF_EMPTY_IDX)
			_eeprom_stat.buff_idx_free = _eeprom_stat.buff_pointer;

		_eeprom_stat.buff_pointer++;

		return;
	}

	_EEPROM_buff_insert((*addr)++, (*data_ptr)++);
	--(*size);
}

void _EEPROM_buff_append(uint16_t addr, void *value, size_t size) {

	eeprom_data_ptr_t data_ptr = (eeprom_data_ptr_t) value;
	register int8_t tmp_idx = EEPROM_BUFF_EMPTY_IDX;

	if (_eeprom_stat.buff_idx >= (EEPROM_BUFF_SIZE -1)) { // если буфер переполнилс¤
		while (_eeprom_stat.buff_idx_free == EEPROM_BUFF_EMPTY_IDX); // ждем, когда по¤витс¤ свободна¤ ¤чейка

		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

			tmp_idx = _eeprom_stat.buff_idx;
			_eeprom_stat.buff_idx      = (_eeprom_stat.buff_idx_free -1);
			_eeprom_stat.buff_idx_free = EEPROM_BUFF_EMPTY_IDX;

			_EEPROM_buff_append(addr, value, size); // заполн¤ем освободившуюс¤ ¤чейку

			_eeprom_stat.buff_idx = tmp_idx;

		}

		return;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		// buff_idx - не должен быть меньше 0 , так как могут быть проблемы при переходе
		// за границу массива и зменени¤ там данных
		if (_eeprom_stat.buff_idx == EEPROM_BUFF_EMPTY_IDX)
			_eeprom_stat.buff_idx++;
		else if (_eeprom_stat.buff_idx < (EEPROM_BUFF_SIZE -1))
			_eeprom_stat.buff_idx++;

		_eeprom_buff[_eeprom_stat.buff_idx].addr   = addr;
		_eeprom_buff[_eeprom_stat.buff_idx].data   = data_ptr;
		_eeprom_buff[_eeprom_stat.buff_idx].r_size = size;

		if (_eeprom_ready == EEPROM_BUFF_IS_READY)
			EEPROM_ENABLE_INTERRUPT(); // включаем обработку EE_RDY_vect
	}
}

void _EEPROM_buff_insert(uint16_t addr, void *value) {
	eeprom_data_ptr_t data_ptr = (eeprom_data_ptr_t) value;

	EEAR = addr;
	EEDR = (byte) *data_ptr;

	_eeprom_ready = EEPROM_BUFF_IS_NOT_READY;
	EECR |= _BV(EEMWE);
	EECR |= _BV(EEWE);
}

void _EEPROM_read_block(uint16_t addr, void *value, size_t size) {
	eeprom_data_ptr_t data_ptr = (eeprom_data_ptr_t) value;

	for (uint16_t i = 0; i < size; ++i) {
		while (EECR & (1 << EEWE));

		EEAR = addr++;
		EECR = _BV(EERE);

		*((byte *) data_ptr++) = EEDR;
	}
}

void _EEPROM_write_block(uint16_t addr, void *value, size_t size) {
	eeprom_data_ptr_t data_ptr = (eeprom_data_ptr_t) value;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		for (uint8_t i = 0; i < size; ++i) {
			EEAR = addr++;

			EEDR = (byte) *(data_ptr++);

			_eeprom_ready = EEPROM_BUFF_IS_NOT_READY;
			EECR |= _BV(EEMWE);
			EECR |= _BV(EEWE);

			while (EECR & (1 << EEWE));
		}

		_eeprom_ready = EEPROM_BUFF_IS_READY;
	}
}
