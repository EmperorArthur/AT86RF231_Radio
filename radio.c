//Radio functions
//Copyright Arthur Moore 2012
//GPL V3
#include "radio.h"
#include "spi.h"
#include <stdlib.h> //for malloc and free
#include "communication.h" //For stderr
#define __ASSERT_USE_STDERR
#include <assert.h>
#include "at86rf230_registermap.h"

//This lets me set individual bits in a register high
#define REG_OR_ENABLE(address,data) uint8_t temp = radio_reg_read(address); temp |= data; radio_reg_write(address,temp);
//Easily defined stuff not in the registermap
#define IRQ_RX_START 2
#define IRQ_TRX_END 3
//I'm using an AT86RF231, so I have some extra features (like some LED Outs, and auto CRC)
#define PA_EXT_EN		7
#define TX_AUTO_CRC_ON	5
#define TRX_CTRL_1		(0x04)

radioFrame::radioFrame(){
	fcf = 0;
	sequenceNumber = 0;
	crc16 = 0;
}
radioFrame::radioFrame(uint8_t newSize){
	crc16=0;
	setSize(newSize);
}
radioFrame::~radioFrame(){
	//Nothing to do, data handles it's own cleanup
}
void radioFrame::setSize(uint8_t newSize){
	//Minimum frame size is 3 (including the crc)
	if(dataPlus >= newSize || maxSize <= newSize){
		data.setSize(0);
	}else{
		data.setSize(newSize-dataPlus);
	}
}
uint8_t radioFrame::size(){
	return data.size()+dataPlus;
}
//This lets me read and write to this thing as though it where a simple array
uint8_t & radioFrame::operator[] (uint8_t location){
	uint8_t mySize = data.size()+dataPlus;
	//Location is unsigned, so not worrying about negative numbers
	assert(location < mySize);
	//Handle fcf
	if(location == 0){
		return fcf8[1];
	}else if(location == 1){
		return fcf8[0];
	//Handle sequence number
	}else if(location == 2){
		return sequenceNumber;
	//Handle crc
	}else if(location == mySize - 2){
		return crc[1];
	}else if(location == mySize - 1){
		return crc[0];
	}
	//If nothing else, return data
	return data[location - 3];
}

radioData::radioData(){
	mySize = 0;
	data = NULL;
}
radioData::~radioData(){
	if (NULL != data){
		free(data);
	}
}
radioData::radioData(uint8_t newSize){
	data = NULL;
	setSize(newSize);
}
void radioData::setSize(uint8_t newSize){
	if (NULL != data){
		free(data);
	}
	//It's unsigned, so I'm not worried about negative numbers
	if(0 == newSize || maxSize <= newSize){
		mySize = 0;
		data = NULL;
	}else{
		mySize = newSize;
		data = (uint8_t *) malloc(mySize+1);
		assert(NULL != data);/*
		if(NULL == data){
			//Malloc failed
			mySize = 0;
		}*/
	}
}
uint8_t radioData::size(){
	return mySize;
}
uint8_t & radioData::operator[] (uint8_t location){
	assert(NULL != data);
	//Location is unsigned, so not worrying about negative numbers
	assert(location < mySize);
	return data[location];
}
void radioData::operator= (char* cString){
	int stringSize = 0;
	char * strPointer = cString;
	//Find the size of the string (excluding null terminator)
	while('\0' != *strPointer){
		strPointer++;
		stringSize++;
	}
	setSize(stringSize);
	for(int i=0;i<stringSize;i++){
		data[i] = cString[i];
	}
}
char * radioData::c_str(){
	//This is the only place were the last byte of data is ever used
	data[mySize] = '\0';
	return (char *) data;
}

//This sets the defaults to known semi-sane values
frameControlField::frameControlField(){
	frameType = DATA;
	securityEnabled = false;
	framePending = false;
	requestACK = false;
	intraPAN = false;
	dstAddrMode = NOADDR;
	frameVersion = 0;		//Compatable with both 2003 and 2006 specs
	srcAddrMode = NOADDR;
}
//Pack the fcf into two bytes
uint16_t frameControlField::pack(){
/*
	union{
		uint8_t fcf8[2];
		uint16_t fcf;
	};
	fcf8[0] |= (frameType&0x07 << 5);
	fcf8[0] |= (securityEnabled&0x01 << 4);
	fcf8[0] |= (framePending&0x01 << 3);
	fcf8[0] |= (requestACK&0x01 << 2);
	fcf8[0] |= (intraPAN&0x01 << 1);
	
	fcf8[1] |= (dstAddrMode&0x03 << 4);
	fcf8[1] |= (frameVersion&0x03 << 2);
	fcf8[1] |= (srcAddrMode&0x03 << 0);
	return fcf;*/
	uint16_t fcfPacked = 0x00;
	fcfPacked |= ((frameType&0x07) << 12);
	fcfPacked |= ((securityEnabled&0x01) << 11);
	fcfPacked |= ((framePending&0x01) << 10);
	fcfPacked |= ((requestACK&0x01) << 9);
	fcfPacked |= ((intraPAN&0x01) << 8);
	fcfPacked |= ((dstAddrMode&0x03) << 5);
	fcfPacked |= ((frameVersion&0x03) << 2);
	fcfPacked |= ((srcAddrMode&0x03) << 0);
	return fcfPacked;
}

uint8_t radio_reg_read(uint8_t address){
	address = (address & 0x3F) | 0b10000000;
	SS_low();
	//Send the register address
	SPI_transaction(address);
	//Once more for reg read
	uint8_t regValue = SPI_transaction(0);
	SS_high();
	return regValue;
}

uint8_t radio_reg_write(uint8_t address, uint8_t data){
	address = (address & 0x3F) | 0b11000000;
	SS_low();
	//Send the register address
	SPI_transaction(address);
	//Once more for reg write
	uint8_t regValue = SPI_transaction(data);
	SS_high();
	return regValue;
}

void radio_Frame_write(rawFrame &outFrame){
	SS_low();
	//fb write mode
	SPI_transaction(0b01100000);
	//Send the size info (PHR)
	SPI_transaction(outFrame.size());
	//Send the data
	for (int i = 0; i < outFrame.size();i++){
		SPI_transaction(outFrame[i]);
	}
	SS_high();
	//Now tell the radio to actually send the frame
	radio_transmit();
}

uint8_t radio_Frame_read(rawFrame &inFrame){
	SS_low();
	//fb read mode
	SPI_transaction(0b00100000);
	//Read the size info (PHR)
	inFrame.setSize(SPI_transaction(0));
	//Data is initalized by setSize(...)
	//Read the data
	for (int i = 0; i < inFrame.size();i++){
		inFrame[i] = SPI_transaction(0);
	}
	//Once more to pull the LQI
	uint8_t LQI = SPI_transaction(0);
	SS_high();
	return LQI;
}

void radio_enable_LED(){
	REG_OR_ENABLE(TRX_CTRL_1,_BV(PA_EXT_EN));
}
void radio_enable_CRC(){
	radio_reg_write(TRX_CTRL_1,_BV(TX_AUTO_CRC_ON));
}
void radio_set_channel(uint8_t channel){
	uint8_t temp = radio_reg_read(RG_PHY_CC_CCA);
	radio_reg_write(RG_PHY_CC_CCA,(temp & 0xE0) |(channel & 0x1F));
}
void radio_set_address(uint16_t address){
	radio_reg_write(RG_SHORT_ADDR_0,address & 0x00FF);
	radio_reg_write(RG_SHORT_ADDR_1,(address & 0xFF00 >> 8));
}
void radio_set_pan_id(uint16_t pan_id){
	radio_reg_write(RG_PAN_ID_0,pan_id & 0x00FF);
	radio_reg_write(RG_PAN_ID_1,(pan_id & 0xFF00 >> 8));
}
void radio_set_mode(uint8_t newMode){
	radio_reg_write(RG_TRX_STATE,newMode);
	//This handles conditions where the status for newmode is not newmode
	if(CMD_FORCE_TRX_OFF == newMode){
		newMode = TRX_OFF;
	}
	//Wait until the state is set
	while( !(radio_reg_read(RG_TRX_STATUS) & newMode));
}
void radio_transmit(){
	//Need to be in PLL_ON to start transmitting
	radio_set_mode(CMD_PLL_ON);
	radio_reg_write(RG_TRX_STATE,CMD_TX_START);
	//This is needed, but I have yet to figure out why
	_delay_ms(10);
	//Wait untill TX is done
	while((radio_reg_read(RG_TRX_STATUS) & 0x1F) == BUSY_TX);
	//Go back to recieve mode
	radio_set_mode(RX_ON);
}
void radio_setup(){
	spi_setup();
	
	SET_SLP_TR_LOW();
	RADIO_RST();
	
	//Set radio state to trx_off
	radio_set_mode(CMD_FORCE_TRX_OFF);
	
	//Use max power (default)
	//uint8_t power = 0x00;
	//radio_reg_write(RG_PHY_TX_PWR,0b11000000 | (power & 0x0F));
	
	// Use automatic CRC on transmit
	radio_enable_CRC();
	//Enable radio LED on transmit
	radio_enable_LED();
	
	//got these from defconfig.mk for my transmitter
	radio_set_channel(0x15);
	radio_set_address(0x0001);
	radio_set_pan_id(0x8842);


	//Set the device role (?What is this?)
    //tat_set_device_role(false);
    // Set up CCA (?What is this?)
    //tat_configure_csma(234, 0xE2);
	
	//Enable the recieving data interupt
	//Note, when not using proper frame protocol, use IRQ_RX_START instead of IRQ_TRX_END
	radio_reg_write(RG_IRQ_MASK,_BV(IRQ_TRX_END));
	
	//Set state to RX_ON
	radio_set_mode(RX_ON);
}