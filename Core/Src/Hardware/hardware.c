/*
 * hardware.c
 *
 *  Created on: Mar 19, 2024
 *      Author: CBT_DEV
 */

#include "main.h"
#include "hardware.h"

//DC
void DC1(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT1_GPIO_Port, GPIO_OUT1_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT1_GPIO_Port, GPIO_OUT1_Pin, GPIO_PIN_RESET);
	}
}

void DC2(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT2_GPIO_Port, GPIO_OUT2_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT2_GPIO_Port, GPIO_OUT2_Pin, GPIO_PIN_RESET);
	}
}

void DC3(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT3_GPIO_Port, GPIO_OUT3_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT3_GPIO_Port, GPIO_OUT3_Pin, GPIO_PIN_RESET);
	}
}

void DC4(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT4_GPIO_Port, GPIO_OUT4_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT4_GPIO_Port, GPIO_OUT4_Pin, GPIO_PIN_RESET);
	}
}


void DC5(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT5_GPIO_Port, GPIO_OUT5_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT5_GPIO_Port, GPIO_OUT5_Pin, GPIO_PIN_RESET);
	}
}

void DC6(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT6_GPIO_Port, GPIO_OUT6_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT6_GPIO_Port, GPIO_OUT6_Pin, GPIO_PIN_RESET);
	}
}

void DC7(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT7_GPIO_Port, GPIO_OUT7_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT7_GPIO_Port, GPIO_OUT7_Pin, GPIO_PIN_RESET);
	}
}

void DC8(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT8_GPIO_Port, GPIO_OUT8_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT8_GPIO_Port, GPIO_OUT8_Pin, GPIO_PIN_RESET);
	}
}

//AC
void AC1(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT11_GPIO_Port, GPIO_OUT11_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT11_GPIO_Port, GPIO_OUT11_Pin, GPIO_PIN_SET);
	}
}

void AC2(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT12_GPIO_Port, GPIO_OUT12_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT12_GPIO_Port, GPIO_OUT12_Pin, GPIO_PIN_SET);
	}
}

void AC3(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT13_GPIO_Port, GPIO_OUT13_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT13_GPIO_Port, GPIO_OUT13_Pin, GPIO_PIN_SET);
	}
}

void AC4(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT14_GPIO_Port, GPIO_OUT14_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT14_GPIO_Port, GPIO_OUT14_Pin, GPIO_PIN_SET);
	}
}

void AC5(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT15_GPIO_Port, GPIO_OUT15_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT15_GPIO_Port, GPIO_OUT15_Pin, GPIO_PIN_SET);
	}
}

void AC6(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT16_GPIO_Port, GPIO_OUT16_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT16_GPIO_Port, GPIO_OUT16_Pin, GPIO_PIN_SET);
	}
}

void AC7(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT17_GPIO_Port, GPIO_OUT17_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT17_GPIO_Port, GPIO_OUT17_Pin, GPIO_PIN_SET);
	}
}

void AC8(int onoff){
	if(onoff==1){
		HAL_GPIO_WritePin(GPIO_OUT18_GPIO_Port, GPIO_OUT18_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIO_OUT18_GPIO_Port, GPIO_OUT18_Pin, GPIO_PIN_SET);
	}
}
