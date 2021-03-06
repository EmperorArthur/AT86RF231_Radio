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
	dstAddr.pan_id = 0;
	dstAddr.address = 0;
	srcAddr.pan_id = 0;
	srcAddr.address = 0;
}
radioFrame::radioFrame(uint8_t newSize){
	crc16=0;
	setSize(newSize);
}
radioFrame::~radioFrame(){
	//Nothing to do, data handles it's own cleanup
}
void radioFrame::setSize(uint8_t newSize){
	//mySize needs to be signed
	//dataPlus is always present in a frame, and is the minimum frame size.
	int mySize = newSize-dataPlus;
	if(SIXTEEN == fcf.dstAddrMode){
		mySize -= 4;
	}
	if(SIXTEEN == fcf.srcAddrMode){
		mySize -= 4;
	}
	//If we end up with a negative, then just set data to zero
	if(mySize < 0 || maxSize <= newSize){
		data.setSize(0);
	}else{
		data.setSize(mySize);
	}
}
uint8_t radioFrame::size(){
	uint8_t mySize = data.size()+dataPlus;
	if(SIXTEEN == fcf.dstAddrMode){
		mySize += 4;
	}
	if(SIXTEEN == fcf.srcAddrMode){
		mySize += 4;
	}
	return mySize;
}
//This lets me read and write to this thing as though it where a simple array
uint8_t & radioFrame::operator[] (uint8_t location){
	uint8_t mySize = data.size()+dataPlus;
	uint8_t dataOffset = 3;
	uint8_t srcAddrOffset = 0;
	if(SIXTEEN == fcf.dstAddrMode){
		mySize += 4;
		dataOffset +=4;
		srcAddrOffset +=4;
	}
	if(SIXTEEN == fcf.srcAddrMode){
		mySize += 4;
		dataOffset +=4;
	}
	//Location is unsigned, so not worrying about negative numbers
	assert(location < mySize);
	
	//Handle fcf
	if(location == 0){
		//return upper 8 bits
		return *(((uint8_t*)&TempFcf)+1);
	}else if(location == 1){
		//Return lower 8 bits
		return (uint8_t&)TempFcf;
	//Handle sequence number
	}else if(location == 2){
		return sequenceNumber;
	}
	
	if(SIXTEEN == fcf.dstAddrMode){
		if(location == 3){
			//return upper 8 bits
			return *(((uint8_t*)&dstAddr.pan_id)+1);
		}else if(location == 4){
			//Return lower 8 bits
			return (uint8_t&)dstAddr.pan_id;
		}else if(location ==5){
			//return upper 8 bits
			return *(((uint8_t*)&dstAddr.address)+1);
		}else if(location == 6){
			//Return lower 8 bits
			return (uint8_t&)dstAddr.address;
		}
	}
	if(SIXTEEN == fcf.srcAddrMode){
		if(location == 3 + srcAddrOffset){
			//return upper 8 bits
			return *(((uint8_t*)&srcAddr.pan_id)+1);
		}else if(location == 4 + srcAddrOffset){
			//Return lower 8 bits
			return (uint8_t&)srcAddr.pan_id;
		}else if(location ==5 + srcAddrOffset){
			//return upper 8 bits
			return *(((uint8_t*)&srcAddr.address)+1);
		}else if(location == 6 + srcAddrOffset){
			//Return lower 8 bits
			return (uint8_t&)srcAddr.address;
		}
	}
	
	//Handle crc
	if(location == mySize - 2){
		return crc[1];
	}else if(location == mySize - 1){
		return crc[0];
	}
	//If nothing else, return data
	return data[location - dataOffset];
}
void radioFrame::pack(){
	TempFcf = fcf.pack();
}
void radioFrame::unpack(){
	fcf.unpack(TempFcf);
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
//Unpack two bytes into an fcf
void frameControlField::unpack(uint16_t inFCF){
	frameType =			(ft)((inFCF >> 12) & 0x07);
	securityEnabled = 	(inFCF >> 11) & 0x01;
	framePending =		(inFCF >> 10) & 0x01;
	requestACK =		(inFCF >> 9) & 0x01;
	intraPAN =			(inFCF >> 8) & 0x01;
	dstAddrMode =		(AddressingMode)((inFCF >> 5) & 0x03);
	frameVersion =		(inFCF >> 2) & 0x03;
	srcAddrMode =		(AddressingMode)((inFCF >> 0) & 0x03);
}

