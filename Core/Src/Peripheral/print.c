/*
 * print.c
 *
 *  Created on: Nov 1, 2022
 *      Author: CBT
 */
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
*/
#include "main.h"

#include "hardware.h"
#include "sensor.h"
#include "peripheral.h"

extern UART_HandleTypeDef huart2;
#define PRINT_USART	&huart2


int autoprintFlag;
int printcopy;
int printgraphFlag;
int printdataFlag;

unsigned char PRINT_rx_data[200]={};
//unsigned char PRINT_rx_data[3]={0x1b,0x0c,0x48};

#define	LF	0x0A;
#define ESC	0x1B;
#define J	0x4A;
#define d	0x64;
#define FF	0x0C;

//init Printer
//ESC	@

char pinrtdata[40];



void initprint(){
	PRINT_rx_data[0]=ESC;
	PRINT_rx_data[1]=0x40;
    HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 2, 10);
}

void print_pagemode(){
	PRINT_rx_data[0]=ESC;
	PRINT_rx_data[1]=FF;
    HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 2, 10);
}

void print_standardmode(){
	PRINT_rx_data[0]=FF;
    HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 1, 10);
}

void print_printnfeed(){
	PRINT_rx_data[0]=LF;
    HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 1, 10);
}

void printline(){
	//initprint();
	for(int i=0;i<32;i++){
		PRINT_rx_data[i]='-';
	}
	//PRINT_rx_data[32]=FF;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 32, 10);
}

void printmsg(char *msg){
	//initprint();
	memset(PRINT_rx_data, 0, 50);
	for(int i = 0; i < 33; i++) {
		PRINT_rx_data[i] = msg[i];
	}
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, sizeof(PRINT_rx_data), 10);
	memset(PRINT_rx_data, 0, 50);
}

void printmsg100(char *msg){
	//initprint();
	memset(PRINT_rx_data, 0, 200);
	for(int i = 0; i <strlen(msg) ; i++) {
		PRINT_rx_data[i] = msg[i];
	}
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, strlen(msg), 10);
	memset(PRINT_rx_data, 0, 200);
}

void doprint(){
	PRINT_rx_data[0]=FF;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 1, 10);
}

void printmain(){
	initprint();
	print_printnfeed();
	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("CYCLE Information               \n");
	printmsg("--------------------------------\n");
	printmsg("MODEL NO      : P32             \n");
	printmsg("STERILIZER NO : FN-P100230101   \n");
	printmsg("FACILITY NAME : SAMSUNG         \n");
	printmsg("DEPART NAME   : DENTAL          \n");
	printmsg("--------------------------------\n");
	printmsg("Sterilant                       \n");
	printmsg("SERIAL NO     : P1-S-230101     \n");
	printmsg("Loading Date  : 2022-11-02      \n");
	printmsg("Expiry Date   : 2022-12-02      \n");
	printmsg("Remain H2O2   : 49              \n");
	printmsg("--------------------------------\n");
    print_printnfeed();
    doprint();
}


void printInformation(){
	initprint();
	print_printnfeed();
	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("DEVICE Information              \n");
	printmsg("--------------------------------\n");

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"MODEL NAME    :  %s\n",flash_MODEL_NAME);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"MODEL NUMBER  :  %s\n",flash_SERIAL_NUMBER);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"FACILITY NAME :  %s\n",flash_FACILITY_NAME);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"DEPART_NAME   :  %s\n",flash_DEPARTMENT_NAME);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"S/W VERSION   :  %s\n",flash_SOFTWARE_VERSION);
	printmsg(pinrtdata);

	if(CurrentRFIDData.open_day==0){
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Expiry Date   :  0000-0000-0000\n");
		printmsg(pinrtdata);

		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Remain H2O2   :   0\n");
		printmsg(pinrtdata);
	}
	else{
		if(CurrentRFIDData.expiry_day==50){
			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Expiry Date   :  0000-0000-0000\n");
			printmsg(pinrtdata);

			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Remain H2O2   : %3d\n",CurrentRFIDData.volume/2);
			printmsg(pinrtdata);
		}
		else{
			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Expiry Date   :  20%2d-%02d-%02d\n",CurrentRFIDData.expiry_year,CurrentRFIDData.expiry_month,CurrentRFIDData.expiry_day);
			printmsg(pinrtdata);

			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Remain H2O2   : %3d\n",CurrentRFIDData.volume/2);
			printmsg(pinrtdata);
		}

	}

	printmsg("--------------------------------\n");
    print_printnfeed();
    doprint();
}

void printSterilant(){
	initprint();
	print_printnfeed();
	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("STERILANT Information           \n");
	printmsg("--------------------------------\n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"SERIAL NO     :  %02d%02d%02d%02d\n",CurrentRFIDData.production_year,CurrentRFIDData.production_month,CurrentRFIDData.production_day, CurrentRFIDData.production_number);
	printmsg(pinrtdata);

	if(CurrentRFIDData.open_day==0){
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Loading Date  :  0000-0000-0000\n");
		printmsg(pinrtdata);
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Expiry Date   :  0000-0000-0000\n");
		printmsg(pinrtdata);

		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Remain H2O2   :   0\n");
		printmsg(pinrtdata);
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Loading Date  :  20%2d-%02d-%02d\n",CurrentRFIDData.open_year,CurrentRFIDData.open_month,CurrentRFIDData.open_day);
		printmsg(pinrtdata);


		if(CurrentRFIDData.expiry_day==50){
			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Expiry Date   :  0000-0000-0000\n");
			printmsg(pinrtdata);

			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Remain H2O2   : %3d\n",CurrentRFIDData.volume/2);
			printmsg(pinrtdata);
		}
		else{
			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Expiry Date   :  20%2d-%02d-%02d\n",CurrentRFIDData.expiry_year,CurrentRFIDData.expiry_month,CurrentRFIDData.expiry_day);
			printmsg(pinrtdata);

			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Remain H2O2   : %3d\n",CurrentRFIDData.volume/2);
			printmsg(pinrtdata);
		}

	}
	printmsg("--------------------------------\n");
    print_printnfeed();
    doprint();
}

void PrintTotalTest(){
	initprint();
	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("Total Self Test                 \n");
	printmsg("--------------------------------\n");
	printmsg("* Temperature Test *            \n");
	printmsg("  HEATER   SETTING  TEMP   P/F  \n");
	memset(pinrtdata,0,40);
	if(HeaterTestResult[1]==0){
		sprintf(pinrtdata,"1.DOOR                   NO TEST\n");
	}
	else if(HeaterTestResult[1]==1){
		sprintf(pinrtdata,"1.DOOR        %3.1f   %3.1f  PASS \n",(float)DoorSettingTemp[0],TestTemp[0]);
	}
	else if(HeaterTestResult[1]==2){
		sprintf(pinrtdata,"1.DOOR        %3.1f   %3.1f  FAIL \n",(float)DoorSettingTemp[0],TestTemp[0]);
	}
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	if(HeaterTestResult[2]==0){
		sprintf(pinrtdata,"2.CHAMBER                NO TEST\n");
	}
	else if(HeaterTestResult[2]==1){
		sprintf(pinrtdata,"2.CHAMBER     %3.1f   %3.1f  PASS \n",(float)ChamberSettingTemp[0],TestTemp[1]);
	}
	else if(HeaterTestResult[2]==2){
		sprintf(pinrtdata,"2.CHAMBER     %3.1f   %3.1f  FAIL \n",(float)ChamberSettingTemp[0],TestTemp[1]);
	}
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	if(HeaterTestResult[3]==0){
		sprintf(pinrtdata,"3.CHAMBER.B              NO TEST\n");
	}
	else if(HeaterTestResult[3]==1){
		sprintf(pinrtdata,"3.CHAMBER.B   %3.1f   %3.1f  PASS \n",(float)ChamberBackSettingTemp[0],TestTemp[2]);
	}
	else if(HeaterTestResult[3]==2){
		sprintf(pinrtdata,"3.CHAMBER.B   %3.1f   %3.1f  FAIL \n",(float)ChamberBackSettingTemp[0],TestTemp[2]);
	}
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	if(HeaterTestResult[4]==0){
		sprintf(pinrtdata,"4.VAPORIZER              NO TEST\n");
	}
	else if(HeaterTestResult[4]==1){
		sprintf(pinrtdata,"4.VAPORIZER   %3.1f   %3.1f  PASS \n",(float)VaporizerSettingTemp[0],TestTemp[3]);
	}
	else if(HeaterTestResult[4]==2){
		sprintf(pinrtdata,"4.VAPORIZER   %3.1f   %3.1f  FAIL \n",(float)VaporizerSettingTemp[0],TestTemp[3]);
	}
	printmsg(pinrtdata);
	if(HeaterTestResult[0]==0){
		printmsg("Result : NO TEST                \n");
	}
	else if(HeaterTestResult[0]==1){
		printmsg("Result : PASS                   \n");
	}
	else if(HeaterTestResult[0]==2){
		printmsg("Result : FAIL                   \n");
	}
	printmsg("--------------------------------\n");
	printmsg("* Valve Test *                  \n");

	//테스트 미진행 체크
	if(ValveTestResult[1]==0){
		printmsg("Result : NO TEST                \n");
	}
	else{
		printmsg("   VALVE                   P/F  \n");
		if(ValveTestResult[1]==0){
			printmsg("1.INJECTION VALVE        NO TEST\n");
		}
		else if(ValveTestResult[1]==1){
			printmsg("1.INJECTION VALVE          PASS \n");
		}
		else if(ValveTestResult[1]==2){
			printmsg("1.INJECTION VALVE          FAIL \n");
		}

		if(ValveTestResult[2]==0){
			printmsg("2.VENT VALVE             NO TEST\n");
		}
		else if(ValveTestResult[2]==1){
			printmsg("2.VENT VALVE               PASS \n");
		}
		else if(ValveTestResult[2]==2){
			printmsg("2.VENT VALVE               FAIL \n");
		}

		if(ValveTestResult[3]==0){
			printmsg("3.VACUUM VALVE           NO TEST\n");
		}
		else if(ValveTestResult[3]==1){
			printmsg("3.VACUUM VALVE             PASS \n");
		}
		else if(ValveTestResult[3]==2){
			printmsg("3.VACUUM VALVE             FAIL \n");
		}
		if(ValveTestResult[0]==0){
			printmsg("Result : NO TEST                \n");
		}
		else if(ValveTestResult[0]==1){
			printmsg("Result : PASS                   \n");
		}
		else if(ValveTestResult[0]==2){
			printmsg("Result : FAIL                   \n");
		}
	}
	printmsg("--------------------------------\n");
	printmsg("* Vacuum&Leak Test *            \n");
	//테스트 미진행 체크
	if(VacuumTestResult[0]==0){
		printmsg("Result : NO TEST                \n");
	}
	else{
		printmsg(" [Maxumum Vacuum Test]  (Torr)  \n");

		//테스트 미진행 체크
		if(VacuumTestResult[1]==0){
			printmsg("Result : NO TEST                \n");
		}
		else{
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 1 min :                  %3d  \n",(int)TestPressure[0]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 2 min :                  %3d  \n",(int)TestPressure[1]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 3 min :                  %3d  \n",(int)TestPressure[2]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 4 min :                  %3d  \n",(int)TestPressure[3]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 5 min :                  %3d  \n",(int)TestPressure[4]);
			printmsg(pinrtdata);
			if(VacuumTestResult[1]==0){
				printmsg("Result : NO TEST                \n");
			}
			else if(VacuumTestResult[1]==1){
				printmsg("Result : PASS                   \n");
			}
			else if(VacuumTestResult[1]==2){
				printmsg("Result : FAIL                   \n");
			}
		}
		printmsg(" [Leak up Rate Test]    (Torr)  \n");

		//테스트 미진행 체크
		if(VacuumTestResult[2]==0){
			printmsg("Result : NO TEST                \n");
		}
		else{
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 1 min :                  %3d  \n",(int)TestPressure[5]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 2 min :                  %3d  \n",(int)TestPressure[6]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 3 min :                  %3d  \n",(int)TestPressure[7]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 4 min :                  %3d  \n",(int)TestPressure[8]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 5 min :                  %3d  \n",(int)TestPressure[9]);
			printmsg(pinrtdata);
			if(VacuumTestResult[2]==0){
				printmsg("Result : NO TEST                \n");
			}
			else if(VacuumTestResult[2]==1){
				printmsg("Result : PASS                   \n");
			}
			else if(VacuumTestResult[2]==2){
				printmsg("Result : FAIL                   \n");
			}
		}
	}
	printmsg("--------------------------------\n");
	printmsg("* Total Result *                \n");

	if(TestResult[1]==0){
		printmsg("Result : STOP                   \n");
	}
	else if(TestResult[1]==1){
		printmsg("Result : PASS                   \n");
	}
	else if(TestResult[1]==2){
		printmsg("Result : FAIL                   \n");
	}
	printmsg("--------------------------------\n");


	GetTime();
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"PRINT DATE : 20%2d-%02d-%02d %02d:%02d:%02d\n",
			today_date.year,today_date.month,today_date.day,today_date.hour,today_date.minute,today_date.second);
	printmsg(pinrtdata);
    print_printnfeed();
    print_printnfeed();
    doprint();
}


void PrintHeaterTest(){
	initprint();
	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("Self Test - Temperature Test    \n");
	printmsg("--------------------------------\n");
	printmsg("  HEATER   SETTING  TEMP   P/F  \n");
	memset(pinrtdata,0,40);
	if(HeaterTestResult[1]==0){
		sprintf(pinrtdata,"1.DOOR                   NO TEST\n");
	}
	else if(HeaterTestResult[1]==1){
		sprintf(pinrtdata,"1.DOOR        %3.1f   %3.1f  PASS \n",(float)DoorSettingTemp[0],TestTemp[0]);
	}
	else if(HeaterTestResult[1]==2){
		sprintf(pinrtdata,"1.DOOR        %3.1f   %3.1f  FAIL \n",(float)DoorSettingTemp[0],TestTemp[0]);
	}
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	if(HeaterTestResult[2]==0){
		sprintf(pinrtdata,"2.CHAMBER                NO TEST\n");
	}
	else if(HeaterTestResult[2]==1){
		sprintf(pinrtdata,"2.CHAMBER     %3.1f   %3.1f  PASS \n",(float)ChamberSettingTemp[0],TestTemp[1]);
	}
	else if(HeaterTestResult[2]==2){
		sprintf(pinrtdata,"2.CHAMBER     %3.1f   %3.1f  FAIL \n",(float)ChamberSettingTemp[0],TestTemp[1]);
	}
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	if(HeaterTestResult[3]==0){
		sprintf(pinrtdata,"3.CHAMBER.B              NO TEST\n");
	}
	else if(HeaterTestResult[3]==1){
		sprintf(pinrtdata,"3.CHAMBER.B   %3.1f   %3.1f  PASS \n",(float)ChamberBackSettingTemp[0],TestTemp[2]);
	}
	else if(HeaterTestResult[3]==2){
		sprintf(pinrtdata,"3.CHAMBER.B   %3.1f   %3.1f  FAIL \n",(float)ChamberBackSettingTemp[0],TestTemp[2]);
	}
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	if(HeaterTestResult[4]==0){
		sprintf(pinrtdata,"4.VAPORIZER              NO TEST\n");
	}
	else if(HeaterTestResult[4]==1){
		sprintf(pinrtdata,"4.VAPORIZER   %3.1f   %3.1f  PASS \n",(float)VaporizerSettingTemp[0],TestTemp[3]);
	}
	else if(HeaterTestResult[4]==2){
		sprintf(pinrtdata,"4.VAPORIZER   %3.1f   %3.1f  FAIL \n",(float)VaporizerSettingTemp[0],TestTemp[3]);
	}
	printmsg(pinrtdata);
	printmsg("--------------------------------\n");
	if(HeaterTestResult[0]==0){
		printmsg("Result : NO TEST                \n");
	}
	else if(HeaterTestResult[0]==1){
		printmsg("Result : PASS                   \n");
	}
	else if(HeaterTestResult[0]==2){
		printmsg("Result : FAIL                   \n");
	}
	printmsg("--------------------------------\n");
	GetTime();
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"PRINT DATE : 20%2d-%02d-%02d %02d:%02d:%02d\n",
			today_date.year,today_date.month,today_date.day,today_date.hour,today_date.minute,today_date.second);
	printmsg(pinrtdata);
    print_printnfeed();
    print_printnfeed();
    doprint();
}

void PrintValveTest(){
	initprint();
	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("Self Test - Valve Test          \n");
	printmsg("--------------------------------\n");

	//테스트 미진행 체크
	if(ValveTestResult[1]==0){
		printmsg("Result : NO TEST                \n");
	}
	else{
		printmsg("   VALVE                   P/F  \n");
		if(ValveTestResult[1]==0){
			printmsg("1.INJECTION VALVE        NO TEST\n");
		}
		else if(ValveTestResult[1]==1){
			printmsg("1.INJECTION VALVE          PASS \n");
		}
		else if(ValveTestResult[1]==2){
			printmsg("1.INJECTION VALVE          FAIL \n");
		}

		if(ValveTestResult[2]==0){
			printmsg("2.VENT VALVE             NO TEST\n");
		}
		else if(ValveTestResult[2]==1){
			printmsg("2.VENT VALVE               PASS \n");
		}
		else if(ValveTestResult[2]==2){
			printmsg("2.VENT VALVE               FAIL \n");
		}

		if(ValveTestResult[3]==0){
			printmsg("3.VACUUM VALVE           NO TEST\n");
		}
		else if(ValveTestResult[3]==1){
			printmsg("3.VACUUM VALVE             PASS \n");
		}
		else if(ValveTestResult[3]==2){
			printmsg("3.VACUUM VALVE             FAIL \n");
		}
		printmsg("--------------------------------\n");
		if(ValveTestResult[0]==0){
			printmsg("Result : NO TEST                \n");
		}
		else if(ValveTestResult[0]==1){
			printmsg("Result : PASS                   \n");
		}
		else if(ValveTestResult[0]==2){
			printmsg("Result : FAIL                   \n");
		}
	}
	printmsg("--------------------------------\n");
	GetTime();
	sprintf(pinrtdata,"PRINT DATE : 20%2d-%02d-%02d %02d:%02d:%02d\n",
			today_date.year,today_date.month,today_date.day,today_date.hour,today_date.minute,today_date.second);
	printmsg(pinrtdata);
    print_printnfeed();
    print_printnfeed();
    doprint();
}

void PrintLeakTest(){
	initprint();
	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("Self Test - Leak Test           \n");
	printmsg("--------------------------------\n");
	//테스트 미진행 체크
	if(VacuumTestResult[0]==0){
		printmsg("Result : NO TEST                \n");
	}
	else{
		printmsg(" [Maxumum Vacuum Test]  (Torr)  \n");

		//테스트 미진행 체크
		if(VacuumTestResult[1]==0){
			printmsg("Result : NO TEST                \n");
		}
		else{
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 1 min :                  %3d  \n",(int)TestPressure[0]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 2 min :                  %3d  \n",(int)TestPressure[1]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 3 min :                  %3d  \n",(int)TestPressure[2]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 4 min :                  %3d  \n",(int)TestPressure[3]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 5 min :                  %3d  \n",(int)TestPressure[4]);
			printmsg(pinrtdata);
			printmsg("--------------------------------\n");
			if(VacuumTestResult[1]==0){
				printmsg("Result : NO TEST                \n");
			}
			else if(VacuumTestResult[1]==1){
				printmsg("Result : PASS                   \n");
			}
			else if(VacuumTestResult[1]==2){
				printmsg("Result : FAIL                   \n");
			}
		}
		printmsg("--------------------------------\n");
		printmsg(" [Leak up Rate Test]    (Torr)  \n");

		//테스트 미진행 체크
		if(VacuumTestResult[2]==0){
			printmsg("Result : NO TEST                \n");
		}
		else{
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 1 min :                  %3d  \n",(int)TestPressure[5]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 2 min :                  %3d  \n",(int)TestPressure[6]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 3 min :                  %3d  \n",(int)TestPressure[7]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 4 min :                  %3d  \n",(int)TestPressure[8]);
			printmsg(pinrtdata);
			memset(pinrtdata,0,40);
			sprintf(pinrtdata," 5 min :                  %3d  \n",(int)TestPressure[9]);
			printmsg(pinrtdata);
			printmsg("--------------------------------\n");
			if(VacuumTestResult[2]==0){
				printmsg("Result : NO TEST                \n");
			}
			else if(VacuumTestResult[2]==1){
				printmsg("Result : PASS                   \n");
			}
			else if(VacuumTestResult[2]==2){
				printmsg("Result : FAIL                   \n");
			}
		}
	}
	printmsg("--------------------------------\n");
	GetTime();
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"PRINT DATE : 20%2d-%02d-%02d %02d:%02d:%02d\n",
			today_date.year,today_date.month,today_date.day,today_date.hour,today_date.minute,today_date.second);
	printmsg(pinrtdata);
    print_printnfeed();
    print_printnfeed();
    doprint();
}

void LoadCyclePrint(){
	initprint();

	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("CYCLE Information               \n");
	printmsg("--------------------------------\n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"MODEL NAME    :  %s\n",flash_MODEL_NAME);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"MODEL NUMBER  :  %s\n",flash_SERIAL_NUMBER);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"FACILITY NAME :  %s\n",flash_FACILITY_NAME);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"DEPART_NAME   :  %s\n",flash_DEPARTMENT_NAME);
	printmsg(pinrtdata);
	printmsg("--------------------------------\n");
	printmsg("Sterilant                       \n");

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"SERIAL NO     :  %02d%02d%02d%02d\n",(int)saveCycleData.serialNumber[0],(int)saveCycleData.serialNumber[1],(int)saveCycleData.serialNumber[2], (int)saveCycleData.serialNumber[3]);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Loading Date  :  20%2d-%02d-%02d\n",(int)saveCycleData.loadingDate[0],(int)saveCycleData.loadingDate[1],(int)saveCycleData.loadingDate[2]);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Expiry Date   :  20%2d-%02d-%02d\n",(int)saveCycleData.expiryDate[0],(int)saveCycleData.expiryDate[1],(int)saveCycleData.expiryDate[2]);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Remain H2O2   :  %-3d\n",saveCycleData.volume);
	printmsg(pinrtdata);
	printmsg("--------------------------------\n");

	if(saveCycleData.cycleName==1){
		printmsg("Selected Cycle:  Short          \n");
	}
	else if(saveCycleData.cycleName==2){
		printmsg("Selected Cycle:  Standard       \n");
	}
	else if(saveCycleData.cycleName==3){
		printmsg("Selected Cycle:  Advanced       \n");
	}
	else{
		printmsg("                                \n");
	}

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Cycle   :  %-3d     \n",saveCycleData.totalCount);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Daily Cycle   :  %-3d     \n",saveCycleData.dailyCount);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"DATE          :  20%2d-%02d-%02d\n",
			saveCycleData.date[0],saveCycleData.date[1],saveCycleData.date[2]);
	printmsg(pinrtdata);
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Start Time    :  %02d:%02d:%02d\n",
			saveCycleData.startTime[0],saveCycleData.startTime[1],saveCycleData.startTime[2]);
	printmsg(pinrtdata);
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"End Time      :  %02d:%02d:%02d\n",
				saveCycleData.endTime[0],saveCycleData.endTime[1],saveCycleData.endTime[2]);
	printmsg(pinrtdata);

	printmsg("--------------------------------\n");
	printmsg("[ VACUUM 1 ]                    \n");

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",saveCycleData.processTime[1][0],saveCycleData.processTime[1][1]);
	printmsg(pinrtdata);

	if((int)saveCycleData.maxTemperature[1]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %-3d     \n",saveCycleData.maxTemperature[1]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.maxPressure[1]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %-4d     \n",saveCycleData.maxPressure[1]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.minPressure[1]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %-4d     \n",saveCycleData.minPressure[1]);
		printmsg(pinrtdata);
	}


	printmsg("[ INJECTION 1 & DIFFUSION 1 ]   \n");

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",saveCycleData.processTime[2][0],saveCycleData.processTime[2][1]);
	printmsg(pinrtdata);

	if((int)saveCycleData.maxTemperature[2]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %-3d     \n",saveCycleData.maxTemperature[2]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.maxPressure[2]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %-4d     \n",saveCycleData.maxPressure[2]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.minPressure[2]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %-4d     \n",saveCycleData.minPressure[2]);
		printmsg(pinrtdata);
	}


	printmsg("[ VACUUM 2 & PLASMA 1 ]         \n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",saveCycleData.processTime[3][0],saveCycleData.processTime[3][1]);
	printmsg(pinrtdata);

	if((int)saveCycleData.maxTemperature[3]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %-3d     \n",saveCycleData.maxTemperature[3]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.maxPressure[3]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %-4d     \n",saveCycleData.maxPressure[3]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.minPressure[3]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %-4d     \n",saveCycleData.minPressure[3]);
		printmsg(pinrtdata);
	}


	printmsg("[ INJECTION 2 & DIFFUSION 2 ]   \n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",saveCycleData.processTime[4][0],saveCycleData.processTime[4][1]);
	printmsg(pinrtdata);

	if((int)saveCycleData.maxTemperature[4]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %-3d     \n",saveCycleData.maxTemperature[4]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.maxPressure[4]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %-4d     \n",saveCycleData.maxPressure[4]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.minPressure[4]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %-4d     \n",saveCycleData.minPressure[4]);
		printmsg(pinrtdata);
	}


	printmsg("[ VACUUM 3 & PLASMA 2 ]         \n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",saveCycleData.processTime[5][0],saveCycleData.processTime[5][1]);
	printmsg(pinrtdata);

	if((int)saveCycleData.maxTemperature[5]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %-3d     \n",saveCycleData.maxTemperature[5]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.maxPressure[5]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %-4d     \n",saveCycleData.maxPressure[5]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.minPressure[5]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %-4d     \n",saveCycleData.minPressure[5]);
		printmsg(pinrtdata);
	}


	printmsg("[ VENT 1 ]                      \n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",saveCycleData.processTime[6][0],saveCycleData.processTime[6][1]);
	printmsg(pinrtdata);

	if((int)saveCycleData.maxTemperature[6]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %-3d     \n",saveCycleData.maxTemperature[6]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.maxPressure[6]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %-4d     \n",saveCycleData.maxPressure[6]);
		printmsg(pinrtdata);
	}
	if((int)saveCycleData.minPressure[6]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %-4d     \n",saveCycleData.minPressure[6]);
		printmsg(pinrtdata);
	}

	printmsg("--------------------------------\n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"CARBON FILTER :  %-4d       \n",saveCycleData.carbonFilter);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"HEPA FILTER   :  %-4d       \n",saveCycleData.hepaFilter);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"PLASMA ASSY   :  %-4d       \n",saveCycleData.plasmaAssy);
	printmsg(pinrtdata);
	printmsg("--------------------------------\n");


	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Cycle Time    :  %02d:%02d      \n",saveCycleData.totalTime[0],saveCycleData.totalTime[1]);
	printmsg(pinrtdata);
	if(saveCycleData.status==11){
		printmsg("Cycle Status  :  Completed      \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Cycle Status  :  ERROR%02d        \n",saveCycleData.status);
		printmsg(pinrtdata);
	}
	//printmsg("Operator ID   :                 \n");

	memset(pinrtdata,0,40);
	if(saveCycleData.ID==10){
		sprintf(pinrtdata,"Operator ID   :  CBT            \n");
	}
	else if(saveCycleData.ID==9){
		sprintf(pinrtdata,"Operator ID   :  ADMIN          \n");
	}
	else{
		sprintf(pinrtdata,"Operator ID   :  %c%c%c%c%c%c%c%c%c%c\n",flash_ID[CurrentUser][0],flash_ID[CurrentUser][1],flash_ID[CurrentUser][2],
					flash_ID[CurrentUser][3],flash_ID[CurrentUser][4],flash_ID[CurrentUser][5],flash_ID[CurrentUser][6],flash_ID[CurrentUser][7],flash_ID[CurrentUser][8],flash_ID[CurrentUser][9]);
	}
	printmsg(pinrtdata);

	printmsg("Approved by   :                 \n");

    print_printnfeed();
    print_printnfeed();
    doprint();
}

void CyclePrint(){
	initprint();
	printmsg("--------------------------------\n");
	printmsg("Clean Bio Tech Corp             \n");
	printmsg("CYCLE Information               \n");
	printmsg("--------------------------------\n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"MODEL NAME    :  %s\n",flash_MODEL_NAME);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"MODEL NUMBER  :  %s\n",flash_SERIAL_NUMBER);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"FACILITY NAME :  %s\n",flash_FACILITY_NAME);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"DEPART_NAME   :  %s\n",flash_DEPARTMENT_NAME);
	printmsg(pinrtdata);
	printmsg("--------------------------------\n");
	printmsg("Sterilant                       \n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"SERIAL NO     :  %02d%02d%02d%02d\n",CurrentRFIDData.production_year,CurrentRFIDData.production_month,CurrentRFIDData.production_day, CurrentRFIDData.production_number);
	printmsg(pinrtdata);

	if(CurrentRFIDData.open_day==0){
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Loading Date  :  0000-0000-0000\n");
		printmsg(pinrtdata);
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Expiry Date   :  0000-0000-0000\n");
		printmsg(pinrtdata);

		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Remain H2O2   :   0\n");
		printmsg(pinrtdata);
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Loading Date  :  20%2d-%02d-%02d\n",CurrentRFIDData.open_year,CurrentRFIDData.open_month,CurrentRFIDData.open_day);
		printmsg(pinrtdata);


		if(CurrentRFIDData.expiry_day==50){
			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Expiry Date   :  0000-0000-0000\n");
			printmsg(pinrtdata);

			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Remain H2O2   : %3d\n",CurrentRFIDData.volume/2);
			printmsg(pinrtdata);
		}
		else{
			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Expiry Date   :  20%2d-%02d-%02d\n",CurrentRFIDData.expiry_year,CurrentRFIDData.expiry_month,CurrentRFIDData.expiry_day);
			printmsg(pinrtdata);

			memset(pinrtdata,0,40);
			sprintf(pinrtdata,"Remain H2O2   : %3d\n",CurrentRFIDData.volume/2);
			printmsg(pinrtdata);
		}

	}
	printmsg("--------------------------------\n");
	if(CycleName==1){
		printmsg("Selected Cycle:  Short          \n");
	}
	else if(CycleName==2){
		printmsg("Selected Cycle:  Standard       \n");
	}
	else if(CycleName==3){
		printmsg("Selected Cycle:  Advanced       \n");
	}
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Cycle   :  %3d     \n",totalCount);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Daily Cycle   :  %3d     \n",dailyCount);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"DATE          :  20%2d-%02d-%02d\n",
			p_data.year,p_data.month,p_data.day);
	printmsg(pinrtdata);
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Start Time    :  %02d:%02d:%02d\n",
			p_data.start_hour,p_data.start_minute,p_data.start_second);
	printmsg(pinrtdata);
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"End Time      :  %02d:%02d:%02d\n",
				p_data.end_hour,p_data.end_minute,p_data.end_second);
	printmsg(pinrtdata);

    if(printdataFlag==1){
    	printdata();
    }
    else{

    }
	printmsg("--------------------------------\n");
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"CARBON FILTER :  %04d     \n",CarbonFilter);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"HEPA FILTER   :  %04d     \n",HEPAFilter);
	printmsg(pinrtdata);

	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"PLASMA ASSY   :  %04d     \n",PlasmaAssy);
	printmsg(pinrtdata);
	printmsg("--------------------------------\n");

	p_data.totalTime[0]=(TotalTime/10)/60;
	p_data.totalTime[1]=(TotalTime/10)%60;
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Cycle Time    :  %02d:%02d      \n",p_data.totalTime[0],p_data.totalTime[1]);
	printmsg(pinrtdata);
	if(errorcode==0){
		p_data.status=11;
		printmsg("Cycle Status  :  Completed      \n");
	}
	else{
		memset(pinrtdata,0,40);
		p_data.status=errorcode;
		sprintf(pinrtdata,"Cycle Status  :  ERROR%02d        \n",errorcode);
		printmsg(pinrtdata);
	}

	memset(pinrtdata,0,40);
	if(CurrentUser==10){
		sprintf(pinrtdata,"Operator ID   :  CBT            \n");
	}
	else if(CurrentUser==9){
		sprintf(pinrtdata,"Operator ID   :  ADMIN          \n");
	}
	else{
		sprintf(pinrtdata,"Operator ID   :  %c%c%c%c%c%c%c%c%c%c\n",flash_ID[CurrentUser][0],flash_ID[CurrentUser][1],flash_ID[CurrentUser][2],
					flash_ID[CurrentUser][3],flash_ID[CurrentUser][4],flash_ID[CurrentUser][5],flash_ID[CurrentUser][6],flash_ID[CurrentUser][7],flash_ID[CurrentUser][8],flash_ID[CurrentUser][9]);
	}
	printmsg(pinrtdata);

	//printmsg("Operator ID   :                 \n");
	//flash_ID[CurrentUser][];
	printmsg("Approved by   :                 \n");

    if(printgraphFlag==1){
        doprint();
    	printtestgraph(CycleName);
    }
    else{
        printmsg("--------------------------------\n");
        print_printnfeed();
        print_printnfeed();
        doprint();
    }
}



/*
 * #define	LF	0x0A;
#define ESC	0x1B;
#define J	0x4A;
#define d	0x64;
#define FF	0x0C;
*/

//void printgraph(){

void SelectPageMode(){
    PRINT_rx_data[0]=0x1B;
    PRINT_rx_data[1]=0x4C;
    HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 2, 10);
}

void Direction_Left2Right(){
	//정상 가로쓰기
	PRINT_rx_data[0]=0x1B;
	PRINT_rx_data[1]=0x54;
	PRINT_rx_data[2]=48;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 3, 10);
}
void Direction_Top2Bottom(){
	//세로 쓰기
	PRINT_rx_data[0]=0x1B;
	PRINT_rx_data[1]=0x54;
	PRINT_rx_data[2]=51;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 3, 10);
}
void Direction_Right2Left(){
	PRINT_rx_data[0]=0x1B;
	PRINT_rx_data[1]=0x54;
	PRINT_rx_data[2]=50;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 3, 10);
}


void printpage(){
	//Print data in page mode
    PRINT_rx_data[0]=0x1B;
    PRINT_rx_data[1]=0x0C;
    HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 2, 10);
}



void AbsoluteVertical(int nL, int nH){
	//절대 위치 이동
	PRINT_rx_data[0]=0x1D;
	PRINT_rx_data[1]=0x24;
	PRINT_rx_data[2]=nL;
	PRINT_rx_data[3]=nH;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 4, 10);
}

void RelativeVertical(int nL, int nH){
	//상대 위치 이동
	PRINT_rx_data[0]=0x1D;
	PRINT_rx_data[1]=0x5C;
	PRINT_rx_data[2]=nL;
	PRINT_rx_data[3]=nH;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 4, 10);
}


void SetPageArea(int xL, int xH, int yL, int yH, int dxL, int dxH, int dyL, int dyH){
	PRINT_rx_data[0]=0x1B;
	PRINT_rx_data[1]=0x57;
	PRINT_rx_data[2]=xL;
	PRINT_rx_data[3]=xH;
	PRINT_rx_data[4]=yL;
	PRINT_rx_data[5]=yH;
	PRINT_rx_data[6]=dxL;
	PRINT_rx_data[7]=dxH;
	PRINT_rx_data[8]=dyL;
	PRINT_rx_data[9]=dyH;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 10, 10);
}

void Drawline(int xL, int xH, int yL, int yH, int n){
	PRINT_rx_data[0]=0x1D;
	PRINT_rx_data[1]=0x69;
	PRINT_rx_data[2]=xL;
	PRINT_rx_data[3]=xH;
	PRINT_rx_data[4]=yL;
	PRINT_rx_data[5]=yH;
	PRINT_rx_data[6]=n;
	HAL_UART_Transmit(PRINT_USART, PRINT_rx_data, 7, 10);
}

void DrawVacuum(int x, int y1,int y2, int n){
	//210+256
	//y scale
	//85->
	int y3;
	y1=y1/8;
	y1=2.4*y1+85;

	y2=y2/8;
	y2=2.4*y2+85;

	y3=y2-y1;


	if(y3<0){
		if(y2<256){
			//SetPageArea(y2,0,0,0,128,1,28,2);
			SetPageArea(y2,0,0,0,128,1,56,4);
			y3=y1-y2;
		}
		else if(y2>=256&&y2<384){
			//SetPageArea(y2-256,1,0,0,128,1,28,2);
			SetPageArea(y2-256,1,0,0,128,1,56,4);
			y3=y1-y2;
		}
	}
	else if(y3>=0){
		if(y1<256){
			//SetPageArea(y1,0,0,0,128,1,28,2);
			SetPageArea(y1,0,0,0,128,1,56,4);
		}
		else if(y1>=256&&y1<384){
			//SetPageArea(y1-256,1,0,0,128,1,28,2);
			SetPageArea(y1-256,1,0,0,128,1,56,4);
		}
		y3=y2-y1;
	}
	if(x<256){
		AbsoluteVertical(x,0);
	}
	else if(x>=256&&x<512){
		AbsoluteVertical(x-256,1);
	}
	else if(x>=512&&x<768){
		AbsoluteVertical(x-512,2);
	}
	else if(x>=768&&x<1024){
		AbsoluteVertical(x-768,3);
	}
	else if(x>=1024){
		AbsoluteVertical(x-1024,4);
	}
	if(y3<256){
		Drawline(y3,0,n,0,n);
		//Drawline(y3,0,n,0,n);
	}
	else if(y3>=256&&y3<384){
		Drawline(y3-256,1,n,0,n);
		//Drawline(10,0,n,0,n);
	}
	/*
	else if(y3>=512&&y3<768){
		Drawline(y3-512,2,n,0,n);
		//Drawline(10,0,n,0,n);
	}
	else if(y3>=768&&y3<768){
		Drawline(y3-,2,n,0,n);
		//Drawline(10,0,n,0,n);
	}
	*/
}

void DrawTemp(int x, int y1,int y2, int n){
	//210+256
	//y scale
	//85->
	int y3;
	y1=y1*1.25;
	y1=2.4*y1+85;

	y2=y2*1.25;
	y2=2.4*y2+85;

	y3=y2-y1;

/*
	if(y3<0){
		if(y2<256){
			SetPageArea(y2,0,0,0,128,1,28,2);
		}
		else if(y2>=256){
			SetPageArea(y2-256,1,0,0,128,1,28,2);
		}
		y3=y1-y2;
	}
	else if(y3>=0){
		if(y1<256){
			SetPageArea(y1,0,0,0,128,1,28,2);
		}
		else if(y1>=256){
			SetPageArea(y1-256,1,0,0,128,1,28,2);
		}
		y3=y2-y1;
	}
	*/
	if(y3<0){
		if(y2<256){
			//SetPageArea(y2,0,0,0,128,1,28,2);
			SetPageArea(y2,0,0,0,128,1,56,4);
		}
		else if(y2>=256&&y2<384){
			//SetPageArea(y2-256,1,0,0,128,1,28,2);
			SetPageArea(y2-256,1,0,0,128,1,56,4);
		}
		y3=y1-y2;
	}
	else if(y3>=0){
		if(y1<256){
			//SetPageArea(y1,0,0,0,128,1,28,2);
			SetPageArea(y1,0,0,0,128,1,56,4);
			y3=y2-y1;
		}
		else if(y1>=256&&y1<384){
			//SetPageArea(y1-256,1,0,0,128,1,28,2);
			SetPageArea(y1-256,1,0,0,128,1,56,4);
			y3=y2-y1;
		}

	}
	if(x<256){
		AbsoluteVertical(x,0);
	}
	else if(x>=256&&x<512){
		AbsoluteVertical(x-256,1);
	}
	else if(x>=512&&x<768){
		AbsoluteVertical(x-512,2);
	}
	else if(x>=768&&x<1024){
		AbsoluteVertical(x-768,3);
	}
	else if(x>=1024){
		AbsoluteVertical(x-1024,4);
	}
	if(y3<256){
		Drawline(y3,0,n,0,n);
		//Drawline(y3,0,n,0,n);
	}
	else if(y3>=256&&y3<384){
		Drawline(y3-256,1,n,0,n);
		//Drawline(10,0,n,0,n);
	}
}

void printdata(){
	int tempminute,tempsecond;

	printmsg("--------------------------------\n");
	printmsg("[ VACUUM 1 ]                    \n");
	tempminute=(fProcessTime[1]/10)/60;
	tempsecond=(fProcessTime[1]/10)%60;
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",tempminute,tempsecond);
	printmsg(pinrtdata);

	if((int)p_data.tempmax[1]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %3.1f   \n",p_data.tempmax[1]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremax[1]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %4.1f   \n",p_data.pressuremax[1]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremin[1]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %4.1f   \n",p_data.pressuremin[1]);
		printmsg(pinrtdata);
	}


	printmsg("[ INJECTION 1 & DIFFUSION 1 ]   \n");
	tempminute=(fProcessTime[2]/10)/60;
	tempsecond=(fProcessTime[2]/10)%60;
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",tempminute,tempsecond);
	printmsg(pinrtdata);

	if((int)p_data.tempmax[2]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %3.1f   \n",p_data.tempmax[2]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremax[2]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %4.1f   \n",p_data.pressuremax[2]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremin[2]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %4.1f   \n",p_data.pressuremin[2]);
		printmsg(pinrtdata);
	}


	printmsg("[ VACUUM 2 & PLASMA 1 ]         \n");
	tempminute=(fProcessTime[3]/10)/60;
	tempsecond=(fProcessTime[3]/10)%60;
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",tempminute,tempsecond);
	printmsg(pinrtdata);

	if((int)p_data.tempmax[3]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %3.1f   \n",p_data.tempmax[3]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremax[3]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %4.1f   \n",p_data.pressuremax[3]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremin[3]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %4.1f   \n",p_data.pressuremin[3]);
		printmsg(pinrtdata);
	}


	printmsg("[ INJECTION 2 & DIFFUSION 2 ]   \n");
	tempminute=(fProcessTime[4]/10)/60;
	tempsecond=(fProcessTime[4]/10)%60;
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",tempminute,tempsecond);
	printmsg(pinrtdata);

	if((int)p_data.tempmax[4]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %3.1f   \n",p_data.tempmax[4]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremax[4]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %4.1f   \n",p_data.pressuremax[4]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremin[4]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %4.1f   \n",p_data.pressuremin[4]);
		printmsg(pinrtdata);
	}


	printmsg("[ VACUUM 3 & PLASMA 2 ]         \n");
	tempminute=(fProcessTime[5]/10)/60;
	tempsecond=(fProcessTime[5]/10)%60;
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",tempminute,tempsecond);
	printmsg(pinrtdata);

	if((int)p_data.tempmax[5]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %3.1f   \n",p_data.tempmax[5]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremax[5]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %4.1f   \n",p_data.pressuremax[5]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremin[5]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %4.1f   \n",p_data.pressuremin[5]);
		printmsg(pinrtdata);
	}


	printmsg("[ VENT 1 ]                      \n");
	tempminute=(fProcessTime[6]/10)/60;
	tempsecond=(fProcessTime[6]/10)%60;
	memset(pinrtdata,0,40);
	sprintf(pinrtdata,"Total Time    :  %02d:%02d      \n",tempminute,tempsecond);
	printmsg(pinrtdata);

	if((int)p_data.tempmax[6]==0){
		printmsg("Temp(Max)     :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Temp(Max)     :  %3.1f   \n",p_data.tempmax[6]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremax[6]==0){
		printmsg("Pressure(Max) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(Max) :  %4.1f   \n",p_data.pressuremax[6]);
		printmsg(pinrtdata);
	}
	if((int)p_data.pressuremin[6]==760){
		printmsg("Pressure(Min) :                 \n");
	}
	else{
		memset(pinrtdata,0,40);
		sprintf(pinrtdata,"Pressure(min) :  %4.1f   \n",p_data.pressuremin[6]);
		printmsg(pinrtdata);
	}
}



void printtestgraph(int cycle){
	initprint();
	print_printnfeed();
	printmsg("--------------------------------\n");
	doprint();

    initprint();
    SelectPageMode();
    SetPageArea(1,0,1,0,128,1,25,0);
	printmsg("          PRESSURE(Torr)        \n");

	printpage();

    initprint();
    SelectPageMode();
	SetPageArea(1,0,1,0,128,1,50,0);
	Direction_Top2Bottom();
	AbsoluteVertical(15,0);
	printmsg100("\n800\n700\n600\n500\n400\n300\n200\n100\n  0");
	printpage();

//

	CycleGrahp(cycle);

//



    initprint();
    SelectPageMode();
	SetPageArea(10,0,1,0,128,1,25,0);
	Direction_Top2Bottom();
	AbsoluteVertical(15,0);
	printmsg100("\n80\n70\n60\n50\n40\n30\n20\n10\n 0");
	printpage();

    initprint();
    SelectPageMode();
	SetPageArea(1,0,1,0,128,1,25,0);
	Direction_Right2Left();
	printmsg("         TEMPRATURE(C)        \n");
	printpage();

	initprint();
	print_printnfeed();
	printmsg("--------------------------------\n");
	doprint();
}


/*
 extern float PressureData[300];
extern float TemperatureData[300];
 */

void CycleGrahp(int cycle){
	if(cycle==1){
	    initprint();
	    SelectPageMode();

		//점찍기

	    DrawVacuum(0,PressureData[0],PressureData[0],4);
		DrawTemp(0,TemperatureData[0],TemperatureData[0],4);
	    for(int i=1; i<=150;i++){
			DrawVacuum(i*4,PressureData[i-1],PressureData[i],4);
			DrawTemp(i*4,TemperatureData[i-1],TemperatureData[i],4);
	    }

	    /*
	    DrawVacuum(0,0,0,2);
		DrawTemp(0,0,0,2);
	    for(int i=1; i<=150;i++){
			DrawVacuum(i*4,i,i,2);
			DrawTemp(i*4,i,i,2);
	    }
	    */

		//SetPageArea(0,0,0,0,128,1,54,1);	//
	    SetPageArea(0,0,0,0,128,1,98,2);	//
		Direction_Top2Bottom();
		AbsoluteVertical(45,1);

		//printmsg100("    5   10   15   20   25\n"
		//		           "         Time(minute)      \r");

		printmsg100("         5        10        15        20        25\n"
						   "                   Time(minute)         \r");
		AbsoluteVertical(40,1);
		//Drawline(54,1,0,0,1);
		Drawline(98,2,0,0,1);
		AbsoluteVertical(0,0);
		Drawline(0,0,110,1,1);

		printpage();

		initprint();
		SelectPageMode();
		Direction_Top2Bottom();
		SetPageArea(0,0,0,0,128,1,10,0);
		Drawline(0,0,110,1,1);
		printpage();
	}

	else if(cycle==2){
	    initprint();
	    SelectPageMode();

		//점찍기

	    DrawVacuum(0,PressureData[0],PressureData[0],4);
		DrawTemp(0,TemperatureData[0],TemperatureData[0],4);
	    for(int i=1; i<=210;i++){
			DrawVacuum(i*4,PressureData[i-1],PressureData[i],4);
			DrawTemp(i*4,TemperatureData[i-1],TemperatureData[i],4);
	    }

	    /*
	    DrawVacuum(0,0,0,3);
		DrawTemp(0,0,0,3);
	    for(int i=1; i<=210;i++){
			DrawVacuum(i*4,i,i,3);
			DrawTemp(i*4,i,i,3);
	    }
	    */

		//SetPageArea(1,0,1,0,128,1,174,1);
	    SetPageArea(1,0,1,0,128,1,82,3);
		Direction_Top2Bottom();
		AbsoluteVertical(45,1);
		printmsg100("        5        10        15        20        25        30        35\n"
				           "                           Time(minute)      \r");
		AbsoluteVertical(40,1);
		//Drawline(174,1,0,0,1);// 가로줄
		Drawline(82,3,0,0,1);// 가로줄

		AbsoluteVertical(0,0);//세로줄
		Drawline(0,0,110,1,1);

		printpage();

		initprint();
		SelectPageMode();
		Direction_Top2Bottom();
		SetPageArea(0,0,0,0,128,1,10,0);
		Drawline(0,0,110,1,1);//세로줄
		printpage();
	}

	else if(cycle==3){
	    initprint();
	    SelectPageMode();
		//점찍기

	    DrawVacuum(0,PressureData[0],PressureData[0],4);
		DrawTemp(0,TemperatureData[0],TemperatureData[0],4);
	    for(int i=1; i<=270;i++){
			DrawVacuum(i*4,PressureData[i-1],PressureData[i],4);
			DrawTemp(i*4,TemperatureData[i-1],TemperatureData[i],4);
	    }
	    /*
	    DrawVacuum(0,0,0,4);
		DrawTemp(0,0,0,4);
	    for(int i=1; i<=270;i++){
			DrawVacuum(i*4,i,i,4);
			DrawTemp(i*4,i,i,4);
	    }
	    */
		//SetPageArea(1,0,1,0,128,1,38,2);
	    SetPageArea(1,0,1,0,128,1,66,4);
		Direction_Top2Bottom();
		AbsoluteVertical(45,1);
		/*
		printmsg100("        5        10        15        20        25        30        35        40        45\n"
				           "                                     Time(minute)           \r");
				           */
		/*
		printmsg100("        5        10        15        20        25        30        35\n"
				           "                           Time(minute)      \r");
		*/
		printmsg100("        5        10        15        20        25        30        35        40        45\n");
		printmsg100("                                     Time(minute)           \n");

		AbsoluteVertical(40,1);
		//Drawline(220,1,0,0,1);
		//Drawline(28,2,0,0,1);

		//Drawline(38,2,0,0,1);
		Drawline(66,4,0,0,1);

		//Drawline(128,1,0,0,10);
		AbsoluteVertical(0,0);
		Drawline(0,0,110,1,1);

		printpage();

		initprint();
		SelectPageMode();
		Direction_Top2Bottom();
		SetPageArea(0,0,0,0,128,1,10,0);
		Drawline(0,0,110,1,1);
		printpage();
	}
}




/*
int printmain(int argc, char *argv[]){


	//int result;
	//printf("Connection failed with reason %d\n", result);
	//result = ConnectPrinter(PRINT_USART, 115200);	// USB


	if (argc == 2){
		int result;

		//result = ConnectPrinter("/dev/ttyS0", 57600);		// COM
		result = ConnectPrinter(PRINT_USART, 115200);	// USB

		if (result != WS_SUCCESS){
			printf("Connection failed with reason %d\n", result);
			return 1;
		}
		switch (atoi(argv[1])){
			case 1: PrintText(); break;
			case 2: PrintReceipt(); break;
			case 3: PrintImage(); break;
			case 4: GetPrinterInfo(); break;
			case 5: ReadMsrInfo(); break;
			default: UsageGuide(argv[0]); break;
		}
		ClosePrinter();
	}

	else{
		UsageGuide(argv[0]);
	}

	return 0;

}



void UsageGuide(char* name){
	printf("****************************\n");
	printf("Usage : %s n\n", name);
	printf("\t n = 1 : Print text\n");
	printf("\t n = 2 : Print sample receipt\n");
	printf("\t n = 3 : Print image\n");
	printf("\t n = 4 : Get printer information\n");
	printf("\t n = 5 : Get MSR data\n");
	printf("****************************\n");
}

void PrintText(){
	InitPrinterStatus();
	SetTextAlignment(1);
	TextSaveSpool("SALES  SLIP\n");
	SetTextAlignment(0);
	TextSaveSpool("--------------------------------\n");
	TextSaveSpool("MERCHANT NAME      Woosim Coffee\n");
	TextSaveSpool("MASTER                LEE IL BOK\n");
	TextSaveSpool("ADDRESS\n");
	TextSaveSpool("     #60, Sandan-ro 388beon-gil,\n");
	TextSaveSpool("    Galsan-myeon, Hongseong-gun,\n");
	TextSaveSpool("       Chungnam-do, 32200, Korea\n");
	TextSaveSpool("HELP DESK        +82-41-339-3700\n");
	TextSaveSpool("--------------------------------\n\n");
	TextSaveSpool("          WOOSIM  CARD          \n");
	TextSaveSpool("--------------------------------\n");
	TextSaveSpool("CARD NO.     4428-8819-0118-****\n");
	TextSaveSpool("EXPIRY                   2022/10\n");
	TextSaveSpool("--------------------------------\n");
	TextSaveSpool("TRANS. YEAR & DATE    2020/04/15\n");
	TextSaveSpool("TRANS CLASS             [Credit]\n");
	TextSaveSpool("TERMINAL ID.           123456789\n");
	TextSaveSpool("APPROVAL NO.           987654321\n");
	TextSaveSpool("AMOUNT                   500.00$\n");
	TextSaveSpool("--------------------------------\n");
	TextSaveSpool("SIGNATURE: \n\n");
	TextSaveSpool("--------------------------------\n\n");
	PrintLineFeed(2);
	PrintSpool();
}

void PrintReceipt(){
	InitPrinterStatus();
	InitPageMode(0, 0, 384, 700);
	SetTextStyle(0, TRUE, 1, 1, FALSE);
	TextSaveSpool("Woosim Cafe\n");

	SetTextStyle(0, FALSE, 0, 0, FALSE);
	TextSaveSpool("Coffee, tea and bread\n");
	PrintDotFeed(15);
	SetFontSize(1);
	Page_SetPosition(277, 105);
	TextSaveSpool("2020-01-27\n");
	SetFontSize(0);

	Page_SetPosition(290, 5);
	QRCodeSaveSpool(0, 'H', 2, "www.woosim.com/english");

	Page_DrawBox(0, 130, 384, 0, 2);
	PrintDotFeed(5);
	TextSaveSpool(" #      Item      Num    Charge\n");
	TextSaveSpool(" 1      Latte      1     $ 3.65\n");
	TextSaveSpool(" 2  House Coffee   2     $ 3.90\n");
	TextSaveSpool(" 3       Tea       1     $ 1.95");
	PrintDotFeed(10);
	Page_DrawBox(0, 270, 384, 0, 2);

	Page_DrawBox(0, 310, 115, 0, 2);
	Page_DrawBox(260, 310, 124, 0, 2);
	Page_SetPosition(123, 300);
	TextSaveSpool("Credit Card\n");
	TextSaveSpool("Number:         ");
	SetFontSize(1);
	TextSaveSpool("6688-9988-6644-****\n");
	SetFontSize(0);
	TextSaveSpool("Payment:                $ 9.50\n");

	Page_DrawBox(0, 400, 120, 0, 2);
	Page_DrawBox(250, 400, 134, 0, 2);
	Page_SetPosition(145, 390);
	TextSaveSpool("Payment\n");
	TextSaveSpool("Price:                  $ 9.28\n");
	TextSaveSpool("Tax:                    $ 0.22\n");
	Page_DrawBox(0, 490, 384, 0, 2);
	PrintDotFeed(15);
	TextSaveSpool("Total:                  $ 9.50\n");
	TextSaveSpool("Credit Card:            $ 9.50\n");

	Page_SetPosition(40, 580);
	OneDimensionBarcodeSaveSpool(CODE128, 2, 50, TRUE, "20150127123456789");
	Page_Print_StandardMode();
	PrintLineFeed(2);
	PrintSpool();
}

void PrintImage(){
	InitPrinterStatus();
	TextSaveSpool("Print Bmp file \n");
	BmpSaveSpool("woosim.bmp");
	PrintLineFeed(2);
	TextSaveSpool("Print pre-downloaded image file #0\n");
	LoadLogoSaveSpool(0);
	PrintLineFeed(2);
	PrintSpool();
}

void GetPrinterInfo(){
	int result;
	unsigned char rcv[100];

	memset(rcv, 0x00, sizeof(rcv));
	result = GetPrinterModelName(rcv, 1000);

	if (result < 0)
	{
		printf("Data receiving failed with reason %d\n", result);
		return;
	}
	printf("Device model name : %s\n", rcv);

	memset(rcv, 0x00, sizeof(rcv));
	result = GetFirmwareVersion(rcv, 1000);

	if (result < 0)
		printf("Data receiving failed with reason %d\n", result);
	else
		printf("Device FW version : %s\n", rcv);
}

void ReadMsrInfo(){
	int result;
	unsigned char rcv[500];

    printf("Swipe card.\n");
	memset(rcv, 0x00, sizeof(rcv));
	result = EnterMSRMode(1, rcv, 5000);
	ProcessMSR(result, rcv, sizeof(rcv));
}

void ProcessMSR(int result, unsigned char* buffer, int bufferLen){
	int waitResult;
	unsigned char msrFail[] = { 0x1B,0x4D,0x31 };

    if (result == WS_TIMEOUT)
    {
        printf("MSR waiting time out.\n");
        result = CancelMSRMode();
    }
    else if (result < 0)
    {
		printf("Data receiving failed with reason %d\n", result);
    }
	else if (result > 3 && memcmp(buffer, msrFail, 3) == 0)
	{
		printf("Card reading failure. Swipe card again\n");
		memset(buffer, 0x00, bufferLen);
		waitResult = WaitMSRMode(buffer, 5000);
		ProcessMSR(waitResult, buffer, bufferLen);
	}
	else
	{
		printf("Card reading Success.\n  %s\n", buffer);
	}
}

*/
