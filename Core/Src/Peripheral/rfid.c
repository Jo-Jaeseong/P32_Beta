/*
 * rfid.c
 *
 *  Created on: May 31, 2019
 *      Author: monster
 */
#include "main.h"
#include "hardware.h"
#include "peripheral.h"
#include "sensor.h"
#include <string.h>

extern SPI_HandleTypeDef hspi2;

#define SPI2_CS_GPIO_Port GPIO_OUT24_GPIO_Port
#define SPI2_CS_Pin GPIO_OUT24_Pin

#define USE_SPI		1

void MFRC522_Init(void);
void MFRC522_WriteReg(uint8_t address, uint8_t data);
void MFRC522_WriteBufReg(uint8_t address, uint8_t* data, uint8_t len);
uint8_t MFRC522_ReadReg(uint8_t address);
void MFRC522_SoftReset(void);
int MFRC522_SelfTest(void);

void SetBitMask(unsigned char reg, unsigned char mask);
void ClearBitMask(unsigned char reg, unsigned char mask);
void AntennaOn(void);
void AntennaOff(void);

#define 	MI_OK		(0)
#define 	MI_NOTAGERR (-1)
#define 	MI_ERR		(-2)

#define MFRC522_MAX_LEN					16

// Page 0 registers - command and status
#define MFRC522_COMMAND_REG     0x01 ///< Starts and stops commands execution
#define MFRC522_COMIEN_REG      0x02 ///< Enabling IRQs
#define MFRC522_DIVIEN_REG      0x03
#define MFRC522_COMIRQ_REG      0x04
#define MFRC522_DIVIRQ_REG      0x05
#define MFRC522_ERROR_REG       0x06
#define MFRC522_STATUS1_REG     0x07
#define MFRC522_STATUS2_REG     0x08
#define MFRC522_FIFODATA_REG    0x09
#define MFRC522_FIFOLEVEL_REG   0x0a
#define MFRC522_WATERLEVEL_REG  0x0b
#define MFRC522_CONTROL_REG     0x0c
#define MFRC522_BITFRAMING_REG  0x0d
#define MFRC522_COLL_REG        0x0e

// Page 1 - communication
#define MFRC522_MODE_REG        0x11
#define MFRC522_TXMODE_REG      0x12
#define MFRC522_RXMODE_REG      0x13
#define MFRC522_TXCONTROL_REG   0x14
#define MFRC522_TXASK_REG       0x15
#define MFRC522_TXSEL_REG       0x16
#define MFRC522_RXSEL_REG       0x17
#define MFRC522_RXTHRESHOLD_REG 0x18
#define MFRC522_DEMOD_REG       0x19
#define MFRC522_MFTX_REG        0x1c
#define MFRC522_MFRX_REG        0x1d
#define MFRC522_SERIALSPEED_REG 0x1f

// Page 2 - configuration
#define MFRC522_CRCRESULT_REG_H 0x21
#define MFRC522_CRCRESULT_REG_L 0x22
#define MFRC522_MODWIDTH_REG    0x24
#define MFRC522_RFCFG_REG       0x26
#define MFRC522_GSN_REG         0x27
#define MFRC522_CWGSP_REG       0x28
#define MFRC522_MODGSP_REG      0x29
#define MFRC522_TMODE_REG       0x2a
#define MFRC522_TPRESCALER_REG  0x2b
#define MFRC522_RELOAD_REG_H    0x2c
#define MFRC522_RELOAD_REG_L    0x2d
#define MFRC522_TCNTRVAL_REG_H  0x2e
#define MFRC522_TCNTRVAL_REG_L  0x2f

// Page 3 - test
#define MFRC522_TESTSEL1_REG    0x31
#define MFRC522_TESTSEL2_REG    0x32
#define MFRC522_TESTPINEN_REG   0x33
#define MFRC522_TESTPINVAL_REG  0x34
#define MFRC522_TESTBUS_REG     0x35
#define MFRC522_AUTOTEST_REG    0x36
#define MFRC522_VERSION_REG     0x37
#define MFRC522_ANALOGTEST_REG  0x38
#define MFRC522_TESTDAC1_REG    0x39
#define MFRC522_TESTDAC2_REG    0x3a
#define MFRC522_TESTADC_REG     0x3b

/**
 *
 */
// Mifare_One card command word
#define PICC_REQIDL						0x26   // find the antenna area does not enter hibernation
#define PICC_REQALL						0x52   // find all the cards antenna area
#define PICC_ANTICOLL					0x93   // anti-collision
#define PICC_SElECTTAG					0x93   // election card
#define PICC_AUTHENT1A					0x60   // authentication key A
#define PICC_AUTHENT1B					0x61   // authentication key B
#define PICC_READ						0x30   // Read Block
#define PICC_WRITE						0xA0   // write block
#define PICC_DECREMENT					0xC0   // debit
#define PICC_INCREMENT					0xC1   // recharge
#define PICC_RESTORE					0xC2   // transfer block data to the buffer
#define PICC_TRANSFER					0xB0   // save the data in the buffer
#define PICC_HALT						0x50   // Sleep

typedef enum {
	CMD_IDLE = 0b0000, //!< CMD_IDLE
	CMD_MEM = 0b0001, //!< CMD_MEM
	CMD_GENERATE_RANDOM_ID = 0b0010, //!< CMD_GENERATE_RANDOM_ID
	CMD_CALC_CRC = 0b0011, //!< CMD_CALC_CRC
	CMD_TRANSMIT = 0b0100, //!< CMD_TRANSMIT
	CMD_NO_CMD_CHANGE = 0b0111, //!< CMD_NO_CMD_CHANGE
	CMD_RECEIVE = 0b1000, //!< CMD_RECEIVE
	CMD_TRANSCEIVE = 0b1100, //!< CMD_TRANSCEIVE
	CMD_MF_AUTHENT = 0b1110, //!< CMD_MF_AUTHENT
	CMD_SOFT_RESET = 0b1111, //!< CMD_SOFT_RESET
} MFRC522_Commands;

typedef enum PICC_Command {
	// The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4)
	PICC_CMD_REQA			= 0x26,		// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
	PICC_CMD_WUPA			= 0x52,		// Wake-UP coE		= 0xC2,		// Reads thE		= 0xC2,		// Reads the contents of a block into the internal data register.e contents of a block into the internal data register.mmand, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
	PICC_CMD_CT				= 0x88,		// Cascade Tag. Not really a command, but used during anti collision.
	PICC_CMD_SEL_CL1		= 0x93,		// Anti collision/Select, Cascade Level 1
	PICC_CMD_SEL_CL2		= 0x95,		// Anti collision/Select, Cascade Level 1
	PICC_CMD_SEL_CL3		= 0x97,		// Anti collision/Select, Cascade Level 1
	PICC_CMD_HALT			= 0x50,		// HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
	// The commands used for MIFARE Classic (from http://www.nxp.com/documents/data_sheet/MF1S503x.pdf, Section 9)
	// Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
	// The read/write commands can also be used for MIFARE Ultralight.
	PICC_CMD_MF_AUTH_KEY_A	= 0x60,		// Perform authentication with Key A
	PICC_CMD_MF_AUTH_KEY_B	= 0x61,		// Perform authentication with Key B
	PICC_CMD_MF_READ		= 0x30,		// Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
	PICC_CMD_MF_WRITE		= 0xA0,		// Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
	PICC_CMD_MF_DECREMENT	= 0xC0,		// Decrements the contents of a block and stores the result in the internal data register.
	PICC_CMD_MF_INCREMENT	= 0xC1,		// Increments the contents of a block and stores the result in the internal data register.
	PICC_CMD_MF_RESTORE		= 0xC2,		// Reads the contents of a block into the internal data register.
	PICC_CMD_MF_TRANSFER	= 0xB0,		// Writes the contents of the internal data register to a block.
	// The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
	// The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
	PICC_CMD_UL_WRITE		= 0xA2		// Writes one 4 byte page to the PICC.
}PICC_CMD_t;

typedef enum{
	PICC_TYPE_NOT_COMPLETE = 0,
	PICC_TYPE_MIFARE_MINI,
	PICC_TYPE_MIFARE_1K,
	PICC_TYPE_MIFARE_4K,
	PICC_TYPE_MIFARE_UL,
	PICC_TYPE_MIFARE_PLUS,
	PICC_TYPE_TNP3XXX,
	PICC_TYPE_ISO_14443_4,
	PICC_TYPE_ISO_18092,
	PICC_TYPE_UNKNOWN
} PICC_TYPE_t;

uint8_t MFRC522_ReadReg(uint8_t address);
void MFRC522_WriteReg(uint8_t address, uint8_t data);
void MFRC522_SoftReset(void);

static int Checking_Card = 0;

int MFRC522_Setup(char Type);
int MFRC522_Check(uint8_t* id);
int MFRC522_Request(uint8_t reqMode, uint8_t* TagType);
int MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen);
int MFRC522_Anticoll(uint8_t* serNum);
void MFRC522_CalculateCRC(uint8_t* pIndata, uint8_t len, uint8_t* pOutData);
uint8_t MFRC522_SelectTag(uint8_t* serNum);
int MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum);
int MFRC522_Read(uint8_t blockAddr, uint8_t* recvData);
int MFRC522_Write(uint8_t blockAddr, uint8_t* writeData);
void MFRC522_Halt(void);
void MFRC522_WakeUp(void);
char *MFRC522_TypeToString(PICC_TYPE_t type);
int MFRC522_ParseType(uint8_t TagSelectRet);

struct RFID_format ReadRFIDData;//현재 RFID
struct RFID_format CurrentRFIDData;//현재 RFID
struct RFID_format FlashRFIDData[5];// FlashRFID
int RFIDflashIndex = 0; // Flash 데이터 인덱스
int CurrentRFIDIndex=0;

int checkret;
int expipry_check1;
int expipry_check2;
int expipry_check3;
int expiry_date1;
int expiry_date2;

uint8_t RFIDbuffer[20] = "";

void InitRFID(void)
{
	MFRC522_Init();
	//MFRC522_SelfTest();

	MFRC522_Setup('A');
}

void RFIDCheck(){
	if(IsSterilantRFIDRequired()==0){
		checkret = SterilantContainerCheck() ? 1 : 0;
		DisplaySterilantData();
		return;
	}

	InitRFID();
	ReadRFID();
	if(checkret==-2){//11.04추가
		/*
		for(int i=0;i<3;i++){
			ReadRFID();
			if(checkret==1){
				break;
			}
		}
		*/
	}
	else if(checkret==1){
		Write_Flash();
		DisplayPage(currentpage);
	}
	DisplaySterilantData();
}


int is_leap_year(unsigned int year) {
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

void add_days_to_date(int day) {
    unsigned int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int openyear=ReadRFIDData.open_year;
    int openmonth=ReadRFIDData.open_month;
    int openday=ReadRFIDData.open_day;

    if (is_leap_year(openyear)) {
        days_in_month[1] = 29; // 윤년인 경우 2월을 29일로 설정
    }

    openday += day;
    while (openday > days_in_month[openmonth - 1]) {
    	openday -= days_in_month[openmonth - 1];
        openmonth++;
        if (openmonth > 12) {
            openmonth = 1;
            openyear++;
            if (is_leap_year(openyear)) {
                days_in_month[1] = 29; // 윤년 업데이트
            } else {
                days_in_month[1] = 28;
            }
        }
    }


	ReadRFIDData.expiry_year=openyear;
	ReadRFIDData.expiry_month=openmonth;
	ReadRFIDData.expiry_day=openday;
}

void add_days_to_date2(int day) {
    unsigned int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int productionyear=ReadRFIDData.production_year;
    int productionmonth=ReadRFIDData.production_month;
    int productionday=ReadRFIDData.production_day;

    if (is_leap_year(productionyear)) {
        days_in_month[1] = 29; // 윤년인 경우 2월을 29일로 설정
    }

    productionday += day;
    while (productionday > days_in_month[productionmonth - 1]) {
    	productionday -= days_in_month[productionmonth - 1];
    	productionmonth++;
        if (productionmonth > 12) {
        	productionmonth = 1;
        	productionyear++;
            if (is_leap_year(productionyear)) {
                days_in_month[1] = 29; // 윤년 업데이트
            } else {
                days_in_month[1] = 28;
            }
        }
    }


	ReadRFIDData.expiry_year=productionyear;
	ReadRFIDData.expiry_month=productionmonth;
	ReadRFIDData.expiry_day=productionday;
}

void processRFIDData() {
    int found = 0;
    // ReadRFIDData와 FlashRFIDData 비교
    for (int i = 0; i < 5; i++) {
        if (FlashRFIDData[i].production_year == ReadRFIDData.production_year &&
            FlashRFIDData[i].production_month == ReadRFIDData.production_month &&
            FlashRFIDData[i].production_day == ReadRFIDData.production_day &&
            FlashRFIDData[i].production_number == ReadRFIDData.production_number) {
			//이전 인식된 RFID와 동일할때

			//만료기간 체크
			GetTime();
			expipry_check2=calculateElapsedDays((int)FlashRFIDData[i].expiry_year,(int)FlashRFIDData[i].expiry_month,(int)FlashRFIDData[i].expiry_day,today_date.year,today_date.month,today_date.day);
			if(expipry_check2>0){
				checkret=1;
			}
			else if(expipry_check2<=0){
				expipry_check3=calculateElapsedDays(today_date.year,today_date.month,today_date.day,(int)FlashRFIDData[i].expiry_year,(int)FlashRFIDData[i].expiry_month,(int)FlashRFIDData[i].expiry_day);
				if(expipry_check3==0){
					checkret=1;
				}
				else{
					checkret=3;
				}
			}
            // 일치하는 데이터가 있을 경우
            CurrentRFIDData = FlashRFIDData[i];
            CurrentRFIDIndex=i;
            found = 1;
            break;
        }
    }

	//새로운 RFID가 인식되었을때
    if (!found) {
		//만료 기간 체크 및 설정
		expipry_check1=(expiry_date1*30+5)-calculateElapsedDays(today_date.year,today_date.month,today_date.day,(int)ReadRFIDData.production_year,(int)ReadRFIDData.production_month,(int)ReadRFIDData.production_day);
		if(expipry_check1>=(expiry_date2*30)){
			add_days_to_date(expiry_date2*30);
			checkret=1;
			//멸균제 유효기간 OK
		}
		else if((expipry_check1<(expiry_date2*30))&&(expipry_check1>=0)){
			add_days_to_date(expipry_check1);
			checkret=1;
		}
		else{
			//기한 지남
			//add_days_to_date(expiry_date1*30);
			add_days_to_date2(expiry_date1*30+5);
			checkret=2;
		}

        // Flash에 저장된 데이터가 5개일 경우, 가장 오래된 데이터를 덮어쓰기
		FlashRFIDData[RFIDflashIndex] = ReadRFIDData;
        CurrentRFIDData = ReadRFIDData;
        RFIDflashIndex = (RFIDflashIndex + 1) % 5; // 인덱스 업데이트
    }
}




uint32_t ReadRFID(void)
{

	uint8_t CardID[5] = { 0x00, };
	uint8_t SectorKeyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	int ret = MFRC522_Check(CardID);

	ret = MFRC522_SelectTag(CardID);

	ret = MFRC522_Auth((uint8_t) PICC_AUTHENT1B, (uint8_t) 0,
				(uint8_t*) SectorKeyA, (uint8_t*) CardID);
	ret = MFRC522_Read(1, RFIDbuffer);

	if(RFIDbuffer[0]!='C' && RFIDbuffer[1]!='B' && RFIDbuffer[2]!='T'){
		//RFID가 인식되지 않거나 잘못된 RFID가 인식되었을때
		ret=-2;
		checkret=-2;
		memset(&ReadRFIDData,0,sizeof(ReadRFIDData));
	}
	else{
		//RFID 인식되었을때
		ret=1;
		checkret=1;

		//인식된 RFID 넘버
		ReadRFIDData.production_year=(RFIDbuffer[6]-'0')*10+(RFIDbuffer[7]-'0');
		ReadRFIDData.production_month=(RFIDbuffer[8]-'0')*10+(RFIDbuffer[9]-'0');
		ReadRFIDData.production_day=(RFIDbuffer[10]-'0')*10+(RFIDbuffer[11]-'0');
		ReadRFIDData.production_number=(RFIDbuffer[12]-'0')*10+(RFIDbuffer[13]-'0');
		ReadRFIDData.volume=(RFIDbuffer[3]-'0')*100+(RFIDbuffer[4]-'0')*10+(RFIDbuffer[5]-'0');
		ReadRFIDData.volumemax=ReadRFIDData.volume;
		GetTime();
		ReadRFIDData.open_year = today_date.year;
		ReadRFIDData.open_month= today_date.month;
		ReadRFIDData.open_day = today_date.day;

		//저장된 RFID 넘버와 비교
		processRFIDData();
	}
	return ret;
}

bool is_same(unsigned char a[], unsigned char b[], int length){
    for (int i = 0; i < length; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Initialize communication with RFID reader
 */
void MFRC522_Init(void)
{
	//MFRC522_HAL_Init();
	uint8_t temp = 0;

	for (int i = 0; i < 20; i++)
	{
		HAL_SPI_Transmit(&hspi2, &temp, 1, 10);
	}

	MFRC522_SoftReset();
	HAL_Delay(100);

	temp = MFRC522_ReadReg(MFRC522_COMMAND_REG);
	temp = MFRC522_ReadReg(MFRC522_VERSION_REG);
}

/**
 * @brief Write to a register
 * @param address
 * @param data
 */
void MFRC522_WriteReg(uint8_t address, uint8_t data)
{
	// MSB = 0 for write
	// send register address
	uint8_t txBuffer[2];
	txBuffer[0] = (address << 1) & 0x7f;
	txBuffer[1] = data;

#if USE_SPI
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	//MFRC522_HAL_Select();
	//while(SPI_I2S_GetFlagStatus(SPI1,SPI_FLAG_TXE) == RESET);
	HAL_SPI_Transmit(&hspi2, txBuffer, 2, 100);
	//HAL_SPI_Transmit(&hspi2, &data, 1, 10);
	//MFRC522_HAL_Transmit(((address << 1) & 0x7f));
	//MFRC522_HAL_Transmit(data);

	//MFRC522_HAL_Deselect();
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
#else
	HAL_UART_Transmit(&huart5, txBuffer, 2, 10);
#endif
}

/**
 * @brief Write buffer to a register
 * @param address
 * @param data
 * @param len
 */
void MFRC522_WriteBufReg(uint8_t address, uint8_t* data, uint8_t len)
{
#if USE_SPI
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	//MFRC522_HAL_Select();

	// MSB = 0 for write
	// send register address
	//MFRC522_HAL_Transmit(((address << 1) & 0x7f));
	unsigned char ucData = (address << 1) & 0x7f;
	HAL_SPI_Transmit(&hspi2, &ucData, 1, 100);
#else
	HAL_UART_Transmit(&huart5, &ucData, 1, 10);
#endif

	for (int i = 0; i < len; i++) {
		//MFRC522_HAL_Transmit(data[i]);
		ucData = data[i];
#if USE_SPI
		HAL_SPI_Transmit(&hspi2, &ucData, 1, 100);
#else
		HAL_UART_Transmit(&huart5, &ucData, 1, 10);
#endif
	}

#if USE_SPI
	//MFRC522_HAL_Deselect();
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
#endif
}

/**
 * @brief Read a register
 * @param address
 * @return
 */
uint8_t MFRC522_ReadReg(uint8_t address)
{
	//uint8_t ret;

	// MSB = 1 for read
	// send register address
	uint8_t txBuffer[2];
	uint8_t rxBuffer[2];
	txBuffer[0] = ((address << 1) & 0x7f) | 0x80;
	txBuffer[1] = 0xff;

#if USE_SPI
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	//MFRC522_HAL_Select();

	//while(SPI_I2S_GetFlagStatus(SPI1,SPI_FLAG_TXE) == RESET);
	HAL_SPI_Transmit(&hspi2, txBuffer, 1, 100);
	//txBuffer[0] = 0xff;
	//HAL_SPI_TransmitReceive(&hspi2, txBuffer, rxBuffer, 2, 100);
	//HAL_SPI_TransmitReceive(&hspi2, txBuffer, rxBuffer, 1, 100);
	//while(SPI_I2S_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET);
	HAL_SPI_Receive(&hspi2, rxBuffer, 1, 100);
	//ret = MFRC522_HAL_Transmit(0x00);
	//HAL_SPI_Receive(&hspi2, &ret, 1, 100);

	//MFRC522_HAL_Deselect();
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
#else
	HAL_UART_Transmit(&huart5, txBuffer, 1, 10);
    HAL_UART_Receive(&huart5, (uint8_t*)rxBuffer, 1, 10);
#endif
	return rxBuffer[0];
}

/**
 * @brief Execute soft reset
 */
void MFRC522_SoftReset(void)
{
	MFRC522_WriteReg(MFRC522_COMMAND_REG, CMD_SOFT_RESET);
}

/*
  * Function Name : SetBitMask
 * Description: Set RC522 register bit
 * Input parameters : reg - register address ; mask - set value
 * Return value: None
 */
void SetBitMask(unsigned char reg, unsigned char mask)
{
    unsigned char tmp;
    tmp = MFRC522_ReadReg(reg);
    MFRC522_WriteReg(reg, tmp | mask);  // set bit mask
}

/*
 * Function Name : ClearBitMask
 * Description : clear RC522 register bit
 * Input parameters : reg - register address ; mask - clear bit value
 * Return value: None
 */
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    unsigned char tmp;
    tmp = MFRC522_ReadReg(reg);
    MFRC522_WriteReg(reg, tmp & (~mask));  // clear bit mask
}

/*
 * Function Name : AntennaOn
 * Description : Open antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
 * Input: None
 * Return value: None
 */
void AntennaOn(void)
{
	unsigned char temp;

	temp = MFRC522_ReadReg(MFRC522_TXCONTROL_REG);
	if (!(temp & 0x03))
	{
		SetBitMask(MFRC522_TXCONTROL_REG, 0x03);
	}
}

/*
 * Function Name : AntennaOff
 * Description : Close antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
 * Input: None
 * Return value: None
 * /
 */
void AntennaOff(void)
{
	ClearBitMask(MFRC522_TXCONTROL_REG, 0x03);
}

// Version 0.0 (0x90)
// Philips Semiconductors; Preliminary Specification Revision 2.0 - 01 August 2005; 16.1 self-test
const unsigned char MFRC522_firmware_referenceV0_0[] = {
	0x00, 0x87, 0x98, 0x0f, 0x49, 0xFF, 0x07, 0x19,
	0xBF, 0x22, 0x30, 0x49, 0x59, 0x63, 0xAD, 0xCA,
	0x7F, 0xE3, 0x4E, 0x03, 0x5C, 0x4E, 0x49, 0x50,
	0x47, 0x9A, 0x37, 0x61, 0xE7, 0xE2, 0xC6, 0x2E,
	0x75, 0x5A, 0xED, 0x04, 0x3D, 0x02, 0x4B, 0x78,
	0x32, 0xFF, 0x58, 0x3B, 0x7C, 0xE9, 0x00, 0x94,
	0xB4, 0x4A, 0x59, 0x5B, 0xFD, 0xC9, 0x29, 0xDF,
	0x35, 0x96, 0x98, 0x9E, 0x4F, 0x30, 0x32, 0x8D
};
// Version 1.0 (0x91)
// NXP Semiconductors; Rev. 3.8 - 17 September 2014; 16.1.1 self-test
const unsigned char MFRC522_firmware_referenceV1_0[] = {
	0x00, 0xC6, 0x37, 0xD5, 0x32, 0xB7, 0x57, 0x5C,
	0xC2, 0xD8, 0x7C, 0x4D, 0xD9, 0x70, 0xC7, 0x73,
	0x10, 0xE6, 0xD2, 0xAA, 0x5E, 0xA1, 0x3E, 0x5A,
	0x14, 0xAF, 0x30, 0x61, 0xC9, 0x70, 0xDB, 0x2E,
	0x64, 0x22, 0x72, 0xB5, 0xBD, 0x65, 0xF4, 0xEC,
	0x22, 0xBC, 0xD3, 0x72, 0x35, 0xCD, 0xAA, 0x41,
	0x1F, 0xA7, 0xF3, 0x53, 0x14, 0xDE, 0x7E, 0x02,
	0xD9, 0x0F, 0xB5, 0x5E, 0x25, 0x1D, 0x29, 0x79
};
// Version 2.0 (0x92)
// NXP Semiconductors; Rev. 3.8 - 17 September 2014; 16.1.1 self-test
const unsigned char MFRC522_firmware_referenceV2_0[] = {
	0x00, 0xEB, 0x66, 0xBA, 0x57, 0xBF, 0x23, 0x95,
	0xD0, 0xE3, 0x0D, 0x3D, 0x27, 0x89, 0x5C, 0xDE,
	0x9D, 0x3B, 0xA7, 0x00, 0x21, 0x5B, 0x89, 0x82,
	0x51, 0x3A, 0xEB, 0x02, 0x0C, 0xA5, 0x00, 0x49,
	0x7C, 0x84, 0x4D, 0xB3, 0xCC, 0xD2, 0x1B, 0x81,
	0x5D, 0x48, 0x76, 0xD5, 0x71, 0x61, 0x21, 0xA9,
	0x86, 0x96, 0x83, 0x38, 0xCF, 0x9D, 0x5B, 0x6D,
	0xDC, 0x15, 0xBA, 0x3E, 0x7D, 0x95, 0x3B, 0x2F
};
// Clone
// Fudan Semiconductor FM17522 (0x88)
const unsigned char FM17522_firmware_reference[] = {
	0x00, 0xD6, 0x78, 0x8C, 0xE2, 0xAA, 0x0C, 0x18,
	0x2A, 0xB8, 0x7A, 0x7F, 0xD3, 0x6A, 0xCF, 0x0B,
	0xB1, 0x37, 0x63, 0x4B, 0x69, 0xAE, 0x91, 0xC7,
	0xC3, 0x97, 0xAE, 0x77, 0xF4, 0x37, 0xD7, 0x9B,
	0x7C, 0xF5, 0x3C, 0x11, 0x8F, 0x15, 0xC3, 0xD7,
	0xC1, 0x5B, 0x00, 0x2A, 0xD0, 0x75, 0xDE, 0x9E,
	0x51, 0x64, 0xAB, 0x3E, 0xE9, 0x15, 0xB5, 0xAB,
	0x56, 0x9A, 0x98, 0x82, 0x26, 0xEA, 0x2A, 0x62
};

int MFRC522_SelfTest(void)
{
	// This follows directly the steps outlined in 16.1.1
	// 1. Perform a soft reset.
	MFRC522_SoftReset();

	// 2. Clear the internal buffer by writing 25 bytes of 00h
	unsigned char ZEROES[25] = {0x00};
	MFRC522_WriteReg(MFRC522_FIFOLEVEL_REG, 0x80);		// flush the FIFO buffer
	MFRC522_WriteBufReg(MFRC522_FIFODATA_REG, ZEROES, 25);	// write 25 bytes of 00h to FIFO
	MFRC522_WriteReg(MFRC522_COMMAND_REG, CMD_MEM);		// transfer to internal buffer

	// 3. Enable self-test
	MFRC522_WriteReg(MFRC522_AUTOTEST_REG, 0x09);

	// 4. Write 00h to FIFO buffer
	MFRC522_WriteReg(MFRC522_FIFODATA_REG, 0x00);

	// 5. Start self-test by issuing the CalcCRC command
	MFRC522_WriteReg(MFRC522_COMMAND_REG, CMD_CALC_CRC);

	// 6. Wait for self-test to complete
	unsigned char n;
	for (uint8_t i = 0; i < 0xFF; i++) {
		// The datasheet does not specify exact completion condition except
		// that FIFO buffer should contain 64 bytes.
		// While selftest is initiated by CalcCRC command
		// it behaves differently from normal CRC computation,
		// so one can't reliably use DivIrqReg to check for completion.
		// It is reported that some devices does not trigger CRCIRq flag
		// during selftest.
		n = MFRC522_ReadReg(MFRC522_FIFOLEVEL_REG);
		if (n >= 64) {
			break;
		}
	}
	MFRC522_WriteReg(MFRC522_COMMAND_REG, CMD_IDLE);		// Stop calculating CRC for new content in the FIFO.

	// 7. Read out resulting 64 bytes from the FIFO buffer.
	unsigned char result[64];
	//MFRC522_ReadReg(FIFODataReg, 64, result, 0);
	for(int i = 0; i < 64; i++) {
		result[i] = MFRC522_ReadReg(MFRC522_FIFODATA_REG);
	}

	// Auto self-test done
	// Reset AutoTestReg register to be 0 again. Required for normal operation.
	MFRC522_WriteReg(MFRC522_AUTOTEST_REG, 0x00);

	// Determine firmware version (see section 9.3.4.8 in spec)
	unsigned char version = MFRC522_ReadReg(MFRC522_VERSION_REG);

	// Pick the appropriate reference values
	const unsigned char *reference;
	switch (version) {
		case 0x88:	// Fudan Semiconductor FM17522 clone
			reference = FM17522_firmware_reference;
			break;
		case 0x90:	// Version 0.0
			reference = MFRC522_firmware_referenceV0_0;
			break;
		case 0x91:	// Version 1.0
			reference = MFRC522_firmware_referenceV1_0;
			break;
		case 0x92:	// Version 2.0
			reference = MFRC522_firmware_referenceV2_0;
			break;
		default:	// Unknown version
			return -1; // abort test
	}

	// Verify that the results match up to our expectations
	for (uint8_t i = 0; i < 64; i++) {
		if (result[i] != reference[i]) {
			return -1;
		}
	}

	// Test passed; all is good.
	return 0;
} // End PCD_PerformSelfTest()

/* HAL prototypes end */

int MFRC522_Setup(char Type)
{
	MFRC522_SoftReset();
	HAL_Delay(200);

	MFRC522_WriteReg(MFRC522_TPRESCALER_REG, 0x3E);
#ifndef NOTEST
	{
		/* test read and write reg functions */
		volatile char test;
		test = MFRC522_ReadReg(MFRC522_TPRESCALER_REG);
		if (test != 0x3E) {
			return MI_ERR;
		}
	}
#endif

#if 0
	MFRC522_WriteReg(MFRC522_TMODE_REG, 0x8D);
	MFRC522_WriteReg(MFRC522_RELOAD_REG_L, 30);
	MFRC522_WriteReg(MFRC522_RELOAD_REG_H, 0);
	MFRC522_WriteReg(MFRC522_TXASK_REG, 0x40);
	MFRC522_WriteReg(MFRC522_MODE_REG, 0x3D);
	if (Type == 'A') {
		ClearBitMask(MFRC522_STATUS2_REG, 0x08);
		MFRC522_WriteReg(MFRC522_MODE_REG, 0x3D);
		MFRC522_WriteReg(MFRC522_RXSEL_REG, 0x86);
		MFRC522_WriteReg(MFRC522_RFCFG_REG, 0x7F);
		MFRC522_WriteReg(MFRC522_RELOAD_REG_L, 30);
		MFRC522_WriteReg(MFRC522_RELOAD_REG_H, 0);
		MFRC522_WriteReg(MFRC522_TMODE_REG, 0x8D);
		MFRC522_WriteReg(MFRC522_TPRESCALER_REG, 0x3E);
	}
#endif
	MFRC522_WriteReg(MFRC522_TMODE_REG, 0x8D);
	MFRC522_WriteReg(MFRC522_TPRESCALER_REG, 0x3E);
	MFRC522_WriteReg(MFRC522_RELOAD_REG_L, 30);
	MFRC522_WriteReg(MFRC522_RELOAD_REG_H, 0);
	MFRC522_WriteReg(MFRC522_TXASK_REG, 0x40);
	MFRC522_WriteReg(MFRC522_MODE_REG, 0x3D);

	AntennaOn();		//Open the antenna
	return MI_OK;
}

int MFRC522_Check(uint8_t* id)
{
	int status;
	/* Must Clear Bit MFCrypto1On in Status2 reg in order to return to the card detect mode*/
	ClearBitMask(MFRC522_STATUS2_REG,(1<<3));
	Checking_Card = 1;
	//Find cards, return card type
	status = MFRC522_Request(PICC_REQIDL, id);
	//status = MFRC522_Request(PICC_CMD_WUPA, id);
	Checking_Card = 0;
	if (status == MI_OK) {
		//Card detected
		//Anti-collision, return card serial number 4 bytes
		status = MFRC522_Anticoll(id);
	}
	return status;
}

int MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID)
{
	uint8_t i;
	for (i = 0; i < 5; i++) {
		if (CardID[i] != CompareID[i]) {
			return MI_ERR;
		}
	}
	return MI_OK;
}

int MFRC522_Request(uint8_t reqMode, uint8_t* TagType)
{
	int status;
	uint16_t backBits;			//The received data bits

	MFRC522_WriteReg(MFRC522_BITFRAMING_REG, 0x07);//TxLastBists = BitFramingReg[2..0]	???

	TagType[0] = reqMode;
	status = MFRC522_ToCard(CMD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK)) {
		return status;
	}
	if (backBits != 0x10) {
		return MI_ERR;
	}
	return MI_OK;
}

int MFRC522_ToCard(uint8_t command, uint8_t* sendData,
		uint8_t sendLen, uint8_t* backData, uint16_t* backLen)
{
	int status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command) {
	case CMD_MF_AUTHENT: {
		irqEn = 0x12;
		waitIRq = 0x10;
		break;
	}
	case CMD_TRANSCEIVE: {
		irqEn = 0x77;
		waitIRq = 0x30;
		break;
	}
	default:
		break;
	}

	MFRC522_WriteReg(MFRC522_COMIEN_REG, irqEn | 0x80);
	ClearBitMask(MFRC522_COMIRQ_REG, 0x80);
	SetBitMask(MFRC522_FIFOLEVEL_REG, 0x80);

	MFRC522_WriteReg(MFRC522_COMMAND_REG, CMD_IDLE);

	//Writing data to the FIFO
	for (i = 0; i < sendLen; i++) {
		MFRC522_WriteReg(MFRC522_FIFODATA_REG, sendData[i]);
	}

	//Execute the command
	MFRC522_WriteReg(MFRC522_COMMAND_REG, command);
	if (command == CMD_TRANSCEIVE) {
		SetBitMask(MFRC522_BITFRAMING_REG, 0x80);//StartSend=1,transmission of data starts
	}

	//Waiting to receive data to complete
	i = 2000;//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
	do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		if (Checking_Card) {
			HAL_Delay(16);
		} else {
			HAL_Delay(2);
		}
		n = MFRC522_ReadReg(MFRC522_COMIRQ_REG);
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

	ClearBitMask(MFRC522_BITFRAMING_REG, 0x80);		//StartSend=0

	if (i != 0) {
		if (!(MFRC522_ReadReg(MFRC522_ERROR_REG) & 0x1B)) {

			if (n & irqEn & 0x01) {
				status = MI_NOTAGERR;
			} else {
				status = MI_OK;
			}

			if (command == CMD_TRANSCEIVE) {
				n = MFRC522_ReadReg(MFRC522_FIFOLEVEL_REG);
				lastBits = MFRC522_ReadReg(MFRC522_CONTROL_REG) & 0x07;
				if (lastBits) {
					*backLen = (n - 1) * 8 + lastBits;
				} else {
					*backLen = n * 8;
				}

				if (n == 0) {
					n = 1;
				}
				if (n > MFRC522_MAX_LEN) {
					n = MFRC522_MAX_LEN;
				}

				//Reading the received data in FIFO
				for (i = 0; i < n; i++) {
					backData[i] = MFRC522_ReadReg(MFRC522_FIFODATA_REG);
				}
			}
		} else {
			status = MI_ERR;
		}
	}

	return status;
}

int MFRC522_Anticoll(uint8_t* serNum)
{
	int status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	MFRC522_WriteReg(MFRC522_BITFRAMING_REG, 0x00);//TxLastBists = BitFramingReg[2..0]

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(CMD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		//Check card serial number
		for (i = 0; i < 4; i++) {
			serNumCheck ^= serNum[i];
		}
		/* check sum with last byte*/
		if (serNumCheck != serNum[i]) {
			status = MI_ERR;
		}
	}
	return status;
}

void MFRC522_CalculateCRC(uint8_t* pIndata, uint8_t len, uint8_t* pOutData)
{
	uint8_t i, n;

	ClearBitMask(MFRC522_DIVIRQ_REG, 0x04);			//CRCIrq = 0
	SetBitMask(MFRC522_FIFOLEVEL_REG, 0x80);	//Clear the FIFO pointer
	//Write_MFRC522(CommandReg, CMD_IDLE);

	//Writing data to the FIFO
	for (i = 0; i < len; i++) {
		MFRC522_WriteReg(MFRC522_FIFODATA_REG, *(pIndata + i));
	}
	MFRC522_WriteReg(MFRC522_COMMAND_REG, CMD_CALC_CRC);

	//Wait CRC calculation is complete
	i = 0xFF;
	do {
		n = MFRC522_ReadReg(MFRC522_DIVIRQ_REG);
		i--;
	} while ((i != 0) && !(n & 0x04));			//CRCIrq = 1

	//Read CRC calculation result
	pOutData[0] = MFRC522_ReadReg(MFRC522_CRCRESULT_REG_L);
	pOutData[1] = MFRC522_ReadReg(MFRC522_CRCRESULT_REG_H);
}

uint8_t MFRC522_SelectTag(uint8_t* serNum)
{
	uint8_t i;
	int status;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[32] = "";

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i + 2] = *(serNum + i);
	}
	MFRC522_CalculateCRC(buffer, 7, &buffer[7]);	//Fill [7:8] with 2byte CRC
	status = MFRC522_ToCard(CMD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {
		size = buffer[0];
	} else {
		size = 0;
	}

	return size;
}

int MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr,
		uint8_t* Sectorkey, uint8_t* serNum)
{
	int status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12];

	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++) {
		buff[i + 2] = *(Sectorkey + i);
	}
	for (i = 0; i < 4; i++) {
		buff[i + 8] = *(serNum + i);
	}
	status = MFRC522_ToCard(CMD_MF_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK)
			|| (!(MFRC522_ReadReg(MFRC522_STATUS2_REG) & 0x08))) {
		status = MI_ERR;
	}

	return status;
}

int MFRC522_Read(uint8_t blockAddr, uint8_t* recvData)
{
	int status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	MFRC522_CalculateCRC(recvData, 2, &recvData[2]);
	status = MFRC522_ToCard(CMD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
		return MI_ERR;
	}

	return unLen;
}

int MFRC522_Write(uint8_t blockAddr, uint8_t* writeData)
{
	int status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18];

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	MFRC522_CalculateCRC(buff, 2, &buff[2]);
	status = MFRC522_ToCard(CMD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {
		goto ERROR;
	}

	if (status == MI_OK) {
		//Data to the FIFO write 16Byte
		for (i = 0; i < 16; i++) {
			buff[i] = *(writeData + i);
		}
		MFRC522_CalculateCRC(buff, 16, &buff[16]);
		status = MFRC522_ToCard(CMD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK) || (recvBits != 4)
				|| ((buff[0] & 0x0F) != 0x0A)) {
				goto ERROR;

		}
	}
	return MI_OK;

	ERROR:
	if (recvBits == 4) {
		status = buff[0] & 0x0F;
	} else {
		status = MI_ERR;
	}
	return status;
}

void MFRC522_Halt(void)
{
	uint16_t unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	MFRC522_CalculateCRC(buff, 2, &buff[2]);

	MFRC522_ToCard(CMD_TRANSCEIVE, buff, 4, buff, &unLen);

}

void MFRC522_WakeUp(void)
{
	uint16_t unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	MFRC522_CalculateCRC(buff, 2, &buff[2]);

	MFRC522_ToCard(CMD_TRANSCEIVE, buff, 4, buff, &unLen);
}

char *PICC_TYPE_STRING[] = { "PICC_TYPE_NOT_COMPLETE", "PICC_TYPE_MIFARE_MINI",
		"PICC_TYPE_MIFARE_1K", "PICC_TYPE_MIFARE_4K", "PICC_TYPE_MIFARE_UL",
		"PICC_TYPE_MIFARE_PLUS", "PICC_TYPE_TNP3XXX", "PICC_TYPE_ISO_14443_4",
		"PICC_TYPE_ISO_18092", "PICC_TYPE_UNKNOWN" };

char *MFRC522_TypeToString(PICC_TYPE_t type)
{
	return PICC_TYPE_STRING[type];
}

int MFRC522_ParseType(uint8_t TagSelectRet)
{
	if (TagSelectRet & 0x04) { // UID not complete
		return PICC_TYPE_NOT_COMPLETE;
	}

	switch (TagSelectRet) {
	case 0x09:
		return PICC_TYPE_MIFARE_MINI;
		break;
	case 0x08:
		return PICC_TYPE_MIFARE_1K;
		break;
	case 0x18:
		return PICC_TYPE_MIFARE_4K;
		break;
	case 0x00:
		return PICC_TYPE_MIFARE_UL;
		break;
	case 0x10:
	case 0x11:
		return PICC_TYPE_MIFARE_PLUS;
		break;
	case 0x01:
		return PICC_TYPE_TNP3XXX;
		break;
	default:
		break;
	}

	if (TagSelectRet & 0x20) {
		return PICC_TYPE_ISO_14443_4;
	}

	if (TagSelectRet & 0x40) {
		return PICC_TYPE_ISO_18092;
	}

	return PICC_TYPE_UNKNOWN;
}
