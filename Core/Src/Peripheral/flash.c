/*
 * flash.c
 *
 *  Created on: 2023. 1. 11.
 *      Author: CBT
 */

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "main.h"
#include "hardware.h"
#include "peripheral.h"
#include "sensor.h"

__attribute__((__section__(".user_data"))) const char userConfig[1024];

#define USER_DATA_FLASH_ADDRESS      0x08040000

char userdata[1024];



//-----------------------------------------------------------------------------------------------------------------------//
/*장비 초기 정보*/
unsigned char Device_First_Boot=0;

#define DEVICE_BOOTFLAG_DATA		0//char[10](문자입력)



/*장비 정보*///(30)
unsigned char flash_MODEL_NAME[10]="FN-P20    ";
unsigned char flash_SERIAL_NUMBER[10];
unsigned char flash_DEPARTMENT_NAME[10];
unsigned char flash_FACILITY_NAME[10];
unsigned char flash_SOFTWARE_VERSION[10]="1.0.0     ";

#define SERIAL_NUMBER				10//char[10](문자입력)
#define DEPARTMENT_NAME				20//char[10](문자입력)
#define FACILITY_NAME				30//char[10](문자입력)

//-----------------------------------------------------------------------------------------------------------------------//

//30~67


//int ActiveWeekday[7];
//int ActiveModeTime[2][7];

#define ACTIVEWEEKDAY_DATA			40//int[7]
#define ACTIVETIME_DATA				50//int[14]

unsigned char AutoLoginFlag;
unsigned char AutoLoginID;

#define AUTO_LOGINFLAG_DATA			68//char[1]
#define AUTO_LOGINID_DATA			69//char[1]


/*계정 정보*///(70)
unsigned char flash_ID[4][10];
unsigned char flash_PW[4][10];
unsigned char flashuserCount;

#define	FLASHID						70//char[4][10](숫자입력)
#define	FLASHPW						110//char[4][10](숫자입력)
#define	FLASHUSERCOUNT				150//char[1](숫자입력)


//-----------------------------------------------------------------------------------------------------------------------//
/*세팅*///(39)

//셀프 테스트 오차
#define TESTVACUUMVALUE_DATA		151//[1]
#define TESTLEAKVALUE_DATA			152//[1]
#define TESTTEMPERRORVALUE_DATA		153//[1]

//기능 플래그 데이터
#define LOGINFLAG_DATA				154//[1]
#define MONITORFLAG_DATA			155//[1]
#define LANGUAGE_DATA				156//[1]

#define RESERVATIONFLAG_DATA		157//[1]
#define AUTOPRINTFLAG_DATA			158//[1]
#define PRINTCOPY_DATA				159//[1]
#define PRINTGRAPHFLAG_DATA			160//[1]
#define PRINTDATAFLAG_DATA			161//[1]

//온도 세팅
#define DOORSETTINGTEMP_DATA		162//[3]2~4
#define CHAMBERSETTINGTEMP_DATA		165//[3]5~7
#define CHAMBERBACKSETTINGTEMP_DATA	168//[3]8~10
#define VAPORIZERSETTINGTEMP_DATA	171//[3]1~3

//진공 조건 세팅
#define PRESSURECONDITION_DATA		174//[3]4~6

//페리 속도 세팅
#define PERISPEED_DATA				177//[1]

//캘리브레이션
#define CALIBRATIONTEMP_DATA		178//[4]8~11
#define CALIBRATIONVACUUM_DATA		182//[1]

//과수 기한 설정
#define EXPIRY_DATE1_DATA			183//[1]
#define EXPIRY_DATE2_DATA			184//[1]

//도어 오픈 압력 설정
#define DOOROPENPRESSURE_DATA		185//[2]5~6
#define STERILANT_CONTAINER_TYPE_DATA	189//[1] 1:Bottle, 2:Vial

//Alarm,Error 체크 세팅
#define ALARMCHECKFLAG_DATA			190//[15]190~204
#define ERRORCHECKFLAG_DATA			205//[15]205~219


//-----------------------------------------------------------------------------------------------------------------------//
/*PM 정보*///(22)
//사용 횟수 카운트
#define	TOTALCOUNT_DATA				220//int[2](숫자입력)	//4자리 환산 필요
#define	DAILYCOUNT_DATA				222//int[1](숫자입력)
#define	BEFOREDAY_DATA				223//int[1](숫자입력)//beforeday

#define	CARBONFILTERMAX 			224//char[2](숫자입력)
#define	HEPAFILTERMAX				226//char[2](숫자입력)
#define	PLASMAASSYMAX				228//char[2](숫자입력)
#define	CARBONFILTER				230//char[2](숫자입력)
#define	HEPAFILTER					232//char[2](숫자입력)
#define	PLASMAASSY					234//char[2](숫자입력)

#define	POWEROFF_DATA				238//char[1](숫자입력)

unsigned char flash_ProcessPowerOffFlag;

//RFID
#define RFIDFLASHINDEX				239//int[1]	//RFIDflashIndex
#define RFID_DATA					240//int[12]240~251//252~263//264~275//276~287//288~299

/*
#define PRODUCTION_YEAR_DATA		240//int[1]
#define PRODUCTION_MONTH_DATA		241//int[1]
#define PRODUCTION_DAY_DATA			242//int[1]
#define PRODUCTION_NUMBER_DATA		243//int[1]
#define OPEN_YEAR_DATA				244//int[1]
#define OPEN_MONTH_DATA				245//int[1]
#define OPEN_DAY_DATA				246//int[1]
#define EXPIRY_YEAR_DATA			247//int[1]
#define EXPIRY_MONTH_DATA			248//int[1]
#define EXPIRY_DAY_DATA				249//int[1]
#define VOLUME_DATA					250//int[1]
#define VOLUMEMAX_DATA				251//int[1]
*/






//-----------------------------------------------------------------------------------------------------------------------//
/*공정*///(720)
#define CYCLEDATA1					300	//[240]
#define CYCLEDATA2					540	//[240]
#define CYCLEDATA3					780	//[240]

#define	NONE				0x00
#define	VACUUMVALVE			0x01
#define	VENTVALVE			0x02
#define	INJECTIONVALVE		0x04
#define	PERIPUMP			0x08
#define	PLASMA				0x10
#define	VAPORIZER			0x20
#define	PRESSURE1			0x40
#define	PRESSURE2			0x80
#define	PRESSURE3			0xC0

//-----------------------------------------------------------------------------------------------------------------------//

static unsigned char NormalizeFlag(unsigned char value, unsigned char default_value){
	if(value==1 || value==2){
		return value;
	}
	if(value==0 || value==0xFF){
		return default_value;
	}
	return default_value;
}

static unsigned char NormalizeByteValue(unsigned char value, unsigned char default_value){
	if(value==0 || value==0xFF){
		return default_value;
	}
	return value;
}

static unsigned int NormalizeWordValue(unsigned char high, unsigned char low, unsigned int default_value){
	if((high==0 && low==0) || (high==0xFF && low==0xFF)){
		return default_value;
	}
	return (high*100)+low;
}

void Write_Flash(){
	DisplayPage(LCD_LOADING_PAGE);
	DisplayPageValue(0x00,0x0A,0);
	unsigned char ucData[1024]={};

	/* 장비 */
	ucData[DEVICE_BOOTFLAG_DATA]=Device_First_Boot;


	/*장비 정보*/
	for(int i=0;i<10;i++){
		ucData[SERIAL_NUMBER+i]=flash_SERIAL_NUMBER[i];
		ucData[DEPARTMENT_NAME+i]=flash_DEPARTMENT_NAME[i];
		ucData[FACILITY_NAME+i]=flash_FACILITY_NAME[i];
	}

	/*절전 모드 세팅*/
	for(int i=0;i<7;i++){
		ucData[ACTIVEWEEKDAY_DATA+i]=ActiveWeekday[i];
		ucData[ACTIVETIME_DATA+i]=ActiveModeTime[0][i];
		ucData[ACTIVETIME_DATA+i+7]=ActiveModeTime[1][i];
	}


	/*자동 로그인*/
	ucData[AUTO_LOGINFLAG_DATA]=AutoLoginFlag;
	ucData[AUTO_LOGINID_DATA]=AutoLoginID;


	/*계정 정보*///(51)
	for(int j=0;j<4;j++){
		for(int i=0;i<10;i++){
			ucData[FLASHID+i+10*j]=flash_ID[j][i];
			ucData[FLASHPW+i+10*j]=flash_PW[j][i];
		}
	}
	ucData[FLASHUSERCOUNT]=flashuserCount;

	/*세팅*///(39)
	//기능 플래그 저장
	for(int i=0;i<15;i++){
		ucData[ALARMCHECKFLAG_DATA+i]=AlarmCheckFlag[i];
		ucData[ERRORCHECKFLAG_DATA+i]=ErrorCheckFlag[i];
	}

	ucData[RESERVATIONFLAG_DATA]=reservationFlag;
	ucData[AUTOPRINTFLAG_DATA]=autoprintFlag;
	ucData[PRINTCOPY_DATA]=printcopy;
	ucData[PRINTDATAFLAG_DATA]=printdataFlag;
	ucData[PRINTGRAPHFLAG_DATA]=printgraphFlag;


	//온도세팅 저장
	ucData[DOORSETTINGTEMP_DATA]=DoorSettingTemp[0];
	ucData[DOORSETTINGTEMP_DATA+1]=DoorSettingTemp[1];
	ucData[DOORSETTINGTEMP_DATA+2]=DoorSettingTemp[2];
	ucData[CHAMBERSETTINGTEMP_DATA]=ChamberSettingTemp[0];
	ucData[CHAMBERSETTINGTEMP_DATA+1]=ChamberSettingTemp[1];
	ucData[CHAMBERSETTINGTEMP_DATA+2]=ChamberSettingTemp[2];
	ucData[CHAMBERBACKSETTINGTEMP_DATA]=ChamberBackSettingTemp[0];
	ucData[CHAMBERBACKSETTINGTEMP_DATA+1]=ChamberBackSettingTemp[1];
	ucData[CHAMBERBACKSETTINGTEMP_DATA+2]=ChamberBackSettingTemp[2];
	ucData[VAPORIZERSETTINGTEMP_DATA]=VaporizerSettingTemp[0];
	ucData[VAPORIZERSETTINGTEMP_DATA+1]=VaporizerSettingTemp[1];
	ucData[VAPORIZERSETTINGTEMP_DATA+2]=VaporizerSettingTemp[2];

	//진공조건 저장
	ucData[PRESSURECONDITION_DATA]=PreesureCondition[0];
	ucData[PRESSURECONDITION_DATA+1]=PreesureCondition[1];
	ucData[PRESSURECONDITION_DATA+2]=PreesureCondition[2];

	//페리 스피드 저장
	ucData[PERISPEED_DATA]=perispeed;

	//캘리브레이션 데이터 저장
	ucData[CALIBRATIONTEMP_DATA]=CalibrationTemp[0];
	ucData[CALIBRATIONTEMP_DATA+1]=CalibrationTemp[1];
	ucData[CALIBRATIONTEMP_DATA+2]=CalibrationTemp[2];
	ucData[CALIBRATIONTEMP_DATA+3]=CalibrationTemp[3];
	ucData[CALIBRATIONVACUUM_DATA]=CalibrationVacuum;



	//셀프 테스트 벨류 및 오차 저장
	ucData[TESTVACUUMVALUE_DATA]=TestVacuumValue;
	ucData[TESTLEAKVALUE_DATA]=TestLeakValue;
	ucData[TESTTEMPERRORVALUE_DATA]=TestTempErrorValue;

	//과수 기한 설정
	ucData[EXPIRY_DATE1_DATA]=expiry_date1;
	ucData[EXPIRY_DATE2_DATA]=expiry_date2;

	//도어 오픈 압력 설정
	ucData[DOOROPENPRESSURE_DATA]=(int)(DoorOpenPressure/100);
	ucData[DOOROPENPRESSURE_DATA+1]=(int)(DoorOpenPressure%100);
	ucData[STERILANT_CONTAINER_TYPE_DATA]=SterilantContainerType;


	/*PM 정보*///(22)
	//사용 횟수 카운트
	ucData[TOTALCOUNT_DATA]=(int)(totalCount/100);
	ucData[TOTALCOUNT_DATA+1]=(int)(totalCount%100);
	ucData[DAILYCOUNT_DATA]=dailyCount;
	ucData[BEFOREDAY_DATA]=beforeday;

	//RFID 저장
	/*
	ucData[RFID_DATA]=RFIDData.production_year;
	ucData[RFID_DATA+1]=RFIDData.production_month;
	ucData[RFID_DATA+2]=RFIDData.production_day;
	ucData[RFID_DATA+3]=RFIDData.production_number;
	ucData[RFID_DATA+4]=RFIDData.open_year;
	ucData[RFID_DATA+5]=RFIDData.open_month;
	ucData[RFID_DATA+6]=RFIDData.open_day;

	ucData[RFID_DATA+7]=RFIDData.expiry_year;
	ucData[RFID_DATA+8]=RFIDData.expiry_month;
	ucData[RFID_DATA+9]=RFIDData.expiry_day;

	ucData[RFID_DATA+10]=RFIDData.volume;
	ucData[RFID_DATA+11]=RFIDData.volumemax;
	 */

	ucData[RFIDFLASHINDEX]=RFIDflashIndex;
	for(int i=0;i<5;i++){
		ucData[RFID_DATA+0+(i*12)]=FlashRFIDData[i].production_year;
		ucData[RFID_DATA+1+(i*12)]=FlashRFIDData[i].production_month;
		ucData[RFID_DATA+2+(i*12)]=FlashRFIDData[i].production_day;
		ucData[RFID_DATA+3+(i*12)]=FlashRFIDData[i].production_number;
		ucData[RFID_DATA+4+(i*12)]=FlashRFIDData[i].open_year;
		ucData[RFID_DATA+5+(i*12)]=FlashRFIDData[i].open_month;
		ucData[RFID_DATA+6+(i*12)]=FlashRFIDData[i].open_day;

		ucData[RFID_DATA+7+(i*12)]=FlashRFIDData[i].expiry_year;
		ucData[RFID_DATA+8+(i*12)]=FlashRFIDData[i].expiry_month;
		ucData[RFID_DATA+9+(i*12)]=FlashRFIDData[i].expiry_day;

		ucData[RFID_DATA+10+(i*12)]=FlashRFIDData[i].volume;
		ucData[RFID_DATA+11+(i*12)]=FlashRFIDData[i].volumemax;
	}




	//필터, 플라즈마 데이터 저장
	ucData[CARBONFILTERMAX]=(int)(CarbonFilterMax/100);
	ucData[CARBONFILTERMAX+1]=(int)(CarbonFilterMax%100);
	ucData[HEPAFILTERMAX]=(int)(HEPAFilterMax/100);
	ucData[HEPAFILTERMAX+1]=(int)(HEPAFilterMax%100);
	ucData[PLASMAASSYMAX]=(int)(PlasmaAssyMax/100);
	ucData[PLASMAASSYMAX+1]=(int)(PlasmaAssyMax%100);
	ucData[CARBONFILTER]=(int)(CarbonFilter/100);
	ucData[CARBONFILTER+1]=(int)(CarbonFilter%100);
	ucData[HEPAFILTER]=(int)(HEPAFilter/100);
	ucData[HEPAFILTER+1]=(int)(HEPAFilter%100);
	ucData[PLASMAASSY]=(int)(PlasmaAssy/100);
	ucData[PLASMAASSY+1]=(int)(PlasmaAssy%100);

	ucData[POWEROFF_DATA]=flash_ProcessPowerOffFlag;

	/*공장 세팅*/
	ucData[LOGINFLAG_DATA]=LoginFlag;
	ucData[MONITORFLAG_DATA]=MonitorFlag;
	ucData[LANGUAGE_DATA]=LanguageFlag;

	/*공정*///(720)
	int j=0;
	j=CYCLEDATA1;
	for(int i2=1;i2<7;i2++){
		for(int i3=1;i3<21;i3++){
			ucData[j]=userdata[j];
			ucData[j+1]=userdata[j+1];
			j+=2;
		}
	}
	j=CYCLEDATA2;
	for(int i2=1;i2<7;i2++){
		for(int i3=1;i3<21;i3++){
			ucData[j]=userdata[j];
			ucData[j+1]=userdata[j+1];
			j+=2;
		}
	}
	j=CYCLEDATA3;
	for(int i2=1;i2<7;i2++){
		for(int i3=1;i3<21;i3++){
			ucData[j]=userdata[j];
			ucData[j+1]=userdata[j+1];
			j+=2;
		}
	}
	if(CycleName==1){
		j=CYCLEDATA1;
	}
	else if(CycleName==2){
		j=CYCLEDATA2;
	}
	else if(CycleName==3){
		j=CYCLEDATA3;
	}
	else{
		j=CYCLEDATA1;
	}
	for(int i2=1;i2<7;i2++){
		for(int i3=1;i3<21;i3++){
			ucData[j]=CycleData[i2][i3].PartsSetting;
			ucData[j+1]=CycleData[i2][i3].Time;
			j+=2;
		}
	}

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
	FLASH_Erase_Sector(FLASH_SECTOR_6, VOLTAGE_RANGE_3);

	for(int i = 0; i < 1024; i++) {
	 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userConfig[0] + i, ucData[i]);
	}

	HAL_FLASH_Lock();
	SaveSetting();
	Read_Flash();
	//SaveSetting();//여기 테스트
}

void Read_Flash(){
	for(int i = 0; i < 1024; i++) {
		// Flash 메모리의 주소에서 직접 데이터를 읽어와 ReadTest 배열에 저장합니다.
	   userdata[i] = Flash_Read_Int(USER_DATA_FLASH_ADDRESS + i);
	}

	/* 장비 */
	Device_First_Boot=NormalizeFlag(userdata[DEVICE_BOOTFLAG_DATA],1);

	/*장비 정보*///(70)
	for(int i=0;i<10;i++){
		flash_SERIAL_NUMBER[i]=userdata[SERIAL_NUMBER+i];
		flash_DEPARTMENT_NAME[i]=userdata[DEPARTMENT_NAME+i];
		flash_FACILITY_NAME[i]=userdata[FACILITY_NAME+i];
	}

	if(flash_SERIAL_NUMBER[0]==0){
		sprintf(flash_SERIAL_NUMBER,"CBTP250701");
	}
	if(flash_FACILITY_NAME[0]==0){
		sprintf(flash_FACILITY_NAME,"CBT");
	}
	if(flash_DEPARTMENT_NAME[0]==0){
		sprintf(flash_DEPARTMENT_NAME,"CleanTeam");
	}

	/*절전 모드 세팅*/
	for(int i=0;i<7;i++){
		ActiveWeekday[i]=userdata[ACTIVEWEEKDAY_DATA+i];
		ActiveModeTime[0][i]=userdata[ACTIVETIME_DATA+i];
		ActiveModeTime[1][i]=userdata[ACTIVETIME_DATA+i+7];
	}
	ActiveWeekday[1]=NormalizeFlag(ActiveWeekday[1],1);
	ActiveWeekday[2]=NormalizeFlag(ActiveWeekday[2],1);
	ActiveWeekday[3]=NormalizeFlag(ActiveWeekday[3],1);
	ActiveWeekday[4]=NormalizeFlag(ActiveWeekday[4],1);
	ActiveWeekday[5]=NormalizeFlag(ActiveWeekday[5],1);
	ActiveWeekday[6]=NormalizeFlag(ActiveWeekday[6],2);
	ActiveWeekday[0]=NormalizeFlag(ActiveWeekday[0],2);
	for(int i=0;i<7;i++){
		if(ActiveModeTime[1][i]==0){
			ActiveModeTime[1][i]=24;
		}
	}

	/*자동 로그인*/
	AutoLoginFlag=NormalizeFlag(userdata[AUTO_LOGINFLAG_DATA],2);
	AutoLoginID=userdata[AUTO_LOGINID_DATA];


	/*계정 정보*///(51)
	for(int j=0;j<4;j++){
		for(int i=0;i<10;i++){
			flash_ID[j][i]=userdata[FLASHID+i+10*j];
			flash_PW[j][i]=userdata[FLASHPW+i+10*j];
		}
	}
	if(flash_ID[0][0]==0){
		sprintf(flash_ID[0],"USER1");
		sprintf(flash_PW[0],"1234");
	}
	flashuserCount=userdata[FLASHUSERCOUNT];
	if(flashuserCount<=0){
		flashuserCount=1;
	}

	/*세팅*///(39)
	//기능 플래그 저장

	//테스트 용
	/*
	memset(AlarmCheckFlag,2,sizeof(AlarmCheckFlag));
	memset(ErrorCheckFlag,2,sizeof(ErrorCheckFlag));
	reservationFlag=1;
	*/

	for(int i=0;i<15;i++){
		AlarmCheckFlag[i]=NormalizeFlag(userdata[ALARMCHECKFLAG_DATA+i],1);
		ErrorCheckFlag[i]=NormalizeFlag(userdata[ERRORCHECKFLAG_DATA+i],1);
	}

	reservationFlag=NormalizeFlag(userdata[RESERVATIONFLAG_DATA],2);

	autoprintFlag=NormalizeFlag(userdata[AUTOPRINTFLAG_DATA],1);
	printcopy=userdata[PRINTCOPY_DATA];
	if(printcopy==0){
		printcopy=1;
	}
	printdataFlag=NormalizeFlag(userdata[PRINTDATAFLAG_DATA],1);

	printgraphFlag=NormalizeFlag(userdata[PRINTGRAPHFLAG_DATA],1);

	//온도세팅 저장
	DoorSettingTemp[0]=NormalizeByteValue(userdata[DOORSETTINGTEMP_DATA],58);
	DoorSettingTemp[1]=NormalizeByteValue(userdata[DOORSETTINGTEMP_DATA+1],58);
	DoorSettingTemp[2]=NormalizeByteValue(userdata[DOORSETTINGTEMP_DATA+2],20);
	ChamberSettingTemp[0]=NormalizeByteValue(userdata[CHAMBERSETTINGTEMP_DATA],58);
	ChamberSettingTemp[1]=NormalizeByteValue(userdata[CHAMBERSETTINGTEMP_DATA+1],58);
	ChamberSettingTemp[2]=NormalizeByteValue(userdata[CHAMBERSETTINGTEMP_DATA+2],20);
	ChamberBackSettingTemp[0]=NormalizeByteValue(userdata[CHAMBERBACKSETTINGTEMP_DATA],58);
	ChamberBackSettingTemp[1]=NormalizeByteValue(userdata[CHAMBERBACKSETTINGTEMP_DATA+1],58);
	ChamberBackSettingTemp[2]=NormalizeByteValue(userdata[CHAMBERBACKSETTINGTEMP_DATA+2],20);
	VaporizerSettingTemp[0]=NormalizeByteValue(userdata[VAPORIZERSETTINGTEMP_DATA],80);
	VaporizerSettingTemp[1]=NormalizeByteValue(userdata[VAPORIZERSETTINGTEMP_DATA+1],130);
	VaporizerSettingTemp[2]=NormalizeByteValue(userdata[VAPORIZERSETTINGTEMP_DATA+2],20);

	//진공조건 저장
	PreesureCondition[0]=NormalizeByteValue(userdata[PRESSURECONDITION_DATA],70);
	PreesureCondition[1]=NormalizeByteValue(userdata[PRESSURECONDITION_DATA+1],25);
	PreesureCondition[2]=NormalizeByteValue(userdata[PRESSURECONDITION_DATA+2],20);

	//페리 스피드 저장
	perispeed=NormalizeByteValue(userdata[PERISPEED_DATA],8);

	//캘리브레이션 데이터 저장
	CalibrationTemp[0]=NormalizeByteValue(userdata[CALIBRATIONTEMP_DATA],20);
	CalibrationTemp[1]=NormalizeByteValue(userdata[CALIBRATIONTEMP_DATA+1],20);
	CalibrationTemp[2]=NormalizeByteValue(userdata[CALIBRATIONTEMP_DATA+2],20);
	CalibrationTemp[3]=NormalizeByteValue(userdata[CALIBRATIONTEMP_DATA+3],20);
	CalibrationVacuum=NormalizeByteValue(userdata[CALIBRATIONVACUUM_DATA],10);


	//셀프 테스트 벨류 및 오차 저장
	TestVacuumValue=NormalizeByteValue(userdata[TESTVACUUMVALUE_DATA],10);
	TestLeakValue=NormalizeByteValue(userdata[TESTLEAKVALUE_DATA],2);
	TestTempErrorValue=NormalizeByteValue(userdata[TESTTEMPERRORVALUE_DATA],3);

	//과수 기한 설정
	expiry_date1=NormalizeByteValue(userdata[EXPIRY_DATE1_DATA],12);
	expiry_date2=NormalizeByteValue(userdata[EXPIRY_DATE2_DATA],2);

	//도어 오픈 압력 설정
	DoorOpenPressure=NormalizeWordValue(userdata[DOOROPENPRESSURE_DATA],userdata[DOOROPENPRESSURE_DATA+1],720);
	SterilantContainerType=NormalizeFlag(userdata[STERILANT_CONTAINER_TYPE_DATA],STERILANT_CONTAINER_BOTTLE);

	/*PM 정보*///(22)
	//사용 횟수 카운트
	totalCount=(userdata[TOTALCOUNT_DATA]*100)+(userdata[TOTALCOUNT_DATA+1]);
	//totalCount=NormalizeWordValue(userdata[TOTALCOUNT_DATA],userdata[TOTALCOUNT_DATA+1],0);
	dailyCount=userdata[DAILYCOUNT_DATA];
	beforeday=userdata[BEFOREDAY_DATA];

	//멸균제 데이터 저장

	RFIDflashIndex=userdata[RFIDFLASHINDEX];

	for(int i=0;i<5;i++){
		FlashRFIDData[i].production_year=userdata[RFID_DATA+0+(i*12)];
		FlashRFIDData[i].production_month=userdata[RFID_DATA+1+(i*12)];
		FlashRFIDData[i].production_day=userdata[RFID_DATA+2+(i*12)];
		FlashRFIDData[i].production_number=userdata[RFID_DATA+3+(i*12)];

		FlashRFIDData[i].open_year=userdata[RFID_DATA+4+(i*12)];
		FlashRFIDData[i].open_month=userdata[RFID_DATA+5+(i*12)];
		FlashRFIDData[i].open_day=userdata[RFID_DATA+6+(i*12)];

		FlashRFIDData[i].expiry_year=userdata[RFID_DATA+7+(i*12)];
		FlashRFIDData[i].expiry_month=userdata[RFID_DATA+8+(i*12)];
		FlashRFIDData[i].expiry_day=userdata[RFID_DATA+9+(i*12)];

		FlashRFIDData[i].volume=userdata[RFID_DATA+10+(i*12)];
		FlashRFIDData[i].volumemax=userdata[RFID_DATA+11+(i*12)];
	}



	//필터, 플라즈마 데이터 저장
	CarbonFilterMax=(userdata[CARBONFILTERMAX]*100)+(userdata[CARBONFILTERMAX+1]);
	if(CarbonFilterMax==0){
		CarbonFilterMax=400;
	}
	HEPAFilterMax=(userdata[HEPAFILTERMAX]*100)+(userdata[HEPAFILTERMAX+1]);
	if(HEPAFilterMax==0){
		HEPAFilterMax=400;
	}
	PlasmaAssyMax=(userdata[PLASMAASSYMAX]*100)+(userdata[PLASMAASSYMAX+1]);
	if(PlasmaAssyMax==0){
		PlasmaAssyMax=400;
	}

	CarbonFilter=(userdata[CARBONFILTER]*100)+(userdata[CARBONFILTER+1]);
	HEPAFilter=(userdata[HEPAFILTER]*100)+(userdata[HEPAFILTER+1]);
	PlasmaAssy=(userdata[PLASMAASSY]*100)+(userdata[PLASMAASSY+1]);

	flash_ProcessPowerOffFlag=NormalizeFlag(userdata[POWEROFF_DATA],2);

	/*공장 세팅*///(3)
	LoginFlag=NormalizeFlag(userdata[LOGINFLAG_DATA],2);
	MonitorFlag=NormalizeFlag(userdata[MONITORFLAG_DATA],1);
	LanguageFlag=NormalizeFlag(userdata[LANGUAGE_DATA],1);



	/*공정*///(720)
	int j=0;
	if(CycleName==1){
		j=CYCLEDATA1;
	}
	else if(CycleName==2){
		j=CYCLEDATA2;

	}
	else if(CycleName==3){
		j=CYCLEDATA3;
	}
	for(int i2=1;i2<7;i2++){
		for(int i3=1;i3<21;i3++){
			CycleData[i2][i3].PartsSetting=userdata[j];
			CycleData[i2][i3].Time=(userdata[j+1]);
			j+=2;
		}
	}
	if(CycleName==1){
		if(CycleData[1][1].Time==0){
			ShortCycle();
		}
		else if(CycleData[1][1].Time==1){
			TestCycle();
		}
	}
	else if(CycleName==2){
		if(CycleData[1][1].Time==0){
			StandardCycle();
		}
	}
	else if(CycleName==3){
		if(CycleData[1][1].Time==0){
			AdvancedCycle();
		}
	}
}

void TestCycle(){
	CycleData[1][1].PartsSetting=VACUUMVALVE;
	CycleData[1][1].Time=1;
	CycleData[1][2].PartsSetting=VACUUMVALVE;
	CycleData[1][2].Time=1;
	CycleData[1][3].PartsSetting=VACUUMVALVE+PRESSURE1;
	CycleData[1][3].Time=1;
	CycleData[1][4].PartsSetting=VENTVALVE+INJECTIONVALVE;
	CycleData[1][4].Time=1;
	CycleData[1][5].PartsSetting=NONE;
	CycleData[1][5].Time=1;
	CycleData[1][6].PartsSetting=NONE;
	CycleData[1][6].Time=1;
	CycleData[1][7].PartsSetting=VACUUMVALVE;
	CycleData[1][7].Time=1;
	CycleData[1][8].PartsSetting=VACUUMVALVE;
	CycleData[1][8].Time=1;
	CycleData[1][9].PartsSetting=VACUUMVALVE;
	CycleData[1][9].Time=1;
	CycleData[1][10].PartsSetting=VACUUMVALVE;
	CycleData[1][10].Time=1;
	CycleData[1][11].PartsSetting=VACUUMVALVE+PRESSURE2;
	CycleData[1][11].Time=1;
	CycleData[1][12].PartsSetting=VACUUMVALVE+PERIPUMP;
	CycleData[1][12].Time=1;

	CycleData[2][1].PartsSetting=INJECTIONVALVE;
	CycleData[2][1].Time=1;
	CycleData[2][2].PartsSetting=PRESSURE3;
	CycleData[2][2].Time=1;
	CycleData[2][3].PartsSetting=NONE;
	CycleData[2][3].Time=1;
	CycleData[2][4].PartsSetting=NONE;
	CycleData[2][4].Time=1;
	CycleData[2][5].PartsSetting=NONE;
	CycleData[2][5].Time=1;
	CycleData[2][6].PartsSetting=NONE;
	CycleData[2][6].Time=1;
	CycleData[2][7].PartsSetting=NONE;
	CycleData[2][7].Time=1;
	CycleData[2][8].PartsSetting=VENTVALVE+INJECTIONVALVE;
	CycleData[2][8].Time=1;
	CycleData[2][9].PartsSetting=VACUUMVALVE;
	CycleData[2][9].Time=1;
	CycleData[2][10].PartsSetting=NONE;
	CycleData[2][10].Time=1;
	CycleData[2][11].PartsSetting=VENTVALVE;
	CycleData[2][11].Time=1;
	CycleData[2][12].PartsSetting=VACUUMVALVE;
	CycleData[2][12].Time=1;
	CycleData[2][13].PartsSetting=VENTVALVE;
	CycleData[2][13].Time=1;

	CycleData[3][1].PartsSetting=VACUUMVALVE;
	CycleData[3][1].Time=1;
	CycleData[3][2].PartsSetting=VACUUMVALVE;
	CycleData[3][2].Time=1;
	CycleData[3][3].PartsSetting=VACUUMVALVE;
	CycleData[3][3].Time=1;
	CycleData[3][4].PartsSetting=VACUUMVALVE;
	CycleData[3][4].Time=1;
	CycleData[3][5].PartsSetting=VACUUMVALVE;
	CycleData[3][5].Time=1;
	CycleData[3][6].PartsSetting=VACUUMVALVE;
	CycleData[3][6].Time=1;
	CycleData[3][7].PartsSetting=VACUUMVALVE;
	CycleData[3][7].Time=1;
	CycleData[3][8].PartsSetting=VACUUMVALVE;
	CycleData[3][8].Time=1;
	CycleData[3][9].PartsSetting=VACUUMVALVE+PRESSURE2;
	CycleData[3][9].Time=1;
	CycleData[3][10].PartsSetting=VACUUMVALVE+PERIPUMP;
	CycleData[3][10].Time=1;

	CycleData[4][1].PartsSetting=INJECTIONVALVE;
	CycleData[4][1].Time=1;
	CycleData[4][2].PartsSetting=NONE;
	CycleData[4][2].Time=1;
	CycleData[4][3].PartsSetting=PRESSURE3;
	CycleData[4][3].Time=1;
	CycleData[4][4].PartsSetting=NONE;
	CycleData[4][4].Time=1;
	CycleData[4][5].PartsSetting=NONE;
	CycleData[4][5].Time=1;
	CycleData[4][6].PartsSetting=NONE;
	CycleData[4][6].Time=1;
	CycleData[4][7].PartsSetting=NONE;
	CycleData[4][7].Time=1;
	CycleData[4][8].PartsSetting=VENTVALVE+INJECTIONVALVE;
	CycleData[4][8].Time=1;
	CycleData[4][9].PartsSetting=VACUUMVALVE;
	CycleData[4][9].Time=1;
	CycleData[4][10].PartsSetting=NONE;
	CycleData[4][10].Time=1;
	CycleData[4][11].PartsSetting=VENTVALVE;
	CycleData[4][11].Time=1;
	CycleData[4][12].PartsSetting=VACUUMVALVE;
	CycleData[4][12].Time=1;
	CycleData[4][13].PartsSetting=VENTVALVE;
	CycleData[4][13].Time=1;

	CycleData[5][1].PartsSetting=VACUUMVALVE;
	CycleData[5][1].Time=1;
	CycleData[5][2].PartsSetting=VACUUMVALVE;
	CycleData[5][2].Time=1;
	CycleData[5][3].PartsSetting=VACUUMVALVE;
	CycleData[5][3].Time=1;
	CycleData[5][4].PartsSetting=VACUUMVALVE;
	CycleData[5][4].Time=1;
	CycleData[5][5].PartsSetting=VACUUMVALVE+INJECTIONVALVE;
	CycleData[5][5].Time=1;
	CycleData[5][6].PartsSetting=VACUUMVALVE;
	CycleData[5][6].Time=1;
	CycleData[5][7].PartsSetting=VACUUMVALVE;
	CycleData[5][7].Time=1;
	CycleData[5][8].PartsSetting=VACUUMVALVE;
	CycleData[5][8].Time=1;
	CycleData[5][9].PartsSetting=VACUUMVALVE;
	CycleData[5][9].Time=1;
	CycleData[5][10].PartsSetting=VACUUMVALVE;
	CycleData[5][10].Time=1;

	CycleData[6][1].PartsSetting=VENTVALVE+INJECTIONVALVE;
	CycleData[6][1].Time=1;
	CycleData[6][2].PartsSetting=VACUUMVALVE;
	CycleData[6][2].Time=1;
	CycleData[6][3].PartsSetting=VENTVALVE+INJECTIONVALVE;
	CycleData[6][3].Time=1;
	CycleData[6][4].PartsSetting=VACUUMVALVE;
	CycleData[6][4].Time=1;
	CycleData[6][5].PartsSetting=VENTVALVE;
	CycleData[6][5].Time=1;
	CycleData[6][6].PartsSetting=NONE;
	CycleData[6][6].Time=1;
}
/*
#define	NONE				0x00

#define	VACUUMVALVE			0x01
#define	VENTVALVE			0x02
#define	INJECTIONVALVE		0x04

#define	PERIPUMP			0x08

#define	PLASMA				0x10

#define	VAPORIZER			0x20

#define	PRESSURE1			0x40
#define	PRESSURE2			0x80
#define	PRESSURE3			0xC0
 */
void ShortCycle(){
	CycleData[1][1].PartsSetting=VACUUMVALVE;
	CycleData[1][1].Time=3;
	CycleData[1][2].PartsSetting=VACUUMVALVE;
	CycleData[1][2].Time=27;
	CycleData[1][3].PartsSetting=VACUUMVALVE+PRESSURE1;
	CycleData[1][3].Time=60;
	CycleData[1][4].PartsSetting=VENTVALVE;
	CycleData[1][4].Time=7;
	CycleData[1][5].PartsSetting=NONE;
	CycleData[1][5].Time=23;
	CycleData[1][6].PartsSetting=NONE;
	CycleData[1][6].Time=30;
	CycleData[1][7].PartsSetting=VACUUMVALVE;
	CycleData[1][7].Time=30;
	CycleData[1][8].PartsSetting=VACUUMVALVE;
	CycleData[1][8].Time=30;
	CycleData[1][9].PartsSetting=VACUUMVALVE;
	CycleData[1][9].Time=50;
	CycleData[1][10].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][10].Time=60;
	CycleData[1][11].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][11].Time=60;
	CycleData[1][12].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][12].Time=60;
	CycleData[1][13].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][13].Time=60;
	CycleData[1][14].PartsSetting=VACUUMVALVE+PERIPUMP+VAPORIZER;
	CycleData[1][14].Time=40;


	CycleData[2][1].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][1].Time=1;
	CycleData[2][2].PartsSetting=VAPORIZER;
	CycleData[2][2].Time=29;
	CycleData[2][3].PartsSetting=VAPORIZER+PRESSURE3;
	CycleData[2][3].Time=30;
	CycleData[2][4].PartsSetting=VAPORIZER;
	CycleData[2][4].Time=30;
	CycleData[2][5].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][5].Time=1;
	CycleData[2][6].PartsSetting=VAPORIZER;
	CycleData[2][6].Time=30;
	CycleData[2][7].PartsSetting=VAPORIZER;
	CycleData[2][7].Time=44;
	CycleData[2][8].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][8].Time=5;
	CycleData[2][9].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA+VAPORIZER;
	CycleData[2][9].Time=6;
	CycleData[2][10].PartsSetting=VAPORIZER;
	CycleData[2][10].Time=34;
	CycleData[2][11].PartsSetting=VAPORIZER;
	CycleData[2][11].Time=30;
	CycleData[2][12].PartsSetting=VAPORIZER;
	CycleData[2][12].Time=30;
	CycleData[2][13].PartsSetting=VAPORIZER;
	CycleData[2][13].Time=26;
	CycleData[2][14].PartsSetting=VAPORIZER;
	CycleData[2][14].Time=4;

	CycleData[3][1].PartsSetting=VACUUMVALVE+INJECTIONVALVE+VAPORIZER;
	CycleData[3][1].Time=3;
	CycleData[3][2].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][2].Time=12;
	CycleData[3][3].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][3].Time=5;
	CycleData[3][4].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][4].Time=10;
	CycleData[3][5].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][5].Time=15;
	CycleData[3][6].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][6].Time=15;
	CycleData[3][7].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][7].Time=30;
	CycleData[3][8].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][8].Time=30;
	CycleData[3][9].PartsSetting=VACUUMVALVE+VAPORIZER+PRESSURE2;
	CycleData[3][9].Time=50;
	CycleData[3][10].PartsSetting=VACUUMVALVE+PERIPUMP+VAPORIZER;
	CycleData[3][10].Time=40;

	CycleData[4][1].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][1].Time=1;
	CycleData[4][2].PartsSetting=VAPORIZER;
	CycleData[4][2].Time=29;
	CycleData[4][3].PartsSetting=VAPORIZER+PRESSURE3;
	CycleData[4][3].Time=30;
	CycleData[4][4].PartsSetting=VAPORIZER;
	CycleData[4][4].Time=30;
	CycleData[4][5].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][5].Time=1;
	CycleData[4][6].PartsSetting=VAPORIZER;
	CycleData[4][6].Time=30;
	CycleData[4][7].PartsSetting=VAPORIZER;
	CycleData[4][7].Time=44;
	CycleData[4][8].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][8].Time=5;
	CycleData[4][9].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA+VAPORIZER;
	CycleData[4][9].Time=6;
	CycleData[4][10].PartsSetting=VAPORIZER;
	CycleData[4][10].Time=34;
	CycleData[4][11].PartsSetting=VAPORIZER;
	CycleData[4][11].Time=30;
	CycleData[4][12].PartsSetting=VAPORIZER;
	CycleData[4][12].Time=30;
	CycleData[4][13].PartsSetting=VAPORIZER;
	CycleData[4][13].Time=26;
	CycleData[4][14].PartsSetting=VAPORIZER;
	CycleData[4][14].Time=4;

	CycleData[5][1].PartsSetting=VACUUMVALVE+INJECTIONVALVE;
	CycleData[5][1].Time=3;
	CycleData[5][2].PartsSetting=VACUUMVALVE;
	CycleData[5][2].Time=12;
	CycleData[5][3].PartsSetting=VACUUMVALVE;
	CycleData[5][3].Time=5;
	CycleData[5][4].PartsSetting=VACUUMVALVE;
	CycleData[5][4].Time=10;
	CycleData[5][5].PartsSetting=VACUUMVALVE;
	CycleData[5][5].Time=15;
	CycleData[5][6].PartsSetting=VACUUMVALVE;
	CycleData[5][6].Time=15;
	CycleData[5][7].PartsSetting=VACUUMVALVE;
	CycleData[5][7].Time=30;
	CycleData[5][8].PartsSetting=VACUUMVALVE;
	CycleData[5][8].Time=30;
	CycleData[5][9].PartsSetting=VACUUMVALVE;
	CycleData[5][9].Time=50;
	CycleData[5][10].PartsSetting=VACUUMVALVE;
	CycleData[5][10].Time=40;


	CycleData[6][1].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA;
	CycleData[6][1].Time=8;
	CycleData[6][2].PartsSetting=VACUUMVALVE;
	CycleData[6][2].Time=44;
	CycleData[6][3].PartsSetting=VENTVALVE+PLASMA;
	CycleData[6][3].Time=7;
	CycleData[6][4].PartsSetting=VACUUMVALVE;
	CycleData[6][4].Time=45;
	CycleData[6][5].PartsSetting=VENTVALVE;
	CycleData[6][5].Time=15;
	CycleData[6][6].PartsSetting=NONE;
	CycleData[6][6].Time=1;
}


void StandardCycle(){
	CycleData[1][1].PartsSetting=VACUUMVALVE;
	CycleData[1][1].Time=3;
	CycleData[1][2].PartsSetting=VACUUMVALVE;
	CycleData[1][2].Time=53;
	CycleData[1][3].PartsSetting=VENTVALVE;
	CycleData[1][3].Time=7;
	CycleData[1][4].PartsSetting=VACUUMVALVE;
	CycleData[1][4].Time=27;
	CycleData[1][5].PartsSetting=VACUUMVALVE+PRESSURE1;
	CycleData[1][5].Time=60;
	CycleData[1][6].PartsSetting=VENTVALVE;
	CycleData[1][6].Time=7;
	CycleData[1][7].PartsSetting=NONE;
	CycleData[1][7].Time=23;
	CycleData[1][8].PartsSetting=NONE;
	CycleData[1][8].Time=30;
	CycleData[1][9].PartsSetting=VACUUMVALVE;
	CycleData[1][9].Time=60;
	CycleData[1][10].PartsSetting=VACUUMVALVE;
	CycleData[1][10].Time=60;
	CycleData[1][11].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][11].Time=50;
	CycleData[1][12].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][12].Time=60;
	CycleData[1][13].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][13].Time=60;
	CycleData[1][14].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][14].Time=60;
	CycleData[1][15].PartsSetting=VACUUMVALVE+PERIPUMP+VAPORIZER;
	CycleData[1][15].Time=40;


	CycleData[2][1].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][1].Time=1;
	CycleData[2][2].PartsSetting=VAPORIZER;
	CycleData[2][2].Time=29;
	CycleData[2][3].PartsSetting=VAPORIZER+PRESSURE3;
	CycleData[2][3].Time=60;
	CycleData[2][4].PartsSetting=VAPORIZER;
	CycleData[2][4].Time=60;
	CycleData[2][5].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][5].Time=1;
	CycleData[2][6].PartsSetting=VAPORIZER;
	CycleData[2][6].Time=29;
	CycleData[2][7].PartsSetting=VAPORIZER;
	CycleData[2][7].Time=60;
	CycleData[2][8].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][8].Time=5;
	CycleData[2][9].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA+VAPORIZER;
	CycleData[2][9].Time=6;
	CycleData[2][10].PartsSetting=VAPORIZER;
	CycleData[2][10].Time=19;
	CycleData[2][11].PartsSetting=VAPORIZER;
	CycleData[2][11].Time=90;
	CycleData[2][12].PartsSetting=VAPORIZER;
	CycleData[2][12].Time=60;
	CycleData[2][13].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[2][13].Time=23;
	CycleData[2][14].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[2][14].Time=60;
	CycleData[2][15].PartsSetting=VENTVALVE+PLASMA+VAPORIZER;
	CycleData[2][15].Time=7;

	CycleData[3][1].PartsSetting=VACUUMVALVE+INJECTIONVALVE+VAPORIZER;
	CycleData[3][1].Time=3;
	CycleData[3][2].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][2].Time=12;
	CycleData[3][3].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][3].Time=15;
	CycleData[3][4].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][4].Time=10;
	CycleData[3][5].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][5].Time=20;
	CycleData[3][6].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][6].Time=30;
	CycleData[3][7].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][7].Time=30;
	CycleData[3][8].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][8].Time=30;
	CycleData[3][9].PartsSetting=VACUUMVALVE+VAPORIZER+PRESSURE2;
	CycleData[3][9].Time=60;
	CycleData[3][10].PartsSetting=VACUUMVALVE+PERIPUMP+VAPORIZER;
	CycleData[3][10].Time=30;

	CycleData[4][1].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][1].Time=1;
	CycleData[4][2].PartsSetting=VAPORIZER;
	CycleData[4][2].Time=29;
	CycleData[4][3].PartsSetting=VAPORIZER+PRESSURE3;
	CycleData[4][3].Time=60;
	CycleData[4][4].PartsSetting=VAPORIZER;
	CycleData[4][4].Time=60;
	CycleData[4][5].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][5].Time=1;
	CycleData[4][6].PartsSetting=VAPORIZER;
	CycleData[4][6].Time=29;
	CycleData[4][7].PartsSetting=VAPORIZER;
	CycleData[4][7].Time=60;
	CycleData[4][8].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][8].Time=5;
	CycleData[4][9].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA+VAPORIZER;
	CycleData[4][9].Time=6;
	CycleData[4][10].PartsSetting=VAPORIZER;
	CycleData[4][10].Time=19;
	CycleData[4][11].PartsSetting=VAPORIZER;
	CycleData[4][11].Time=90;
	CycleData[4][12].PartsSetting=VAPORIZER;
	CycleData[4][12].Time=60;
	CycleData[4][13].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[4][13].Time=23;
	CycleData[4][14].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[4][14].Time=60;
	CycleData[4][15].PartsSetting=VENTVALVE+PLASMA+VAPORIZER;
	CycleData[4][15].Time=7;

	CycleData[5][1].PartsSetting=VACUUMVALVE+INJECTIONVALVE;
	CycleData[5][1].Time=3;
	CycleData[5][2].PartsSetting=VACUUMVALVE;
	CycleData[5][2].Time=12;
	CycleData[5][3].PartsSetting=VACUUMVALVE;
	CycleData[5][3].Time=15;
	CycleData[5][4].PartsSetting=VACUUMVALVE;
	CycleData[5][4].Time=10;
	CycleData[5][5].PartsSetting=VACUUMVALVE;
	CycleData[5][5].Time=20;
	CycleData[5][6].PartsSetting=VACUUMVALVE;
	CycleData[5][6].Time=30;
	CycleData[5][7].PartsSetting=VACUUMVALVE;
	CycleData[5][7].Time=30;
	CycleData[5][8].PartsSetting=VACUUMVALVE;
	CycleData[5][8].Time=30;
	CycleData[5][9].PartsSetting=VACUUMVALVE;
	CycleData[5][9].Time=60;
	CycleData[5][10].PartsSetting=VACUUMVALVE;
	CycleData[5][10].Time=30;

	CycleData[6][1].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA;
	CycleData[6][1].Time=8;
	CycleData[6][2].PartsSetting=VACUUMVALVE;
	CycleData[6][2].Time=74;
	CycleData[6][3].PartsSetting=VENTVALVE+PLASMA;
	CycleData[6][3].Time=7;
	CycleData[6][4].PartsSetting=VACUUMVALVE;
	CycleData[6][4].Time=75;
	CycleData[6][5].PartsSetting=VENTVALVE;
	CycleData[6][5].Time=15;
	CycleData[6][6].PartsSetting=NONE;
	CycleData[6][6].Time=1;
}

void AdvancedCycle(){
	CycleData[1][1].PartsSetting=VACUUMVALVE;
	CycleData[1][1].Time=62;
	CycleData[1][2].PartsSetting=VENTVALVE;
	CycleData[1][2].Time=7;
	CycleData[1][3].PartsSetting=VACUUMVALVE;
	CycleData[1][3].Time=76;
	CycleData[1][4].PartsSetting=VENTVALVE;
	CycleData[1][4].Time=7;
	CycleData[1][5].PartsSetting=VACUUMVALVE+PRESSURE1;
	CycleData[1][5].Time=120;
	CycleData[1][6].PartsSetting=VENTVALVE;
	CycleData[1][6].Time=8;
	CycleData[1][7].PartsSetting=NONE;
	CycleData[1][7].Time=20;
	CycleData[1][8].PartsSetting=NONE;
	CycleData[1][8].Time=60;
	CycleData[1][9].PartsSetting=NONE;
	CycleData[1][9].Time=120;
	CycleData[1][10].PartsSetting=VACUUMVALVE;
	CycleData[1][10].Time=120;
	CycleData[1][11].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][11].Time=80;
	CycleData[1][12].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][12].Time=60;
	CycleData[1][13].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][13].Time=60;
	CycleData[1][14].PartsSetting=VACUUMVALVE+PRESSURE2+VAPORIZER;
	CycleData[1][14].Time=60;
	CycleData[1][15].PartsSetting=VACUUMVALVE+PERIPUMP+VAPORIZER;
	CycleData[1][15].Time=40;


	CycleData[2][1].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][1].Time=1;
	CycleData[2][2].PartsSetting=VAPORIZER;
	CycleData[2][2].Time=29;
	CycleData[2][3].PartsSetting=VAPORIZER+PRESSURE3;
	CycleData[2][3].Time=60;
	CycleData[2][4].PartsSetting=VAPORIZER;
	CycleData[2][4].Time=60;
	CycleData[2][5].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][5].Time=1;
	CycleData[2][6].PartsSetting=VAPORIZER;
	CycleData[2][6].Time=119;
	CycleData[2][7].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[2][7].Time=5;
	CycleData[2][8].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA+VAPORIZER;
	CycleData[2][8].Time=6;
	CycleData[2][9].PartsSetting=VAPORIZER;
	CycleData[2][9].Time=49;
	CycleData[2][10].PartsSetting=VAPORIZER;
	CycleData[2][10].Time=54;
	CycleData[2][11].PartsSetting=VAPORIZER;
	CycleData[2][11].Time=60;
	CycleData[2][12].PartsSetting=VAPORIZER;
	CycleData[2][12].Time=60;
	CycleData[2][13].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[2][13].Time=60;
	CycleData[2][14].PartsSetting=VENTVALVE+PLASMA+VAPORIZER;
	CycleData[2][14].Time=6;
	CycleData[2][15].PartsSetting=VAPORIZER;
	CycleData[2][15].Time=60;

	CycleData[3][1].PartsSetting=VACUUMVALVE+INJECTIONVALVE+VAPORIZER;
	CycleData[3][1].Time=3;
	CycleData[3][2].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][2].Time=12;
	CycleData[3][3].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][3].Time=15;
	CycleData[3][4].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][4].Time=10;
	CycleData[3][5].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][5].Time=20;
	CycleData[3][6].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][6].Time=30;
	CycleData[3][7].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][7].Time=30;
	CycleData[3][8].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[3][8].Time=30;
	CycleData[3][9].PartsSetting=VACUUMVALVE+VAPORIZER+PRESSURE2;
	CycleData[3][9].Time=60;
	CycleData[3][10].PartsSetting=VACUUMVALVE+PERIPUMP+VAPORIZER;
	CycleData[3][10].Time=30;

	CycleData[4][1].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][1].Time=1;
	CycleData[4][2].PartsSetting=VAPORIZER;
	CycleData[4][2].Time=29;
	CycleData[4][3].PartsSetting=VAPORIZER+PRESSURE3;
	CycleData[4][3].Time=60;
	CycleData[4][4].PartsSetting=VAPORIZER;
	CycleData[4][4].Time=60;
	CycleData[4][5].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][5].Time=1;
	CycleData[4][6].PartsSetting=VAPORIZER;
	CycleData[4][6].Time=119;
	CycleData[4][7].PartsSetting=INJECTIONVALVE+VAPORIZER;
	CycleData[4][7].Time=5;
	CycleData[4][8].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA+VAPORIZER;
	CycleData[4][8].Time=6;
	CycleData[4][9].PartsSetting=VAPORIZER;
	CycleData[4][9].Time=49;
	CycleData[4][10].PartsSetting=VAPORIZER;
	CycleData[4][10].Time=54;
	CycleData[4][11].PartsSetting=VAPORIZER;
	CycleData[4][11].Time=60;
	CycleData[4][12].PartsSetting=VAPORIZER;
	CycleData[4][12].Time=60;
	CycleData[4][13].PartsSetting=VACUUMVALVE+VAPORIZER;
	CycleData[4][13].Time=60;
	CycleData[4][14].PartsSetting=VENTVALVE+PLASMA+VAPORIZER;
	CycleData[4][14].Time=6;
	CycleData[4][15].PartsSetting=VAPORIZER;
	CycleData[4][15].Time=60;

	CycleData[5][1].PartsSetting=VACUUMVALVE+INJECTIONVALVE;
	CycleData[5][1].Time=3;
	CycleData[5][2].PartsSetting=VACUUMVALVE;
	CycleData[5][2].Time=12;
	CycleData[5][3].PartsSetting=VACUUMVALVE;
	CycleData[5][3].Time=15;
	CycleData[5][4].PartsSetting=VACUUMVALVE;
	CycleData[5][4].Time=10;
	CycleData[5][5].PartsSetting=VACUUMVALVE;
	CycleData[5][5].Time=20;
	CycleData[5][6].PartsSetting=VACUUMVALVE;
	CycleData[5][6].Time=30;
	CycleData[5][7].PartsSetting=VACUUMVALVE;
	CycleData[5][7].Time=30;
	CycleData[5][8].PartsSetting=VACUUMVALVE;
	CycleData[5][8].Time=30;
	CycleData[5][9].PartsSetting=VACUUMVALVE;
	CycleData[5][9].Time=60;
	CycleData[5][10].PartsSetting=VACUUMVALVE;
	CycleData[5][10].Time=30;


	CycleData[6][1].PartsSetting=VENTVALVE+INJECTIONVALVE+PLASMA;
	CycleData[6][1].Time=8;
	CycleData[6][2].PartsSetting=VACUUMVALVE;
	CycleData[6][2].Time=104;
	CycleData[6][3].PartsSetting=VENTVALVE+PLASMA;
	CycleData[6][3].Time=7;
	CycleData[6][4].PartsSetting=VACUUMVALVE;
	CycleData[6][4].Time=105;
	CycleData[6][5].PartsSetting=VENTVALVE;
	CycleData[6][5].Time=15;
	CycleData[6][6].PartsSetting=NONE;
	CycleData[6][6].Time=1;
}

HAL_StatusTypeDef Flash_Write_Int(uint32_t address, uint32_t value)
{
    HAL_StatusTypeDef status;
    // 플래시 잠금 해제
    HAL_FLASH_Unlock();

    // 필요하다면 페이지 지우기
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = FLASH_SECTOR_11; // 마지막 섹터 번호, MCU에 따라 다름
    EraseInitStruct.NbSectors = 1;
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
    if (status != HAL_OK) return status;

    // 데이터 쓰기
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, value);

    // 플래시 잠금
    HAL_FLASH_Lock();

    return status;
}

uint32_t Flash_Read_Int(uint32_t address)
{
    return *(uint32_t*)address;
}


union fc {
	float fValue;
	char  cValue[4];
};

float char2float(const unsigned char *data)
{
	union fc	temp;
	for(int i = 0; i < 4; i++) {
		temp.cValue[i] = data[i];
	}
	return(temp.fValue);
}

void float2char(float fValue, char *data)
{
	union fc	temp;

	temp.fValue = fValue;
	for(int i = 0; i < 4; i++) {
		data[i] = temp.cValue[i];
	}
}
