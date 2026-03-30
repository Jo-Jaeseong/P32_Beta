/*
 * Process.c
 *
 *  Created on: Aug 10, 2022
 *      Author: CBT
 */

#include "main.h"
#include "stdbool.h"
#include "string.h"

#include "hardware.h"
#include "sensor.h"
#include "peripheral.h"

#define NORMAL		1
#define FACTORYTEST	0

int Select_NORMAL_MODE=1;
int LiquidFlag=0;
int SelfTestMode=0;
int SelfTestInitFlag=0;
int SelfTestProcess=0;

float TestPressure[10]={};
float TestTemp[5]={};

int VacuumTestResult[3];
int HeaterTestResult[5];
int ValveTestResult[4];

int TestResult[4];

int devicealram[14]={};

int StopFlag=0;

int leaktesttime=60;

int VacuumCheck=0;

/*
알람1	장비 전원 정전	장비 전원 차단 됨
알람2	멸균제 사용 기간 지남	장비내 멸균제 삽입 후 60일 지남
알람3	멸균제 제조 만기일 지남	멸균제 제조 후 1년 지남
알람4	멸균제 S/N 오류	멸균제 S/N가 맞지 안음
알람5	챔버 도어 열림	도어가 열림, 스위치 동작 안됨
알람6	멸균제 도어 열림	도어 열림, 스위치 동작 안됨
알람7	멸균제 병 감지 안됨	멸균제 병 없거나, 스위치 동작 안됨
알람8	도어 히터 온도 확인	도어 히터 및 히터 전선, TC 및 TC선 확인
알람9	챔버 히터 온도 확인	챔버 히터 및 히터 전선, TC 및 TC선 확인
알람10	챔버 후면 히터 온도 확인	챔버 후면 히터 및 히터 전선, TC 및 TC선 확인
알람11	기화기 히터 온도 확인	기화기 히터 및 히터 전선, TC 및 TC선 확인
알람12	PM 촉매제 필터 교체	촉매제 설정값 0임, 교체 후 재설정
알람13	PM 헤파필터 교체	헤파필터 설정값 0임, 교체 후 재설정
알람14	PM 플라즈마 전극 교체	플라즈마 전극 설정값 0임, 교체 후 재설정
*/


int TestVacuumValue=20;
int TestLeakValue=5;


int Solenoid_Flag=0;
int LED1=0;
int printmode=0;

float PressureData[300];
float TemperatureData[300];



unsigned int TensecondCounter=0;
unsigned int TensecondCounter2=0;
unsigned int OneMinuteCounter=0;

unsigned int DataCounter=0;
unsigned char ProcessFlag=0 ,CurrentProcess=0, CurrentStep=0, ProcessStep=0;

unsigned int TotalTime;
unsigned int fProcessTime[7];
unsigned int CycleTime=10;
unsigned int FullCycleTime=10;
unsigned int EndTime=0;
unsigned int ProcessTime[7]={};
unsigned int StepTime[21]={};

int Vaccum_Check=0;

struct Process_data_format	CycleData[7][21];

struct data_format p_data;

int Vacuum_Check_Count=0;

int HeaterControlMode=1;

#define SHORT		1
#define STANDARD	2
#define ADVANCED	3

int CycleName=SHORT;
int ProcessNum,StepNum;

int tempcnt=0;

int VaporizerHeaterCheckFlag=0;

int TempErrorCount[4]={};

uint32_t startTick, endTick, duration;
/*
startTick = HAL_GetTick(); // 기능 실행 전 타임스탬프 캡처
// 여기에 측정하고자 하는 기능 혹은 함수 호출
endTick = HAL_GetTick(); // 기능 실행 후 타임스탬프 캡처
duration = endTick - startTick; // 실행 시간(밀리초 단위)
*/


void loop(){
	if(UART_Receive_Flag) {
		UART_Receive_Flag = 0;
		LCD_Process();
	}
	if(Timer_DeliSecond_Flag) {
		Timer_DeliSecond_Flag=0;
		DeliSecondProcess();
	}
	if(Timer_Half_1s_Flag){
		Timer_Half_1s_Flag=0;
		HalfSecondProcess();
	}
	if(Timer_1s_Flag) {
		Timer_1s_Flag=0;
		OneSecondProcess();
	}
	if(EndTimer_Flag) {
		ProcessEndTimer();
	}
}

void DeliSecondProcess(void){
	//ADC 센서
	//GetValue();
	//printf("max:4095, min:3500, data1:%d, data2:%d, data3:%d  \n",data1/10, Pressure, Pressure2);

	//리퀴드 레벨 체크 및 페리펌프 멈춤
	/*
	if(LevelSensor2Check()){
		LiquidFlag=2;
		TurnOffPeristalticPump();
	}
	*/
}

void HalfSecondProcess(void){
	//온도 측정
	Check_Temp(tempcnt);
	if(tempcnt>=4){
		tempcnt=0;
	}
	else{
		tempcnt++;
	}

	//ADC 센서
	//압력 보정
	//ValueFilter();
	DisplayVacuumSensor();

	if(Running_Flag==0){
		DisplayIcons();
	}
	if(Running_Flag==0&&SelfTestMode==0&&VacuumOnOff==0){
		//도어 오픈
		if(DoorOpenFlag==1){
			if(DoorLatchCheck()){
				DoorLatch(1);
				DoorOpenFlag=0;
			}
			else{
				DoorLatch(0);
				DoorOpenFlag=0;
			}

	    }
		else if(DoorOpenFlag==0){
			DoorLatch(0);
			DoorOpenFlag=0;
		}
		if(DoorOpenVentFlag==0){
			DoorOpenFlag=DoorOpenProcess();
		}
		if(DoorOpenVentFlag==1){
			if(DoorOpenVentCnt>=0){
				DoorOpenVentCnt--;
				VentValve(1);
			}
			else{
				DoorOpenVentCnt=0;
				DoorOpenVentFlag=0;
				VentValve(0);
				if(VacuumCheck==1){
					VacuumCheck=0;
				}
				else{
					DoorOpenFlag=1;
				}
	        	DisplayPage(beforepage);
			}
		}
	}


	//리퀴드 레벨 체크 및 페리펌프 멈춤
	if(LevelSensor2Check()){
		LiquidFlag=2;
		TurnOffPeristalticPump();
	}


	if(Select_NORMAL_MODE){
		DisplayNormalValues();
	}
	else{
		if(Running_Flag){
			DisplayProcessTestValues();
		}
	}
	if(SelfTestMode!=0){
    	DisplayIcon(0x30,0x10,(EndTestTimeCounter/10)/60/10);
    	DisplayIcon(0x30,0x20,(EndTestTimeCounter/10)/60%10);
    	DisplayIcon(0x30,0x30,(EndTestTimeCounter/10)%60/10);
    	DisplayIcon(0x30,0x40,(EndTestTimeCounter/10)%60%10);
	}
	if(Running_Flag==1){
		if(StopFlag==0){
			DeviceErrorProcess();
		}
	}
	ReadLCD();
}


void OneSecondProcess(void){
	//임시 테스트
	SleepModeProcess();
	CheckReservationTime();
	if(Running_Flag){
		TensecondCounter++;
		if(ErrorCheckFlag[0]==1){
			if(ErrorCheckFlag[2]==1){
				//deviceerror[2]=DoorHeaterCheck();
				TempErrorCount[0]+=DoorHeaterCheck();
				if(TempErrorCount[0]>=5){//온도 연속 체크
					deviceerror[2]=1;
					TempErrorCount[0]=0;
				}
			}
			if(ErrorCheckFlag[3]==1){
				//deviceerror[3]=ChamberHeaterCheck();
				TempErrorCount[1]+=ChamberHeaterCheck();
				if(TempErrorCount[1]>=5){//온도 연속 체크
					deviceerror[3]=1;
					TempErrorCount[1]=0;
				}
			}
			if(ErrorCheckFlag[4]==1){
				//deviceerror[4]=ChamberBackHeaterCheck();
				TempErrorCount[2]+=ChamberBackHeaterCheck();
				if(TempErrorCount[2]>=5){//온도 연속 체크
					deviceerror[4]=1;
					TempErrorCount[2]=0;
				}
			}
			if(ErrorCheckFlag[5]==1){
				if(VaporizerHeaterCheckFlag==1){
					//deviceerror[5]=VaporizerHeaterCheck();
					TempErrorCount[3]+=VaporizerHeaterCheck();
					if(TempErrorCount[3]>=5){//온도 연속 체크
						deviceerror[5]=1;
						TempErrorCount[3]=0;
					}
				}
			}
		}
		GetData();
		if(TensecondCounter>=10){
			DataCounter++;
			PressureData[DataCounter]=Pressure;
			TemperatureData[DataCounter]=Temperature[1];
			TensecondCounter=0;
        	//DisplayInitTempGraph();
        	//DisplayInitVacuumGraph();
			DisplayTempGraph(DataCounter-1,0);
			DisplayVacuumGraph(DataCounter-1,1);
			ReadLCD();

			//TempErrorCount초기화
			memset(TempErrorCount,0,sizeof(TempErrorCount));
		}
	}

	if(Running_Flag==0){
		DisplaySterilantData();
	}

	//도어 오픈 아이콘
	if(DoorLatchCheck()==1){
		DisplayIcon(0x02, 0x20, 1);
	}
	else{
		DisplayIcon(0x02, 0x20, 0);
	}

	//RS485 센서
	Read_Vacuumsensor();

	if(SelfTestMode==4){
		if(SelfTestProcess==2){
			DisplayPageValue(0x34,0x10,Pressure*10);
		}
		else if(SelfTestProcess==3){
			DisplayPageValue(0x34,0x14,Pressure*10);
		}
		else if(SelfTestProcess==4){
			DisplayPageValue(0x34,0x18,Pressure*10);
		}
		else if(SelfTestProcess==5){
			DisplayPageValue(0x34,0x1C,Pressure*10);
		}
		else if(SelfTestProcess==6){
			DisplayPageValue(0x34,0x20,Pressure*10);
		}

		//
		else if(SelfTestProcess==7){
			DisplayPageValue(0x34,0x24,Pressure*10);
		}
		else if(SelfTestProcess==8){
			DisplayPageValue(0x34,0x28,Pressure*10);
		}
		else if(SelfTestProcess==9){
			DisplayPageValue(0x34,0x2C,Pressure*10);
		}
		else if(SelfTestProcess==10){
			DisplayPageValue(0x34,0x30,Pressure*10);
		}
		else if(SelfTestProcess==11){
			DisplayPageValue(0x34,0x34,Pressure*10);
		}
	}



	if(HeaterControlMode==1){
		HeaterControl();
	}
	else if(HeaterControlMode==2){
		HeaterControl();
	}


	if(currentpage==LCD_ALARM1_PAGE){
		if(DoorLatchCheck()==1){
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
		}
	}
	else if(currentpage==LCD_ALARM3_PAGE){
		if(devicealarm[0]==0){
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
			memset(devicealarm,0,sizeof(devicealarm));
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

			for(int i=1;i<16;i++){
				devicealarm[0]+=devicealarm[i];
			}
		}

	}
	OneMinuteCounter++;
	if(OneMinuteCounter>=60){
		OneMinuteCounter=0;
		OneMinuteProcess();
	}
	TensecondCounter2++;
	if(TensecondCounter2>=10){
		TensecondCounter2=0;
		TensecondProcess();
	}
}

void TensecondProcess(){
	if(Running_Flag!=1){
		SetRTCFromLCD();
		DisplayIcon(0x02,0xA0,today_date.hour/10);
		DisplayIcon(0x02,0xB0,today_date.hour%10);
		DisplayIcon(0x02,0xC0,today_date.minute/10);
		DisplayIcon(0x02,0xD0,today_date.minute%10);
	}
}

void OneMinuteProcess(){

}

void Init_Device(){
	//hardware reset
	Inithardware();

	//ADC 센서
	//InitADC();

	//RS485 센서
	Init_Vacuumsensor();
	//HAL_Delay(1000);
	HAL_Delay(100);
    Read_Flash();
    //Read_Setting_Data_Flash();
    if(Device_First_Boot==1){
    	SleepModeInit();
    	PMinit();
    	Device_First_Boot=2;
    	Write_Flash();
    }
    InitLCD();
	RFIDCheck();
    if(flash_ProcessPowerOffFlag==1){
    	DisplayPage(LCD_POWEROFFCHECK_PAGE);
    }
    else{
    	DisplayFirstPage();
    }
	//HAL_Delay(4000);
}

void Inithardware(){
	DoorHeater(0);
	ChamberHeater(0);
	ChamberBackHeater(0);
	VaporizerHeater(0);
	VacuumPump(0);
	Plasma(0);

	DoorLatch(0);
	VacuumValve(0);
	VentValve(0);
	InjectionValve(0);

	Fan(0);
	PeristalticSpeed();
	TurnOffPeristalticPump();

	HeaterControlMode=1;
}


/*
 * Process
 */

void StartProcess(){
	//SetRTCFromLCD();
	//Data save-related variables
	memset(TempErrorCount,0,sizeof(TempErrorCount));
	memset(PressureData, 0, sizeof(PressureData));
	memset(TemperatureData, 0, sizeof(TemperatureData));
	DataCounter=0;
	TensecondCounter=0;
	PressureData[0]=Pressure;
	TemperatureData[0]=Temperature[1];

	flash_ProcessPowerOffFlag=1;
	Select_NORMAL_MODE=1;
	Running_Flag=1;

	HeaterControlMode=2;

	CurrentProcess=1;
	CurrentStep=1;
	EndTimer_Flag=1;
	Vacuum_Check_Count=0;
	StopFlag=0;
	TotalTime=0;
	LiquidFlag=0;
    memset(deviceerror, 0, sizeof(deviceerror));
    VaporizerHeaterCheckFlag=0;

	Fan(1);
	VacuumValve(1);
	VentValve(0);
	InjectionValve(0);
	Plasma(0);
	PeriPump(0);

	HAL_Delay(500);
	VacuumPump(1);
	p_data.cyclename=CycleName;
	GetStartTime();

	//데일리 카운트 초기화
	if(beforeday!=p_data.day){
		beforeday=p_data.day;
		dailyCount=0;
	}

	DailyCyle_Count();
	TotalCyle_Count();
	DisplayPageValue(0x21,0x20,dailyCount);
	DisplayPageValue(0x21,0x10,totalCount);


	DisplayInitTempGraph();
	DisplayInitVacuumGraph();


	InitData();
	DisplayNormalValues();
	Write_Flash();//테스트중
	ReadLCD();
}

void StopProcess(){
	deviceerror[0]=1;
	deviceerror[1]=1;
	errorcode=1;
	DisplayPage4Char(0x09,0x10,"01  ");
	DisplayPage(LCD_EORROR_POPUP_PAGE);
	ErrorEndProcess();
	StopFlag=1;
}


void FactoryTestStart(){
	//SetRTCFromLCD();
	//Data save-related variables
	memset(TempErrorCount,0,sizeof(TempErrorCount));
	memset(PressureData, 0, sizeof(PressureData));
	memset(TemperatureData, 0, sizeof(TemperatureData));
	DataCounter=0;
	TensecondCounter=0;
	PressureData[0]=Pressure;
	TemperatureData[0]=Temperature[1];

	flash_ProcessPowerOffFlag=1;
	Select_NORMAL_MODE=0;
	Running_Flag=1;
	HeaterControlMode=2;

	CurrentProcess=1;
	CurrentStep=1;
	EndTimer_Flag=1;

	Vacuum_Check_Count=0;
	StopFlag=0;
	TotalTime=0;
	LiquidFlag=0;
    memset(deviceerror, 0, sizeof(deviceerror));
    VaporizerHeaterCheckFlag=0;


	Fan(1);
	VacuumValve(1);
	VentValve(0);
	InjectionValve(0);
	Plasma(0);
	PeriPump(0);

	HAL_Delay(500);
	VacuumPump(1);
	GetStartTime();

	//데일리 카운트 초기화
	if(beforeday!=p_data.day){
		beforeday=p_data.day;
		dailyCount=0;
	}
	TotalCyle_Count();
	DailyCyle_Count();
	DisplayPageValue(0x21,0x10,totalCount);
	DisplayPageValue(0x21,0x20,dailyCount);
	InitData();
	Write_Flash();//테스트중
}

void FactoryTestStop(){
	deviceerror[0]=1;
	deviceerror[1]=1;
	DisplayPage4Char(0x09,0x10,"01  ");
	ErrorEndProcess();
	StopFlag=1;
}
void SelfTestModeStart(int mode){
	SelfTestMode=mode;
	SelfTestProcess=1;
	if(mode==1||mode==4){
		Fan(1);
	}
	EndTimer_Flag=1;
}

void SelfTestModeStop(int mode){
	TestResult[0]=1; //정지
	TestResult[1]=0;
	if(mode==1||mode==4){
		Fan(0);
	}
	if(mode==1){
		if(SelfTestProcess>=1&&SelfTestProcess<20){ //가열시험 혹은 밸브 시험중 중지
			SelfTestProcess=41;
			EndTimer_Flag=1;
			EndTestTimeCounter=10;
		}
		else if(SelfTestProcess>=20&&SelfTestProcess<40){ //진공 시험 중 중지
			SelfTestProcess=50;
			EndTimer_Flag=1;
			EndTestTimeCounter=10;
		}
		else{

		}
	}
	else if(mode==2){
		SelfTestProcess=41;
		EndTimer_Flag=1;
		EndTestTimeCounter=10;
	}
	else if(mode==3){
		SelfTestProcess=41;
		EndTimer_Flag=1;
		EndTestTimeCounter=10;
	}
	else if(mode==4){
		SelfTestProcess=50;
		EndTimer_Flag=1;
		EndTestTimeCounter=10;
	}
	//EndTestTimeCounter=10*10;
}


void Endtime_Check_Process(){
	if(Vaccum_Check==0){

	}
	else if(Vaccum_Check==1){
		if(ErrorCheckFlag[0]==1){
			if(ErrorCheckFlag[6]==1){
				if(PreesureCondition[0]+TestLeakValue<=Pressure){
					deviceerror[6]=1;
				}
			}
		}
	}
	else if(Vaccum_Check==2){
		if(Pressure>=PreesureCondition[1]+TestLeakValue){
			//에러 혹은 재동작(최대 3회)
			if((CycleData[CurrentProcess][CurrentStep+1].PartsSetting&0xC0)==0x80){
				//Pressure2 체크
				//다음 스탭에서 체크시 계속 진행
			}
			else{
				if(CurrentProcess==1){
					if(ErrorCheckFlag[0]==1){
						if(ErrorCheckFlag[7]==1){
							deviceerror[7]=1;
						}
					}
				}
				else if(CurrentProcess==3){
					if(ErrorCheckFlag[0]==1){
						if(ErrorCheckFlag[8]==1){
							deviceerror[8]=1;
						}
					}

				}
			}
		}
		else{
			if((CycleData[CurrentProcess][CurrentStep+1].PartsSetting&0xC0)==0x80){
				if((CycleData[CurrentProcess][CurrentStep+2].PartsSetting&0xC0)==0x80){
					if((CycleData[CurrentProcess][CurrentStep+3].PartsSetting&0xC0)==0x80){
						if((CycleData[CurrentProcess][CurrentStep+4].PartsSetting&0xC0)==0x80){
							CurrentStep=CurrentStep+4;
						}
						else{
							CurrentStep=CurrentStep+3;
						}
					}
					else{
						CurrentStep=CurrentStep+2;
					}
				}
				else{
					CurrentStep++;
				}
			}
			else{

			}
		}
	}
	else if(Vaccum_Check==3){
		if(CurrentProcess==2){
			if(Pressure<=PreesureCondition[2]-TestLeakValue){
				if(ErrorCheckFlag[0]==1){
					if(ErrorCheckFlag[9]==1){
						deviceerror[9]=1;
					}
				}
			}
		}
		else if(CurrentProcess==4){
			if(Pressure<=PreesureCondition[2]-TestLeakValue){
				if(ErrorCheckFlag[0]==1){
					if(ErrorCheckFlag[10]==1){
						deviceerror[10]=1;
					}
				}
			}
		}
	}
	if((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x08)==0x08){
		if(CurrentProcess==1){
			if(ErrorCheckFlag[0]==1){
				if(ErrorCheckFlag[11]==1){
					if(LiquidFlag==1){
						deviceerror[11]=1;
					}
				}
			}

		}
		else if(CurrentProcess==3){
			if(ErrorCheckFlag[0]==1){
				if(ErrorCheckFlag[12]==1){
					if(LiquidFlag==1){
						deviceerror[12]=1;
					}
				}
			}

		}
	}

}
void ErrorEndProcess(){	//에러 후 종료 프로세스
	if(CurrentProcess==1){
		if(LiquidFlag==1){
			CycleTime=ProcessTime[5]+ProcessTime[6];
			CurrentProcess=5;
			CurrentStep=1;
			EndTimer_Flag=1;
			LiquidFlag=0;
		}
		else if(LiquidFlag==2){
			CycleTime=ProcessTime[5]+ProcessTime[6];
			CurrentProcess=5;
			CurrentStep=1;
			EndTimer_Flag=1;
			LiquidFlag=0;
		}
		else{
			if(CycleName==SHORT){
				CurrentProcess=6;
				CurrentStep=5;
				ReadStepTime();
				CycleTime=StepTime[5]+StepTime[6];
				EndTimer_Flag=1;
			}
			else if(CycleName==STANDARD){
				CurrentProcess=6;
				CurrentStep=5;
				ReadStepTime();
				CycleTime=StepTime[5]+StepTime[6];
				EndTimer_Flag=1;
			}
			else{
				CurrentProcess=6;
				CurrentStep=7;
				ReadStepTime();
				CycleTime=StepTime[7]+StepTime[8];
				EndTimer_Flag=1;
			}
		}
		/*
		else{
			CycleTime=ProcessTime[6];
			CurrentProcess=6;
			CurrentStep=1;
			EndTimer_Flag=1;
		}
		*/

	}
	else if(CurrentProcess==2){
		CycleTime=ProcessTime[5]+ProcessTime[6];
		CurrentProcess=5;
		CurrentStep=1;
		EndTimer_Flag=1;
	}
	else if(CurrentProcess==3){
		if(LiquidFlag==1){
			CycleTime=ProcessTime[5]+ProcessTime[6];
			CurrentProcess=5;
			CurrentStep=1;
			EndTimer_Flag=1;
			LiquidFlag=0;
		}
		else if(LiquidFlag==2){
			CycleTime=ProcessTime[5]+ProcessTime[6];
			CurrentProcess=5;
			CurrentStep=1;
			EndTimer_Flag=1;
			LiquidFlag=0;
		}
		else{
			CycleTime=ProcessTime[6];
			CurrentProcess=6;
			CurrentStep=1;
			EndTimer_Flag=1;
		}
	}
	else if(CurrentProcess==4){
		CycleTime=ProcessTime[5]+ProcessTime[6];
		CurrentProcess=5;
		CurrentStep=1;
		EndTimer_Flag=1;
	}
	ReadStepTime();
	ReadProcessTime();
}


void NormalMode(){
	/*
	if(CurrentStep>20){
		CurrentProcess++;
		CurrentStep=1;
		ReadStepTime();
	}

	while(CycleData[CurrentProcess][CurrentStep].Time==0){	//Time 0 Skip
		CurrentStep++;
		if(CurrentStep>20){
			CurrentProcess++;
			CurrentStep=1;
			ReadStepTime();
		}
		if(CurrentProcess>6){
			break;
		}

	}
	*/
	if(CycleData[CurrentProcess][CurrentStep].Time==0){	//Time 0 Skip
		CurrentProcess++;
		CurrentStep=1;
		ReadStepTime();
	}
	if(CurrentProcess>6){	//종료 시 초기화
		EndTimeCounter=10;


		VacuumPump(0);
		Fan(0);
		VacuumValve(0);
		VentValve(0);
		InjectionValve(0);
		Plasma(0);
		PeriPump(0);

		HeaterControlMode=1;

		if(StopFlag==0){
			//정상 종료
		}
		else{
			p_data.status=errorcode;
		}

		//비정상 종료

		//필터 카운트
		CarbonFilter_Count();
		HEPAFilter_Count();
		PlasmaAssy_Count();

		GetEndTime();
		if(autoprintFlag==1){
			CyclePrint();
		}
		SaveCycle();	//여기 확인
		//Write_Setting_Data_Flash();
		HAL_Delay(100);
		//Write_Data_Flash();

		flash_ProcessPowerOffFlag=2;

		Write_Flash();

		Running_Flag=0;
		ProcessNum=1;
		CurrentProcess=1;
		StepNum=1;
		CurrentStep=1;

		if(deviceerror[0]==0){
			DisplayPage(LCD_RESULT_COMPLETE_PAGE);// 수정 요망
		}
		else{
			DisplayPage(LCD_RESULT_ERROR_PAGE);
		}

		ReadStepTime();
		ReadProcessTime();

		DisplayNormalValues();
	}
	else{
		//정상 동작
		ProcessNum=CurrentProcess;
		StepNum=CurrentStep;
		Fan(1);
		VacuumValve((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x01)==0x01);
		VentValve((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x02)==0x02);
		InjectionValve((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x04)==0x04);
		if((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x04)==0x04){
			LiquidFlag=0;
		}
		if((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x08)==0x08){
			LiquidFlag=1;
			PeriPump(1);
			//볼륨 카은트
			CurrentRFIDData.volume--;
			if(CurrentRFIDData.volume<=0){
				CurrentRFIDData.volume=0;
			}
			FlashRFIDData[CurrentRFIDIndex].volume=CurrentRFIDData.volume;
			DisplaySterilantData();
		}
		else{
			PeriPump(0);
		}
		Plasma((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x10)==0x10);

		if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x00){
			Vaccum_Check=0;
		}
		else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x40){
			Vaccum_Check=1;
		}
		else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x80){
			Vaccum_Check=2;
		}
		else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0xC0){
			Vaccum_Check=3;
		}

		if(CurrentProcess==5){
			HeaterControlMode=1;
		}
		else if(CurrentProcess==6){
			HeaterControlMode=1;
		}

		if((CycleData[ProcessNum][StepNum].PartsSetting&0x20)==0x20){
			//기화기 온도 체크
			VaporizerHeaterCheckFlag=1;
		}
		else{
			VaporizerHeaterCheckFlag=0;
		}

		EndTimeCounter = CycleData[CurrentProcess][CurrentStep].Time*10;;//공정 시간(초)
		ReadLCD();
	}
}

void FactoryTestMode(){
	/*
	if(CurrentStep>20){
		CurrentProcess++;
		CurrentStep=1;
		ReadStepTime();
	}

	while(CycleData[CurrentProcess][CurrentStep].Time==0){	//Time 0 Skip
		CurrentStep++;
		if(CurrentStep>20){
			CurrentProcess++;
			CurrentStep=1;
			ReadStepTime();
		}
		if(CurrentProcess>6){
			break;
		}

	}
	*/
	if(CycleData[CurrentProcess][CurrentStep].Time<=0){	//Time 0 Skip
		CurrentProcess++;
		CurrentStep=1;
		ReadStepTime();
	}
	if(CurrentProcess>6){	//종료 시 초기화
		EndTimeCounter=10;
		Running_Flag=0;
		ProcessNum=1;
		CurrentProcess=1;
		StepNum=1;
		CurrentStep=1;

		VacuumPump(0);
		Fan(0);
		VacuumValve(0);
		VentValve(0);
		InjectionValve(0);
		Plasma(0);
		PeriPump(0);

		HeaterControlMode=1;

		if(StopFlag==0){			//정상 종료
			p_data.status=11;
		}
		else{
			p_data.status=errorcode;
		}

		CarbonFilter_Count();
		HEPAFilter_Count();
		PlasmaAssy_Count();


		GetEndTime();
		if(autoprintFlag==1){
			CyclePrint();
		}
		SaveCycle();	//여기 확인

		flash_ProcessPowerOffFlag=2;

		Write_Flash();


		ReadStepTime();
		ReadProcessTime();

		DisplayProcessTestValues();
		DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE);

	}
	else{	//정상 동작
		if(CurrentStep>10){
			DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST4_PAGE);
		}
		else{
			DisplayPage(LCD_FACTORY_TESTMODE_PROCESSTEST3_PAGE);
		}

		ProcessNum=CurrentProcess;
		StepNum=CurrentStep;
		Fan(1);
		VacuumValve((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x01)==0x01);
		VentValve((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x02)==0x02);
		InjectionValve((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x04)==0x04);
		if((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x08)==0x08){
			LiquidFlag=1;
			PeriPump(1);
			//볼륨 카은트
			CurrentRFIDData.volume--;
			if(CurrentRFIDData.volume<=0){
				CurrentRFIDData.volume=0;
			}
			FlashRFIDData[CurrentRFIDIndex].volume=CurrentRFIDData.volume;
			DisplaySterilantData();
		}
		else{
			PeriPump(0);
		}
		Plasma((CycleData[CurrentProcess][CurrentStep].PartsSetting&0x10)==0x10);


		if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x00){
			Vaccum_Check=0;
		}
		else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x40){
			Vaccum_Check=1;
		}
		else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0x80){
			Vaccum_Check=2;
		}
		else if((CycleData[ProcessNum][StepNum].PartsSetting&0xC0)==0xC0){
			Vaccum_Check=3;
		}

		if(CurrentProcess==5){
			HeaterControlMode=1;
		}
		else if(CurrentProcess==6){
			HeaterControlMode=1;
		}

		if((CycleData[ProcessNum][StepNum].PartsSetting&0x20)==0x20){
			//기화기 온도 체크
			VaporizerHeaterCheckFlag=1;
		}
		else{
			VaporizerHeaterCheckFlag=0;
		}

		EndTimeCounter = CycleData[CurrentProcess][CurrentStep].Time*10;;//공정 시간(초)
	}
}

void TotalSelfTest(){
	if(SelfTestProcess==1){
		//테스트 Start
		//초기화
		EndTestTimeCounter=(leaktesttime*10+20+5)*10;
		memset(HeaterTestResult, 0, sizeof(HeaterTestResult));
		memset(ValveTestResult, 0, sizeof(ValveTestResult));
		memset(VacuumTestResult, 0, sizeof(VacuumTestResult));
		memset(TestResult, 0, sizeof(TestResult));

		memset(TestTemp, 0, sizeof(TestTemp));
		memset(TestPressure, 0, sizeof(TestPressure));

		//다음프로세스
		SelfTestProcess=2;

		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==2){
		//도어 히터 테스트
		TestTemp[0]=Temperature[0];
		if((DoorSettingTemp[0]-TestTempErrorValue)>TestTemp[0]){
			//불량
			HeaterTestResult[0]=2;
			HeaterTestResult[1]=2;
		}
		else{
			//정상
			HeaterTestResult[1]=1;
		}

		//다음프로세스
		SelfTestProcess=3;

		//테스트 시간
		EndTimeCounter=10;
	}
	else if(SelfTestProcess==3){
		//챔버 히터 테스트
		TestTemp[1]=Temperature[1];
		if((ChamberSettingTemp[0]-TestTempErrorValue)>TestTemp[1]){
			//불량
			HeaterTestResult[0]=2;
			HeaterTestResult[2]=2;
		}
		else{
			//정상
			HeaterTestResult[2]=1;
		}

		//다음 프로세스
		SelfTestProcess=4;
		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==4){
		//챔버 후면 히터 테스트
		TestTemp[2]=Temperature[2];
		if((ChamberBackSettingTemp[0]-TestTempErrorValue)>TestTemp[2]){
			//불량
			HeaterTestResult[0]=2;
			HeaterTestResult[3]=2;
		}
		else{
			//정상
			HeaterTestResult[3]=1;
		}

		//다음 프로세스
		SelfTestProcess=5;

		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==5){
		//기화기 히터 테스트
		TestTemp[3]=Temperature[3];
		if((VaporizerSettingTemp[0]-TestTempErrorValue)>TestTemp[3]){
			//불량
			HeaterTestResult[0]=2;
			HeaterTestResult[4]=2;
		}
		else{
			//정상
			HeaterTestResult[4]=1;
		}

		//히터 테스트 결과 확인
		if(HeaterTestResult[0]==2){
			//불량
			TestResult[1]=2;

			//다음 프로세스
			SelfTestProcess=40;

			//테스트 시간
			EndTimeCounter=1*10;
		}
		else{
			//정상
			HeaterTestResult[0]=1;
			//다음 프로세스
			SelfTestProcess=10;

			//테스트 시간
			EndTimeCounter=1*10;
		}
	}

	//밸브 테스트
	else if(SelfTestProcess==10){
		//1.주입 밸브 동작
		VacuumValve(0);
		VentValve(0);
		InjectionValve(1);

		//다음프로세스
		SelfTestProcess=11;
		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==11){
		//1.주입 밸브 체크
		if(ValveCheck()!=1){
			ValveTestResult[0]=2;
			ValveTestResult[1]=2;
		}
		else{
			ValveTestResult[1]=1;
		}

		//2.밴트 밸브 동작
		VacuumValve(0);
		VentValve(1);
		InjectionValve(0);

		//다음프로세스
		SelfTestProcess=12;
		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==12){
		//2.밴트 밸브 체크
		if(ValveCheck()!=1){
			ValveTestResult[0]=2;
			ValveTestResult[2]=2;
		}
		else{
			ValveTestResult[2]=1;
		}

		//3.진공 밸브 동작
		VacuumValve(1);
		VentValve(0);
		InjectionValve(0);

		//다음프로세스
		SelfTestProcess=13;
		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==13){
		//3.진공 밸브 체크
		if(ValveCheck()!=1){
			ValveTestResult[0]=2;
			ValveTestResult[3]=2;
		}
		else{
			ValveTestResult[3]=1;
		}

		//4.밸브 동작 종료
		VacuumValve(0);
		VentValve(0);
		InjectionValve(0);

		//5. 밸브 시험 결과 확인
		if(ValveTestResult[0]==2){
			//불량
			TestResult[1]=2;

			//다음 프로세스
			SelfTestProcess=40;

			//테스트 시간
			EndTimeCounter=1*10;
		}
		else{
			//정상
			ValveTestResult[0]=1;

			//다음 프로세스
			SelfTestProcess=20;
			//테스트 시간
			EndTimeCounter=1*10;
		}
	}


	//진공 시험
	if(SelfTestProcess==20){
		//진공시험 시간
		EndTestTimeCounter=(leaktesttime*10+12)*10;
		SelfTestProcess=21;
		VacuumPump(1);
		VacuumValve(1);
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==21){
		TestPressure[0]=Pressure;
		SelfTestProcess=22;
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==22){
		TestPressure[1]=Pressure;
		SelfTestProcess=23;
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==23){
		TestPressure[2]=Pressure;
		SelfTestProcess=24;
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==24){
		TestPressure[3]=Pressure;
		SelfTestProcess=25;
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==25){
		//진공 시험 결과 종료
		VacuumPump(0);
		VacuumValve(0);
		TestPressure[4]=Pressure;

		//진공 시험 결과 결과
		if(TestPressure[4]>TestVacuumValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[1]=2;
			TestResult[1]=2;

			//정지 프로세스
			SelfTestProcess=35;
			EndTimeCounter=1*10;
		}
		else{
			//정상
			VacuumTestResult[0]=1;
			VacuumTestResult[1]=1;

			//계속 테스트
			SelfTestProcess=30;
			EndTimeCounter=leaktesttime*10;
		}
	}

	//리크 시험
	else if(SelfTestProcess==30){
		TestPressure[5]=Pressure;
		//리크 시험 1차
		if((TestPressure[5]-TestPressure[4])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//정지 프로세스
			SelfTestProcess=35;
			EndTimeCounter=1*10;
		}
		else{
			if(TestPressure[5]==0||TestPressure[5]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//정지 프로세스
				SelfTestProcess=35;
				EndTimeCounter=1*10;
			}
			else{
				//정상
				SelfTestProcess=31;
				EndTimeCounter=leaktesttime*10;
			}
		}
	}
	else if(SelfTestProcess==31){
		TestPressure[6]=Pressure;
		//리크 시험 2차
		if((TestPressure[6]-TestPressure[5])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//정지 프로세스
			SelfTestProcess=35;
			EndTimeCounter=1*10;
		}
		else{
			if(TestPressure[6]==0||TestPressure[6]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//정지 프로세스
				SelfTestProcess=35;
				EndTimeCounter=1*10;
			}
			else{
				//정상
				SelfTestProcess=32;
				EndTimeCounter=leaktesttime*10;
			}
		}
	}
	else if(SelfTestProcess==32){
		TestPressure[7]=Pressure;
		//리크 시험 3차
		if((TestPressure[7]-TestPressure[6])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//정지 프로세스
			SelfTestProcess=35;
			EndTimeCounter=1*10;
		}
		else{
			if(TestPressure[7]==0||TestPressure[7]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//정지 프로세스
				SelfTestProcess=35;
				EndTimeCounter=1*10;
			}
			else{
				//정상
				SelfTestProcess=33;
				EndTimeCounter=leaktesttime*10;
			}
		}
	}
	else if(SelfTestProcess==33){
		TestPressure[8]=Pressure;
		//리크 시험 4차
		if((TestPressure[8]-TestPressure[7])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//정지 프로세스
			SelfTestProcess=35;
			EndTimeCounter=1*10;
		}
		else{
			if(TestPressure[8]==0||TestPressure[8]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//정지 프로세스
				SelfTestProcess=35;
				EndTimeCounter=1*10;
			}
			else{
				//정상
				SelfTestProcess=34;
				EndTimeCounter=leaktesttime*10;
			}
		}
	}
	else if(SelfTestProcess==34){
		TestPressure[9]=Pressure;
		//리크 시험 5차
		if((TestPressure[9]-TestPressure[8])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//정지 프로세스
			SelfTestProcess=35;
			EndTimeCounter=1*10;
		}
		else{
			if(TestPressure[9]==0||TestPressure[9]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//정지 프로세스
				SelfTestProcess=35;
				EndTimeCounter=1*10;
			}
			else{
				//정상
				VacuumTestResult[0]=1;
				VacuumTestResult[2]=1;
				SelfTestProcess=35;
				EndTimeCounter=1*10;
			}
		}
	}
	else if(SelfTestProcess==35){
			//밴트
			VentValve(1);
			SelfTestProcess=36;
			EndTimeCounter=14*10;
	}
	else if(SelfTestProcess==36){//종료
		VentValve(0);
		Fan(0);
		if(HeaterTestResult[0]==1&&ValveTestResult[0]==1&&VacuumTestResult[0]==1){
			TestResult[1]=1;
		}
		else{
			TestResult[1]=2;
		}
    	SelfTestMode=0;
    	EndTimeCounter=1*10;
    	DisplayPage(LCD_USER_TOTALTEST_COMPLETE_PAGE);
	}
	else if(SelfTestProcess==40){//가열, 밸브 정지(불량 종료)
		TestResult[1]=2;
		Fan(0);
		VacuumValve(0);
		VentValve(0);
		InjectionValve(0);
		VacuumPump(0);
		VacuumValve(0);
		VentValve(0);

		SelfTestMode=0;
		EndTimeCounter=1*10;

		DisplayPage(LCD_USER_TOTALTEST_COMPLETE_PAGE);
	}
	else if(SelfTestProcess==41){//가열, 밸브 정지(정지 종료)
		TestResult[1]=0;
		Fan(0);
		VacuumValve(0);
		VentValve(0);
		InjectionValve(0);
		VacuumPump(0);
		VacuumValve(0);
		VentValve(0);

		SelfTestMode=0;
		EndTimeCounter=1*10;

		DisplayPage(LCD_USER_TOTALTEST_COMPLETE_PAGE);
	}
	else if(SelfTestProcess==50){//진공 정지
		Fan(0);
		EndTestTimeCounter=15*10;
		TestResult[1]=0;
		VacuumPump(0);
		VacuumValve(1);
		VentValve(1);
		SelfTestProcess=51;
		EndTimeCounter=14*10;
	}
	else if(SelfTestProcess==51){
		Fan(0);
		VacuumPump(0);
		VacuumValve(0);
		VentValve(0);

		SelfTestMode=0;
		EndTimeCounter=10;

		DisplayPage(LCD_USER_TOTALTEST_COMPLETE_PAGE);
	}

	Display31page();
}


void HeaterTest(){
	if(SelfTestProcess==1){
		//테스트 Start
		//초기화
		EndTestTimeCounter=5*10;
		memset(HeaterTestResult, 0, sizeof(HeaterTestResult));
		memset(TestResult, 0, sizeof(TestResult));

		memset(TestTemp, 0, sizeof(TestTemp));

		//다음프로세스
		SelfTestProcess=2;

		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==2){
		//도어 히터 테스트
		TestTemp[0]=Temperature[0];
		if((DoorSettingTemp[0]-TestTempErrorValue)>TestTemp[0]){
			//불량
			HeaterTestResult[0]=2;
			HeaterTestResult[1]=2;
		}
		else{
			//정상
			HeaterTestResult[1]=1;
		}

		//다음프로세스
		SelfTestProcess=3;

		//테스트 시간
		EndTimeCounter=10;
	}
	else if(SelfTestProcess==3){
		//챔버 히터 테스트
		TestTemp[1]=Temperature[1];
		if((ChamberSettingTemp[0]-TestTempErrorValue)>TestTemp[1]){
			//불량
			HeaterTestResult[0]=2;
			HeaterTestResult[2]=2;
		}
		else{
			//정상
			HeaterTestResult[2]=1;
		}

		//다음 프로세스
		SelfTestProcess=4;
		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==4){
		//챔버 후면 히터 테스트
		TestTemp[2]=Temperature[2];
		if((ChamberBackSettingTemp[0]-TestTempErrorValue)>TestTemp[2]){
			//불량
			HeaterTestResult[0]=2;
			HeaterTestResult[3]=2;
		}
		else{
			//정상
			HeaterTestResult[3]=1;
		}

		//다음 프로세스
		SelfTestProcess=5;

		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==5){
		//기화기 히터 테스트
		TestTemp[3]=Temperature[3];
		if((VaporizerSettingTemp[0]-TestTempErrorValue)>TestTemp[3]){
			//불량
			HeaterTestResult[0]=2;
			HeaterTestResult[4]=2;
		}
		else{
			//정상
			HeaterTestResult[4]=1;
		}

		//히터 테스트 결과 확인
		if(HeaterTestResult[0]==2){
			//불량
			TestResult[1]=2;

			//다음 프로세스
			SelfTestMode=0;

			//테스트 시간
			EndTimeCounter=1*10;
			DisplayPage(LCD_USER_HEATINGTEST_COMPLETE_PAGE);
		}
		else{
			//정상
			HeaterTestResult[0]=1;
			TestResult[1]=1;
			//다음 프로세스
			SelfTestMode=0;

			//테스트 시간
			EndTimeCounter=1*10;
			DisplayPage(LCD_USER_HEATINGTEST_COMPLETE_PAGE);
		}
	}
	else if(SelfTestProcess==41){//가열, 밸브 정지(정지 종료)
		TestResult[1]=0;
		Fan(0);

		SelfTestMode=0;
		EndTimeCounter=1*10;

		DisplayPage(LCD_USER_HEATINGTEST_COMPLETE_PAGE);
	}
	Display32page();
}

void ValveTest(){
	if(SelfTestProcess==1){
		EndTestTimeCounter=5*10;
		memset(ValveTestResult, 0, sizeof(ValveTestResult));
		memset(TestResult, 0, sizeof(TestResult));

		//1.주입 밸브 동작
		VacuumValve(0);
		VentValve(0);
		InjectionValve(1);

		//다음프로세스
		SelfTestProcess=2;
		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==2){
		//1.주입 밸브 체크
		if(ValveCheck()!=1){
			ValveTestResult[0]=2;
			ValveTestResult[1]=2;
		}
		else{
			ValveTestResult[1]=1;
		}

		//2.밴트 밸브 동작
		VacuumValve(0);
		VentValve(1);
		InjectionValve(0);

		//다음프로세스
		SelfTestProcess=3;
		//테스트 시간
		EndTimeCounter=1*10;
	}
	else if(SelfTestProcess==3){
		//2.밴트 밸브 체크
		if(ValveCheck()!=1){
			ValveTestResult[0]=2;
			ValveTestResult[2]=2;
		}
		else{
			ValveTestResult[2]=1;
		}

		//3.진공 밸브 동작
		VacuumValve(1);
		VentValve(0);
		InjectionValve(0);

		//다음프로세스
		SelfTestProcess=4;
		//테스트 시간
		EndTimeCounter=2*10;
	}
	else if(SelfTestProcess==4){
		//3.진공 밸브 체크
		if(ValveCheck()!=1){
			ValveTestResult[0]=2;
			ValveTestResult[3]=2;
		}
		else{
			ValveTestResult[3]=1;
		}

		//4.밸브 동작 종료
		VacuumValve(0);
		VentValve(0);
		InjectionValve(0);

		//5. 밸브 시험 결과 확인
		if(ValveTestResult[0]==2){
			//불량
			TestResult[1]=2;

			//다음 프로세스
			SelfTestMode=0;

			//테스트 시간
			EndTimeCounter=1*10;
			DisplayPage(LCD_USER_PARTTEST_COMPLETE_PAGE);
		}
		else{
			//정상
			ValveTestResult[0]=1;
			TestResult[1]=1;

			//다음 프로세스
			SelfTestMode=0;

			//테스트 시간
			EndTimeCounter=1*10;
			DisplayPage(LCD_USER_PARTTEST_COMPLETE_PAGE);
		}
	}
	else if(SelfTestProcess==41){//가열, 밸브 정지(정지 종료)
		TestResult[1]=0;
		Fan(0);

		SelfTestMode=0;
		EndTimeCounter=1*10;

		DisplayPage(LCD_USER_PARTTEST_COMPLETE_PAGE);
	}
	Display33page();
}

void VacuumTest(){
	//진공 시험
	if(SelfTestProcess==1){
		//진공시험 시간
		EndTestTimeCounter=(leaktesttime*10+10+5)*10;
		SelfTestProcess=2;
		VacuumPump(1);
		VacuumValve(1);
		DisplayPage4Char(0x34,0x40,"TEST");	//결과값
		DisplayPage4Char(0x34,0x44,"TEST");	//결과값
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==2){
		TestPressure[0]=Pressure;
		SelfTestProcess=3;
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==3){
		TestPressure[1]=Pressure;
		SelfTestProcess=4;
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==4){
		TestPressure[2]=Pressure;
		SelfTestProcess=5;
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==5){
		TestPressure[3]=Pressure;
		SelfTestProcess=6;
		EndTimeCounter=leaktesttime*10;
	}
	else if(SelfTestProcess==6){
		//진공 시험 결과 종료
		VacuumPump(0);
		VacuumValve(0);
		TestPressure[4]=Pressure;

		//진공 시험 결과 결과
		if(TestPressure[4]>TestVacuumValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[1]=2;
			TestResult[1]=2;

			//계속 시험
			SelfTestProcess=7;
			EndTimeCounter=leaktesttime*10;
		}
		else{
			//정상
			VacuumTestResult[0]=1;
			VacuumTestResult[1]=1;

			//계속 시험
			SelfTestProcess=7;
			EndTimeCounter=leaktesttime*10;
		}
	}

	//리크 시험
	else if(SelfTestProcess==7){
		TestPressure[5]=Pressure;
		//리크 시험 1차
		if((TestPressure[5]-TestPressure[4])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//계속 시험
			SelfTestProcess=8;
			EndTimeCounter=leaktesttime*10;
		}
		else{
			if(TestPressure[5]==0||TestPressure[5]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//계속 시험
				SelfTestProcess=8;
				EndTimeCounter=leaktesttime*10;
			}
			else{
				//정상
				SelfTestProcess=8;
				EndTimeCounter=leaktesttime*10;
			}
		}
	}
	else if(SelfTestProcess==8){
		TestPressure[6]=Pressure;
		//리크 시험 2차
		if((TestPressure[6]-TestPressure[5])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//계속 시험
			SelfTestProcess=9;
			EndTimeCounter=leaktesttime*10;
		}
		else{
			if(TestPressure[6]==0||TestPressure[6]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//계속 시험
				SelfTestProcess=9;
				EndTimeCounter=leaktesttime*10;
			}
			else{
				//정상
				SelfTestProcess=9;
				EndTimeCounter=leaktesttime*10;
			}
		}
	}
	else if(SelfTestProcess==9){
		TestPressure[7]=Pressure;
		//리크 시험 3차
		if((TestPressure[7]-TestPressure[6])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//계속 시험
			SelfTestProcess=10;
			EndTimeCounter=leaktesttime*10;
		}
		else{
			if(TestPressure[7]==0||TestPressure[7]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//계속 시험
				SelfTestProcess=10;
				EndTimeCounter=leaktesttime*10;
			}
			else{
				//정상
				SelfTestProcess=10;
				EndTimeCounter=leaktesttime*10;
			}
		}
	}
	else if(SelfTestProcess==10){
		TestPressure[8]=Pressure;
		//리크 시험 4차
		if((TestPressure[8]-TestPressure[7])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//계속 시험
			SelfTestProcess=11;
			EndTimeCounter=leaktesttime*10;
		}
		else{
			if(TestPressure[8]==0||TestPressure[8]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//계속 시험
				SelfTestProcess=11;
				EndTimeCounter=leaktesttime*10;
			}
			else{
				//정상
				SelfTestProcess=11;
				EndTimeCounter=leaktesttime*10;
			}
		}
	}
	else if(SelfTestProcess==11){
		TestPressure[9]=Pressure;
		//리크 시험 5차
		if((TestPressure[9]-TestPressure[8])>TestLeakValue){
			//불량
			VacuumTestResult[0]=2;
			VacuumTestResult[2]=2;
			TestResult[1]=2;

			//계속 시험
			SelfTestProcess=12;
			EndTimeCounter=1*10;
		}
		else{
			if(TestPressure[9]==0||TestPressure[9]>=700){
				//불량
				VacuumTestResult[0]=2;
				VacuumTestResult[2]=2;
				TestResult[1]=2;

				//계속 시험
				SelfTestProcess=12;
				EndTimeCounter=1*10;
			}
			else{
				//정상
				if(VacuumTestResult[2]!=2){
					VacuumTestResult[2]=1;
				}
				if(VacuumTestResult[1]==1&&VacuumTestResult[2]==1){
					VacuumTestResult[0]=1;
					TestResult[1]=1;
				}
				SelfTestProcess=12;
				EndTimeCounter=1*10;
			}
		}
	}
	else if(SelfTestProcess==12){
			//밴트
			VacuumPump(0);
			VacuumValve(1);
			VentValve(1);
			SelfTestProcess=13;
			EndTimeCounter=13*10;
	}
	else if(SelfTestProcess==13){//종료
		Fan(0);
		VacuumPump(0);
		VacuumValve(0);
		VentValve(0);

    	SelfTestMode=0;
    	EndTimeCounter=1*10;
    	DisplayPage(LCD_USER_VACUUMTEST_COMPLETE_PAGE);
	}


	else if(SelfTestProcess==50){//진공 정지
		Fan(0);
		EndTestTimeCounter=15*10;
		TestResult[1]=0;
		VacuumPump(0);
		VacuumValve(1);
		VentValve(1);
		SelfTestProcess=51;
		EndTimeCounter=14*10;
	}
	else if(SelfTestProcess==51){
		Fan(0);
		VacuumPump(0);
		VacuumValve(0);
		VentValve(0);

		SelfTestMode=0;
		EndTimeCounter=10;

		DisplayPage(LCD_USER_VACUUMTEST_COMPLETE_PAGE);
	}

	Display34page();
}


void ProcessEndTimer(void){
	if(Running_Flag) {
		if(Select_NORMAL_MODE==1){
			NormalMode();
		}
		else{
			FactoryTestMode();
		}
	}

	//종합 테스트
	if(SelfTestMode==1) {
		TotalSelfTest();
	}
	//히터 테스트
	else if(SelfTestMode==2) {
		HeaterTest();
	}
	//벨브 테스트
	else if(SelfTestMode==3) {
		ValveTest();
	}
	//진공 테스트
	else if(SelfTestMode==4) {
		VacuumTest();
	}
	EndTimer_Flag = 0;
}



void GetStartTime(){
	SetRTCFromLCD();
	Get_RTC_Time( &p_data.year, &p_data.month, &p_data.day, &p_data.week, &p_data.start_hour, &p_data.start_minute, &p_data.start_second);
}
void GetEndTime(){
	SetRTCFromLCD();
	unsigned char year,month,day,week;
	Get_RTC_Time( &year, &month, &day, &week, &p_data.end_hour, &p_data.end_minute, &p_data.end_second);
}

void GetData(){
	if(Temperature[1]>p_data.tempmax[CurrentProcess]){
		p_data.tempmax[CurrentProcess]=Temperature[1];	//Max Temperature
	}
	if(Pressure>p_data.pressuremax[CurrentProcess]){
		p_data.pressuremax[CurrentProcess]=Pressure;	//Max Pressure
	}
	if(Pressure<p_data.pressuremin[CurrentProcess]){
		p_data.pressuremin[CurrentProcess]=Pressure;	//Max Pressure
	}
}
void InitData(){
	for(int i=1;i<7;i++){
		p_data.tempmax[i]=0;
		p_data.pressuremax[i]=0;
		p_data.pressuremin[i]=760;
		fProcessTime[i]=0;
	}
	errorcode=0;
}

int calculateElapsedDays(int current_year, int current_month, int current_day, int open_year, int open_month, int open_day) {
    int elapsed_days = 0;
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Calculate the number of elapsed days since the open date
    while (open_year < current_year ||
           (open_year == current_year && open_month < current_month) ||
           (open_year == current_year && open_month == current_month && open_day < current_day)) {
        elapsed_days++;
        open_day++;

        if (open_day > days_in_month[open_month]) {
            open_day = 1;
            open_month++;
        }

        if (open_month > 12) {
            open_month = 1;
            open_year++;
        }
    }
    return elapsed_days;
}

int check_expiry() {
    // BCD를 바이너리 값으로 변환
    unsigned int year = today_date.year;
    unsigned int month = today_date.month;
    unsigned int day = today_date.day;

    // 연도 비교
    if (CurrentRFIDData.expiry_year < year) {
        return 0; // 만료됨
    } else if (CurrentRFIDData.expiry_year > year) {
        return 1; // 만료 안됨
    } else { // 연도가 같을 경우
        // 월 비교
        if (CurrentRFIDData.expiry_month < month) {
            return 0; // 만료됨
        } else if (CurrentRFIDData.expiry_month > month) {
            return 1; // 만료 안됨
        } else { // 월도 같을 경우
            // 일 비교
            if (CurrentRFIDData.expiry_day < day) {
                return 0; // 만료됨
            } else {
                return 1; // 만료 안됨 (같거나 이후)
            }
        }
    }
}
