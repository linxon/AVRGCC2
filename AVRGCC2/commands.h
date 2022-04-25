/*
 * commands.h
 *
 * Created: 22.01.2022 13:00:26
 *  Author: linxon
 */


#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "typedef.h"
#include "functions.h"
#ifdef USART_DISABLE_STD
#include "usart.h"
#endif

#define AT_CMD_BUFF_SIZE		32
#define AT_CMD_DELIMITER		"="
#define AT_CMD_ARGS_DELIMITER	","
#define AT_CMD_UNKNOWN_IDX		-1

#define AT_CMD_STATUS_RESET()	({					\
	cmd_stat.last_cmd_idx = AT_CMD_UNKNOWN_IDX;		\
	cmd_stat.res = UNKNOWN_ERROR;					\
})

typedef struct intrnl_cmd {
	const char *name;
	int8_t (*call)(void *);
} INTERNAL_CMD_t[];

typedef struct cmd_status {
	int8_t last_cmd_idx;
	int8_t res;
} CMD_STATUS_t;

extern CMD_STATUS_t cmd_stat;

extern void cmd_init(struct intrnl_cmd (*)[], uint8_t);
extern void cmd_ISR_handler(void);

int8_t _cmd_handler(char *);

#endif /* COMMANDS_H_ */
