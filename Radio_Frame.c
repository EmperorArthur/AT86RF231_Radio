//IEEE 802.15.4 Radio Frame stuff
//Copyright Arthur Moore 2012
//GPL V3

#include "Radio_Frame.h"
//for malloc and free
#include <stdlib.h>
//For stderr
//#include "communication.h"
//#define __ASSERT_USE_STDERR
#include <assert.h>

radioFrame::radioFrame(){
	TempFcf = 0;
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
		return TempFcf8[1];
	}else if(location == 1){
		return TempFcf8[0];
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
void radioFrame::pack(){
	TempFcf = fcf.pack();
}
void radioFrame::unpack(){
	//fcf.unpack(TempFcf);//////////////////////////////////////////////////////////////////
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

