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
	if(2 >= newSize || maxSize <= newSize){
		data.setSize(0);
	}else{
		data.setSize(newSize-2);
	}
}
uint8_t radioFrame::size(){
	return data.size()+2;
}
//This lets me read and write to this thing as though it where a simple array
uint8_t & radioFrame::operator[] (uint8_t location){
	uint8_t mySize = data.size()+2;
	//Location is unsigned, so not worrying about negative numbers
	assert(location < mySize);
	//Handle crc
	if(location == mySize - 2){
		return crc[0];
	}else if(location == mySize - 1){
		return crc[1];
	}else{
		return data[location];
	}
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
	//Find the size of the string
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
	radio_reg_write(RG_SHORT_ADDR_1,address & 0xFF00);
}
void radio_set_pan_id(uint16_t pan_id){
	radio_reg_write(RG_PAN_ID_0,pan_id & 0x00FF);
	radio_reg_write(RG_PAN_ID_1,pan_id & 0xFF00);
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
	//Wait untill we're back in PLL_ON (TX is done)
	while( !(radio_reg_read(RG_TRX_STATUS) & CMD_PLL_ON));
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
	//I'm ignoring proper frame protocol, so recieve done doesn't work
	radio_reg_write(RG_IRQ_MASK,_BV(IRQ_RX_START));
	
	//Set state to RX_ON
	radio_set_mode(RX_ON);
}