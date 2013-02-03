//IEEE 802.15.4 Radio Frame stuff
//Copyright Arthur Moore 2012
//GPL V3
#ifndef RADIO_FRAME_H
#define RADIO_FRAME_H

//Needed for uint8_t
#include <avr/io.h>

//This is a raw radio frame, used as a parent class only
struct rawFrame{
	virtual void setSize(uint8_t newSize);
	virtual uint8_t size();
	virtual uint8_t & operator[] (uint8_t location);
	virtual void pack(){};
	virtual void unpack(){};
};

//This is the header for the radio frame
//It is two bytes worth of data packed in here
enum AddressingMode {NOADDR = 0,RESERVED = 1, SIXTEEN = 2, SIXTYFOUR = 3};
struct frameControlField{
	frameControlField();
	uint16_t pack();				//Returns the 2 bytes of packed data
	void unpack(uint16_t inFCF);	//Unpacks an incoming fcf
	
	//4-7 are reserved
	enum ft {BEACON = 0, DATA = 1, ACK = 2, MACCMD = 3, R4=4,R5=5,R6=6,R7=7} frameType;
	bool securityEnabled;	//This should always be false
	bool framePending;		//Not sure about this one, leave it false
	bool requestACK;		//Only use if in TX_ARET state, and reciever is in RX_AACK state
							//Only use if frameType is DATA or MACCMD
	bool intraPAN;			//if true, PAN-ID is omitted from the Source address field
	AddressingMode dstAddrMode;
	int frameVersion;		//0 is IEEE...2003, 1 is IEEE...2006, 2-4 are reserved.
	AddressingMode srcAddrMode;
};

//This is the data in the radio frame
//Warning, changing the data size deletes everything
struct radioData: public rawFrame{
	public:
		radioData();
		~radioData();
		radioData(uint8_t newSize);
		void setSize(uint8_t newSize);
		uint8_t size();
		uint8_t & operator[] (uint8_t location);
		void operator= (char* cString);
		char * c_str();
	private:
		//Max size is 127 bytes -2 for crc16
		static const uint8_t maxSize = 125;
		uint8_t mySize;
		uint8_t * data;	//Data is 1 byte larger than provisioned to allow for conversion into a C string
		
};
//This is the frame to send to the radio
//Warning, changing the frame size deletes everything in data
struct radioFrame: public rawFrame{
	public:
		radioFrame();
		radioFrame(uint8_t newSize);
		~radioFrame();
		void setSize(uint8_t newSize);
		uint8_t size();
		//Pack and unpack the fcf
		void pack();
		void unpack();
		uint8_t & operator[] (uint8_t location);
		//This is the actual data in the frame
		//These probably should be private, but I'm lazy
		//Note for the unions: (0 is the low bit, 1 is the high bit)
		frameControlField fcf;
		uint8_t sequenceNumber;
		radioData data;
		union{
			uint8_t crc[2];
			uint16_t crc16;
		};
	private:
		//Max size is 127 bytes
		static const uint8_t maxSize = 127;
		//This is how many extra bytes to add to the data size
		static const uint8_t dataPlus = 5;
		//Use pack and unpack functions to set/read this
		uint16_t TempFcf;
};

#endif