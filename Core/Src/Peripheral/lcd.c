/*
 * lcd.c
 *
 *  Created on: Oct 25, 2022
 *      Author: CBT
 */
#include <string.h>
#include <stdio.h>
#include "main.h"

#include "hardware.h"
#include "sensor.h"
#include "peripheral.h"

#define SHORT		1
#define STANDARD	2
#define ADVANCED	3

extern UART_HandleTypeDef huart1;
extern RTC_HandleTypeDef hrtc;
#define LCD_USART	&huart1

unsigned char	LCD_rx_data[30]={};

int currentpage=0;
int beforepage=0;

int beforeday;

int currentHistoryPage;
int HistoryPageCount;

int LoginFlag, MonitorFlag, LanguageFlag;

float FlashSettingTemp[4][3]={};

int FlashPreesureCondition[3]={};
int Flashperispeed;

struct date_format today_date;
struct date_format reserve_date;

unsigned int inputyear, inputmonth, inputday;
int Adminpassword[4]={1,1,1,1};
int Masterpassword[4]={4,8,5,0};

static int LCD_IsValidFrame(const uint8_t *buffer, uint16_t length)
{
	if (length < 3) {
		return 0;
	}
	if (buffer[0] != 0x5A || buffer[1] != 0xA5) {
		return 0;
	}
	if ((uint16_t)buffer[2] + 3 != length) {
		return 0;
	}
	return 1;
}

int LCD_ReceiveFrame(uint8_t *buffer, uint16_t length, uint32_t timeout, uint8_t retries)
{
	HAL_StatusTypeDef rx_status;

	for (uint8_t attempt = 0; attempt <= retries; attempt++) {
		memset(buffer, 0, length);
		rx_status = HAL_UART_Receive(LCD_USART, buffer, length, timeout);
		if (rx_status == HAL_OK && LCD_IsValidFrame(buffer, length)) {
			return 1;
		}
	}
	return 0;
}

void InitLCD(void){	//LCD 초기화
	HAL_Delay(1000);
	DisplayFirstPage();
	CurrentUser=10;	//Factory 계정
	//Display21page();
	//DisplayPage(21);
	ReadLCD();
	HAL_Delay(100);
	SetRTCFromLCD();
	HAL_Delay(100);
    ReadLCD();
}

void ReadLCD(){
	HAL_UART_Receive_IT(LCD_USART, (uint8_t*)LCD_rx_data, 9);
}

void DisplayFirstPage(){
    DisplayPage(LCD_FIRST_PAGE);
}

void DisplayPage(int page){
	unsigned char   main_page[7] = {0x5A, 0xA5, 0x04, 0x80, 0x03, 0x00, 0x0a};
	main_page[6] = page;
	if(page!=LCD_LOADING_PAGE)
		currentpage=page;
    HAL_UART_Transmit(LCD_USART, main_page, 7, 10);
}

void DisplayDot(int vp1,int vp2,int x, int y){
	unsigned char   Display_Dot[18] = {0x5A, 0xA5,
														0x0F,
														0x82,
														0x40, 0x00,
														0x00, 0x01,
														0x00, 0x01,
														0x00, 0x80,
														0x00, 0x80,
														0xF8, 0x00,
														0xFF, 0x00};
	Display_Dot[10] = x>>8;
	Display_Dot[11] = x&0xff;
	Display_Dot[12] = y>>8;
	Display_Dot[13] = y&0xff;

    HAL_UART_Transmit(LCD_USART, Display_Dot, 18, 10);
}

void DisplayTempGraph(int number, int color){
	unsigned char   Display_Dot[1500] = {0x5A, 0xA5,//헤더
														0x17,//데이터 개수
														0x82,//명령어
														0x40, 0x00,//주소
														0x00, 0x02,//선연결 데이터
														0x00, 0x02,//연결선의 수//8,9
														//0x00, 0x1F,//컬러//10,11
														0xF8, 0x00,//컬러//10,11
														0x00, 0x64, 0x01, 0x9A,//점1	//dot1	12,13,14,15
														0x03, 0x98, 0x00, 0x82,//점2//dot2	16,17,18,19
														0x00, 0xB0, 0x00, 0x80,//점3//dot3	20,21,22,23
														//.....
														0xFF, 0x00}; //End//20
	int DisplayStartPoint[10] = {0,110,200,290,380,470,560,650,740,830};
	int Displaysector;
	unsigned char DisplaysectorAddress1[10]={0x00,0x17,0x17,0x18,0x18,0x19,0x19,0x1A,0x1A,0x1B};
	unsigned char DisplaysectorAddress2[10]={0x00,0x00,0x50,0x00,0x50,0x00,0x50,0x00,0x50,0x00};

	Displaysector=(int)(number+1)/30;

	for(int ii=0;ii<Displaysector;ii++){
		Display_Dot[2]=15+4*30;
		Display_Dot[4]=DisplaysectorAddress1[ii+1];
		Display_Dot[5]=DisplaysectorAddress2[ii+1];
		Display_Dot[8]=30>>8;
		Display_Dot[9]=30&0xFF;

		Display_Dot[12]=DisplayStartPoint[ii+1]>>8;
		Display_Dot[13]=DisplayStartPoint[ii+1]&0xFF;
		Display_Dot[14]=(410-(int)(TemperatureData[ii*30]*3.5))>>8;
		Display_Dot[15]=(410-(int)(TemperatureData[ii*30]*3.5))&0xFF;

		for(int i=1;i<=30;i++){
			Display_Dot[12+i*4]=(3*i+DisplayStartPoint[ii+1])>>8;
			Display_Dot[13+i*4]=(3*i+DisplayStartPoint[ii+1])&0xFF;
			Display_Dot[14+i*4]=(410-(int)(TemperatureData[i+ii*30]*3.5))>>8;
			Display_Dot[15+i*4]=(410-(int)(TemperatureData[i+ii*30]*3.5))&0xFF;
		}
		Display_Dot[16+4*30]=0xFF;
		Display_Dot[17+4*30]=0x00;
		HAL_UART_Transmit(LCD_USART, Display_Dot, 18+4*30, 100);

	}
	if(number!=Displaysector*30){
		Display_Dot[2]=15+4*(number-Displaysector*30);
		Display_Dot[4]=DisplaysectorAddress1[Displaysector+1];
		Display_Dot[5]=DisplaysectorAddress2[Displaysector+1];
		Display_Dot[8]=(number-Displaysector*30)>>8;
		Display_Dot[9]=(number-Displaysector*30)&0xFF;

		Display_Dot[12]=DisplayStartPoint[Displaysector+1]>>8;
		Display_Dot[13]=DisplayStartPoint[Displaysector+1]&0xFF;
		Display_Dot[14]=(410-(int)(TemperatureData[Displaysector*30]*3.5))>>8;
		Display_Dot[15]=(410-(int)(TemperatureData[Displaysector*30]*3.5))&0xFF;

		for(int i=1;i<=(number-Displaysector*30);i++){
			Display_Dot[12+i*4]=(3*i+DisplayStartPoint[Displaysector+1])>>8;
			Display_Dot[13+i*4]=(3*i+DisplayStartPoint[Displaysector+1])&0xFF;
			Display_Dot[14+i*4]=(410-(int)(TemperatureData[i+Displaysector*30]*3.5))>>8;
			Display_Dot[15+i*4]=(410-(int)(TemperatureData[i+Displaysector*30]*3.5))&0xFF;
		}
		Display_Dot[16+4*(number-Displaysector*30)]=0xFF;
		Display_Dot[17+4*(number-Displaysector*30)]=0x00;

		HAL_UART_Transmit(LCD_USART, Display_Dot, 18+4*(number-Displaysector*30), 100);
	}
}



void DisplayInitTempGraph(){
	unsigned char   Display_Dot[1500] = {0x5A, 0xA5,//헤더
															0x17,//데이터 개수
															0x82,//명령어
															0x40, 0x00,//주소
															0x00, 0x02,//선연결 데이터
															0x00, 0x02,//연결선의 수//8,9
															0x00, 0x1F,//컬러//10,11
															0x00, 0x64, 0x01, 0x9A,//점1	//dot1	12,13,14,15
															0x03, 0x98, 0x00, 0x82,//점2//dot2	16,17,18,19
															0x00, 0xB0, 0x00, 0x80,//점3//dot3	20,21,22,23
															//.....
															0xFF, 0x00}; //End//20
		unsigned char DisplaysectorAddress1[10]={0x00,0x17,0x17,0x18,0x18,0x19,0x19,0x1A,0x1A,0x1B};
		unsigned char DisplaysectorAddress2[10]={0x00,0x00,0x50,0x00,0x50,0x00,0x50,0x00,0x50,0x00};

		for(int ii=1;ii<=9;ii++){
			Display_Dot[2]=15+4*30;
			Display_Dot[4]=DisplaysectorAddress1[ii];
			Display_Dot[5]=DisplaysectorAddress2[ii];
			Display_Dot[8]=0x00;
			Display_Dot[9]=0x00;
			Display_Dot[16+4*30]=0xFE;
			Display_Dot[17+4*30]=0x00;
			HAL_UART_Transmit(LCD_USART, Display_Dot, 18+4*30, 100);
		}
}

void DisplayInitVacuumGraph(){
	unsigned char   Display_Dot[1500] = {0x5A, 0xA5,//헤더
															0x17,//데이터 개수
															0x82,//명령어
															0x40, 0x00,//주소
															0x00, 0x02,//선연결 데이터
															0x00, 0x02,//연결선의 수//8,9
															0x00, 0x1F,//컬러//10,11
															0x00, 0x64, 0x01, 0x9A,//점1	//dot1	12,13,14,15
															0x03, 0x98, 0x00, 0x82,//점2//dot2	16,17,18,19
															0x00, 0xB0, 0x00, 0x80,//점3//dot3	20,21,22,23
															//.....
															0xFF, 0x00}; //End//20
		unsigned char DisplaysectorAddress1[10]={0x00,0x1B,0x1C,0x1C,0x1D,0x1D,0x1E,0x1E,0x1F,0x1F};
		unsigned char DisplaysectorAddress2[10]={0x00,0x50,0x00,0x50,0x00,0x50,0x00,0x50,0x00,0x50};

		for(int ii=1;ii<=9;ii++){
			Display_Dot[2]=15+4*30;
			Display_Dot[4]=DisplaysectorAddress1[ii];
			Display_Dot[5]=DisplaysectorAddress2[ii];
			Display_Dot[8]=0x00;
			Display_Dot[9]=0x00;
			Display_Dot[16+4*30]=0xFE;
			Display_Dot[17+4*30]=0x00;
			HAL_UART_Transmit(LCD_USART, Display_Dot, 18+4*30, 100);
		}
}


void DisplayVacuumGraph(int number, int color){
	unsigned char   Display_Dot[1500] = {0x5A, 0xA5,//헤더
														0x17,//데이터 개수
														0x82,//명령어
														0x40, 0x00,//주소
														0x00, 0x02,//선연결 데이터
														0x00, 0x02,//연결선의 수//8,9
														0x00, 0x1F,//컬러//10,11
														0x00, 0x64, 0x01, 0x9A,//점1	//dot1	12,13,14,15
														0x03, 0x98, 0x00, 0x82,//점2//dot2	16,17,18,19
														0x00, 0xB0, 0x00, 0x80,//점3//dot3	20,21,22,23
														//.....
														0xFF, 0x00}; //End//20
	int DisplayStartPoint[10] = {0,110,200,290,380,470,560,650,740,830};
	int Displaysector;
	unsigned char DisplaysectorAddress1[10]={0x00,0x1B,0x1C,0x1C,0x1D,0x1D,0x1E,0x1E,0x1F,0x1F};
	unsigned char DisplaysectorAddress2[10]={0x00,0x50,0x00,0x50,0x00,0x50,0x00,0x50,0x00,0x50};

	Displaysector=(int)(number+1)/30;

	for(int ii=0;ii<Displaysector;ii++){
		Display_Dot[2]=15+4*30;
		Display_Dot[4]=DisplaysectorAddress1[ii+1];
		Display_Dot[5]=DisplaysectorAddress2[ii+1];
		Display_Dot[8]=30>>8;
		Display_Dot[9]=30&0xFF;

		Display_Dot[12]=DisplayStartPoint[ii+1]>>8;
		Display_Dot[13]=DisplayStartPoint[ii+1]&0xFF;
		Display_Dot[14]=(410-(int)(PressureData[ii*30]*0.35))>>8;
		Display_Dot[15]=(410-(int)(PressureData[ii*30]*0.35))&0xFF;

		for(int i=1;i<=30;i++){
			Display_Dot[12+i*4]=(3*i+DisplayStartPoint[ii+1])>>8;
			Display_Dot[13+i*4]=(3*i+DisplayStartPoint[ii+1])&0xFF;
			Display_Dot[14+i*4]=(410-(int)(PressureData[i+ii*30]*0.35))>>8;
			Display_Dot[15+i*4]=(410-(int)(PressureData[i+ii*30]*0.35))&0xFF;
		}
		Display_Dot[16+4*30]=0xFF;
		Display_Dot[17+4*30]=0x00;
		HAL_UART_Transmit(LCD_USART, Display_Dot, 18+4*30, 100);
	}
	if(number!=Displaysector*30){
		Display_Dot[2]=15+4*(number-Displaysector*30);
		Display_Dot[4]=DisplaysectorAddress1[Displaysector+1];
		Display_Dot[5]=DisplaysectorAddress2[Displaysector+1];
		Display_Dot[8]=(number-Displaysector*30)>>8;
		Display_Dot[9]=(number-Displaysector*30)&0xFF;

		Display_Dot[12]=DisplayStartPoint[Displaysector+1]>>8;
		Display_Dot[13]=DisplayStartPoint[Displaysector+1]&0xFF;
		Display_Dot[14]=(410-(int)(PressureData[Displaysector*30]*0.35))>>8;
		Display_Dot[15]=(410-(int)(PressureData[Displaysector*30]*0.35))&0xFF;

		for(int i=1;i<=(number-Displaysector*30);i++){
			Display_Dot[12+i*4]=(3*i+DisplayStartPoint[Displaysector+1])>>8;
			Display_Dot[13+i*4]=(3*i+DisplayStartPoint[Displaysector+1])&0xFF;
			Display_Dot[14+i*4]=(410-(int)(PressureData[i+Displaysector*30]*0.35))>>8;
			Display_Dot[15+i*4]=(410-(int)(PressureData[i+Displaysector*30]*0.35))&0xFF;
		}
		Display_Dot[16+4*(number-Displaysector*30)]=0xFF;
		Display_Dot[17+4*(number-Displaysector*30)]=0x00;

		HAL_UART_Transmit(LCD_USART, Display_Dot, 18+4*(number-Displaysector*30), 100);
	}
	ReadLCD();
}

//LCD 수신
void LCD_Process(){
    int iValue;
	if (!LCD_IsValidFrame(LCD_rx_data, 9)) {
		ReadLCD();
		return;
	}
    switch(LCD_rx_data[4]) {
        case 0x00 :    //button
            LCD_Function_Process(LCD_rx_data[5], LCD_rx_data[8]);
            break;
        case 0x01 :
        	iValue = LCD_rx_data[7];
        	iValue <<= 8;
        	iValue |= LCD_rx_data[8];
        	LCD_Password(LCD_rx_data[5], iValue);
            break;
        case 0x02 :
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_02(LCD_rx_data[5], iValue);
            break;
        case 0x03 :
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_03(LCD_rx_data[5], iValue);
            break;
        case 0x04 :
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_04(LCD_rx_data[5], iValue);
            break;

        case 0x05 :
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_05(LCD_rx_data[5], iValue);
            break;

        case 0x06 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_06(LCD_rx_data[5], iValue);
			break;

        case 0x07 :
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_07(LCD_rx_data[5], iValue);
            break;

        case 0x11 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_11(LCD_rx_data[5], iValue);
			break;

        case 0x12 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_12(LCD_rx_data[5], iValue);
			break;

        case 0x14 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_14(LCD_rx_data[5], iValue);
			break;

        case 0x15 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_15(LCD_rx_data[5], iValue);
			break;



			//user setting
        case 0x21 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_21(LCD_rx_data[5], iValue);
			break;
        case 0x22 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_22(LCD_rx_data[5], iValue);
			break;
        case 0x23 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_23(LCD_rx_data[5], iValue);
			break;
        case 0x24 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_24(LCD_rx_data[5], iValue);
			break;

        case 0x25 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_25(LCD_rx_data[5], iValue);
			break;

        case 0x31 :    // set value;
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_31(LCD_rx_data[5], iValue);
            break;
            //maintenance
        case 0x32 :    // set value;
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_32(LCD_rx_data[5], iValue);
            break;
        case 0x33 :    // set value;
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_33(LCD_rx_data[5], iValue);
            break;
        case 0x34 :    // set value;
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_34(LCD_rx_data[5], iValue);
            break;



			//admin setting
        case 0x41 :    // set value;
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_41(LCD_rx_data[5], iValue);
            break;
        case 0x43 :    // alarm Setting
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_43(LCD_rx_data[5], iValue);
			break;
        case 0x44 :    // error setting
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_44(LCD_rx_data[5], iValue);
			break;

			//factory setting


        case 0x51 :    //
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_51(LCD_rx_data[5], iValue);
			break;
        case 0x52 :    //
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_52(LCD_rx_data[5], iValue);
            break;
        case 0x53 :    // set value;
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_53(LCD_rx_data[5], iValue);
            break;
        case 0x55 :    // set setting;
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_55(LCD_rx_data[5], iValue);
			break;

        case 0x56 :    // set setting;
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_56(LCD_rx_data[5], iValue);
			break;
        case 0x57 :    // set setting;
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_57(LCD_rx_data[5], iValue);
			break;
        case 0x58 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_58(LCD_rx_data[5], iValue);
			break;

        case 0x60 :    // 60page board test
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_60(LCD_rx_data[5], iValue);
			break;

			//login
        case 0x61 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_61(LCD_rx_data[5], iValue);
			break;
        case 0x63 :
			iValue = LCD_rx_data[7];
			iValue <<= 8;
			iValue |= LCD_rx_data[8];
			LCD_63(LCD_rx_data[5], iValue);
			break;
        case 0x64 :
            iValue = LCD_rx_data[7];
            iValue <<= 8;
            iValue |= LCD_rx_data[8];
            LCD_64(LCD_rx_data[5], iValue);
            break;
    }
    ReadLCD();
    SleepPageCount=0;
}

void LCD_Function_Process(int index, int value){
    switch(index){
        case 0:
            DoActionButton(value);
            break;
        case 1:
        	GoToPage(value);
            break;
        case 3:
        	ProcessSettingButton(value);
            break;
            /*
        case 4:
            MaintenanceButton(value);
            break;
        case 5:
			DeveloperButton(value);
			break;
			*/
    }
}

void DoActionButton(int key){	//0000 XXXX(key)
    switch(key) {
    	case 0:
            break;
        case 1: // goto login page
        	if(Pressure<DoorOpenPressure){
        		DisplayPage(LCD_VACUUM_MESSAGE_PAGE);
        	}
        	else{
            	if(LoginFlag==1){
            		if(AutoLoginFlag==1){// 자동로그인
            			CurrentUser=AutoLoginID;
            			DisplayIcon(0x02, 0x30, 1);
                		Display02page();
                		DisplayPage(LCD_CYCLESELECT_PAGE);
            		}
            		else{
            			Display61page();
    					DisplayPage(LCD_LOGIN_PAGE);
            		}
            	}
            	else{
            		DisplayIcon(0x02, 0x30, 0);
            		Display02page();
            		DisplayPage(LCD_CYCLESELECT_PAGE);
            	}
        	}
        	flash_ProcessPowerOffFlag=2;
        	//fortest
/*
        	DisplayPage(LCD_MONITOR_PAGE);
        	PressureData[0]=750;
        	TemperatureData[0]=50;
        	for(int i=1;i<=270;i++){
        		TemperatureData[i]=i;
        		if(TemperatureData[i]>=80){
        			TemperatureData[i]=80;
        		}
        		PressureData[i]=650;
        	}
        	DisplayInitTempGraph();
        	DisplayInitVacuumGraph();

        	DisplayTempGraph(270-1,0);
        	DisplayVacuumGraph(270-1,1);
*/
			//DisplayVacuumGraph(200,1);
        	//DisplayVacuumGraph(10,1);
            break;
        case 2:	//LOGIN BUTTON
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	Display02page();
        	DisplayPage(LCD_CYCLESELECT_PAGE);
        	break;

        case 0x11:	//설정 페이지
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
    		Display21page();
       		DisplayPage(LCD_INFO_INFORMATION_PAGE);
       		currentpage=LCD_INFO_INFORMATION_PAGE;
        	break;

        case 0x12:
        	Display22page();
       		DisplayPage(LCD_INFO_STERILANT_PAGE);
        	break;

        case 0x13:
        	for(int i=0;i<6;i++){
        		temptotalcycle[i]=0;
        	}
        	inputyear=today_date.year;//여기
        	inputmonth=today_date.month;
        	inputday=today_date.day;
        	DisplayPageValue(0x23,0xB0,inputyear);
        	DisplayPageValue(0x23,0xB5,inputmonth);
        	DisplayPageValue(0x23,0xB9,inputday);
        	DisplaySelectIcon(1,0);
			DisplaySelectIcon(2,0);
			DisplaySelectIcon(3,0);
			DisplaySelectIcon(4,0);
			DisplaySelectIcon(5,0);
			saveCycleData.cycleName=0;
			currentHistoryPage=1;
			Display23page();
        	DisplayPage(LCD_INFO_HISTORY_PAGE);
        	break;

        case 0x14:

        	break;
        case 0x21:
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	Display24page();
       		DisplayPage(LCD_USER_SETTING_PAGE);
        	break;

        case 0x30:
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	Display41page();
        	if(CurrentUser==10){
        		DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
    		}
        	else if(CurrentUser==9){
				DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
			}
        	else{
        		beforepage=currentpage;
        		DisplayPage(LCD_ADMIN_MESSAGE_PAGE);
        	}
        	break;

        case 0x31:
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	Display41page();
        	DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
        	break;
        case 0x32:
			VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	//다른 화면으로 이동할때 초기화 추가해야함
			//중요
        	DisplayPage(LCD_ADMIN_PARTSTEST_PAGE);

			break;

        case 0x40:
        	Display51page();
        	if(CurrentUser==10){
        		DisplayPage(LCD_FACTORY_INFOSETTING_PAGE);
        	}
        	else{
        		DisplayPage(LCD_FACTORY_MESSAGE_PAGE);
        	}
        	break;

        case 0x41:
        	Display51page();
        	DisplayPage(LCD_FACTORY_INFOSETTING_PAGE);
        	break;

        case 0x42:
            Display53page();
        	break;

        case 0x43:
        	DisplayPage(LCD_FACTORY_PROCESSSETTING_PAGE);
        	break;

        case 0x44:	//LCD_FACTORY_TEMPSETTING_PAGE
        	//여기
        	Read_Flash();
        	Display55page();
        	break;

        case 0x45:
        	Display55page();
        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
        	CycleName=SHORT;
        	ProcessNum=1;
        	StepNum=1;
        	Read_Flash();
        	DisplayProcessSettingValues();
        	break;

        case 0x46:
        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE);
        	CycleName=SHORT;
        	ProcessNum=1;
        	StepNum=1;
        	CurrentProcess=1;
        	CurrentStep=1;
        	Read_Flash();
        	ReadProcessTime();
        	ReadCycleTime();
        	ReadStepTime();
        	DisplayProcessTestValues();
        	break;

        case 0x47:
        	Inithardware();
        	HeaterControlMode=0;
        	DisplayPage(LCD_FACTORY_CONTROLTEST_PAGE);
        	break;

        case 0x50:
        	Display51page();
			DisplayPage(LCD_FACTORY_FUNCSETTING_PAGE);
			break;

        case 0x51://여기 수정중
			Display51page();
			DisplayPage(LCD_ADMIN_ALARM1_PAGE);
			break;

        case 0x52://여기 수정중
			Display51page();
			DisplayPage(LCD_ADMIN_ALARM2_PAGE);
			break;

        case 0x53://여기 수정중
			Display53page();
			DisplayPage(LCD_ADMIN_ERROR1_PAGE);
			break;

        case 0x54://여기 수정중
			Display53page();
			DisplayPage(LCD_ADMIN_ERROR2_PAGE);
			break;


        case 0x70://
			DisplayPage(beforepage);
			break;

        case 0x71://
        	VacuumCheck=1;
        	DoorOpenVentFlag=1;
			DoorOpenVentCnt=30;
			//여기
			//진공 상태에 따라 시간 차등 분배 구현 예정
			DisplayPage(LCD_LOADING_PAGE);
			break;

        case 0x72://
        	if(LoginFlag==1){
				if(AutoLoginFlag==1){// 자동로그인
					CurrentUser=AutoLoginID;
					DisplayIcon(0x02, 0x30, 1);
					Display02page();
					DisplayPage(LCD_CYCLESELECT_PAGE);
				}
				else{
					Display61page();
					DisplayPage(LCD_LOGIN_PAGE);
				}
			}
			else{
				DisplayIcon(0x02, 0x30, 0);
				Display02page();
				DisplayPage(LCD_CYCLESELECT_PAGE);
			}
			break;

        case 0x99:
        	DisplayPage(LCD_FACTORY_PROCESSSETTING_PAGE);
        	Inithardware();
        	HeaterControlMode=1;
        	break;

        case 0xFD:	//도어 오픈 NO
        	DisplayPage(beforepage);
        	break;

        case 0xFE:	//도어 오픈 YES
        	DoorOpenVentFlag=1;
			DoorOpenVentCnt=30;
			//여기
			//진공 상태에 따라 시간 차등 분배 구현 예정
			DisplayPage(LCD_LOADING_PAGE);
			break;

        case 0xFF:	//도어 오픈
        	if(Running_Flag==0){
        		if(Pressure>DoorOpenPressure){
        			DoorOpenFlag=1;
        		}
        		else{
        			beforepage=currentpage;
        			DisplayPage(LCD_DOOROPENMESSAGE_PAGE);
        		}
        	}
        	break;
    }
    ReadLCD();
}

void GoToPage(int key){	//0001 XXXX(key)
	switch(key) {
  		case 0:
            break;
        case 0x20:
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	DisplayPage(LCD_SETTING_SELECT_PAGE);
        	break;
        case 0x21:	//설정 페이지
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
    		Display21page();
       		DisplayPage(LCD_INFO_INFORMATION_PAGE);
       		currentpage=LCD_INFO_INFORMATION_PAGE;
        	break;
        case 0x22:
        	Display22page();
       		DisplayPage(LCD_INFO_STERILANT_PAGE);
        	break;
        case 0x23:
        	for(int i=0;i<6;i++){
        		temptotalcycle[i]=0;
        	}
        	inputyear=today_date.year;//여기
        	inputmonth=today_date.month;
        	inputday=today_date.day;
        	DisplayPageValue(0x23,0xB0,inputyear);
        	DisplayPageValue(0x23,0xB5,inputmonth);
        	DisplayPageValue(0x23,0xB9,inputday);
        	DisplaySelectIcon(1,0);
			DisplaySelectIcon(2,0);
			DisplaySelectIcon(3,0);
			DisplaySelectIcon(4,0);
			DisplaySelectIcon(5,0);
			saveCycleData.cycleName=0;
			currentHistoryPage=1;
			Display23page();
        	DisplayPage(LCD_INFO_HISTORY_PAGE);
        	break;
        case 0x24:
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	Display24page();
       		DisplayPage(LCD_USER_SETTING_PAGE);
        	break;
        case 0x31:
        	SelfTestInitFlag=1;
        	Display31page();
        	DisplayPage(LCD_USER_TOTALTEST_PAGE);
        	break;
        case 0x32:
        	SelfTestInitFlag=1;
        	Display32page();
        	DisplayPage(LCD_USER_HEATINGTEST_PAGE);
        	break;
        case 0x33:
        	SelfTestInitFlag=1;
        	Display33page();
       		DisplayPage(LCD_USER_PARTTEST_PAGE);
        	break;
        case 0x34:
        	SelfTestInitFlag=1;
        	Display34page();
        	DisplayPage(LCD_USER_VACUUMTEST_PAGE);
			break;
        case 0x40:
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	Display41page();
        	if(CurrentUser==10){
        		DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
    		}
        	else if(CurrentUser==9){
				DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
			}
        	else{
        		beforepage=currentpage;
        		DisplayPage(LCD_ADMIN_MESSAGE_PAGE);
        	}
        	break;
        case 0x41:
        	VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	Display41page();
			DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
        	break;
        case 0x42:
			VacuumPump(0);
			InjectionValve(0);
			VacuumValve(0);
			VentValve(0);
			Plasma(0);
        	//다른 화면으로 이동할때 초기화 추가해야함
			//중요
        	DisplayPage(LCD_ADMIN_PARTSTEST_PAGE);
			break;
        case 0x43:	//ALARM1
			Display43page();
			DisplayPage(LCD_ADMIN_ALARM1_PAGE);
			break;

        case 0x44:	//ERROR1
			Display44page();
			DisplayPage(LCD_ADMIN_ERROR1_PAGE);
			break;


        case 0x50:
        	Display51page();
        	if(CurrentUser==10){
        		DisplayPage(LCD_FACTORY_INFOSETTING_PAGE);
        	}
        	else{
        		beforepage=currentpage;
        		DisplayPage(LCD_FACTORY_MESSAGE_PAGE);
        	}
        	break;

        case 0x51:
        	Display51page();
        	DisplayPage(LCD_FACTORY_INFOSETTING_PAGE);
        	break;

        case 0x52:
        	Display52page();
			DisplayPage(LCD_FACTORY_FUNCSETTING_PAGE);
			break;

        case 0x53:
            Display53page();
        	DisplayPage(LCD_FACTORY_CALIBRATION_PAGE);
        	break;

        case 0x54:
        	DisplayPage(LCD_FACTORY_PROCESSSETTING_PAGE);
        	break;

        case 0x55:
        	Read_Flash();
        	Display55page();
        	DisplayPage(LCD_FACTORY_CONTROLSETTING_PAGE);
        	break;

        case 0x56:
        	CycleName=SHORT;
        	ProcessNum=1;
        	StepNum=1;
        	Read_Flash();
        	ReadProcessTime();
        	ReadCycleTime();
        	ReadStepTime();
        	currentpage=LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE;
        	Display56page();
        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
        	break;

        case 0x57:
        	CycleName=SHORT;
        	ProcessNum=1;
        	StepNum=1;
        	CurrentProcess=1;
        	CurrentStep=1;
        	Read_Flash();
        	ReadProcessTime();
        	ReadCycleTime();
        	ReadStepTime();
        	currentpage=LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE;
        	Display57page();
        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE);
        	break;

        case 0x60:
        	Inithardware();
        	HeaterControlMode=0;
        	DisplayPage(LCD_FACTORY_CONTROLTEST_PAGE);
        	break;
    }
    ReadLCD();
}

//Process Setting Page
void ProcessSettingButton(int key){	//03XX
    switch(key) {
        case 0x55:
        	//센서 세팅 값에서 저장된 값 저장시
        	if(FlashSettingTemp[0][0]!=0){
				for(int i=0;i<3;i++){
					DoorSettingTemp[i]=FlashSettingTemp[0][i];
					ChamberSettingTemp[i]=FlashSettingTemp[1][i];
					ChamberBackSettingTemp[i]=FlashSettingTemp[2][i];
					VaporizerSettingTemp[i]=FlashSettingTemp[3][i];
				}
        	}
        	if(FlashPreesureCondition[0]!=0){
        		PreesureCondition[0]=FlashPreesureCondition[0];
        		PreesureCondition[1]=FlashPreesureCondition[1];
        		PreesureCondition[2]=FlashPreesureCondition[2];
        	}
        	if(Flashperispeed!=0){
        		perispeed=Flashperispeed;
        	}

        	Write_Flash();
        	DisplayProcessSettingValues();
        	DisplayPage(currentpage);
			break;
        case 0x60:	//TEST START
        	ReadProcessTime();
        	ReadCycleTime();
        	ReadStepTime();
        	StartProcess();
        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST3_PAGE);
        	ReadProcessTime();
        	ReadStepTime();
        	DisplayProcessTestValues();
			break;

        case 0x61:
        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE);
        	StepNum=0;
        	DisplayProcessSettingValues();
        	break;

        case 0x62:
        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST2_PAGE);
        	StepNum=11;
        	DisplayProcessSettingValues();
			break;

        case 0x65:	//TEST STOP
        	ReadProcessTime();
        	ReadCycleTime();
        	ReadStepTime();
        	StopProcess();
        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE);
        	DisplayProcessTestValues();
			break;

    }
    DisplayProcessSettingValues();
    //ReadLCD();
}



void LCD_Password(int index, int value){
	char InputPassword[4];
	//int Adminpassword[4]={1,1,1,1};
	//int Masterpassword[4]={4,8,5,0};
    switch(index) {
		case 0x01 : //
			InputPassword[0]=value/1000;
			InputPassword[1]=(value%1000)/100;
			InputPassword[2]=(value%100)/10;
			InputPassword[3]=value%10;
			if((InputPassword[0]==Masterpassword[0])&&(InputPassword[1]==Masterpassword[1])&&(InputPassword[2]==Masterpassword[2])&&(InputPassword[3]==Masterpassword[3])){
				CurrentUser=10;
				DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
			}
			else if((InputPassword[0]==Adminpassword[0])&&(InputPassword[1]==Adminpassword[1])&&(InputPassword[2]==Adminpassword[2])&&(InputPassword[3]==Adminpassword[3])){
				CurrentUser=9;
				//DisplayPage(LCD_INFO_INFORMATION_PAGE+50);
			}
			else{

			}
			break;
		case 0x71 : //
			InputPassword[0]=value/1000;
			InputPassword[1]=(value%1000)/100;
			InputPassword[2]=(value%100)/10;
			InputPassword[3]=value%10;
			if((InputPassword[0]==Masterpassword[0])&&(InputPassword[1]==Masterpassword[1])&&(InputPassword[2]==Masterpassword[2])&&(InputPassword[3]==Masterpassword[3])){
				DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
			}
			else if((InputPassword[0]==Adminpassword[0])&&(InputPassword[1]==Adminpassword[1])&&(InputPassword[2]==Adminpassword[2])&&(InputPassword[3]==Adminpassword[3])){
				DisplayPage(LCD_ADMIN_PMSCHEDULE_PAGE);
			}
			else{
				DisplayPage(LCD_WRONG_PW_MESSAGE_PAGE);
			}
			break;
		case 0x72 : //
			InputPassword[0]=value/1000;
			InputPassword[1]=(value%1000)/100;
			InputPassword[2]=(value%100)/10;
			InputPassword[3]=value%10;
			if((InputPassword[0]==Masterpassword[0])&&(InputPassword[1]==Masterpassword[1])&&(InputPassword[2]==Masterpassword[2])&&(InputPassword[3]==Masterpassword[3])){
				DisplayPage(LCD_FACTORY_INFOSETTING_PAGE);
			}
			else{
				DisplayPage(LCD_WRONG_PW_MESSAGE_PAGE);
			}
			break;
    }
    ReadLCD();
}


void LCD_02(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					//SHORT MODE
					DisplayPage(LCD_LOADING_PAGE);
					CycleName=SHORT;
					Read_Flash();
					ProcessNum=1;
					StepNum=1;
					CurrentProcess=1;
					CurrentStep=1;
		        	ReadProcessTime();
		        	ReadCycleTime();
		        	ReadStepTime();
		        	DisplayCycleValue(CycleName);
		        	DisplayIcon(0x03,0x10,CycleName);
		        	DisplayIcon(0x05,0x10,(CycleTime/10)/60/10);
		        	DisplayIcon(0x05,0x20,(CycleTime/10)/60%10);
		        	DisplayIcon(0x05,0x30,(CycleTime/10)%60/10);
		        	DisplayIcon(0x05,0x40,(CycleTime/10)%60%10);
		        	RFIDCheck();
		        	DisplayPage(LCD_STANDBY_PAGE);
					break;
				case 0x02 :
					//STANDARD MODE
					DisplayPage(LCD_LOADING_PAGE);
					CycleName=STANDARD;
					Read_Flash();
					ProcessNum=1;
					StepNum=1;
					CurrentProcess=1;
					CurrentStep=1;
		        	ReadProcessTime();
		        	ReadCycleTime();
		        	ReadStepTime();
		        	DisplayCycleValue(CycleName);
		        	DisplayIcon(0x03,0x10,CycleName);
		        	DisplayIcon(0x05,0x10,(CycleTime/10)/60/10);
		        	DisplayIcon(0x05,0x20,(CycleTime/10)/60%10);
		        	DisplayIcon(0x05,0x30,(CycleTime/10)%60/10);
		        	DisplayIcon(0x05,0x40,(CycleTime/10)%60%10);
		        	RFIDCheck();
		        	DisplayPage(LCD_STANDBY_PAGE);
					break;
				case 0x03 :
					//ADVANCED MODE
					DisplayPage(LCD_LOADING_PAGE);
					CycleName=ADVANCED;
					Read_Flash();
					ProcessNum=1;
					StepNum=1;
					CurrentProcess=1;
					CurrentStep=1;
					ReadProcessTime();
					ReadCycleTime();
					ReadStepTime();
					DisplayCycleValue(CycleName);
					DisplayIcon(0x03,0x10,CycleName);
					DisplayIcon(0x05,0x10,(CycleTime/10)/60/10);
					DisplayIcon(0x05,0x20,(CycleTime/10)/60%10);
					DisplayIcon(0x05,0x30,(CycleTime/10)%60/10);
					DisplayIcon(0x05,0x40,(CycleTime/10)%60%10);
					RFIDCheck();
					DisplayPage(LCD_STANDBY_PAGE);
					break;
				case 0x04 :
					DisplayPage(201);
					break;
				case 0x05 :
					DisplayPage(202);
					break;
				case 0x06 :
					DisplayPage(203);
					break;

				case 0x10 :
					//Logout Popup
					if(LoginFlag==1){
						DisplayPage(LCD_LOGOUT_POPUP);
					}
					break;
				case 0x11 :
					//Logout yes
					//아이디 초기화
					AutoLoginFlag=2;
					AutoLoginID=0;
					CurrentUser=0;
					DisplayPage(LCD_FIRST_PAGE);
					break;
				case 0x12 :
					//Logout no
					DisplayPage(LCD_CYCLESELECT_PAGE);
					break;
			}
			break;
    }
}
void LCD_03(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					//GO TO HOME
		        	DisplayPage(LCD_CYCLESELECT_PAGE);
					break;
			}
			break;
    }
    Display03page();
}
void LCD_04(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					//GO TO HOME
		        	DisplayPage(LCD_CYCLESELECT_PAGE);
					break;
				case 0x02 :
					if(SleepModeStatus==1){
						beforepage=currentpage;
						DisplayPage(LCD_SLEEPMODE_MESSAGE_PAGE);
					}
					else{
						RFIDCheck();
						if(Alarm_Check()==0){
							if(PreAlarm_Check()==0){
								StartProcess();
								DisplayPage(LCD_RUNNING_PAGE);
							}
							else{
								if(devicePreAlarm[1]==1){
									DisplayIcon(0x14, 0x10,0);
									DisplayIcon(0x14, 0x20,0);
									DisplayPage(LCD_PREALARM_PAGE);
									devicePreAlarm[1]=0;
								}
								else if(devicePreAlarm[2]==1){
									DisplayIcon(0x14, 0x10,1);
									DisplayIcon(0x14, 0x20,1);
									DisplayPage(LCD_PREALARM_PAGE);
									devicePreAlarm[2]=0;
								}
								else if(devicePreAlarm[3]==1){
									DisplayIcon(0x14, 0x10,2);
									DisplayIcon(0x14, 0x20,2);
									DisplayPage(LCD_PREALARM_PAGE);
									devicePreAlarm[3]=0;
								}
								else if(devicePreAlarm[4]==1){
									DisplayIcon(0x14, 0x10,3);
									DisplayIcon(0x14, 0x20,3);
									DisplayPage(LCD_PREALARM_PAGE);
									devicePreAlarm[4]=0;
								}
								else{
									StartProcess();
									DisplayPage(LCD_RUNNING_PAGE);
								}
							}
						}
						else{
							if(devicealarm[1]==1){
								DisplayPage(LCD_ALARM1_PAGE);
							}
							else if(devicealarm[2]==1){
								DisplayPage(LCD_ALARM2_PAGE);
							}
							else if(devicealarm[3]==1){
								DisplayPage(LCD_ALARM2_PAGE);
							}
							else if(devicealarm[4]==1){
								DisplayPage(LCD_ALARM2_PAGE);
							}
							else if(devicealarm[5]==1){
								DisplayPage(LCD_ALARM2_PAGE);
							}
							else if(devicealarm[11]==1){
								DisplayPage(LCD_ALARM4_PAGE);
							}
							else if(devicealarm[12]==1){
								DisplayPage(LCD_ALARM4_PAGE);
							}
							else if(devicealarm[13]==1){
								DisplayPage(LCD_ALARM4_PAGE);
							}
							else if(devicealarm[6]==1){
								DisplayPage(LCD_ALARM3_PAGE);
							}
							else if(devicealarm[7]==1){
								DisplayPage(LCD_ALARM3_PAGE);
							}
							else if(devicealarm[8]==1){
								DisplayPage(LCD_ALARM3_PAGE);
							}
							else if(devicealarm[9]==1){
								DisplayPage(LCD_ALARM3_PAGE);
							}

							else{
								DisplayPage(LCD_ALARM2_PAGE);
							}
						}
					}
					break;
			}
			break;
    }
}

void LCD_05(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					if(MonitorFlag==1){
						beforepage=currentpage;
						DisplayPage(LCD_MONITOR_PAGE);
						Display11page();
			        	DisplayInitTempGraph();
			        	DisplayInitVacuumGraph();
			        	DisplayTempGraph(DataCounter-1,0);
			        	DisplayVacuumGraph(DataCounter-1,1);
					}
					break;
			}
			break;
    }
}


void LCD_06(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					if(MonitorFlag==1){
						beforepage=currentpage;
						DisplayPage(LCD_MONITOR_PAGE);
						Display11page();
						DisplayInitTempGraph();
						DisplayInitVacuumGraph();
						DisplayTempGraph(DataCounter-1,0);
						DisplayVacuumGraph(DataCounter-1,1);
					}
					break;
				case 0x02 :
		        	DisplayPage(LCD_CYCLESELECT_PAGE);
					break;
				case 0x03 :
					//공정 결과 프린트
					CyclePrint();
					break;
			}
			break;
    }
}

void LCD_07(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					StopProcess();
					break;
			}
			break;
    }
}
void LCD_11(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					DisplayPage(beforepage);
					break;
			}
			break;
    }
}

void LCD_12(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					//예약
					//1.입력 값이 현재시간보다 뒤 인지 확인하고 늦는다면 알람 띄워주기
					//2.
					if(Calculate_reserve_time()==1){
						reservationRunningFlag=1;
						DisplayPage(LCD_RESERVATION_RUNNING_PAGE);
					}
					else if(Calculate_reserve_time()==2){
						//절전모드와 충돌
						//시간을 다시 입력해주세요. 팝업
						reservationRunningFlag=0;
						DisplayPage(LCD_RESERVATION_TIME_MESSAGE2_PAGE);
					}
					else{
						//10분 미만으로 남음 시간 다시 입력
						//시간을 다시 입력해주세요. 팝업
						reservationRunningFlag=0;
						DisplayPage(LCD_RESERVATION_TIME_MESSAGE1_PAGE);
					}
					break;
				case 0x02 :
					//예약 취소
					reservationRunningFlag=0;
					DisplayPage(LCD_CYCLESELECT_PAGE);
					break;

				case 0x03 :
					//예약 취소
					ReservationInit();
					Display12page();
					DisplayPage(LCD_RESERVATION_PAGE);
					break;

				case 0x05 :
					//예약설정
					if(reservationFlag==1){
						ReservationInit();
						Display12page();
						DisplayPage(LCD_RESERVATION_PAGE);
					}
					else{

					}
					break;
			}
			break;
		case 0x20 :
			reserve_date.month=value;
			DisplayPageValue(0x12,0x20,reserve_date.month);
			DisplayIcon(0x12,0x40,reserve_date.month/10);
			DisplayIcon(0x12,0x50,reserve_date.month%10);
			break;

		case 0x25 :
			reserve_date.day=value;
			DisplayPageValue(0x12,0x25,reserve_date.day);
			DisplayIcon(0x12,0x60,reserve_date.day/10);
			DisplayIcon(0x12,0x70,reserve_date.day%10);
			break;

		case 0x30 :
			reserve_date.hour=value;
			DisplayPageValue(0x12,0x30,reserve_date.hour);
			DisplayIcon(0x12,0x80,reserve_date.hour/10);
			DisplayIcon(0x12,0x90,reserve_date.hour%10);
			break;

		case 0x35 :
			reserve_date.minute=value;
			DisplayPageValue(0x12,0x35,reserve_date.minute);
			DisplayIcon(0x12,0xA0,reserve_date.minute/10);
			DisplayIcon(0x12,0xB0,reserve_date.minute%10);
			break;
    }
}

void LCD_14(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
        			if(devicePreAlarm[1]==1){
        				DisplayIcon(0x14, 0x10,0);
        				DisplayIcon(0x14, 0x20,0);
        				DisplayPage(LCD_PREALARM_PAGE);
        				devicePreAlarm[1]=0;
        			}
        			else if(devicePreAlarm[2]==1){
        				DisplayIcon(0x14, 0x10,1);
        				DisplayIcon(0x14, 0x20,1);
        				DisplayPage(LCD_PREALARM_PAGE);
        				devicePreAlarm[2]=0;
        			}
        			else if(devicePreAlarm[3]==1){
        				DisplayIcon(0x14, 0x10,2);
        				DisplayIcon(0x14, 0x20,2);
						DisplayPage(LCD_PREALARM_PAGE);
						devicePreAlarm[3]=0;
					}
        			else if(devicePreAlarm[4]==1){
        				DisplayIcon(0x14, 0x10,3);
        				DisplayIcon(0x14, 0x20,3);
						DisplayPage(LCD_PREALARM_PAGE);
						devicePreAlarm[4]=0;
					}
        			else{
						StartProcess();
						DisplayPage(LCD_RUNNING_PAGE);
            			//DisplayPage(LCD_STANDBY_PAGE);
        			}
					break;
			}
			break;
    }

}

void LCD_15(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					Read_Flash();
					ProcessNum=1;
					StepNum=1;
					CurrentProcess=1;
					CurrentStep=1;
		        	ReadProcessTime();
		        	ReadCycleTime();
		        	ReadStepTime();
		        	DisplayCycleValue(CycleName);
		        	DisplayIcon(0x03,0x10,1);
		        	DisplayIcon(0x05,0x10,(CycleTime/10)/60/10);
		        	DisplayIcon(0x05,0x20,(CycleTime/10)/60%10);
		        	DisplayIcon(0x05,0x30,(CycleTime/10)%60/10);
		        	DisplayIcon(0x05,0x40,(CycleTime/10)%60%10);
		        	RFIDCheck();
		        	if(Alarm_Check()==0){
		        		if(PreAlarm_Check()==0){
							DisplayPage(LCD_STANDBY_PAGE);
						}
						else{
							if(devicePreAlarm[1]==1){
								DisplayIcon(0x14, 0x10,0);
								DisplayIcon(0x14, 0x20,0);
								DisplayPage(LCD_PREALARM_PAGE);
								devicePreAlarm[1]=0;
							}
							else if(devicePreAlarm[2]==1){
								DisplayIcon(0x14, 0x10,1);
								DisplayIcon(0x14, 0x20,1);
								DisplayPage(LCD_PREALARM_PAGE);
								devicePreAlarm[2]=0;
							}
							else if(devicePreAlarm[3]==1){
								DisplayIcon(0x14, 0x10,2);
								DisplayIcon(0x14, 0x20,2);
								DisplayPage(LCD_PREALARM_PAGE);
								devicePreAlarm[3]=0;
							}
							else if(devicePreAlarm[4]==1){
								DisplayIcon(0x14, 0x10,3);
								DisplayIcon(0x14, 0x20,3);
								DisplayPage(LCD_PREALARM_PAGE);
								devicePreAlarm[4]=0;
							}
						}
		        	}
		        	else{
		        		if(devicealarm[1]==1){
		        			DisplayPage(LCD_ALARM1_PAGE);
		        		}
		        		else if(devicealarm[2]==1){
							DisplayPage(LCD_ALARM2_PAGE);
						}
		        		else if(devicealarm[3]==1){
							DisplayPage(LCD_ALARM2_PAGE);
						}
		        		else if(devicealarm[4]==1){
							DisplayPage(LCD_ALARM2_PAGE);
						}
		        		else if(devicealarm[5]==1){
							DisplayPage(LCD_ALARM2_PAGE);
						}
		        		else if(devicealarm[6]==1){
							DisplayPage(LCD_ALARM3_PAGE);
						}
		        		else if(devicealarm[7]==1){
							DisplayPage(LCD_ALARM3_PAGE);
						}
		        		else if(devicealarm[8]==1){
							DisplayPage(LCD_ALARM3_PAGE);
						}
		        		else if(devicealarm[9]==1){
							DisplayPage(LCD_ALARM3_PAGE);
						}

		        		else if(devicealarm[11]==1){
							DisplayPage(LCD_ALARM4_PAGE);
						}
		        		else if(devicealarm[12]==1){
							DisplayPage(LCD_ALARM4_PAGE);
						}
		        		else if(devicealarm[13]==1){
							DisplayPage(LCD_ALARM4_PAGE);
						}

		        		else{
		        			DisplayPage(LCD_ALARM2_PAGE);
		        		}
		        	}
					break;
			}
			break;
    }

}

void LCD_21(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x03 :
					printInformation();
					break;
			}
			break;
    }
}

void LCD_22(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					beforepage=currentpage;
					DisplayPage(LCD_LOADING_PAGE);
					RFIDCheck();
					DisplayPage(beforepage);
					break;

				case 0x03 :
					if(CurrentRFIDData.production_year==0){

					}
					else{
						printSterilant();
					}
					break;
			}
			break;
    }
    Display22page();
}

void LCD_23(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					if(inputyear==0||inputmonth==0||inputday==0){

					}
					else{
						ReadListData(inputyear,inputmonth,inputday);
						DisplaySelectIcon(1,0);
						DisplaySelectIcon(2,0);
						DisplaySelectIcon(3,0);
						DisplaySelectIcon(4,0);
						DisplaySelectIcon(5,0);
						if(fileCount>0&&fileCount<=5){
							HistoryPageCount=1;
						}
						else{
							HistoryPageCount=(int)fileCount/5+1;
						}
						currentHistoryPage=1;

					}
		        	if(CurrentUser==10){
		        		DisplayPage(LCD_INFO_HISTORY_PAGE);
		    		}
		        	else if(CurrentUser==9){
		        		//DisplayPage(LCD_INFO_HISTORY_PAGE+50);
		        	}
		        	else{
						//DisplayPage(LCD_INFO_HISTORY_PAGE+60);
		        	}
					break;

				case 0x03 :
					break;

				case 0x0A :
					if(saveCycleData.cycleName!=0){
						LoadCyclePrint();
					}
					break;
				case 0x11 :
					if(fileCount>0&&temptotalcycle[5*(currentHistoryPage-1)+1]!=0){
					LoadCycleData(inputyear,inputmonth,inputday,temptotalcycle[5*(currentHistoryPage-1)+1]);
						DisplaySelectIcon(1,1);
						DisplaySelectIcon(2,0);
						DisplaySelectIcon(3,0);
						DisplaySelectIcon(4,0);
						DisplaySelectIcon(5,0);
					}
		        	if(CurrentUser==10){
		        		DisplayPage(LCD_INFO_HISTORY_PAGE);
		    		}
		        	else if(CurrentUser==9){
		        		//DisplayPage(LCD_INFO_HISTORY_PAGE+50);
		        	}
		        	else{
						//DisplayPage(LCD_INFO_HISTORY_PAGE+60);
		        	}
					break;
				case 0x12 :
					if(fileCount>0&&temptotalcycle[5*(currentHistoryPage-1)+2]!=0){
						LoadCycleData(inputyear,inputmonth,inputday,temptotalcycle[5*(currentHistoryPage-1)+2]);
						DisplaySelectIcon(1,0);
						DisplaySelectIcon(2,1);
						DisplaySelectIcon(3,0);
						DisplaySelectIcon(4,0);
						DisplaySelectIcon(5,0);
					}
		        	if(CurrentUser==10){
		        		DisplayPage(LCD_INFO_HISTORY_PAGE);
		    		}
		        	else if(CurrentUser==9){
		        		//DisplayPage(LCD_INFO_HISTORY_PAGE+50);
		        	}
		        	else{
						//DisplayPage(LCD_INFO_HISTORY_PAGE+60);
		        	}
					break;
				case 0x13 :
					if(fileCount>0&&temptotalcycle[5*(currentHistoryPage-1)+3]!=0){
						LoadCycleData(inputyear,inputmonth,inputday,temptotalcycle[5*(currentHistoryPage-1)+3]);
						DisplaySelectIcon(1,0);
						DisplaySelectIcon(2,0);
						DisplaySelectIcon(3,1);
						DisplaySelectIcon(4,0);
						DisplaySelectIcon(5,0);
					}
		        	if(CurrentUser==10){
		        		DisplayPage(LCD_INFO_HISTORY_PAGE);
		    		}
		        	else if(CurrentUser==9){
		        		//DisplayPage(LCD_INFO_HISTORY_PAGE+50);
		        	}
		        	else{
						//DisplayPage(LCD_INFO_HISTORY_PAGE+60);
		        	}
					break;
				case 0x14 :
					if(fileCount>0&&temptotalcycle[5*(currentHistoryPage-1)+4]!=0){
						LoadCycleData(inputyear,inputmonth,inputday,temptotalcycle[5*(currentHistoryPage-1)+4]);
						DisplaySelectIcon(1,0);
						DisplaySelectIcon(2,0);
						DisplaySelectIcon(3,0);
						DisplaySelectIcon(4,1);
						DisplaySelectIcon(5,0);
					}
		        	if(CurrentUser==10){
		        		DisplayPage(LCD_INFO_HISTORY_PAGE);
		    		}
		        	else if(CurrentUser==9){
		        		//DisplayPage(LCD_INFO_HISTORY_PAGE+50);
		        	}
		        	else{
						//DisplayPage(LCD_INFO_HISTORY_PAGE+60);
		        	}
					break;
				case 0x15 :
					if(fileCount>0&&temptotalcycle[5*(currentHistoryPage-1)+5]!=0){
						LoadCycleData(inputyear,inputmonth,inputday,temptotalcycle[5*(currentHistoryPage-1)+5]);
						DisplaySelectIcon(1,0);
						DisplaySelectIcon(2,0);
						DisplaySelectIcon(3,0);
						DisplaySelectIcon(4,0);
						DisplaySelectIcon(5,1);
					}
		        	if(CurrentUser==10){
		        		DisplayPage(LCD_INFO_HISTORY_PAGE);
		    		}
		        	else if(CurrentUser==9){
		        		//DisplayPage(LCD_INFO_HISTORY_PAGE+50);
		        	}
		        	else{
						//DisplayPage(LCD_INFO_HISTORY_PAGE+60);
		        	}
					break;
				case 0x16 :
					if(HistoryPageCount==1){
						currentHistoryPage=1;
					}
					else if(HistoryPageCount==2){
						if(currentHistoryPage==1){

						}
						else{
							currentHistoryPage=1;
						}
					}
					else if(HistoryPageCount==3){
						if(currentHistoryPage==1){
							currentHistoryPage=1;
						}
						else if(currentHistoryPage==2){
							currentHistoryPage=1;
						}
						else if(currentHistoryPage>=3){
							currentHistoryPage=2;
						}
					}
					else if(HistoryPageCount==4){
						if(currentHistoryPage==1){
							currentHistoryPage=1;
						}
						else if(currentHistoryPage==2){
							currentHistoryPage=1;
						}
						else if(currentHistoryPage==3){
							currentHistoryPage=2;
						}
						else if(currentHistoryPage>=4){
							currentHistoryPage=3;
						}
					}
					else if(HistoryPageCount>=5){
						if(currentHistoryPage==1){
							currentHistoryPage=1;
						}
						else if(currentHistoryPage==2){
							currentHistoryPage=1;
						}
						else if(currentHistoryPage==3){
							currentHistoryPage=2;
						}
						else if(currentHistoryPage==4){
							currentHistoryPage=3;
						}
						else if(currentHistoryPage>=5){
							currentHistoryPage=4;
						}
					}

					break;

				case 0x17 :
					if(HistoryPageCount==1){
						currentHistoryPage=1;
					}
					else if(HistoryPageCount==2){
						if(currentHistoryPage==1){
							currentHistoryPage=2;
						}
						else{
							currentHistoryPage=2;
						}
					}
					else if(HistoryPageCount==3){
						if(currentHistoryPage==1){
							currentHistoryPage=2;
						}
						else if(currentHistoryPage>=2){
							currentHistoryPage=3;
						}
					}
					else if(HistoryPageCount==4){
						if(currentHistoryPage==1){
							currentHistoryPage=2;
						}
						else if(currentHistoryPage==2){
							currentHistoryPage=3;
						}
						else if(currentHistoryPage>=3){
							currentHistoryPage=4;
						}
					}
					else if(HistoryPageCount>=5){
						if(currentHistoryPage==1){
							currentHistoryPage=2;
						}
						else if(currentHistoryPage==2){
							currentHistoryPage=3;
						}
						else if(currentHistoryPage==3){
							currentHistoryPage=4;
						}
						else if(currentHistoryPage>=4){
							currentHistoryPage=5;
						}
					}
					break;
			}

			break;
		case 0xB0 :
			inputyear=value;
			DisplaySelectIcon(1,0);
			DisplaySelectIcon(2,0);
			DisplaySelectIcon(3,0);
			DisplaySelectIcon(4,0);
			DisplaySelectIcon(5,0);
			DisplayPage10Char(0x21,0x60,"");
			DisplayPage10Char(0x21,0x70,"");
			DisplayPage10Char(0x21,0x80,"");
			DisplayPage10Char(0x21,0x90,"");
			DisplayPage10Char(0x21,0xA0,"");
			saveCycleData.cycleName=0;
			currentHistoryPage=1;
			break;

		case 0xB5 :
			inputmonth=value;
			DisplaySelectIcon(1,0);
			DisplaySelectIcon(2,0);
			DisplaySelectIcon(3,0);
			DisplaySelectIcon(4,0);
			DisplaySelectIcon(5,0);
			DisplayPage10Char(0x21,0x60,"");
			DisplayPage10Char(0x21,0x70,"");
			DisplayPage10Char(0x21,0x80,"");
			DisplayPage10Char(0x21,0x90,"");
			DisplayPage10Char(0x21,0xA0,"");
			saveCycleData.cycleName=0;
			currentHistoryPage=1;
			break;

		case 0xB9 :
			inputday=value;
			DisplaySelectIcon(1,0);
			DisplaySelectIcon(2,0);
			DisplaySelectIcon(3,0);
			DisplaySelectIcon(4,0);
			DisplaySelectIcon(5,0);
			DisplayPage10Char(0x21,0x60,"");
			DisplayPage10Char(0x21,0x70,"");
			DisplayPage10Char(0x21,0x80,"");
			DisplayPage10Char(0x21,0x90,"");
			DisplayPage10Char(0x21,0xA0,"");
			saveCycleData.cycleName=0;
			currentHistoryPage=1;
			break;
    }
    Display23page();
}

void LCD_24(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					if(MonitorFlag==1){
						MonitorFlag=2;
					}
					else{
						MonitorFlag=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display24page();
					break;
				case 0x02 :
					if(reservationFlag==1){
						reservationFlag=2;
					}
					else{
						reservationFlag=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display24page();
					break;
				case 0x04 :
					if(autoprintFlag==1){
						autoprintFlag=2;
					}
					else{
						autoprintFlag=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display24page();
					break;
				case 0x06 :
					if(printdataFlag==1){
						printdataFlag=2;
					}
					else{
						printdataFlag=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display24page();
					break;
				case 0x07 :
					if(printgraphFlag==1){
						printgraphFlag=2;
					}
					else{
						printgraphFlag=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display24page();
					break;
				case 0x10 :
					if(LanguageFlag==1){
						LanguageFlag=2;
						currentpage=24;
					}
					else{
						LanguageFlag=1;
						currentpage=24;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display24page();
					break;
				case 0x11 :
					Display25page();
				    DisplayPage(LCD_USER_SLEEPMODE_SETTING_PAGE);
					break;
			}
			break;
		case 0x50 : // PrintCopy
			printcopy=value;
		    beforepage=currentpage;
		    Write_Flash();
		    DisplayPage(beforepage);
		    Display24page();
			break;
    }
}

void LCD_25(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					if(SleepModeFlag==1){
						SleepModeFlag=0;
						HeaterControlMode=1;
					}
					else{
						SleepModeFlag=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display25page();
					break;
				case 0x02 :
					if(ActiveWeekday[1]==1){
						ActiveWeekday[1]=2;
					}
					else{
						ActiveWeekday[1]=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display25page();
					break;
				case 0x03 :
					if(ActiveWeekday[2]==1){
						ActiveWeekday[2]=2;
					}
					else{
						ActiveWeekday[2]=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display25page();
					break;
				case 0x04 :
					if(ActiveWeekday[3]==1){
						ActiveWeekday[3]=2;
					}
					else{
						ActiveWeekday[3]=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display25page();
					break;
				case 0x05 :
					if(ActiveWeekday[4]==1){
						ActiveWeekday[4]=2;
					}
					else{
						ActiveWeekday[4]=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display25page();
					break;
				case 0x06 :
					if(ActiveWeekday[5]==1){
						ActiveWeekday[5]=2;
					}
					else{
						ActiveWeekday[5]=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display25page();
					break;
				case 0x07 :
					if(ActiveWeekday[6]==1){
						ActiveWeekday[6]=2;
					}
					else{
						ActiveWeekday[6]=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display25page();
					break;
				case 0x08 :
					if(ActiveWeekday[0]==1){
						ActiveWeekday[0]=2;
					}
					else{
						ActiveWeekday[0]=1;
					}
				    beforepage=currentpage;
				    Write_Flash();
				    DisplayPage(beforepage);
				    Display25page();
					break;

				case 0x10 :
					//절전 해제 메세지
					//DisplayPage(LCD_SLEEPMODE_STOP_MESSAGE_PAGE);
					SleepModeRunning_Flag=0;
					if(beforepage==LCD_SLEEPMODE_PAGE){
						DisplayPage(LCD_CYCLESELECT_PAGE);
					}
					else{
						DisplayPage(beforepage);
					}

					break;

				case 0x11 :
					//여기
					SleepModeCancel();
					break;

				case 0x12 :
					//절전 모드 계속
					//DisplayPage(LCD_SLEEPMODE_PAGE);
					DisplayPage(beforepage);

					break;

				case 0x13 :
					//절전 해제 메세지
					if(SleepModeFlag==1){
						beforepage=currentpage;
						DisplayPage(LCD_SLEEPMODE_STOP_MESSAGE_PAGE);
					}
					break;

			}
			break;

		case 0x90 : //Monday Start Time
			if(ActiveModeTime[1][1]<value){
			}
			else{
				ActiveModeTime[0][1]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;
		case 0x94 : //Monday End Time
			if(ActiveModeTime[0][1]>value){
			}
			else{
				ActiveModeTime[1][1]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;

		case 0x98 : //Tuesday Start Time
			if(ActiveModeTime[1][2]<value){
			}
			else{
				ActiveModeTime[0][2]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;
		case 0x9C : //Tuesday End Time
			if(ActiveModeTime[0][2]>value){
			}
			else{
				ActiveModeTime[1][2]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;

		case 0xA0 : //Wednesday Start Time
			if(ActiveModeTime[1][3]<value){
			}
			else{
				ActiveModeTime[0][3]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;

		case 0xA4 : //Wednesday End Time
			if(ActiveModeTime[0][3]>value){
			}
			else{
				ActiveModeTime[1][3]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;

		case 0xA8 : //Thursday Start Time
			if(ActiveModeTime[1][4]<value){
			}
			else{
				ActiveModeTime[0][4]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;
		case 0xAC : //Thursday End Time
			if(ActiveModeTime[0][4]>value){
			}
			else{
				ActiveModeTime[1][4]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;



		case 0xB0 : //Friday Start Time
			if(ActiveModeTime[1][5]<value){
			}
			else{
				ActiveModeTime[0][5]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;
		case 0xB4 : //Friday End Time
			if(ActiveModeTime[0][5]>value){
			}
			else{
				ActiveModeTime[1][5]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;

		case 0xB8 : //Saturday Start Time
			if(ActiveModeTime[1][6]<value){
			}
			else{
				ActiveModeTime[0][6]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;
		case 0xBC : //Saturday End Time
			if(ActiveModeTime[0][6]>value){
			}
			else{
				ActiveModeTime[1][6]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;

		case 0xC0 : //Sunday Start Time
			if(ActiveModeTime[1][0]<value){
			}
			else{
				ActiveModeTime[0][0]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;
		case 0xC4 : //Sunday End Time
			if(ActiveModeTime[0][0]>value){
			}
			else{
				ActiveModeTime[1][0]=value;
			    beforepage=currentpage;
			    Write_Flash();
			    DisplayPage(beforepage);
			    Display25page();
			}
			break;
    }
}

void LCD_31(int index, int value){	//Total Test
    switch(index) {
		case 0x00 :
			if(SleepModeFlag==1){
				beforepage=currentpage;
				DisplayPage(LCD_SLEEPMODE_STOP_MESSAGE_PAGE);
				break;
			}
			switch(value) {
				case 0x01 :
					SelfTestModeStart(1);
					DisplayPage(LCD_USER_TOTALTEST_RUNNING_PAGE);
					break;
				case 0x02 :
					//정지 확인 메시지
					if(TestResult[0]==0){
						DisplayPage(LCD_USER_TOTALTEST_STOP_MESSAGE_PAGE);
					}
					break;
				case 0x03 :
					//정지
					SelfTestModeStop(1);
					DisplayPage(LCD_USER_TOTALTEST_RUNNING_PAGE);
					break;
				case 0x04 :
					//정지 취소
					DisplayPage(LCD_USER_TOTALTEST_RUNNING_PAGE);
					break;
				case 0x05 :
					//프린트
					PrintTotalTest();
					break;

			}
			break;
    }
    Display32page();
}

void LCD_32(int index, int value){	//Heater Test
    switch(index) {
		case 0x00 :
			if(SleepModeFlag==1){
				beforepage=currentpage;
				DisplayPage(LCD_SLEEPMODE_STOP_MESSAGE_PAGE);
				break;
			}
			switch(value) {
				case 0x01 :
					SelfTestModeStart(2);
					DisplayPage(LCD_USER_HEATINGTEST_RUNNING_PAGE);
					break;
				case 0x02 :
					if(TestResult[0]==0){
						SelfTestModeStop(2);
					}
					break;
				case 0x05 :
					PrintHeaterTest();
					break;
			}
			break;
    }
    Display32page();
}

void LCD_33(int index, int value){	//Valve Test
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					SelfTestModeStart(3);
					DisplayPage(LCD_USER_PARTTEST_RUNNING_PAGE);
					break;
				case 0x02 :
					if(TestResult[0]==0){
						SelfTestModeStop(3);
					}
					break;
				case 0x05 :
					PrintValveTest();
					break;
			}
			break;
    }
    Display33page();

}

void LCD_34(int index, int value){	//Vacuum Test
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					SelfTestModeStart(4);
					DisplayPage(LCD_USER_VACUUMTEST_RUNNING_PAGE);
					break;
				case 0x02 :
					if(TestResult[0]==0){
						DisplayPage(LCD_USER_VACUUMTEST_STOP_MESSAGE_PAGE);
					}
					break;
				case 0x03 :
					//정지
					SelfTestModeStop(4);
					DisplayPage(LCD_USER_VACUUMTEST_RUNNING_PAGE);
					break;
				case 0x04 :
					//정지 취소
					DisplayPage(LCD_USER_VACUUMTEST_RUNNING_PAGE);
					break;
				case 0x05 :
					//프린트
					PrintLeakTest();
					break;
			}
			break;
    }
    Display34page();
}


void LCD_41(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					CarbonFilter=CarbonFilterMax;
					break;
				case 0x02 :
					HEPAFilter=HEPAFilterMax;
					break;
				case 0x03 :
					PlasmaAssy=PlasmaAssyMax;
					break;
			}
			break;
		case 0x10 :
			CarbonFilterMax=value;
			if(CarbonFilterMax<CarbonFilter){
				CarbonFilter=CarbonFilterMax;
			}
			break;
		case 0x20 :
			HEPAFilterMax=value;
			if(HEPAFilterMax<HEPAFilter){
				HEPAFilter=HEPAFilterMax;
			}
			break;
		case 0x30 :
			PlasmaAssyMax=value;
			if(PlasmaAssyMax<PlasmaAssy){
				PlasmaAssy=PlasmaAssyMax;
			}
			break;

		case 0x40 :
			CarbonFilter=value;
			if(CarbonFilterMax<CarbonFilter){
				CarbonFilter=CarbonFilterMax;
			}
			break;
		case 0x50 :
			HEPAFilter=value;
			if(HEPAFilterMax<HEPAFilter){
				HEPAFilter=HEPAFilterMax;
			}
			break;
		case 0x60 :
			PlasmaAssy=value;
			if(PlasmaAssyMax<PlasmaAssy){
				PlasmaAssy=PlasmaAssyMax;
			}
			break;
    }
	Write_Flash();
	Display41page();
	DisplayPage(currentpage);
}

void LCD_43(int index, int value){	//input Value
	switch(index) {
		case 0x00 :
			switch(value) {
				case 0x0F :
					if(AlarmCheckFlag[0]==1){
						AlarmCheckFlag[0]=2;
					}
					else{
						AlarmCheckFlag[0]=1;
					}
					break;
				case 0x01 :
					if(AlarmCheckFlag[1]==1){
						AlarmCheckFlag[1]=2;
					}
					else{
						AlarmCheckFlag[1]=1;
					}
					break;
				case 0x02 :
					if(AlarmCheckFlag[2]==1){
						AlarmCheckFlag[2]=2;
					}
					else{
						AlarmCheckFlag[2]=1;
					}
					break;
				case 0x03 :
					if(AlarmCheckFlag[3]==1){
						AlarmCheckFlag[3]=2;
					}
					else{
						AlarmCheckFlag[3]=1;
					}
					break;
				case 0x04 :
					if(AlarmCheckFlag[4]==1){
						AlarmCheckFlag[4]=2;
					}
					else{
						AlarmCheckFlag[4]=1;
					}
					break;
				case 0x05 :
					if(AlarmCheckFlag[5]==1){
						AlarmCheckFlag[5]=2;
					}
					else{
						AlarmCheckFlag[5]=1;
					}
					break;
				case 0x06 :
					if(AlarmCheckFlag[6]==1){
						AlarmCheckFlag[6]=2;
					}
					else{
						AlarmCheckFlag[6]=1;
					}
					break;
				case 0x07 :
					if(AlarmCheckFlag[7]==1){
						AlarmCheckFlag[7]=2;
					}
					else{
						AlarmCheckFlag[7]=1;
					}
					break;
				case 0x08 :
					if(AlarmCheckFlag[8]==1){
						AlarmCheckFlag[8]=2;
					}
					else{
						AlarmCheckFlag[8]=1;
					}
					break;
				case 0x09 :
					if(AlarmCheckFlag[9]==1){
						AlarmCheckFlag[9]=2;
					}
					else{
						AlarmCheckFlag[9]=1;
					}
					break;
				case 0x0A :
					if(AlarmCheckFlag[10]==1){
						AlarmCheckFlag[10]=2;
					}
					else{
						AlarmCheckFlag[10]=1;
					}
					break;
				case 0x0B :
					if(AlarmCheckFlag[11]==1){
						AlarmCheckFlag[11]=2;
					}
					else{
						AlarmCheckFlag[11]=1;
					}
					break;
				case 0x0C :
					if(AlarmCheckFlag[12]==1){
						AlarmCheckFlag[12]=2;
					}
					else{
						AlarmCheckFlag[12]=1;
					}
					break;
				case 0x0D :
					if(AlarmCheckFlag[13]==1){
						AlarmCheckFlag[13]=2;
					}
					else{
						AlarmCheckFlag[13]=1;
					}
					break;

				case 0x10 :
					DisplayPage(LCD_ADMIN_ALARM2_PAGE);
					break;

				case 0x11 :
					DisplayPage(LCD_ADMIN_ALARM1_PAGE);
					break;
			}
			break;
	}
    Write_Flash();
	Display43page();
	DisplayPage(currentpage);
}

void LCD_44(int index, int value){	//input Value
	switch(index) {
		case 0x00 :
			switch(value) {
				case 0x0F :
					if(ErrorCheckFlag[0]==1){
						ErrorCheckFlag[0]=2;
					}
					else{
						ErrorCheckFlag[0]=1;
					}
					break;

				case 0x01 :
					if(ErrorCheckFlag[1]==1){
						ErrorCheckFlag[1]=2;
					}
					else{
						ErrorCheckFlag[1]=1;
					}
					break;
				case 0x02 :
					if(ErrorCheckFlag[2]==1){
						ErrorCheckFlag[2]=2;
					}
					else{
						ErrorCheckFlag[2]=1;
					}
					break;
				case 0x03 :
					if(ErrorCheckFlag[3]==1){
						ErrorCheckFlag[3]=2;
					}
					else{
						ErrorCheckFlag[3]=1;
					}
					break;
				case 0x04 :
					if(ErrorCheckFlag[4]==1){
						ErrorCheckFlag[4]=2;
					}
					else{
						ErrorCheckFlag[4]=1;
					}
					break;
				case 0x05 :
					if(ErrorCheckFlag[5]==1){
						ErrorCheckFlag[5]=2;
					}
					else{
						ErrorCheckFlag[5]=1;
					}
					break;
				case 0x06 :
					if(ErrorCheckFlag[6]==1){
						ErrorCheckFlag[6]=2;
					}
					else{
						ErrorCheckFlag[6]=1;
					}
					break;
				case 0x07 :
					if(ErrorCheckFlag[7]==1){
						ErrorCheckFlag[7]=2;
					}
					else{
						ErrorCheckFlag[7]=1;
					}
					break;
				case 0x08 :
					if(ErrorCheckFlag[8]==1){
						ErrorCheckFlag[8]=2;
					}
					else{
						ErrorCheckFlag[8]=1;
					}
					break;
				case 0x09 :
					if(ErrorCheckFlag[9]==1){
						ErrorCheckFlag[9]=2;
					}
					else{
						ErrorCheckFlag[9]=1;
					}
					break;
				case 0x0A :
					if(ErrorCheckFlag[10]==1){
						ErrorCheckFlag[10]=2;
					}
					else{
						ErrorCheckFlag[10]=1;
					}
					break;
				case 0x0B :
					if(ErrorCheckFlag[11]==1){
						ErrorCheckFlag[11]=2;
					}
					else{
						ErrorCheckFlag[11]=1;
					}
					break;
				case 0x0C :
					if(ErrorCheckFlag[12]==1){
						ErrorCheckFlag[12]=2;
					}
					else{
						ErrorCheckFlag[12]=1;
					}
					break;
				case 0x10 :
					DisplayPage(LCD_ADMIN_ERROR2_PAGE);
					break;
				case 0x11 :
					DisplayPage(LCD_ADMIN_ERROR1_PAGE);
					break;

			}
			break;
	}
    Write_Flash();
	Display44page();
	DisplayPage(currentpage);
}

void LCD_51(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					ReadInforDataFromLCD();
					break;
			}
			break;
		case 0x01 : // TestValue
			TestVacuumValue=value;
			break;
		case 0x05 : // 리크 률
			TestLeakValue=value;
			break;
		case 0x09 : // 에러 온도 률
			TestTempErrorValue=value;
			break;
		case 0x10 : //
			expiry_date1=value;
			break;
		case 0x14 : //
			expiry_date2=value;
			break;
		case 0x18 : //
			DoorOpenPressure=value;
			break;

    }
    beforepage=currentpage;
	Write_Flash();
	DisplayPage(beforepage);
    Display51page();
}

void LCD_52(int index, int value){	//input Value
	switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					if(LoginFlag==1){
						LoginFlag=2;
						DisplayIcon(0x02, 0x30, 0);
						DisplayIcon(0x52,0x10,0);
					}
					else{
						LoginFlag=1;
						DisplayIcon(0x02, 0x30, 1);
						DisplayIcon(0x52,0x10,1);
					}
					break;

				case 0x04 :
					//과수데이터 초기화
					/*
					RFIDData.production_year=0;
					RFIDData.production_month=0;
					RFIDData.production_day=0;
					RFIDData.production_number=0;

					RFIDData.open_year=0;
					RFIDData.open_month=0;
					RFIDData.open_day=0;

					RFIDData.expiry_year=0;
					RFIDData.expiry_month=0;
					RFIDData.expiry_day=0;

					RFIDData.volume=0;
					RFIDData.volumemax=0;
					*/
					break;

				case 0x05 :
					memset(flash_ID,0,sizeof(flash_ID));
					memset(flash_PW,0,sizeof(flash_PW));
					sprintf(flash_ID[0],"USER1");
					sprintf(flash_PW[0],"1234");
					flashuserCount=1;
					break;

				case 0x06 :
					sprintf(flash_MODEL_NAME,"FN-P20    ");
					sprintf(flash_SERIAL_NUMBER,"CBTP250701");
					sprintf(flash_FACILITY_NAME,"CBT");
					sprintf(flash_DEPARTMENT_NAME,"CleanTeam");
					sprintf(flash_SOFTWARE_VERSION,"1.0.0     ");
					LoadSetting();
					break;

				case 0x07 :
					//여기 수정중
					totalCount=0;
                    dailyCount=0;
					break;
			}
			break;
	}
    Write_Flash();
	Display52page();
	DisplayPage(LCD_FACTORY_FUNCSETTING_PAGE);
}

void LCD_53(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			break;
		case 0x21 :
			CalibrationTemp[0]=value;
			break;
		case 0x25 :
			CalibrationTemp[1]=value;
			break;
		case 0x29 :
			CalibrationTemp[2]=value;
			break;
		case 0x2D :
			CalibrationTemp[3]=value;
			break;
		case 0x35 :
			CalibrationVacuum=value;
			break;

    }
    Write_Flash();
    Display53page();
    DisplayPage(LCD_FACTORY_CALIBRATION_PAGE);
}

void LCD_55(int index, int value){	//input Value
    switch(index) {
    	case 0x01 : // Set Chamber Temp	//대기
    		FlashSettingTemp[0][0]=(float)value/10;
            break;
    	case 0x05 : // Set Chamber Temp	//공정
    		FlashSettingTemp[0][1]=(float)value/10;
            break;
    	case 0x09 : // Set Chamber Temp	//슬립
    		FlashSettingTemp[0][2]=(float)value/10;
            break;


    	case 0x11 : // Set Chamber Temp
    		FlashSettingTemp[1][0]=(float)value/10;
            break;
    	case 0x15 : // Set Chamber Temp
    		FlashSettingTemp[1][1]=(float)value/10;
            break;
    	case 0x19 : // Set Chamber Temp
    		FlashSettingTemp[1][2]=(float)value/10;
            break;


    	case 0x21 : // Set Chamber Temp
    		FlashSettingTemp[2][0]=(float)value/10;
			break;
    	case 0x25 : // Set Chamber Temp
    		FlashSettingTemp[2][1]=(float)value/10;
			break;
    	case 0x29 : // Set Chamber Temp
    		FlashSettingTemp[2][2]=(float)value/10;
			break;

    	case 0x31 : // Set Chamber Temp
    		FlashSettingTemp[3][0]=(float)value/10;
			break;
    	case 0x35 : // Set Chamber Temp
    		FlashSettingTemp[3][1]=(float)value/10;
			break;
    	case 0x39 : // Set Chamber Temp
    		FlashSettingTemp[3][2]=(float)value/10;
			break;

    	case 0x41 : // Pressure condition1
    		FlashPreesureCondition[0]=(float)value;
			break;
    	case 0x45 : // Pressure condition2
    		FlashPreesureCondition[1]=(float)value;
			break;
    	case 0x49 : // Pressure condition3
    		FlashPreesureCondition[2]=(float)value;
			break;

    	case 0x51 : // Pressure condition1
    		Flashperispeed=value;
			break;
    }
}

void LCD_56(int index, int value){	//input Value
	switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01:
					ProcessNum=1;
					DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
					StepNum=0;
					break;
				case 0x02:
					ProcessNum=2;
					DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
					StepNum=0;
					break;
				case 0x03:
					ProcessNum=3;
					DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);

					StepNum=0;
					break;
				case 0x04:
					ProcessNum=4;
					DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);

					StepNum=0;
					break;
				case 0x05:
					ProcessNum=5;
					DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);

					StepNum=0;
					break;
				case 0x06:
					ProcessNum=6;
					DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
					StepNum=0;
					break;


				case 0x11:
					StepNum=1;
					break;
				case 0x12:
					StepNum=2;
					break;
				case 0x13:
					StepNum=3;
					break;
				case 0x14:
					StepNum=4;
					break;
				case 0x15:
					StepNum=5;
					break;
				case 0x16:
					StepNum=6;
					break;
				case 0x17:
					StepNum=7;
					break;
				case 0x18:
					StepNum=8;
					break;
				case 0x19:
					StepNum=9;
					break;
				case 0x1A:
					StepNum=10;
					break;
				case 0x21:
					StepNum=11;
					break;
				case 0x22:
					StepNum=12;
					break;
				case 0x23:
					StepNum=13;
					break;
				case 0x24:
					StepNum=14;
					break;
				case 0x25:
					StepNum=15;
					break;
				case 0x26:
					StepNum=16;
					break;
				case 0x27:
					StepNum=17;
					break;
				case 0x28:
					StepNum=18;
					break;
				case 0x29:
					StepNum=19;
					break;
				case 0x2A:
					StepNum=20;
					break;


				case 0x41:
					if((CycleData[ProcessNum][StepNum].PartsSetting&0x01)==0x01){
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-0x01;
					}
					else{
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting+0x01;
					}
					break;
				case 0x42:
					if((CycleData[ProcessNum][StepNum].PartsSetting&0x02)==0x02){
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-0x02;
					}
					else{
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting+0x02;
					}
					break;
				case 0x43:
					if((CycleData[ProcessNum][StepNum].PartsSetting&0x04)==0x04){
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-0x04;
					}
					else{
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting+0x04;
					}
					break;
				case 0x44:
					if((CycleData[ProcessNum][StepNum].PartsSetting&0x08)==0x08){
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-0x08;
					}
					else{
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting+0x08;
					}
					break;
				case 0x45:
					if((CycleData[ProcessNum][StepNum].PartsSetting&0x10)==0x10){
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-0x10;
					}
					else{
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting+0x10;
					}
					break;
				case 0x46:	//0x80
					if((CycleData[ProcessNum][StepNum].PartsSetting&0x20)==0x20){
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-0x20;
					}
					else{
						CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting+0x20;
					}
					break;

				case 0x47:	//NO check	0x00
					CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-(CycleData[ProcessNum][StepNum].PartsSetting&0xC0);
					break;
				case 0x48:	//0x20
					CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-(CycleData[ProcessNum][StepNum].PartsSetting&0xC0)+0x40;
					break;
				case 0x49:	//0x40
					CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-(CycleData[ProcessNum][StepNum].PartsSetting&0xC0)+0x80;
					break;
				case 0x4A:	//0x60
					CycleData[ProcessNum][StepNum].PartsSetting=CycleData[ProcessNum][StepNum].PartsSetting-(CycleData[ProcessNum][StepNum].PartsSetting&0xC0)+0xC0;
					break;

				case 0x50 : // Cycle 선택
					beforepage=currentpage;
					Display58page();
					break;

				case 0x51:
					DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS2_PAGE);
					StepNum=11;
					Display80page();
					break;

				case 0x52:
		        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
		        	StepNum=1;
					Display55page();
					break;

		        case 0x55:
		        	//센서 세팅 값에서 저장된 값 저장시
		        	if(FlashSettingTemp[0][0]!=0){
						for(int i=0;i<3;i++){
							DoorSettingTemp[i]=FlashSettingTemp[0][i];
							ChamberSettingTemp[i]=FlashSettingTemp[1][i];
							ChamberBackSettingTemp[i]=FlashSettingTemp[2][i];
							VaporizerSettingTemp[i]=FlashSettingTemp[3][i];
						}
		        	}
		        	if(FlashPreesureCondition[0]!=0){
		        		PreesureCondition[0]=FlashPreesureCondition[0];
		        		PreesureCondition[1]=FlashPreesureCondition[1];
		        		PreesureCondition[2]=FlashPreesureCondition[2];
		        	}
		        	if(Flashperispeed!=0){
		        		perispeed=Flashperispeed;
					}
		        	Write_Flash();
		        	DisplayProcessSettingValues();
		        	DisplayPage(currentpage);
					break;
			}
			break;
		case 0x51 : // Time 시간 저장
			if(StepNum==0){
				CycleData[ProcessNum][StepNum].Time=0;
			}
			else{
				CycleData[ProcessNum][StepNum].Time=(float)value;
			}
			break;

	}

	Display56page();
}

void LCD_57(int index, int value){	//input Value
	switch(index) {
		case 0x00 :
			if(SleepModeFlag==1){
				beforepage=currentpage;
				DisplayPage(LCD_SLEEPMODE_STOP_MESSAGE_PAGE);
				break;
			}
			switch(value) {
				case 0x50 : // Cycle 선택
					beforepage=currentpage;
					Display58page();
					break;

		        case 0x60:	//TEST START
		        	ReadProcessTime();
		        	ReadCycleTime();
		        	ReadStepTime();
		        	FactoryTestStart();
		        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST3_PAGE);
		        	DisplayProcessTestValues();
					break;

				case 0x61:
		        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST2_PAGE);
		        	StepNum=11;
					Display81page();
		        	break;

				case 0x62:
		        	DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE);
		        	StepNum=1;
					Display56page();
		        	break;

		        case 0x70:	//TEST STOP
		        	if(StopFlag==0){
		        		ReadProcessTime();
		        		ReadCycleTime();
		        		ReadStepTime();
		        		FactoryTestStop();
		        		DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST3_PAGE);
		        		DisplayProcessTestValues();
		        	}
					break;
			}
			break;
		case 0x01 :
			break;

	}
	Display57page();
}



void LCD_58(int index, int value){	//input Value
	switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 : // Cycle 선택
					CycleName=SHORT;
					Read_Flash();
					ProcessNum=1;
					StepNum=1;
					CurrentProcess=1;
					CurrentStep=1;
		        	ReadProcessTime();
		        	ReadCycleTime();
		        	ReadStepTime();
		        	DisplayPage(beforepage);
					Display56page();
					break;
				case 0x02 : // Cycle 선택
					CycleName=STANDARD;
					Read_Flash();
					ProcessNum=1;
					StepNum=1;
					CurrentProcess=1;
					CurrentStep=1;
		        	ReadProcessTime();
		        	ReadCycleTime();
		        	ReadStepTime();
					DisplayPage(beforepage);
					Display56page();
					break;
				case 0x03 : // Cycle 선택
					CycleName=ADVANCED;
					Read_Flash();
					ProcessNum=1;
					StepNum=1;
					CurrentProcess=1;
					CurrentStep=1;
		        	ReadProcessTime();
		        	ReadCycleTime();
		        	ReadStepTime();
					DisplayPage(beforepage);
					Display56page();
					break;
			}
			break;
		case 0x01 : // Cycle 선택
			break;

	}
}


void LCD_60(int index, int value){	//input Value
	switch(index) {
		case 0x00 :
			switch(value) {
				//AC Test
				case 0x01 ://Doorheater
					if(!HAL_GPIO_ReadPin(GPIO_OUT11_GPIO_Port, GPIO_OUT11_Pin)){
						AC1(0);
		        		//DisplayIcon(0x6A,0x10,0);
					}
					else{
						AC1(1);
		        		//DisplayIcon(0x6A,0x10,1);
					}
					//SaveSettingData(&myProcessData);
					SaveCycle();
					break;
				case 0x02 ://not used
					if(!HAL_GPIO_ReadPin(GPIO_OUT12_GPIO_Port, GPIO_OUT12_Pin)){
						AC2(0);
		        		//DisplayIcon(0x6A,0x20,0);
					}
					else{
						AC2(1);
		        		//DisplayIcon(0x6A,0x20,1);
					}
					//LoadSettingData(&myProcessData);
					break;
				case 0x03 :
					if(!HAL_GPIO_ReadPin(GPIO_OUT13_GPIO_Port, GPIO_OUT13_Pin)){
						AC3(0);
		        		//DisplayIcon(0x6A,0x30,0);
					}
					else{
						AC3(1);
		        		//DisplayIcon(0x6A,0x30,1);
					}
					break;
				case 0x04 :
					if(!HAL_GPIO_ReadPin(GPIO_OUT14_GPIO_Port, GPIO_OUT14_Pin)){
						VacuumPump(0);
						if(currentpage==LCD_ADMIN_PARTSTEST_PAGE){
							Fan(0);
						}
		        		//DisplayIcon(0x6A,0x40,0);
					}
					else{
						VacuumPump(1);
						if(currentpage==LCD_ADMIN_PARTSTEST_PAGE){
							Fan(1);
						}
		        		//DisplayIcon(0x6A,0x40,1);

					}
					break;
				case 0x05 :
					if(!HAL_GPIO_ReadPin(GPIO_OUT15_GPIO_Port, GPIO_OUT15_Pin)){
						AC5(0);
		        		//DisplayIcon(0x6A,0x50,0);
					}
					else{
						AC5(1);
		        		//DisplayIcon(0x6A,0x50,1);
					}
					break;
				case 0x06 :
					if(!HAL_GPIO_ReadPin(GPIO_OUT16_GPIO_Port, GPIO_OUT16_Pin)){
						//AC6(0);
						AC6(0);
		        		//DisplayIcon(0x6A,0x60,0);
					}
					else{
						AC6(1);
		        		//DisplayIcon(0x6A,0x60,1);

					}
					break;
				case 0x07 :
					if(!HAL_GPIO_ReadPin(GPIO_OUT17_GPIO_Port, GPIO_OUT17_Pin)){
						AC7(0);
		        		//DisplayIcon(0x6A,0x70,0);
					}
					else{
						AC7(1);
		        		//DisplayIcon(0x6A,0x70,1);
					}
					break;
				case 0x08 :
					if(!HAL_GPIO_ReadPin(GPIO_OUT18_GPIO_Port, GPIO_OUT18_Pin)){
						AC8(0);
		        		//DisplayIcon(0x6A,0x80,0);
					}
					else{
						AC8(1);
		        		//DisplayIcon(0x6A,0x80,1);
					}
					break;

				//DC Test
				case 0x09 ://Vacuum Valve
					if(HAL_GPIO_ReadPin(GPIO_OUT1_GPIO_Port, GPIO_OUT1_Pin)){
						DC1(0);
		        		//DisplayIcon(0x6B,0x10,0);
					}
					else{
						DC1(1);
		        		//DisplayIcon(0x6B,0x10,1);
					}
					break;
				case 0x0A :
					if(HAL_GPIO_ReadPin(GPIO_OUT2_GPIO_Port, GPIO_OUT2_Pin)){
						DC2(0);
		        		//DisplayIcon(0x6B,0x20,0);
					}
					else{
						DC2(1);
		        		//DisplayIcon(0x6B,0x20,1);
					}
					break;
				case 0x0B :
					if(HAL_GPIO_ReadPin(GPIO_OUT3_GPIO_Port, GPIO_OUT3_Pin)){
						DC3(0);
		        		//DisplayIcon(0x6B,0x30,0);
					}
					else{
						DC3(1);
		        		//DisplayIcon(0x6B,0x30,1);
					}
					break;
				case 0x0C ://Air Inje Valve
					if(HAL_GPIO_ReadPin(GPIO_OUT4_GPIO_Port, GPIO_OUT4_Pin)){
						AirInjeValve(0);
		        		//DisplayIcon(0x6B,0x40,0);
					}
					else{
						AirInjeValve(1);
		        		//DisplayIcon(0x6B,0x40,1);
					}
					break;
				case 0x0D ://Injectnion Valve
					if(HAL_GPIO_ReadPin(GPIO_OUT5_GPIO_Port, GPIO_OUT5_Pin)){
						DC5(0);
		        		//DisplayIcon(0x6B,0x50,0);
					}
					else{
						DC5(1);
		        		//DisplayIcon(0x6B,0x50,1);
					}
					break;
				case 0x0E :
					if(HAL_GPIO_ReadPin(GPIO_OUT6_GPIO_Port, GPIO_OUT6_Pin)){
						DC6(0);
		        		//DisplayIcon(0x6B,0x60,0);
					}
					else{
						DC6(1);
		        		//DisplayIcon(0x6B,0x60,1);
					}
					break;
				case 0x0F :
					if(HAL_GPIO_ReadPin(GPIO_OUT7_GPIO_Port, GPIO_OUT7_Pin)){
						DC7(0);
		        		//DisplayIcon(0x6B,0x70,0);
					}
					else{
						DC7(1);
		        		//DisplayIcon(0x6B,0x70,1);
					}
					break;
				case 0x10 :
					if(HAL_GPIO_ReadPin(GPIO_OUT8_GPIO_Port, GPIO_OUT8_Pin)){
						DC8(0);
					}
					else{
						DC8(1);
					}
					break;
				case 0x11 :
					if(HAL_GPIO_ReadPin(GPIO_OUT26_GPIO_Port, GPIO_OUT26_Pin)){
						PeriPump(0);
					}
					else{
						PeriPump(1);
					}
					break;
				case 0x12 :
					beforepage=currentpage;
					DisplayPage(LCD_LOADING_PAGE);
					RFIDCheck();
					DisplayPage(beforepage);
		        	break;
			}
			break;
		case 0x01 :
			break;
	}
	//Display60page();
}

void LCD_61(int index, int value){	//
	switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 : // Sign in
					if(loginProcess()){
						AutoLoginID=CurrentUser;
						Write_Flash();
						DisplayPage(LCD_CYCLESELECT_PAGE);
						Display02page();
					}
					else{
						beforepage=currentpage;
						Display61page();
						DisplayPage(LCD_MANAGEMENT_WRONG_PW_POPUP);
					}
					//DisplayPage();
					break;
		        case 0x02:	//Create Account
		        	DisplayPage(LCD_CREATE_ID);
		        	Display63page();
					break;

		        case 0x03:	//자동 로그인 체크
		        	if(AutoLoginFlag==2){
		        		AutoLoginFlag=1;
		        		DisplayIcon(0x61, 0x30, AutoLoginFlag);
		        	}
		        	else{
		        		AutoLoginFlag=2;
		        		DisplayIcon(0x61, 0x30, AutoLoginFlag);
		        	}
					break;

		        case 0x45:	//Return to login page
	        		DisplayPage(LCD_LOGIN_PAGE);
					Display63page();
					break;
			}
			break;
		case 0x01 :
			break;
	}
}

void LCD_63(int index, int value){
	switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 : // management
	        		DisplayPage(LCD_MANAGEMENT_ID);
	        		Display64page();
					break;
		        case 0x02:	// sign up(생성)
		        	switch(createUser()) {
						case 0 :
							beforepage=currentpage;
			        		DisplayPage(LCD_MANAGEMENT_WRONG_PW_POPUP);
			        		Display63page();
							break;
						case 1 :
			        		Write_Flash();
			        		Display61page();
			        		DisplayPage(LCD_LOGIN_PAGE);
							break;
						case 2 :
							beforepage=currentpage;
			        		DisplayPage(LCD_MANAGEMENT_MAXOVER_PW_POPUP);
			        		Display63page();
							break;
						case 3 :
							beforepage=currentpage;
			        		DisplayPage(LCD_MANAGEMENT_WRONG_PW_POPUP);
			        		Display63page();
							break;
		        	}
					break;
		        case 0x03:	// cancel
		        	DisplayPage(LCD_LOGIN_PAGE);
		        	Display61page();
					break;

		        case 0x46:	//Return to LCD_CREATE_ID page
		        	DisplayPage(LCD_CREATE_ID);
		        	Display61page();
					break;

		        case 0x47:	//Return to LCD_CREATE_ID page
		        	DisplayPage(LCD_CREATE_ID);
		        	Display61page();
					break;
			}
			break;
		case 0x01 :
			break;
	}
}

void LCD_64(int index, int value){
	switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 : // sign up(생성)
		        	DisplayPage(LCD_CREATE_ID);
		        	Display63page();
					break;

		        case 0x02:	// Change ID1
		        	if(flash_ID[0][0]!=0){
						Select_ID=0;
						Display64page();
						DisplayPage10Char(0x64,0x70,"");
						DisplayPage(LCD_MANAGEMENT_CHANGE_PW_POPUP1);
		        	}
					break;
		        case 0x03:	// Change ID2
		        	if(flash_ID[1][0]!=0){
						Select_ID=1;
						Display64page();
						DisplayPage10Char(0x64,0x70,"");
						DisplayPage(LCD_MANAGEMENT_CHANGE_PW_POPUP1);
		        	}
					break;
		        case 0x04:	// Change ID3
		        	if(flash_ID[2][0]!=0){
						Select_ID=2;
						Display64page();
						DisplayPage10Char(0x64,0x70,"");
						DisplayPage(LCD_MANAGEMENT_CHANGE_PW_POPUP1);
		        	}
					break;
		        case 0x05:	// Change ID4
		        	if(flash_ID[3][0]!=0){
						Select_ID=3;
						Display64page();
						DisplayPage10Char(0x64,0x70,"");
						DisplayPage(LCD_MANAGEMENT_CHANGE_PW_POPUP1);
		        	}
					break;

		        case 0x07:	// DELET ID1
		        	if(flash_ID[0][0]!=0){
						Select_ID=0;
						Display64page();
						DisplayPage10Char(0x64,0x70,"");
						DisplayPage(LCD_MANAGEMENT_DELET_ID_POPUP);
		        	}
					break;
		        case 0x08:	// DELET ID2
		        	if(flash_ID[1][0]!=0){
						Select_ID=1;
						Display64page();
						DisplayPage10Char(0x64,0x70,"");
						DisplayPage(LCD_MANAGEMENT_DELET_ID_POPUP);
		        	}
					break;
		        case 0x09:	// DELET ID3
		        	if(flash_ID[2][0]!=0){
						Select_ID=2;
						Display64page();
						DisplayPage10Char(0x64,0x70,"");
						DisplayPage(LCD_MANAGEMENT_DELET_ID_POPUP);
		        	}
					break;
		        case 0x0A:	// DELET ID4
		        	if(flash_ID[3][0]!=0){
						Select_ID=3;
						Display64page();
						DisplayPage10Char(0x64,0x70,"");
						DisplayPage(LCD_MANAGEMENT_DELET_ID_POPUP);
		        	}
					break;

		        case 0x0C:	// BACK
		        	DisplayPage(LCD_LOGIN_PAGE);
		        	Display61page();
					break;

		        case 0x10:	// ID,PASSWORD CHECK
		        	if(changePWloginUser()){
		        		DisplayPage(LCD_MANAGEMENT_CHANGE_PW_POPUP2);
		        		DisplayPage10Char(0x64,0x70,"");
		        		DisplayPage10Char(0x64,0x80,"");
		        	}
		        	else{
		        		DisplayPage(LCD_MANAGEMENT_WRONG_PW_POPUP);
		        	}
					break;

		        case 0x12:	// Change PW
		        	if(changePWUser()){
		        		Write_Flash();
		        		Display64page();
		        		DisplayPage(LCD_MANAGEMENT_ID);
		        	}
		        	else{
		        		DisplayPage(LCD_MANAGEMENT_WRONG_PW_POPUP);
		        	}
					break;

		        case 0x13:	// CANCEL
		        	DisplayPage(beforepage);
					break;

		        case 0x14:	// DELET ID
		        	if(DeletConfirmloginUser()){
		        		Write_Flash();
			        	DisplayPage(LCD_MANAGEMENT_ID);
			            Display64page();
		        		//ID 카운트 및 정렬문제 추가 확인 필요
		        	}
		        	else{
		        		DisplayPage(LCD_MANAGEMENT_WRONG_PW_POPUP);
		        	}
					break;
			}
			break;
		case 0x01 :
			break;
	}
}




//ICON Display
unsigned char   icon_display[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x01};

unsigned char   icon_process_index[7] = {0x00,0x10,0x20,0x30,0x40,0x50,0x60};
unsigned char   icon_step_index[11] = {0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0xA0};
unsigned char   icon_parts_index[11] = {0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0xA0};
unsigned char   icon_select_index[6] = {0x00,0x10,0x20,0x30,0x40,0x50};
/*
 * 1.Vacuum Valve
 * 2.Vent Valve
 * 3.Injection Valve
 * 4.Peri Pump
 * 5.Plasma
 *
 * Vacuum Check
 * 6.No check
 * 7.Value1
 * 8.Value2
 * 9.Value3
 * */

/*
* 1.Vacuum Pump
* 2.Vacuum Valve
* 3.Vent Valve
* 4.Injection Valve

5. Door handle
6. Door Latch
7. Bottle
8. Bottle Door
9. Liquid Level
*/

void DisplayProcessIcon(int index, int value){
	icon_display[4] = 0x6F;
	icon_display[5] = icon_process_index[index];
    icon_display[7] = value;
    HAL_UART_Transmit(LCD_USART, icon_display, 8, 10);
}
void DisplayStepIcon(int index, int value){
	icon_display[4] = 0x6E;
	icon_display[5] = icon_step_index[index];
	icon_display[7] = value;
	HAL_UART_Transmit(LCD_USART, icon_display, 8, 10);
}
void DisplayPartsIcon(int index, int value){
	icon_display[4] = 0x6D;
	icon_display[5] = icon_parts_index[index];
	icon_display[7] = value;
	HAL_UART_Transmit(LCD_USART, icon_display, 8, 10);
}
void DisplaySelectIcon(int index, int value){
	icon_display[4] = 0x21;
	icon_display[5] = icon_select_index[index];
	icon_display[7] = value;
	HAL_UART_Transmit(LCD_USART, icon_display, 8, 10);
}
void DisplayIcon(int page, int index, int value){
	unsigned char   icondisplay[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x01};
	icondisplay[4] = page;
	icondisplay[5] = index;
	icondisplay[7] = value;
	HAL_UART_Transmit(LCD_USART, icondisplay, 8, 10);
}

void DisplayProcessIcons(int index){
	for(int i=1;i<7;i++){
		DisplayProcessIcon(i,0);
	}
	DisplayProcessIcon(index,1);
}
void DisplayStepIcons(int index){
	for(int i=1;i<11;i++){
		DisplayStepIcon(i,0);
	}
	if(index>10){
		DisplayStepIcon(index-10,1);
	}
	else{
		DisplayStepIcon(index,1);
	}
}
void DisplayPartsIcons(){
	DisplayIcon(0x6D,0x10,(CycleData[ProcessNum][StepNum].PartsSetting&0x01)==0x01);
	DisplayIcon(0x6D,0x20,(CycleData[ProcessNum][StepNum].PartsSetting&0x02)==0x02);
	DisplayIcon(0x6D,0x30,(CycleData[ProcessNum][StepNum].PartsSetting&0x04)==0x04);
	DisplayIcon(0x6D,0x40,(CycleData[ProcessNum][StepNum].PartsSetting&0x08)==0x08);
	DisplayIcon(0x6D,0x50,(CycleData[ProcessNum][StepNum].PartsSetting&0x10)==0x10);
	DisplayIcon(0x6D,0x60,(CycleData[ProcessNum][StepNum].PartsSetting&0x20)==0x20);


	if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0xC0){//3
		DisplayIcon(0x6D,0x70,0);
		DisplayIcon(0x6D,0x80,0);
		DisplayIcon(0x6D,0x90,0);
		DisplayIcon(0x6D,0xA0,1);
	}
	else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x80){//2
		DisplayIcon(0x6D,0x70,0);
		DisplayIcon(0x6D,0x80,0);
		DisplayIcon(0x6D,0x90,1);
		DisplayIcon(0x6D,0xA0,0);
	}
	else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x40){//1
		DisplayIcon(0x6D,0x70,0);
		DisplayIcon(0x6D,0x80,1);
		DisplayIcon(0x6D,0x90,0);
		DisplayIcon(0x6D,0xA0,0);
	}
	else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x00){//0
		DisplayIcon(0x6D,0x70,1);
		DisplayIcon(0x6D,0x80,0);
		DisplayIcon(0x6D,0x90,0);
		DisplayIcon(0x6D,0xA0,0);
	}

}


void DisplayProcessSettingValues(){
	DisplayCycleValue(CycleName);
	DisplayProcessIcons(ProcessNum);
	DisplayStepIcons(StepNum);
	DisplayPartsIcons();
	DisplayTimeValues();
}

void DisplayProcessTestValues(){
	DisplayCycleValue(CycleName);
	DisplayProcessIcons(CurrentProcess);
	DisplayStepIcons(CurrentStep);
	DisplayPartsIcons();
	DisplayTimeValues();
	DisplayIcon(0x6C,0x60,LevelSensor2Check());
}
void DisplayNormalValues(){
	DisplayTime2(0x11,0x10,CycleTime);
	DisplayIcon(0x05,0x10,(CycleTime/10)/60/10);
	DisplayIcon(0x05,0x20,(CycleTime/10)/60%10);
	DisplayIcon(0x05,0x30,(CycleTime/10)%60/10);
	DisplayIcon(0x05,0x40,(CycleTime/10)%60%10);

	int statusPercent=TotalTime*100/FullCycleTime;
	int icon1Value, icon2Value;
	if (statusPercent == 0) {
		icon1Value = 0;
		icon2Value = 0;
	} else if (statusPercent < 50) {
		icon1Value = statusPercent / 5;
		icon2Value = 0;
	} else if (statusPercent < 100) {
		icon1Value = 10;
		icon2Value = (statusPercent - 50) / 5;
	} else {
		icon1Value = 10;
		icon2Value = 10;
	}
	DisplayIcon(0x05,0x50,icon1Value);
	DisplayIcon(0x05,0x60,icon2Value);

	DisplayPageValue(0x05,0x70,statusPercent);


	if(CurrentProcess==1){
		DisplayIcon(0x11,0x50,0);
	}
	else if(CurrentProcess==2){
		DisplayIcon(0x11,0x50,1);
	}
	else if(CurrentProcess==3){
		DisplayIcon(0x11,0x50,2);
	}
	else if(CurrentProcess==4){
		DisplayIcon(0x11,0x50,3);
	}
	else if(CurrentProcess==5){
		DisplayIcon(0x11,0x50,4);
	}
	else if(CurrentProcess==6){
		DisplayIcon(0x11,0x50,5);
	}
	else{
		DisplayIcon(0x11,0x50,0);
	}

	if(Running_Flag){
		DisplayTime2(0x11,0x40,ProcessTime[CurrentProcess]);
	}
	else{
		DisplayTime2(0x11,0x40,0);
	}

}




//Time display

void DisplayTime(int page, int index, unsigned int icentisecond){
	unsigned char   time_display[9] = {0x5A, 0xA5, 0x06, 0x82, 0x02, 0x00, 0x00, 0x00, 0x00};
	time_display[4] = page;
    time_display[5] = index;
    unsigned int iMinute = icentisecond / 600;
    unsigned int iHour = iMinute / 60;
    iMinute = iMinute - (iHour * 60);
    time_display[6] = hex2bcd(iHour);   // Hour
    time_display[7] = hex2bcd(iMinute); // Minute
    time_display[8] = hex2bcd((icentisecond / 10) % 60);   // Second
    HAL_UART_Transmit(LCD_USART, time_display, 9, 10);
}

void DisplayTime2(int page, int index, unsigned int icentisecond){
	unsigned char   time_display[8] = {0x5A, 0xA5, 0x05, 0x82, 0x02, 0x00, 0x00, 0x00};
	time_display[4] = page;
    time_display[5] = index;
    unsigned int iMinute = icentisecond / 600;
    unsigned int iHour = iMinute / 60;
    iMinute = iMinute - (iHour * 60);
    time_display[6] = hex2bcd(iMinute);   // Hour
    time_display[7] = hex2bcd((icentisecond / 10) % 60); // Minute
    HAL_UART_Transmit(LCD_USART, time_display, 8, 10);
}



#define first_steptime_index 0x54
#define first_processtime_index 0x72
unsigned char   steptime_index[11] = {0x00, first_steptime_index, first_steptime_index+0x03,first_steptime_index+0x06, first_steptime_index+0x09, first_steptime_index+0x0c,
								first_steptime_index+0x0f,first_steptime_index+0x12, first_steptime_index+0x15, first_steptime_index+0x18, first_steptime_index+0x1b};
unsigned char   processtime_index[7] = {0x00, first_processtime_index, first_processtime_index+0x03,first_processtime_index+0x06, first_processtime_index+0x09, first_processtime_index+0x0c,
								first_processtime_index+0x0f};
void DisplayTimeValues(){
	if(currentpage==LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE){
		DisplayTime(0x56,0x51,CycleData[ProcessNum][StepNum].Time*10);
		for(int i=1;i<11;i++){
			DisplayTime(0x56,steptime_index[i],CycleData[ProcessNum][i].Time*10);
		}
		CycleTime=0;
		for(int i=1;i<7;i++){
			float ptime=0;
			for(int j=1;j<21;j++){
				ptime+=CycleData[i][j].Time;
			}
			DisplayTime(0x56,processtime_index[i],ptime*10);
			CycleTime+=ptime*10;
		}
		DisplayTime(0x56,0x89,CycleTime);
	}
	else if(currentpage==LCD_FACTORY_TESTMODE_PROCESSSETTINGS2_PAGE){
		DisplayTime(0x56,0x51,CycleData[ProcessNum][StepNum].Time*10);
		for(int i=1;i<11;i++){
			DisplayTime(0x56,steptime_index[i],CycleData[ProcessNum][i+10].Time*10);
		}
		CycleTime=0;
		for(int i=1;i<7;i++){
			float ptime=0;
			for(int j=1;j<21;j++){
				ptime+=CycleData[i][j].Time;
			}
			DisplayTime(0x56,processtime_index[i],ptime*10);
			CycleTime+=ptime*10;
		}
		DisplayTime(0x56,0x89,CycleTime);
	}
	else if(currentpage==LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE){
		for(int i=1;i<11;i++){	//step time display
			DisplayTime(0x56,steptime_index[i],StepTime[i]);
		}
		for(int i=1;i<7;i++){	//process time display
			DisplayTime(0x56,processtime_index[i],ProcessTime[i]);
		}
		DisplayTime(0x56,0x89,CycleTime);
	}
	else if(currentpage==LCD_FACTORY_TESTMODE_PROCESSTEST2_PAGE){
		for(int i=1;i<11;i++){	//step time display
			DisplayTime(0x56,steptime_index[i],StepTime[i+10]);
		}
		for(int i=1;i<7;i++){	//process time display
			DisplayTime(0x56,processtime_index[i],ProcessTime[i]);
		}
		DisplayTime(0x56,0x89,CycleTime);
	}
	else if(currentpage==LCD_FACTORY_TESTMODE_PROCESSTEST3_PAGE){
		for(int i=1;i<11;i++){	//step time display
			DisplayTime(0x56,steptime_index[i],StepTime[i]);
		}
		for(int i=1;i<7;i++){	//process time display
			DisplayTime(0x56,processtime_index[i],ProcessTime[i]);
		}
		DisplayTime(0x56,0x89,CycleTime);
	}
	else if(currentpage==LCD_FACTORY_TESTMODE_PROCESSTEST4_PAGE){
		for(int i=1;i<11;i++){	//step time display
			DisplayTime(0x56,steptime_index[i],StepTime[i+10]);
		}
		for(int i=1;i<7;i++){	//process time display
			DisplayTime(0x56,processtime_index[i],ProcessTime[i]);
		}
		DisplayTime(0x56,0x89,CycleTime);
	}
}



void ReadStepTime(){
	for(int i=1;i<21;i++){
		StepTime[i]=CycleData[CurrentProcess][i].Time*10;
	}
}

void ReadProcessTime(){
	for(int i=1;i<7;i++){
		ProcessTime[i]=0;
		for(int j=1;j<21;j++){
			ProcessTime[i]+=(CycleData[i][j].Time*10);
		}
	}
}

void ReadCycleTime(){
	CycleTime=0;
	for(int i=1;i<7;i++){
		CycleTime+=ProcessTime[i];
	}
	FullCycleTime=CycleTime;
}



//Value display
unsigned char   value_display[10] = {0x5A, 0xA5, 0x05, 0x82, 0x02, 0x00, 0x00, 0x00, 0x00};

void DisplayValue(int index, float value){
    unsigned int uivalue;
    uivalue = (value * 10);
    value_display[2] = 0x05;
    value_display[5] = index;
    value_display[6] = uivalue >> 8;
    value_display[7] = uivalue & 0xff;
    HAL_UART_Transmit(LCD_USART, value_display, 8, 10);
}
void DisplayCycleValue(int value){
	if(value==1){
		DisplayPage8Char(0x56,0x10,"  SHORT ");
	}
	else if(value==2){
			DisplayPage8Char(0x56,0x10,"STANDARD");
	}
	else if(value==3){
			DisplayPage8Char(0x56,0x10,"ADVANCED");
	}
}


void DisplayVacuumSensor(){
	DisplayPageValue(0x60,0x11,Pressure*10);
}



//페이지 별 데이터 입출력

void Display02page(){
	//도어 상태 표기
	//아이디 표기
	if(LoginFlag==1){
		if(CurrentUser==10){
			DisplayPage10Char(0x02,0x10,"CBT          ");
		}
		else if(CurrentUser==9){
			DisplayPage10Char(0x02,0x10,"ADMIN        ");
		}
		else{
			DisplayPage10Char(0x02,0x10,flash_ID[CurrentUser]);
		}
		DisplayIcon(0x02, 0x30, 1);
	}
	else{
		DisplayIcon(0x02, 0x30, 0);
		DisplayPage10Char(0x02,0x10,"             ");
	}
	DisplaySterilantData();
	if(reservationFlag==1){
		DisplayIcon(0x12, 0x10, 1);
	}
	else{
		DisplayIcon(0x12, 0x10, 0);
	}
	if(MonitorFlag==1){
		DisplayIcon(0x05, 0x90, 1);
	}
	else{
		DisplayIcon(0x05, 0x90, 0);
	}
	DisplayIcon(0x02,0xA0,today_date.hour/10);
	DisplayIcon(0x02,0xB0,today_date.hour%10);
	DisplayIcon(0x02,0xC0,today_date.minute/10);
	DisplayIcon(0x02,0xD0,today_date.minute%10);
}

void Display03page(){

}

void Display04page(){
	DisplayPageValue(0x28,0x40,CarbonFilter);
	DisplayPageValue(0x28,0x50,HEPAFilter);
	DisplayPageValue(0x28,0x60,PlasmaAssy);
	//DisplayPageValue(0x22,0x40,CurrentRFIDData.volume);
	DisplaySterilantData();


}

void Display05page(){

}

void Display06page(){

}
void Display07page(){

}
void Display10page(){

}
void Display11page(){

}
void Display12page(){
	DisplayPageValue(0x12,0x20,reserve_date.month);
	DisplayPageValue(0x12,0x25,reserve_date.day);
	DisplayPageValue(0x12,0x30,reserve_date.hour);
	DisplayPageValue(0x12,0x35,reserve_date.minute);

	DisplayIcon(0x12,0x40,reserve_date.month/10);
	DisplayIcon(0x12,0x50,reserve_date.month%10);
	DisplayIcon(0x12,0x60,reserve_date.day/10);
	DisplayIcon(0x12,0x70,reserve_date.day%10);
	DisplayIcon(0x12,0x80,reserve_date.hour/10);
	DisplayIcon(0x12,0x90,reserve_date.hour%10);
	DisplayIcon(0x12,0xA0,reserve_date.minute/10);
	DisplayIcon(0x12,0xB0,reserve_date.minute%10);
}



void Display21page(){
	DisplayPage10Char(0x51,0x30,flash_MODEL_NAME);
	DisplayPage10Char(0x51,0x40,flash_SERIAL_NUMBER);
	DisplayPage10Char(0x51,0x50,flash_FACILITY_NAME);
	DisplayPage10Char(0x51,0x60,flash_DEPARTMENT_NAME);
	DisplayPage10Char(0x51,0x70,flash_SOFTWARE_VERSION);

	DisplayPageValue(0x41,0x40,CarbonFilter);
	DisplayPageValue(0x41,0x50,HEPAFilter);
	DisplayPageValue(0x41,0x60,PlasmaAssy);

	GetTime();
	//데일리 카운트 초기화
	if(beforeday!=today_date.day){
		beforeday=today_date.day;
		dailyCount=0;
	}
	DisplaySterilantData();
	DisplayPageValue(0x21,0x10,totalCount);
	DisplayPageValue(0x21,0x20,dailyCount);
	DisplaySterilantData();
	DisplayPageValue(0x28,0x40,CarbonFilter);
	DisplayPageValue(0x28,0x50,HEPAFilter);
	DisplayPageValue(0x28,0x60,PlasmaAssy);
	//Write_Flash();
}

void Display22page(){
	DisplaySterilantData();
}

void Display23page(){
	if(saveCycleData.cycleName==0){
		DisplayPage10Char(0x23,0x10,"          ");
		DisplayPage10Char(0x23,0x20,"          ");
		DisplayPage10Char(0x23,0x30,"          ");
		DisplayPage10Char(0x23,0x40,"          ");
	}
	else{
		if(saveCycleData.cycleName==1){
			DisplayPage10Char(0x23,0x10,"SHORT     ");
		}
		else if(saveCycleData.cycleName==2){
			DisplayPage10Char(0x23,0x10,"STANDARD  ");
		}
		else if(saveCycleData.cycleName==3){
			DisplayPage10Char(0x23,0x10,"ADVANCED  ");
		}

		if(saveCycleData.status==11){
			DisplayPage10Char(0x23,0x20,"COMPELTE  ");
		}
		else if(saveCycleData.status==1){
			DisplayPage10Char(0x23,0x20,"ERROR01   ");
		}
		else if(saveCycleData.status==2){
			DisplayPage10Char(0x23,0x20,"ERROR02   ");
		}
		else if(saveCycleData.status==3){
			DisplayPage10Char(0x23,0x20,"ERROR03   ");
		}
		else if(saveCycleData.status==4){
			DisplayPage10Char(0x23,0x20,"ERROR04   ");
		}
		else if(saveCycleData.status==5){
			DisplayPage10Char(0x23,0x20,"ERROR05   ");
		}
		else if(saveCycleData.status==6){
			DisplayPage10Char(0x23,0x20,"ERROR06   ");
		}
		else if(saveCycleData.status==7){
			DisplayPage10Char(0x23,0x20,"ERROR07   ");
		}
		else if(saveCycleData.status==8){
			DisplayPage10Char(0x23,0x20,"ERROR08   ");
		}
		else if(saveCycleData.status==9){
			DisplayPage10Char(0x23,0x20,"ERROR09   ");
		}
		else if(saveCycleData.status==10){
			DisplayPage10Char(0x23,0x20,"ERROR10   ");
		}
		else{
			DisplayPage10Char(0x23,0x20,"NODATA    ");
		}
		char msg[10];

		sprintf(msg,"%02d:%02d:%02d  ",	saveCycleData.startTime[0],saveCycleData.startTime[1],saveCycleData.startTime[2]);
		DisplayPage10Char(0x23,0x30,msg);
		sprintf(msg,"%02d:%02d:%02d  ",	saveCycleData.endTime[0],saveCycleData.endTime[1],saveCycleData.endTime[2]);
		DisplayPage10Char(0x23,0x40,msg);
	}



	char msg1[18]={};

	if(temptotalcycle[5*(currentHistoryPage-1)+1]==0){
		DisplayPage10Char(0x21,0x60,"");
	}
	else{
		sprintf(msg1,"Cycle #%03d",temptotalcycle[5*(currentHistoryPage-1)+1]);
		DisplayPage10Char(0x21,0x60,msg1);
	}

	if(temptotalcycle[5*(currentHistoryPage-1)+2]==0){
		DisplayPage10Char(0x21,0x70,"");
	}
	else{
		sprintf(msg1,"Cycle #%03d",temptotalcycle[5*(currentHistoryPage-1)+2]);
		DisplayPage10Char(0x21,0x70,msg1);
	}

	if(temptotalcycle[5*(currentHistoryPage-1)+3]==0){
		DisplayPage10Char(0x21,0x80,"");
	}
	else{
		sprintf(msg1,"Cycle #%03d",temptotalcycle[5*(currentHistoryPage-1)+3]);
		DisplayPage10Char(0x21,0x80,msg1);
	}

	if(temptotalcycle[5*(currentHistoryPage-1)+4]==0){
		DisplayPage10Char(0x21,0x90,"");
	}
	else{
		sprintf(msg1,"Cycle #%03d",temptotalcycle[5*(currentHistoryPage-1)+4]);
		DisplayPage10Char(0x21,0x90,msg1);
	}

	if(temptotalcycle[5*(currentHistoryPage-1)+5]==0){
		DisplayPage10Char(0x21,0xA0,"");
	}
	else{
		sprintf(msg1,"Cycle #%03d",temptotalcycle[5*(currentHistoryPage-1)+5]);
		DisplayPage10Char(0x21,0xA0,msg1);
	}

}

void Display24page(){
	if(MonitorFlag==1){
		DisplayIcon(0x24,0x10,1);
	}
	else{
		DisplayIcon(0x24,0x10,0);
	}
	if(reservationFlag==1){
		DisplayIcon(0x24,0x20,1);
	}
	else{
		DisplayIcon(0x24,0x20,0);
	}

	//프린트 토글
	if(autoprintFlag==1){
		DisplayIcon(0x24,0x40,1);
	}
	else{
		DisplayIcon(0x24,0x40,0);
	}

	DisplayPageValue(0x24,0x50,printcopy);

	if(printdataFlag==1){
		DisplayIcon(0x24,0x60,1);
	}
	else{
		DisplayIcon(0x24,0x60,0);
	}

	if(printgraphFlag==1){
		DisplayIcon(0x24,0x70,1);
	}
	else{
		DisplayIcon(0x24,0x70,0);
	}
	DisplayIcon(0x25,0x10,SleepModeFlag);
}

void Display25page(){
	DisplayIcon(0x25,0x10,SleepModeFlag);
	DisplayIcon(0x25,0x20,ActiveWeekday[1]);
	DisplayIcon(0x25,0x30,ActiveWeekday[2]);
	DisplayIcon(0x25,0x40,ActiveWeekday[3]);
	DisplayIcon(0x25,0x50,ActiveWeekday[4]);
	DisplayIcon(0x25,0x60,ActiveWeekday[5]);
	DisplayIcon(0x25,0x70,ActiveWeekday[6]);
	DisplayIcon(0x25,0x80,ActiveWeekday[0]);

	//Monday
	DisplayIcon(0x26,0x10,ActiveModeTime[0][1]/10);
	DisplayIcon(0x26,0x20,ActiveModeTime[0][1]%10);
	DisplayIcon(0x26,0x30,ActiveModeTime[1][1]/10);
	DisplayIcon(0x26,0x40,ActiveModeTime[1][1]%10);

	//Tuesday
	DisplayIcon(0x26,0x50,ActiveModeTime[0][2]/10);
	DisplayIcon(0x26,0x60,ActiveModeTime[0][2]%10);
	DisplayIcon(0x26,0x70,ActiveModeTime[1][2]/10);
	DisplayIcon(0x26,0x80,ActiveModeTime[1][2]%10);

	//Wednesday
	DisplayIcon(0x26,0x90,ActiveModeTime[0][3]/10);
	DisplayIcon(0x26,0xA0,ActiveModeTime[0][3]%10);
	DisplayIcon(0x26,0xB0,ActiveModeTime[1][3]/10);
	DisplayIcon(0x26,0xC0,ActiveModeTime[1][3]%10);

	//Thursday
	DisplayIcon(0x26,0xD0,ActiveModeTime[0][4]/10);
	DisplayIcon(0x26,0xE0,ActiveModeTime[0][4]%10);
	DisplayIcon(0x26,0xF0,ActiveModeTime[1][4]/10);
	DisplayIcon(0x27,0x00,ActiveModeTime[1][4]%10);

	//Friday
	DisplayIcon(0x27,0x10,ActiveModeTime[0][5]/10);
	DisplayIcon(0x27,0x20,ActiveModeTime[0][5]%10);
	DisplayIcon(0x27,0x30,ActiveModeTime[1][5]/10);
	DisplayIcon(0x27,0x40,ActiveModeTime[1][5]%10);

	//Saturday
	DisplayIcon(0x27,0x50,ActiveModeTime[0][6]/10);
	DisplayIcon(0x27,0x60,ActiveModeTime[0][6]%10);
	DisplayIcon(0x27,0x70,ActiveModeTime[1][6]/10);
	DisplayIcon(0x27,0x80,ActiveModeTime[1][6]%10);

	//Sunday
	DisplayIcon(0x27,0x90,ActiveModeTime[0][0]/10);
	DisplayIcon(0x27,0xA0,ActiveModeTime[0][0]%10);
	DisplayIcon(0x27,0xB0,ActiveModeTime[1][0]/10);
	DisplayIcon(0x27,0xC0,ActiveModeTime[1][0]%10);
}

void Display31page(){
	if(SelfTestInitFlag==1){
    	EndTestTimeCounter=(60*10+20+5)*10;//종합테스트 시간
    	memset(HeaterTestResult, 0, sizeof(HeaterTestResult));
    	memset(ValveTestResult, 0, sizeof(ValveTestResult));
    	memset(VacuumTestResult, 0, sizeof(VacuumTestResult));
    	memset(TestResult, 0, sizeof(TestResult));
		memset(TestTemp, 0, sizeof(TestTemp));
		memset(TestPressure, 0, sizeof(TestPressure));
    	SelfTestInitFlag=0;
	}

	//시간
	DisplayIcon(0x30,0x10,(EndTestTimeCounter/10)/60/10);
	DisplayIcon(0x30,0x20,(EndTestTimeCounter/10)/60%10);
	DisplayIcon(0x30,0x30,(EndTestTimeCounter/10)%60/10);
	DisplayIcon(0x30,0x40,(EndTestTimeCounter/10)%60%10);

	//아이콘
	DisplayIcon(0x31,0x10,HeaterTestResult[0]);
	DisplayIcon(0x31,0x20,ValveTestResult[0]);
	DisplayIcon(0x31,0x30,VacuumTestResult[0]);

	DisplayIcon(0x31,0x40,HeaterTestResult[1]);
	DisplayIcon(0x31,0x50,HeaterTestResult[2]);
	DisplayIcon(0x31,0x60,HeaterTestResult[3]);
	DisplayIcon(0x31,0x70,HeaterTestResult[4]);

	DisplayIcon(0x31,0x80,ValveTestResult[1]);
	DisplayIcon(0x31,0x90,ValveTestResult[2]);
	DisplayIcon(0x31,0xA0,ValveTestResult[3]);

	DisplayIcon(0x31,0xB0,VacuumTestResult[1]);
	DisplayIcon(0x31,0xC0,VacuumTestResult[2]);

	//점검,정지
	DisplayIcon(0x30,0x50,TestResult[0]);

	//정지,정상,불량
	DisplayIcon(0x30,0x60,TestResult[1]);
}

void Display32page(){
	if(SelfTestInitFlag==1){
    	EndTestTimeCounter=5*10;//여기
    	memset(HeaterTestResult, 0, sizeof(HeaterTestResult));
    	memset(TestResult, 0, sizeof(TestResult));
		memset(TestTemp, 0, sizeof(TestTemp));
    	SelfTestInitFlag=0;
	}
	//시간
	DisplayIcon(0x30,0x10,(EndTestTimeCounter/10)/60/10);
	DisplayIcon(0x30,0x20,(EndTestTimeCounter/10)/60%10);
	DisplayIcon(0x30,0x30,(EndTestTimeCounter/10)%60/10);
	DisplayIcon(0x30,0x40,(EndTestTimeCounter/10)%60%10);

	//아이콘
	DisplayIcon(0x31,0x10,HeaterTestResult[0]);
	DisplayIcon(0x31,0x40,HeaterTestResult[1]);
	DisplayIcon(0x31,0x50,HeaterTestResult[2]);
	DisplayIcon(0x31,0x60,HeaterTestResult[3]);
	DisplayIcon(0x31,0x70,HeaterTestResult[4]);

	//데이터
	DisplayPageValue(0x32,0x10,TestTemp[0]*10);
	DisplayPageValue(0x32,0x14,TestTemp[1]*10);
	DisplayPageValue(0x32,0x18,TestTemp[2]*10);
	DisplayPageValue(0x32,0x1C,TestTemp[3]*10);

	//점검,정지
	DisplayIcon(0x30,0x50,TestResult[0]);

	//정지,정상,불량
	DisplayIcon(0x30,0x60,TestResult[1]);

	Display55page();
}

void Display33page(){
	if(SelfTestInitFlag==1){
    	EndTestTimeCounter=5*10;//여기
    	memset(ValveTestResult, 0, sizeof(ValveTestResult));
    	memset(TestResult, 0, sizeof(TestResult));
    	SelfTestInitFlag=0;
	}

	//시간
	DisplayIcon(0x30,0x10,(EndTestTimeCounter/10)/60/10);
	DisplayIcon(0x30,0x20,(EndTestTimeCounter/10)/60%10);
	DisplayIcon(0x30,0x30,(EndTestTimeCounter/10)%60/10);
	DisplayIcon(0x30,0x40,(EndTestTimeCounter/10)%60%10);

	//아이콘
	DisplayIcon(0x31,0x20,ValveTestResult[0]);
	DisplayIcon(0x31,0x80,ValveTestResult[1]);
	DisplayIcon(0x31,0x90,ValveTestResult[2]);
	DisplayIcon(0x31,0xA0,ValveTestResult[3]);

	//점검,정지
	DisplayIcon(0x30,0x50,TestResult[0]);

	//정지,정상,불량
	DisplayIcon(0x30,0x60,TestResult[1]);
}


void Display34page(){
	if(SelfTestInitFlag==1){
    	EndTestTimeCounter=(60*10+10+5)*10;//진공 테스트 시간
    	memset(VacuumTestResult, 0, sizeof(VacuumTestResult));
    	memset(TestResult, 0, sizeof(TestResult));
		memset(TestPressure, 0, sizeof(TestPressure));
    	DisplayPageValue(0x51,0x01,TestVacuumValue);
    	DisplayPageValue(0x51,0x05,TestLeakValue);

    	SelfTestInitFlag=0;
	}

	//시간
	DisplayIcon(0x30,0x10,(EndTestTimeCounter/10)/60/10);
	DisplayIcon(0x30,0x20,(EndTestTimeCounter/10)/60%10);
	DisplayIcon(0x30,0x30,(EndTestTimeCounter/10)%60/10);
	DisplayIcon(0x30,0x40,(EndTestTimeCounter/10)%60%10);

	//데이터
	DisplayPageValue(0x34,0x10,TestPressure[0]*10);
	DisplayPageValue(0x34,0x14,TestPressure[1]*10);
	DisplayPageValue(0x34,0x18,TestPressure[2]*10);
	DisplayPageValue(0x34,0x1C,TestPressure[3]*10);
	DisplayPageValue(0x34,0x20,TestPressure[4]*10);

	DisplayPageValue(0x34,0x24,TestPressure[5]*10);
	DisplayPageValue(0x34,0x28,TestPressure[6]*10);
	DisplayPageValue(0x34,0x2C,TestPressure[7]*10);
	DisplayPageValue(0x34,0x30,TestPressure[8]*10);
	DisplayPageValue(0x34,0x34,TestPressure[9]*10);

	if(VacuumTestResult[1]==0){
		DisplayPage4Char(0x34,0x40,"");
	}
	else if(VacuumTestResult[1]==1){
		DisplayPage4Char(0x34,0x40,"PASS");
	}
	else if(VacuumTestResult[1]==2){
		DisplayPage4Char(0x34,0x40,"FAIL");
	}

	if(VacuumTestResult[2]==0){
		DisplayPage4Char(0x34,0x44,"");
	}
	else if(VacuumTestResult[2]==1){
		DisplayPage4Char(0x34,0x44,"PASS");
	}
	else if(VacuumTestResult[2]==2){
		DisplayPage4Char(0x34,0x44,"FAIL");
	}

	//아이콘
	//점검,정지
	DisplayIcon(0x30,0x50,TestResult[0]);

	//정지,정상,불량
	DisplayIcon(0x30,0x60,TestResult[1]);
}


void Display41page(){
	DisplayPageValue(0x41,0x10,CarbonFilterMax);
	DisplayPageValue(0x41,0x20,HEPAFilterMax);
	DisplayPageValue(0x41,0x30,PlasmaAssyMax);

	DisplayPageValue(0x41,0x40,CarbonFilter);
	DisplayPageValue(0x41,0x50,HEPAFilter);
	DisplayPageValue(0x41,0x60,PlasmaAssy);

}



void Display43page(){
	for(int i=1;i<15;i++){
		if(AlarmCheckFlag[i]==1){
			DisplayIcon(0x43,0x10*i,1);
		}
		else{
			DisplayIcon(0x43,0x10*i,0);
		}
	}
	if(AlarmCheckFlag[0]==1){
		DisplayIcon(0x43,0xF0,1);
	}
	else{
		DisplayIcon(0x43,0xF0,0);
	}
}

void Display44page(){
	for(int i=1;i<15;i++){
		if(ErrorCheckFlag[i]==1){
			DisplayIcon(0x44,0x10*i,1);
		}
		else{
			DisplayIcon(0x44,0x10*i,0);
		}
	}
	if(ErrorCheckFlag[0]==1){
		DisplayIcon(0x44,0xF0,1);
	}
	else{
		DisplayIcon(0x44,0xF0,0);
	}
}



void Display51page(){
	DisplayPageValue(0x51,0x01,TestVacuumValue);
	DisplayPageValue(0x51,0x05,TestLeakValue);
	DisplayPageValue(0x51,0x09,TestTempErrorValue);

	DisplayPageValue(0x51,0x10,expiry_date1);
	DisplayPageValue(0x51,0x14,expiry_date2);

	DisplayPageValue(0x51,0x18,DoorOpenPressure);

	DisplayPage10Char(0x51,0x30,flash_MODEL_NAME);
	DisplayPage10Char(0x51,0x40,flash_SERIAL_NUMBER);
	DisplayPage10Char(0x51,0x50,flash_FACILITY_NAME);
	DisplayPage10Char(0x51,0x60,flash_DEPARTMENT_NAME);
	DisplayPage10Char(0x51,0x70,flash_SOFTWARE_VERSION);
}

void Display52page(){
	if(LoginFlag==1){
		DisplayIcon(0x52,0x10,1);
	}
	else{
		DisplayIcon(0x52,0x10,0);
	}
}

void Display53page(){
	//출력값
	/*
	현재 온도 값
	스팬 값
	제로 값
	 */
	DisplayPageValue(0x53,0x21,CalibrationTemp[0]);
	DisplayPageValue(0x53,0x25,CalibrationTemp[1]);
	DisplayPageValue(0x53,0x29,CalibrationTemp[2]);
	DisplayPageValue(0x53,0x2D,CalibrationTemp[3]);
	DisplayPageValue(0x53,0x35,CalibrationVacuum);
}


void Display55page(){
	DisplayPageValue(0x55,0x01,DoorSettingTemp[0]*10);
	DisplayPageValue(0x55,0x05,DoorSettingTemp[1]*10);
	DisplayPageValue(0x55,0x09,DoorSettingTemp[2]*10);

	DisplayPageValue(0x55,0x11,ChamberSettingTemp[0]*10);
	DisplayPageValue(0x55,0x15,ChamberSettingTemp[1]*10);
	DisplayPageValue(0x55,0x19,ChamberSettingTemp[2]*10);

	DisplayPageValue(0x55,0x21,ChamberBackSettingTemp[0]*10);
	DisplayPageValue(0x55,0x25,ChamberBackSettingTemp[1]*10);
	DisplayPageValue(0x55,0x29,ChamberBackSettingTemp[2]*10);

	DisplayPageValue(0x55,0x31,VaporizerSettingTemp[0]*10);
	DisplayPageValue(0x55,0x35,VaporizerSettingTemp[1]*10);
	DisplayPageValue(0x55,0x39,VaporizerSettingTemp[2]*10);

	DisplayPageValue(0x55,0x41,PreesureCondition[0]);
	DisplayPageValue(0x55,0x45,PreesureCondition[1]);
	DisplayPageValue(0x55,0x49,PreesureCondition[2]);

	DisplayPageValue(0x55,0x51,perispeed);

	for(int i=0;i<3;i++){
		FlashSettingTemp[0][i]=DoorSettingTemp[i];
		FlashSettingTemp[1][i]=ChamberSettingTemp[i];
		FlashSettingTemp[2][i]=ChamberBackSettingTemp[i];
		FlashSettingTemp[3][i]=VaporizerSettingTemp[i];
	}
	Flashperispeed=perispeed;

	FlashPreesureCondition[0]=PreesureCondition[0];
	FlashPreesureCondition[1]=PreesureCondition[1];
	FlashPreesureCondition[2]=PreesureCondition[2];
}

void Display56page(){
	DisplayPageValue(0x55,0x41,PreesureCondition[0]);
	DisplayPageValue(0x55,0x45,PreesureCondition[1]);
	DisplayPageValue(0x55,0x49,PreesureCondition[2]);
	DisplayProcessSettingValues();
}

void Display57page(){
	DisplayProcessTestValues();
}
void Display58page(){
	DisplayPage(LCD_FACTORY_TESTMODE_SELECT_CYCLE_PAGE);
}



void Display61page(){
	DisplayPage10Char(0x61,0x10,"");
	DisplayPage10Char(0x61,0x20,"");
	DisplayIcon(0x61, 0x30, AutoLoginFlag);
}

//3 페이지
void Display63page(){
	DisplayPage10Char(0x63,0x10,"");
	DisplayPage10Char(0x63,0x20,"");
	DisplayPage10Char(0x63,0x30,"");
}

void Display64page(){
	DisplayPage10Char(0x64,0x10,flash_ID[0]);
	DisplayPage10Char(0x64,0x20,flash_ID[1]);
	DisplayPage10Char(0x64,0x30,flash_ID[2]);
	DisplayPage10Char(0x64,0x40,flash_ID[3]);
	DisplayPage10Char(0x64,0x50,admin_ID);
	DisplayPage10Char(0x64,0x60,flash_ID[Select_ID]);
}

void Display80page(){
	DisplayCycleValue(CycleName);
	DisplayProcessIcons(ProcessNum);
	DisplayStepIcons(StepNum);
	DisplayPartsIcons();
	DisplayTimeValues();
	//DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
}
void Display81page(){
	DisplayCycleValue(CycleName);
	DisplayProcessIcons(ProcessNum);
	DisplayStepIcons(StepNum);
	DisplayPartsIcons();
	DisplayTimeValues();
	//DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
}

void Display82page(){
	DisplayCycleValue(CycleName);
	DisplayProcessIcons(ProcessNum);
	DisplayStepIcons(StepNum);
	DisplayPartsIcons();
	DisplayTimeValues();
	//DisplayPage(LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE);
}







void DisplayPageValue(int page ,int index, int value){
	unsigned char   PageValue[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00};
	PageValue[4] = page;
	PageValue[5] = index;
	PageValue[6] = value >> 8;
	PageValue[7] = value & 0xff;
    HAL_UART_Transmit(LCD_USART, PageValue, 8, 10);
}



//문자 출력
void DisplayPage4Char(int page ,int index, char *msg){
	unsigned char   PageChar[10] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	PageChar[2]=7; //주소1+주소2+Data	lenth
	PageChar[4]=page;
	PageChar[5]=index;
	PageChar[6]=msg[0];
	PageChar[7]=msg[1];
	PageChar[8]=msg[2];
	PageChar[9]=msg[3];
    HAL_UART_Transmit(LCD_USART, PageChar, 10, 10);
}

void DisplayPage8Char(int page ,int index, char *msg){
	unsigned char   PageChar[14] = {0x5A, 0xA5, 0x0b, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	PageChar[2]=11; //주소1+주소2+Data	lenth
	PageChar[4]=page;
	PageChar[5]=index;
	PageChar[6]=msg[0];
	PageChar[7]=msg[1];
	PageChar[8]=msg[2];
	PageChar[9]=msg[3];
	PageChar[10]=msg[4];
	PageChar[11]=msg[5];
	PageChar[12]=msg[6];
	PageChar[13]=msg[7];
    HAL_UART_Transmit(LCD_USART, PageChar, 14, 10);
}

void DisplayPage10Char(int page ,int index, char *msg){
	unsigned char   PageChar[16] = {0x5A, 0xA5, 0x0d, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	PageChar[2]=13; //주소1+주소2+Data	lenth
	PageChar[4]=page;
	PageChar[5]=index;
	PageChar[6]=msg[0];
	PageChar[7]=msg[1];
	PageChar[8]=msg[2];
	PageChar[9]=msg[3];
	PageChar[10]=msg[4];
	PageChar[11]=msg[5];
	PageChar[12]=msg[6];
	PageChar[13]=msg[7];
	PageChar[14]=msg[8];
	PageChar[15]=msg[9];
    HAL_UART_Transmit(LCD_USART, PageChar, 16, 10);
}

void DisplayPage20Char(int page ,int index, char *msg){
	unsigned char   PageChar[16] = {0x5A, 0xA5, 0x0d, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	PageChar[2]=23; //주소1+주소2+Data	lenth
	PageChar[4]=page;
	PageChar[5]=index;
	PageChar[6]=msg[0];
	PageChar[7]=msg[1];
	PageChar[8]=msg[2];
	PageChar[9]=msg[3];
	PageChar[10]=msg[4];
	PageChar[11]=msg[5];
	PageChar[12]=msg[6];
	PageChar[13]=msg[7];
	PageChar[14]=msg[8];
	PageChar[15]=msg[9];
	PageChar[16]=msg[10];
	PageChar[17]=msg[11];
	PageChar[18]=msg[12];
	PageChar[19]=msg[13];
	PageChar[20]=msg[14];
	PageChar[21]=msg[15];
	PageChar[22]=msg[16];
	PageChar[23]=msg[17];
	PageChar[24]=msg[18];
	PageChar[25]=msg[19];
    HAL_UART_Transmit(LCD_USART, PageChar, 26, 10);
}


void GetTime(void){
	Get_RTC_Time(&today_date.year, &today_date.month, &today_date.day, &today_date.week,
			&today_date.hour, &today_date.minute, &today_date.second);
}


void DisplayIcons(){
	DisplayIcon(0x6A,0x10,!HAL_GPIO_ReadPin(GPIO_OUT11_GPIO_Port, GPIO_OUT11_Pin));
	DisplayIcon(0x6A,0x20,!HAL_GPIO_ReadPin(GPIO_OUT12_GPIO_Port, GPIO_OUT12_Pin));
	DisplayIcon(0x6A,0x30,!HAL_GPIO_ReadPin(GPIO_OUT13_GPIO_Port, GPIO_OUT13_Pin));
	DisplayIcon(0x6A,0x40,!HAL_GPIO_ReadPin(GPIO_OUT14_GPIO_Port, GPIO_OUT14_Pin));
	DisplayIcon(0x6A,0x50,!HAL_GPIO_ReadPin(GPIO_OUT15_GPIO_Port, GPIO_OUT15_Pin));
	DisplayIcon(0x6A,0x60,!HAL_GPIO_ReadPin(GPIO_OUT16_GPIO_Port, GPIO_OUT16_Pin));
	DisplayIcon(0x6A,0x70,!HAL_GPIO_ReadPin(GPIO_OUT17_GPIO_Port, GPIO_OUT17_Pin));
	DisplayIcon(0x6A,0x80,!HAL_GPIO_ReadPin(GPIO_OUT18_GPIO_Port, GPIO_OUT18_Pin));

	DisplayIcon(0x6B,0x10,HAL_GPIO_ReadPin(GPIO_OUT1_GPIO_Port, GPIO_OUT1_Pin));
	DisplayIcon(0x6B,0x20,HAL_GPIO_ReadPin(GPIO_OUT2_GPIO_Port, GPIO_OUT2_Pin));
	DisplayIcon(0x6B,0x30,HAL_GPIO_ReadPin(GPIO_OUT3_GPIO_Port, GPIO_OUT3_Pin));
	DisplayIcon(0x6B,0x40,HAL_GPIO_ReadPin(GPIO_OUT4_GPIO_Port, GPIO_OUT4_Pin));
	DisplayIcon(0x6B,0x50,HAL_GPIO_ReadPin(GPIO_OUT5_GPIO_Port, GPIO_OUT5_Pin));
	DisplayIcon(0x6B,0x60,HAL_GPIO_ReadPin(GPIO_OUT6_GPIO_Port, GPIO_OUT6_Pin));
	DisplayIcon(0x6B,0x70,HAL_GPIO_ReadPin(GPIO_OUT7_GPIO_Port, GPIO_OUT7_Pin));
	DisplayIcon(0x6B,0x80,HAL_GPIO_ReadPin(GPIO_OUT8_GPIO_Port, GPIO_OUT8_Pin));
	DisplayIcon(0x6B,0x90,HAL_GPIO_ReadPin(GPIO_OUT26_GPIO_Port, GPIO_OUT26_Pin));

	DisplayIcon(0x6C,0x10,DoorHandleCheck());
	DisplayIcon(0x6C,0x20,DoorLatchCheck());
	DisplayIcon(0x6C,0x30,SterilantContainerCheck());
	DisplayIcon(0x6C,0x40,SterilantSliderCheck());
	DisplayIcon(0x6C,0x50,LevelSensor1Check());
	DisplayIcon(0x6C,0x60,LevelSensor2Check());

	DisplayIcon(0x02,0x60,SleepModeFlag);
	DisplayIcon(0x02,0x70,HeaterControlMode);
}

void DisplaySterilantData(){
	if(SterilantContainerType==STERILANT_CONTAINER_VIAL){
		checkret = SterilantContainerCheck() ? 1 : 0;
	}

	//과수 정보 디스플레이
	if(checkret==1){
		char msg[10];
		sprintf(msg,"%02d%02d%02d%02d  ",
					CurrentRFIDData.production_year,CurrentRFIDData.production_month,CurrentRFIDData.production_day, CurrentRFIDData.production_number);
		DisplayPage10Char(0x22,0x10,msg);

		memset(msg, 0, 10);
		sprintf(msg,"20%2d-%02d-%02d",
				CurrentRFIDData.open_year,CurrentRFIDData.open_month,CurrentRFIDData.open_day);
		DisplayPage10Char(0x22,0x20,msg);

		memset(msg, 0, 10);
		sprintf(msg,"20%2d-%02d-%02d",
				CurrentRFIDData.expiry_year,CurrentRFIDData.expiry_month,CurrentRFIDData.expiry_day);
		DisplayPage10Char(0x22,0x30,msg);


		memset(msg, 0, 10);
		sprintf(msg,"%-2d(%-2d)    ",CurrentRFIDData.volume/2,CurrentRFIDData.volume);
		DisplayPage10Char(0x22,0x40,msg);
		//DisplayPageValue(0x22,0x40,CurrentRFIDData.volume);

		if(CurrentRFIDData.volume>=60){
			DisplayIcon(0x22,0x50,1);
		}
		else if(CurrentRFIDData.volume<60&&CurrentRFIDData.volume>=44){
			DisplayIcon(0x22,0x50,2);
		}
		else if(CurrentRFIDData.volume<44&&CurrentRFIDData.volume>=38){
			DisplayIcon(0x22,0x50,3);
		}
		else if(CurrentRFIDData.volume<38&&CurrentRFIDData.volume>=26){
			DisplayIcon(0x22,0x50,4);
		}
		else if(CurrentRFIDData.volume<26&&CurrentRFIDData.volume>=20){
			DisplayIcon(0x22,0x50,5);
		}
		else if(CurrentRFIDData.volume<20&&CurrentRFIDData.volume>=8){
			DisplayIcon(0x22,0x50,6);
		}
		else if(CurrentRFIDData.volume<8&&CurrentRFIDData.volume>=2){
			DisplayIcon(0x22,0x50,7);
		}
		else if(CurrentRFIDData.volume<2){
			DisplayIcon(0x22,0x50,8);
		}
	}
	else if(checkret==2){
		char msg[10];
		sprintf(msg,"%02d%02d%02d%02d  ",
					CurrentRFIDData.production_year,CurrentRFIDData.production_month,CurrentRFIDData.production_day, CurrentRFIDData.production_number);
		DisplayPage10Char(0x22,0x10,msg);

		memset(msg, 0, 10);
		sprintf(msg,"20%2d-%02d-%02d",
				CurrentRFIDData.open_year,CurrentRFIDData.open_month,CurrentRFIDData.open_day);
		DisplayPage10Char(0x22,0x20,msg);

		memset(msg, 0, 10);
		sprintf(msg,"20%2d-%02d-%02d",
				CurrentRFIDData.expiry_year,CurrentRFIDData.expiry_month,CurrentRFIDData.expiry_day);
		DisplayPage10Char(0x22,0x30,msg);

		memset(msg, 0, 10);
		sprintf(msg,"%-2d(%-2d)    ",CurrentRFIDData.volume/2,CurrentRFIDData.volume);
		DisplayPage10Char(0x22,0x40,msg);
		//DisplayPageValue(0x22,0x40,CurrentRFIDData.volume);

		DisplayIcon(0x22,0x50,9);
	}
	else if(checkret==3){
		char msg[10];
		sprintf(msg,"%02d%02d%02d%02d  ",
					CurrentRFIDData.production_year,CurrentRFIDData.production_month,CurrentRFIDData.production_day, CurrentRFIDData.production_number);
		DisplayPage10Char(0x22,0x10,msg);

		memset(msg, 0, 10);
		sprintf(msg,"20%2d-%02d-%02d",
				CurrentRFIDData.open_year,CurrentRFIDData.open_month,CurrentRFIDData.open_day);
		DisplayPage10Char(0x22,0x20,msg);

		memset(msg, 0, 10);
		sprintf(msg,"20%2d-%02d-%02d",
				CurrentRFIDData.expiry_year,CurrentRFIDData.expiry_month,CurrentRFIDData.expiry_day);
		DisplayPage10Char(0x22,0x30,msg);

		memset(msg, 0, 10);
		sprintf(msg,"%-2d(%-2d)    ",CurrentRFIDData.volume/2,CurrentRFIDData.volume);
		DisplayPage10Char(0x22,0x40,msg);
		//DisplayPageValue(0x22,0x40,CurrentRFIDData.volume);

		DisplayIcon(0x22,0x50,9);
	}
	else{
		DisplayPage10Char(0x22,0x10,"No Bottle ");
		DisplayPage10Char(0x22,0x20,"No Data   ");
		DisplayPage10Char(0x22,0x30,"No Data   ");
		DisplayPage10Char(0x22,0x40,"0         ");
		//DisplayPageValue(0x22,0x40,CurrentRFIDData.volume);
		CurrentRFIDData.volume=0;
		DisplayIcon(0x22,0x50,0);
	}
	//과수량 표기 숫자

	if(checkret==1||checkret==2||checkret==3){
		DisplayIcon(0x22,0x50,2);
	}


	if(currentpage==LCD_INFO_STERILANT_PAGE){
		int vialMounted = SterilantContainerCheck();
		int sliderClosed = SterilantSliderCheck();

		//멸균제 아이콘
		DisplayIcon(0x22,0xA0, vialMounted ? 1 : 0);
		//슬라이드 도어 아이콘
		DisplayIcon(0x22,0xB0, sliderClosed ? 1 : 0);
		//상태 아이콘
		if(vialMounted && sliderClosed){
			DisplayIcon(0x22,0xC0,0);
		}
		else if(vialMounted==0){
			DisplayIcon(0x22,0xC0,1);
		}
		else{
			DisplayIcon(0x22,0xC0,2);
		}
	}
	if(SterilantContainerType==STERILANT_CONTAINER_VIAL){
		DisplayIcon(0x02, 0x80, 10);
		DisplayIcon(0x02, 0x90, 10);
	}
	else{
		DisplayIcon(0x02, 0x80, (CurrentRFIDData.volume/2)/10);
		DisplayIcon(0x02, 0x90, (CurrentRFIDData.volume/2)%10);
	}
}

int ReadRTC(unsigned char *year, unsigned char *month, unsigned char *day, unsigned char *week, unsigned char *hour, unsigned char *minute, unsigned char *second){
	//RTC
	const unsigned char rtc_date_get[6] = {0x5A, 0xA5, 0x03, 0x81, 0x20, 0x04};
	const unsigned char rtc_time_get[6] = {0x5A, 0xA5, 0x03, 0x81, 0x24, 0x03};
	int result = 0;

	__disable_irq();
    huart1.RxState= HAL_UART_STATE_READY;
    memset(LCD_rx_data, 0, 30);

    HAL_UART_Transmit(LCD_USART, (uint8_t*)rtc_date_get, 6, 10);
    if (!LCD_ReceiveFrame(LCD_rx_data, 10, 10, 1)) {
    	goto read_rtc_done;
    }
    *year = LCD_rx_data[6];
    *month = LCD_rx_data[7];
    *day = LCD_rx_data[8];
    *week = LCD_rx_data[9];

    memset(LCD_rx_data, 0, 30);
    HAL_UART_Transmit(LCD_USART, (uint8_t*)rtc_time_get, 6, 10);
    if (!LCD_ReceiveFrame(LCD_rx_data, 9, 10, 1)) {
    	goto read_rtc_done;
    }
    *hour = LCD_rx_data[6];
    *minute = LCD_rx_data[7];
    *second = LCD_rx_data[8];
    result = 1;

read_rtc_done:
    UART_Receive_Flag = 0;
    __enable_irq();
    ReadLCD();
    return result;
}

void SetRTCFromLCD(){
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    if (!ReadRTC(&sDate.Year, &sDate.Month, &sDate.Date, &sDate.WeekDay, &sTime.Hours, &sTime.Minutes, &sTime.Seconds)) {
    	return;
    }

    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
        //타임 에러 핸들러 추가
    }
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
        //타임 에러 핸들러 추가
    }
}

void Get_RTC_Time(unsigned char *year, unsigned char *month, unsigned char *day, unsigned char *week, unsigned char *hour, unsigned char *minute, unsigned char *second){
    RTC_TimeTypeDef gTime = {0};
    RTC_DateTypeDef gDate = {0};

    gTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    gTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
        //타임 에러 핸들러 추가
    }
    if (HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
        //타임 에러 핸들러 추가
    }
    //*year, unsigned char *month, unsigned char *day, unsigned char *week, unsigned char *hour, unsigned char *minute, unsigned char *second){
    *year=bcd2bin(gDate.Year);
    *month=bcd2bin(gDate.Month);
    *day=bcd2bin(gDate.Date);
    *week=bcd2bin(gDate.WeekDay);

    *hour=bcd2bin(gTime.Hours);
	*minute=bcd2bin(gTime.Minutes);
	*second=bcd2bin(gTime.Seconds);
}

void ReadInforDataFromLCD(){
    __disable_irq();
    huart1.RxState= HAL_UART_STATE_READY;
    unsigned char get_lcd_data[7] = {0x5A, 0xA5, 0x04, 0x83, 0x51, 0x30, 0x05};
    memset(LCD_rx_data, 0, 30);

	get_lcd_data[5]=0x40;//SERIAL NUMBER
    HAL_UART_Transmit(LCD_USART, (uint8_t*)get_lcd_data, 7, 10);
    if (!LCD_ReceiveFrame(LCD_rx_data, 17, 10, 1)) {
    	goto read_info_done;
    }

    memset(flash_FACILITY_NAME,0,10);
    for(int i=0;i<10;i++){
    	if(LCD_rx_data[i+7]==0xFF){
    		break;
    	}
    	else{
    		flash_SERIAL_NUMBER[i]=LCD_rx_data[i+7];
    	}
    }
	memset(LCD_rx_data, 0, 30);
    get_lcd_data[5]=0x50;//FACILITY
	HAL_UART_Transmit(LCD_USART, (uint8_t*)get_lcd_data, 7, 10);
	if (!LCD_ReceiveFrame(LCD_rx_data, 17, 10, 1)) {
		goto read_info_done;
	}

	memset(flash_FACILITY_NAME,0,10);
    for(int i=0;i<10;i++){
    	if(LCD_rx_data[i+7]==0xFF){
    		break;
    	}
    	else{
    		flash_FACILITY_NAME[i]=LCD_rx_data[i+7];
    	}
    }
	memset(LCD_rx_data, 0, 30);
    get_lcd_data[5]=0x60;//DEPARTMENT
	HAL_UART_Transmit(LCD_USART, (uint8_t*)get_lcd_data, 7, 10);
	if (!LCD_ReceiveFrame(LCD_rx_data, 17, 10, 1)) {
		goto read_info_done;
	}

	memset(flash_DEPARTMENT_NAME,0,10);
    for(int i=0;i<10;i++){
    	if(LCD_rx_data[i+7]==0xFF){
    		break;
    	}
    	else{
    		flash_DEPARTMENT_NAME[i]=LCD_rx_data[i+7];
    	}
    }
read_info_done:
    UART_Receive_Flag = 0;
    __enable_irq();
    ReadLCD();
}


unsigned char hex2bcd (unsigned char x)
{
    unsigned char y;
    y = (x / 10) << 4;
    y = y | (x % 10);
    return (y);
}

unsigned char bcd2bin (unsigned char x)
{
    return 	(x&0x0f)+(x>>4)*10;
}
