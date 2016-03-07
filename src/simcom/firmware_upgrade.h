/*
 * firmware_upgrade.h
 *
 *  Created on: Feb 28, 2016
 *      Author: gcy
 */

#ifndef FIRMWARE_UPGRADE_H_
#define FIRMWARE_UPGRADE_H_

int getLastVersionWithFileNameAndSizeStored(void);
int getLastFileSize(void);
int getDataSegmentWithGottenSize(int gottenSize, char *data, int *pSendSize);
int getLastFileChecksum(void);

#endif /* FIRMWARE_UPGRADE_H_ */