/*
 * log.h
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */

#ifndef SRC_LOG_H_
#define SRC_LOG_H_

#include <zlog.h>

enum
{
	MOD_MAIN,
	MOD_SERVER_MC,
	MOD_SERVER_GIZ,
	MOD_MAX
};

extern zlog_category_t* cat[];

int log_init();

#endif /* SRC_LOG_H_ */