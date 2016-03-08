/*
 * cell.h
 *
 *  Created on: Oct 1, 2015
 *      Author: jk
 */

#ifndef SRC_COMMON_SYNC_H_
#define SRC_COMMON_SYNC_H_


int sync_init(struct event_base *base);
int sync_exit();

void sync_newIMEI(const char *imei);
void sync_gps(const char* imei, int timestamp, float lat, float lng, char speed, short course);

#endif /* SRC_COMMON_SYNC_H_ */
