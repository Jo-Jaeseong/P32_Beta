/*
 * lcd.h
 *
 *  Created on: Oct 25, 2022
 *      Author: CBT
 */
#ifndef SRC_PERIPHERAL_LCD_H_
#define SRC_PERIPHERAL_LCD_H_

#define LCD_FIRST_PAGE									0

#define LCD_CYCLESELECT_PAGE							2
#define LCD_ALARM1_PAGE									3
#define LCD_STANDBY_PAGE								4
#define LCD_RUNNING_PAGE								5
#define LCD_RESULT_COMPLETE_PAGE						6
#define LCD_SELECT_STOP_PAGE							7
#define LCD_EORROR_POPUP_PAGE							8
#define LCD_EORROR_WAIT_PAGE							9
#define LCD_RESULT_ERROR_PAGE							10
#define LCD_MONITOR_PAGE								11

#define LCD_RESERVATION_PAGE							12
#define LCD_RESERVATION_RUNNING_PAGE					13


#define LCD_PREALARM_PAGE								14
#define LCD_ALARM2_PAGE									15
#define LCD_ALARM3_PAGE									16
#define LCD_ALARM4_PAGE									17


#define LCD_DOOROPENMESSAGE_PAGE						18
#define LCD_LOGOUT_POPUP								19

#define LCD_SETTING_SELECT_PAGE							20

//유저 20~
#define LCD_INFO_INFORMATION_PAGE						21
#define LCD_INFO_STERILANT_PAGE							22
#define LCD_INFO_STERILANT_VIAL_PAGE						122
#define LCD_INFO_HISTORY_PAGE							23
#define LCD_USER_SETTING_PAGE							24

#define LCD_USER_SLEEPMODE_SETTING_PAGE					25

//유지보수 30~
#define LCD_USER_TOTALTEST_PAGE							31
#define LCD_USER_HEATINGTEST_PAGE						32
#define LCD_USER_PARTTEST_PAGE							33
#define LCD_USER_VACUUMTEST_PAGE						34

#define LCD_USER_TOTALTEST_RUNNING_PAGE					36
#define LCD_USER_HEATINGTEST_RUNNING_PAGE				37
#define LCD_USER_PARTTEST_RUNNING_PAGE					38
#define LCD_USER_VACUUMTEST_RUNNING_PAGE				39

#define LCD_USER_TOTALTEST_COMPLETE_PAGE				87
#define LCD_USER_HEATINGTEST_COMPLETE_PAGE				88
#define LCD_USER_PARTTEST_COMPLETE_PAGE					89
#define LCD_USER_VACUUMTEST_COMPLETE_PAGE				90

//관리자 40~
#define LCD_ADMIN_PMSCHEDULE_PAGE						41
#define LCD_ADMIN_PARTSTEST_PAGE						42
#define LCD_ADMIN_ALARM1_PAGE							43
#define LCD_ADMIN_ALARM2_PAGE							85
#define LCD_ADMIN_ERROR1_PAGE							44
#define LCD_ADMIN_ERROR2_PAGE							86


//개발자 50~
#define LCD_FACTORY_INFOSETTING_PAGE					51
#define LCD_FACTORY_FUNCSETTING_PAGE					52
#define LCD_FACTORY_CALIBRATION_PAGE					53
#define LCD_FACTORY_PROCESSSETTING_PAGE					54
#define LCD_FACTORY_CONTROLSETTING_PAGE					55
#define LCD_FACTORY_TESTMODE_PROCESSSETTINGS1_PAGE		56
#define LCD_FACTORY_TESTMODE_PROCESSTEST1_PAGE			57
#define LCD_FACTORY_TESTMODE_SELECT_CYCLE_PAGE			58



#define LCD_FACTORY_TESTMODE_PROCESSSETTINGS2_PAGE		80
#define LCD_FACTORY_TESTMODE_PROCESSTEST2_PAGE			81
#define LCD_FACTORY_TESTMODE_PROCESSTEST3_PAGE			82
#define LCD_FACTORY_TESTMODE_PROCESSTEST4_PAGE			83



//로그인 61~
#define LCD_LOGIN_PAGE	 								61
#define LCD_LOGIN2_PAGE	 								62
#define LCD_CREATE_ID	 								63
#define LCD_MANAGEMENT_ID	 							64
#define LCD_MANAGEMENT_CHANGE_PW_POPUP1					65
#define LCD_MANAGEMENT_CHANGE_PW_POPUP2					66
#define LCD_MANAGEMENT_DELET_ID_POPUP					67
#define LCD_MANAGEMENT_WRONG_PW_POPUP					68
#define LCD_MANAGEMENT_MAXOVER_PW_POPUP					69

#define LCD_FACTORY_CONTROLTEST_PAGE					60

#define LCD_VACUUM_MESSAGE_PAGE							70
#define LCD_ADMIN_MESSAGE_PAGE							71
#define LCD_FACTORY_MESSAGE_PAGE						72
#define LCD_WRONG_PW_MESSAGE_PAGE						73
#define LCD_USER_TOTALTEST_STOP_MESSAGE_PAGE			74
#define LCD_USER_VACUUMTEST_STOP_MESSAGE_PAGE			75
#define LCD_SLEEPMODE_STOP_MESSAGE_PAGE					76
#define LCD_SLEEPMODE_MESSAGE_PAGE						77
#define LCD_RESERVATION_TIME_MESSAGE1_PAGE				78
#define LCD_RESERVATION_TIME_MESSAGE2_PAGE				79


#define LCD_LOADING_PAGE								100
#define LCD_SLEEPMODE_PAGE								101
#define LCD_SLEEPMODE_RESERVATION_PAGE					102
#define LCD_POWEROFFCHECK_PAGE							103

extern int currentpage;
extern int beforepage;

extern int beforeday;

struct date_format {
	unsigned char year, month, day, week;
	unsigned char hour, minute, second;
};

extern struct date_format today_date;
extern struct date_format reserve_date;

extern unsigned char LCD_rx_data[30];

extern int LoginFlag, MonitorFlag, LanguageFlag;

void InitLCD();
void ReadLCD();
void DisplayFirstPage();
void DisplayPage();

void LCD_Process();
void LCD_Function_Process(int index, int value);
void LCD_Password(int index, int value);
void LCD_02(int index, int value);
void LCD_03(int index, int value);
void LCD_04(int index, int value);
void LCD_05(int index, int value);
void LCD_06(int index, int value);
void LCD_07(int index, int value);
void LCD_08(int index, int value);
void LCD_09(int index, int value);
void LCD_10(int index, int value);
void LCD_11(int index, int value);
void LCD_12(int index, int value);

void LCD_14(int index, int value);
void LCD_15(int index, int value);

void LCD_21(int index, int value);
void LCD_22(int index, int value);
void LCD_23(int index, int value);
void LCD_24(int index, int value);
void LCD_25(int index, int value);
void LCD_26(int index, int value);
void LCD_27(int index, int value);

void LCD_29(int index, int value);

void LCD_30(int index, int value);
void LCD_31(int index, int value);
void LCD_32(int index, int value);
void LCD_33(int index, int value);
void LCD_34(int index, int value);


void LCD_41(int index, int value);
void LCD_43(int index, int value);
void LCD_44(int index, int value);

void LCD_51(int index, int value);
void LCD_52(int index, int value);
void LCD_53(int index, int value);

void LCD_55(int index, int value);
void LCD_56(int index, int value);
void LCD_57(int index, int value);
void LCD_58(int index, int value);

void LCD_60(int index, int value);
void LCD_61(int index, int value);
void LCD_63(int index, int value);
void LCD_64(int index, int value);

void DoActionButton(int key);
void GoToPage(int key);
void ProcessSettingButton(int key);

void DisplayProcessIcon(int index, int value);
void DisplayStepIcon(int index, int value);
void DisplayPartsIcon(int index, int value);
void DisplayProcessIcons(int index);
void DisplayStepIcons(int index);
void DisplayPartsIcons(void);
void DisplaySelectIcon(int index, int value);
void DisplayIcon(int page, int index, int value);

void DisplayTime(int page, int index, unsigned int icentisecond);
void DisplayTime2(int page, int index, unsigned int icentisecond);
void DisplayTimeValues();
void ReadStepTime();
void ReadProcessTime();
void ReadCycleTime();

void DisplayValue(int index, float value);
void DisplayCycleValue(int value);
void DisplayProcessSettingValues();
void DisplayProcessTestValues();
void DisplayNormalValues();
void DisplayVacuumSensor();
void DisplayRFIDNumber();

//페이지별 디스플레이 구현
void Display02page();
void Display03page();
void Display04page();
void Display05page();

void Display06page();
void Display07page();
void Display08page();
void Display09page();
void Display10page();
void Display11page();
void Display12page();

void Display21page();
void Display22page();
void Display23page();
void Display24page();
void Display25page();

void Display31page();
void Display32page();
void Display33page();
void Display34page();

void Display35page();
void Display36page();
void Display37page();

void Display41page();
void Display43page();
void Display44page();


void Display50page();
void Display51page();
void Display52page();
void Display53page();

void Display54page();
void Display55page();
void Display56page();
void Display57page();
void Display58page();

void Display61page();
void Display63page();
void Display64page();


void Display80page();
void Display81page();
void Display82page();
void Display83page();
void Display84page();


void DisplayPageValue(int page ,int index, int value);
void DisplayPage4Char(int page ,int index, char *msg);
void DisplayPage8Char(int page ,int index, char *msg);
void DisplayPage10Char(int page ,int index, char *msg);
void DisplayPage20Char(int page ,int index, char *msg);

int LCD_ReceiveFrame(uint8_t *buffer, uint16_t length, uint32_t timeout, uint8_t retries);


//GET_TIME&DATA
void GetTime();


//RTC
int ReadRTC(unsigned char *year, unsigned char *month, unsigned char *day, unsigned char *week,
				unsigned char *hour, unsigned char *minute, unsigned char *second);
void SetRTCFromLCD();
void Get_RTC_Time(unsigned char *year, unsigned char *month, unsigned char *day, unsigned char *week,
				unsigned char *hour, unsigned char *minute, unsigned char *second);

void ReadInforDataFromLCD();

void DisplayIcons();
void DisplaySterilantData();


unsigned char hex2bcd (unsigned char x);
unsigned char bcd2bin (unsigned char x);

//Graph
void DisplayDot(int vp1,int vp2,int x, int y);
void DisplayInitTempGraph();
void DisplayInitVacuumGraph();
void DisplayTempGraph(int number, int color);
void DisplayVacuumGraph(int number, int color);

#endif /* SRC_PERIPHERAL_LCD_H_ */
