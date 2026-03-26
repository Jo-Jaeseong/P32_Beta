/*
 * door_sensor.h
 *
 *  Created on: Nov 10, 2022
 *      Author: CBT
 */

#ifndef SRC_SENSOR_DOORSENSOR_H_
#define SRC_SENSOR_DOORSENSOR_H_

#define STERILANT_CONTAINER_BOTTLE	1
#define STERILANT_CONTAINER_VIAL	2

extern int DoorOpenFlag;
extern int DoorOpenVentFlag;
extern int DoorOpenVentCnt;
extern unsigned char SterilantContainerType;

int DoorHandleCheck();
int DoorLatchCheck();
int SliderOpenCheck();
int BottleDoorCheck();
int BottleCheck();
int VialSensorCheck();
int SterilantContainerCheck();
int SterilantSliderCheck();
int IsSterilantRFIDRequired();
int DoorOpenProcess();

#endif /* SRC_SENSOR_DOORSENSOR_H_ */
