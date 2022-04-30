/*
 * commands.c
 *
 * Created: 22.01.2022 13:00:17
 *  Author: linxon
 */


#include "commands.h"

static INTERNAL_CMD_t *_cmd_l;

static char cmd[AT_CMD_BUFF_SIZE];
static uint8_t cmd_index;
static uint8_t cmd_count;

CMD_STATUS_t cmd_stat = { AT_CMD_UNKNOWN_IDX, UNKNOWN_ERROR };
static int8_t _cmd_res;

void cmd_init(struct intrnl_cmd (*c)[], uint8_t size) {
	_cmd_l = c;
	cmd_count = size;
}

void cmd_ISR_handler(void) {
	char cmd_buff = '\0';

#ifndef USART_DISABLE_STD
	cmd_buff = getchar();
	putchar(cmd_buff);

	if ((cmd_buff == '\r') || (cmd_buff == '\n')) {
		putchar('\r');
		putchar('\n');
#else
	cmd_buff = rx_usart(NULL);
	tx_usart(cmd_buff, NULL);

	if ((cmd_buff == '\r') || (cmd_buff == '\n')) {
		tx_usart('\r', NULL);
		tx_usart('\n', NULL);
#endif

		_cmd_res = _cmd_handler(cmd);
		// TODO: добавить обработчик для _cmd_res

		cmd_index = 0;
		cmd[cmd_index] = '\0';
	}

	else if (cmd_buff == 0x7F || cmd_buff == 0x08) {
		if (cmd_index > 0)
			--cmd_index;

		cmd[cmd_index] = '\0';
	}

	else {
		if (cmd_index > AT_CMD_BUFF_SIZE)
			return;

		cmd[cmd_index++] = cmd_buff;
		cmd[cmd_index] = '\0';
	}
}

int8_t _cmd_handler(char *cmd) {
	register uint8_t i = 0;
	char *curr_cmd = NULL;

	if (*cmd == '\0')
		return SUCCESS;

	AT_CMD_STATUS_RESET();

	curr_cmd = strtok(cmd, AT_CMD_DELIMITER);
	for (; curr_cmd != NULL && i < cmd_count / sizeof(struct intrnl_cmd); ++i) {
		if (strcmp((*_cmd_l)[i].name, curr_cmd) == 0) {

			cmd_stat.last_cmd_idx = i;
			cmd_stat.res = (int8_t) (*_cmd_l)[i].call(strtok(NULL, AT_CMD_DELIMITER));

			return cmd_stat.res;
		}
	}

	return UNKNOWN_ERROR;
}
