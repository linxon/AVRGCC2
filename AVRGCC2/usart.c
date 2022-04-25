#include "usart.h"

#ifndef USART_DISABLE_STD
FILE usart_in = FDEV_SETUP_STREAM(NULL, rx_usart, _FDEV_SETUP_READ);
FILE usart_out = FDEV_SETUP_STREAM(tx_usart, NULL, _FDEV_SETUP_WRITE);
#endif

void usart_init(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		UBRRH |= (USART_SCALED_UBRR >> 8);
		UBRRL |= USART_SCALED_UBRR;
	}

#ifdef USART_USE_2X
	UCSRA |= _BV(U2X);
#else
	UCSRA &= ~_BV(U2X);
#endif

	UCSRB |= _BV(RXEN)  | _BV(TXEN)  | _BV(RXCIE);
	UCSRC |= _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);

#ifndef USART_DISABLE_STD
	stdin = &usart_in;
	stdout = &usart_out;

	printf_P(PSTR("\n\nReady!\n"));
#ifdef DEBUG_USART
	printf_P(PSTR("\nUSART conf:\n\tBaud: %lu\n\tUBRR: 0x%X\n\n"),
		USART_BAUDRATE, USART_SCALED_UBRR);
#endif
#endif /* USART_DISABLE_STD */
}

int rx_usart(FILE *stream) {
	(void) stream;

	loop_until_bit_is_set(UCSRA, RXC);
	return USART_DATA_REG;
}

int tx_usart(char data, FILE *stream) {
	if (data == '\n')
		tx_usart('\r', stream);

	loop_until_bit_is_set(UCSRA, UDRE);
	USART_DATA_REG = data;

	return SUCCESS;
}

void flush_usart(void) {
	loop_until_bit_is_set(UCSRA, RXC);
	asm volatile(
		"in __tmp_reg__, %[reg_addr]"	"\n\t"
		::[reg_addr] "I" (_SFR_IO_ADDR(USART_DATA_REG))
	);
}

