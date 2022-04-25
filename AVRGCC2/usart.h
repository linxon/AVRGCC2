#ifndef _USART_H_
#define _USART_H_ 				1

//#define BAUD					38400
//#include <util/setbaud.h>
#include "typedef.h"

#define USART_BAUDRATE				9600UL
#define USART_SCALED_UBRR			(F_CPU / (USART_BAUDRATE * 16UL) -1)

#define USART_DATA_REG				UDR	// используется в другим модулях, поэтому дадим этому регистру абстрактное имя

#define USART_CLEAR_RXC_FLAG()		(UCSRA |= _BV(RXC))
#define USART_CLEAR_TXC_FLAG()		(UCSRA |= _BV(TXC))

#define USART_IS_RX_COMPLETE()		(UCSRA & _BV(RXC))
#define USART_IS_TX_COMPLETE()		(UCSRA & _BV(TXC))
#define USART_IS_DATA_EMPTY()		(UCSRA & _BV(UDRE))
#define USART_RXC_INTRR_ENABLE()	(UCSRB |=  _BV(RXCIE))
#define USART_RXC_INTRR_DISABLE()	(UCSRB &= ~_BV(RXCIE))
#define USART_TXC_INTRR_ENABLE()	(UCSRB |=  _BV(TXCIE))
#define USART_TXC_INTRR_DISABLE()	(UCSRB &= ~_BV(TXCIE))
#define USART_UDRE_INTRR_ENABLE()	(UCSRB |=  _BV(UDRIE))
#define USART_UDRE_INTRR_DISABLE()	(UCSRB &= ~_BV(UDRIE))

#ifndef USART_DISABLE_STD
extern FILE usart_in;
extern FILE usart_out;
#endif

extern void usart_init(void);
extern int rx_usart(FILE *);
extern int tx_usart(char, FILE *);
extern void flush_usart(void);

#endif /* _USART_H_ */
