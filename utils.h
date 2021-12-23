/*
 * utils.h
 *
 *  Created on: Dec 23, 2021
 *      Author: kevinche
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <stdio.h>

#define CHK_HAL(action) \
	do { \
		HAL_StatusTypeDef retcode = action; \
		if (retcode != HAL_OK) { \
			char buf[256]; \
			snprintf(buf, sizeof(buf), "[%s:%d]: HAL code %d on action %s\r\n", __FILE__, __LINE__, retcode, #action); \
			UART_Transmit(buf); \
			return retcode; \
		} \
	} while (0);

#endif /* SRC_UTILS_H_ */
