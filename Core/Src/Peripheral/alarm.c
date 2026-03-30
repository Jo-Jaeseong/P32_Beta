/*
 * alarm.c
 *
 *  Created on: 2023. 3. 29.
 *      Author: CBT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "hardware.h"
#include "sensor.h"
#include "peripheral.h"

int errorcode=0;

/*
에러1 공정 취수
에러2 챔버 온도 에러
에러3 도어 온도 에러
에러4 도어백 온도 에러
에러5 기화기 온도 에러
에러6 압력1 에러
에러7 압력2 에러(사이클1)
에러8 압력2 에러(사이클2)
에러9 압력3 에러(사이클1)
에러10 압력3 에러(사이클2)
에러11 멸균제 에러(사이클1)
에러12 멸균제 에러(사이클2)
*/

unsigned char devicealarm[15]={};
unsigned char devicePreAlarm[5]={};
unsigned char deviceerror[15]={};

unsigned char AlarmCheckFlag[15]={};
unsigned char ErrorCheckFlag[15]={};

/*
알람1 챔버 도어 열림	도어가 열림, 스위치 동작 안됨
알람2 멸균제 병이 감지가 안됨.
알람3 멸균제 제조 만기일 지남. 멸균제 제조 후 1년 지남
알람4 멸균제 사용 기간 지남. 장비내 멸균제 삽입 후 60일 지남
알람5 멸균제 양이 부족함.
알람6 도어 히터 온도 확인	도어 히터 및 히터 전선, TC 및 TC선 확인
알람7 챔버 히터 온도 확인	챔버 히터 및 히터 전선, TC 및 TC선 확인
알람8 챔버 후면 히터 온도 확인	챔버 후면 히터 및 히터 전선, TC 및 TC선 확인
알람9 기화기 히터 온도 확인	기화기 히터 및 히터 전선, TC 및 TC선 확인
알람10 비정상적으로 제품이 정지.
알람11 PM 카본필터 만료.
알람12 PM 헤파필터 만료.
알람13 PM 플라즈마 만료.
*/


int Alarm_Check(){
	memset(devicealarm,0,sizeof(devicealarm));
	int isVialMode = (SterilantContainerType==STERILANT_CONTAINER_VIAL);
	//도어 확인
	if(AlarmCheckFlag[0]==1){
		if(AlarmCheckFlag[1]==1){
			if(DoorLatchCheck()){
				devicealarm[1]=0;
			}
			else{
				devicealarm[1]=1;
			}
		}

		//멸균제 인식 확인 (Bottle: RFID, Vial: Sensor)
		if(AlarmCheckFlag[2]==1){
			if(isVialMode){
				devicealarm[2]=(SterilantContainerCheck()==0);
			}
			else{
				devicealarm[2]=(checkret==-2);
			}
		}

		//멸균제 제조 기간 확인 (RFID bottle only)
		if(AlarmCheckFlag[3]==1){
			if(isVialMode){
				devicealarm[3]=0;
			}
			else{
				devicealarm[3]=(checkret==3);
			}
		}

		//멸균제 장착 기간 확인 (RFID bottle only)
		if(AlarmCheckFlag[4]==1){
			if(isVialMode){
				devicealarm[4]=0;
			}
			else{
				devicealarm[4]=(checkret==2);
			}
		}

		//멸균제 양 확인 (RFID bottle only)
		if(AlarmCheckFlag[5]==1){
			if(isVialMode){
				devicealarm[5]=0;
			}
			else{
				devicealarm[5]=(CurrentRFIDData.volume<2);
			}
		}

		//기화기 온도 체크
		if(AlarmCheckFlag[9]==1){
			if((VaporizerSettingTemp[0]-TestTempErrorValue)>Temperature[3]){
				devicealarm[9]=1;
				DisplayIcon(0x15, 0x30, 3);
			}
			else{
				devicealarm[9]=0;
			}
		}

		//도어 온도 체크
		if(AlarmCheckFlag[6]==1){
			if((DoorSettingTemp[0]-TestTempErrorValue)>Temperature[0]){
				devicealarm[6]=1;
				DisplayIcon(0x15, 0x30, 0);
			}
			else{
				devicealarm[6]=0;
			}
		}

		//챔버 온도 체크
		if(AlarmCheckFlag[7]==1){
			if((ChamberSettingTemp[0]-TestTempErrorValue)>Temperature[1]){
				devicealarm[7]=1;
				DisplayIcon(0x15, 0x30, 1);
			}
			else{
				devicealarm[7]=0;
			}
		}

		//챔버 백 온도 체크
		if(AlarmCheckFlag[8]==1){
			if((ChamberBackSettingTemp[0]-TestTempErrorValue)>Temperature[2]){
				devicealarm[8]=1;
				DisplayIcon(0x15, 0x30, 2);
			}
			else{
				devicealarm[8]=0;
			}
		}

		//장비 차단 전원 관련 확인
		if(AlarmCheckFlag[10]==1){

		}

		//PM1(카본필터) 체크
		if(AlarmCheckFlag[11]==1){
			if(CarbonFilter<=0){
				devicealarm[11]=1;
				DisplayIcon(0x15, 0x40, 0);
			}
			else{
				devicealarm[11]=0;
			}
		}

		//PM2(헤파필터) 체크
		if(AlarmCheckFlag[12]==1){
			if(HEPAFilter<=0){
				devicealarm[12]=1;
				DisplayIcon(0x15, 0x40, 1);
			}
			else{
				devicealarm[12]=0;
			}
		}

		//PM3(플라즈마 모듈) 체크
		if(AlarmCheckFlag[13]==1){
			if(PlasmaAssy<=0){
				devicealarm[13]=1;
				DisplayIcon(0x15, 0x40, 2);
			}
			else{
				devicealarm[13]=0;
			}
		}
	}
	for(int i=1;i<16;i++){
		devicealarm[0]+=devicealarm[i];
	}
	return devicealarm[0];
}

int PreAlarm_Check(){
	//devicePreAlarm
	int isVialMode = (SterilantContainerType==STERILANT_CONTAINER_VIAL);
	devicePreAlarm[0]=0;
	if((isVialMode==0) && CurrentRFIDData.volume<8&&CurrentRFIDData.volume>=2){
		devicePreAlarm[1]=1;
	}
	else{
		devicePreAlarm[1]=0;
	}

	if((CarbonFilter<=20)){
		devicePreAlarm[2]=1;
	}
	else{
		devicePreAlarm[2]=0;
	}

	if((HEPAFilter<=20)){
		devicePreAlarm[3]=1;
	}
	else{
		devicePreAlarm[3]=0;
	}

	if((PlasmaAssy<=20)){
		devicePreAlarm[4]=1;
	}
	else{
		devicePreAlarm[4]=0;
	}

	for(int i=1;i<5;i++){
		devicePreAlarm[0]+=devicePreAlarm[i];
	}

	return devicePreAlarm[0];
}


void DisplayAlarmCheck(){
	if(devicealarm[1]==1){
		DisplayPage10Char(0x07,0x10," ALARM001 ");
		DisplayPage10Char(0x07,0x20,"DOOR OPEN ");
	}
	else if(devicealarm[2]==1){
		DisplayPage10Char(0x07,0x10," ALARM002 ");
		if(SterilantContainerType==STERILANT_CONTAINER_VIAL){
			DisplayPage10Char(0x07,0x20,"NO VIAL   ");
		}
		else{
			DisplayPage10Char(0x07,0x20,"NO BOTTLE ");
		}
	}
	else if(devicealarm[3]==1){
		DisplayPage10Char(0x07,0x10," ALARM003 ");
		DisplayPage10Char(0x07,0x20,"RFID DATE ");
	}
	else if(devicealarm[4]==1){
		DisplayPage10Char(0x07,0x10," ALARM004 ");
		DisplayPage10Char(0x07,0x20,"RFID OPEN ");
	}
	else if(devicealarm[5]==1){
		DisplayPage10Char(0x07,0x10," ALARM005 ");
		DisplayPage10Char(0x07,0x20,"LOW RFID  ");
	}
	else if(devicealarm[6]==1){
		DisplayPage10Char(0x07,0x10," ALARM006 ");
		DisplayPage10Char(0x07,0x20,"D.H ALARM ");
	}
	else if(devicealarm[7]==1){
		DisplayPage10Char(0x07,0x10," ALARM007 ");
		DisplayPage10Char(0x07,0x20,"C.H ALARM ");
	}
	else if(devicealarm[8]==1){
		DisplayPage10Char(0x07,0x10," ALARM008 ");
		DisplayPage10Char(0x07,0x20,"B.H ALARM ");
	}
	else if(devicealarm[9]==1){
		DisplayPage10Char(0x07,0x10," ALARM009 ");
		DisplayPage10Char(0x07,0x20,"V.H ALARM ");
	}
	else if(devicealarm[10]==1){
		DisplayPage10Char(0x07,0x10," ALARM010 ");
		DisplayPage10Char(0x07,0x20,"          ");
	}
	else if(devicealarm[11]==1){
		DisplayPage10Char(0x07,0x10," ALARM011 ");
		DisplayPage10Char(0x07,0x20,"C.Filter  ");
	}
	else if(devicealarm[12]==1){
		DisplayPage10Char(0x07,0x10," ALARM012 ");
		DisplayPage10Char(0x07,0x20,"HEPAFilter");
	}
	else if(devicealarm[13]==1){
		DisplayPage10Char(0x07,0x10," ALARM013 ");
		DisplayPage10Char(0x07,0x20,"PLASMAASSY");
	}
}

/*
에러1 공정 취수
에러2 챔버 온도 에러
에러3 도어 온도 에러
에러4 도어백 온도 에러
에러5 기화기 온도 에러
에러6 압력1 에러
에러7 압력2 에러(사이클1)
에러8 압력2 에러(사이클2)
에러9 압력3 에러(사이클1)
에러10 압력3 에러(사이클2)
에러11 멸균제 에러(사이클1)
에러12 멸균제 에러(사이클2)
*/


void DeviceErrorProcess(){
	deviceerror[0]=0;
	if(ErrorCheckFlag[0]==1){
		for(int i=1;i<13;i++){
			deviceerror[0]+=deviceerror[i];
		}
	}
	if(deviceerror[0]!=0){
		if(deviceerror[1]==1){//stop 프로레스에서 일부 구현
			//취소
			DisplayPage4Char(0x09,0x10,"01  ");
			errorcode=1;
		}
		else if(deviceerror[2]==1){//도어 온도
			DisplayPage4Char(0x09,0x10,"02  ");
			errorcode=2;
		}
		else if(deviceerror[3]==1){//챔버 온도
			DisplayPage4Char(0x09,0x10,"03  ");
			errorcode=3;
		}
		else if(deviceerror[4]==1){//챔버 백 온도
			DisplayPage4Char(0x09,0x10,"04  ");
			errorcode=4;
		}
		else if(deviceerror[5]==1){//기화기 온도
			DisplayPage4Char(0x09,0x10,"05  ");
			errorcode=5;
		}
		else if(deviceerror[6]==1){
			DisplayPage4Char(0x09,0x10,"06  ");
			errorcode=6;
		}
		else if(deviceerror[7]==1){
			DisplayPage4Char(0x09,0x10,"07  ");
			errorcode=7;
		}
		else if(deviceerror[8]==1){
			DisplayPage4Char(0x09,0x10,"08  ");
			errorcode=8;
		}
		else if(deviceerror[9]==1){
			DisplayPage4Char(0x09,0x10,"09  ");
			errorcode=9;
		}
		else if(deviceerror[10]==1){
			DisplayPage4Char(0x09,0x10,"10  ");
			errorcode=10;
		}
		else if(deviceerror[11]==1){
			DisplayPage4Char(0x09,0x10,"11  ");
			errorcode=11;
		}
		else if(deviceerror[12]==1){
			DisplayPage4Char(0x09,0x10,"12  ");
			errorcode=12;
		}
		else{
		}
		if(Select_NORMAL_MODE==1){
			DisplayPage(LCD_EORROR_POPUP_PAGE);
		}
		ErrorEndProcess();
		//deviceerror[0]=0;
		StopFlag=1;
	}
}
