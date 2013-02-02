//Radio functions
//Copyright Arthur Moore 2012
//GPL V3
#ifndef RADIO_H
#define RADIO_H

//Needed for uint8_t
#include <avr/io.h>
//This contains all the AVR specific code
#include "radio-avr.h"

//This is the data in the radio frame
//Warning, changing the data size deletes everything
struct radioData{
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
struct radioFrame {
	public:
		radioFrame();
		radioFrame(uint8_t newSize);
		~radioFrame();
		void setSize(uint8_t newSize);
		uint8_t size();
		uint8_t & operator[] (uint8_t location);
		//These probably should be private, but I'm lazy
		union{
			uint8_t crc[2];
			uint16_t crc16;
		};
		radioData data; //mySize -2 (since the last two bytes are the crc
	private:
		//Max size is 127 bytes
		static const uint8_t maxSize = 127;
};


uint8_t radio_reg_read(uint8_t address);
uint8_t radio_reg_write(uint8_t address, uint8_t data);
//Write to the frame buffer
void radio_Frame_write(radioFrame &outFrame);
//read from the frame buffer
//Returns the LQI (Link Quality Information)
uint8_t radio_Frame_read(radioFrame &inFrame);
//Enable the LED on the transmit side (don't know if the LED works)
void radio_enable_LED();
// Use automatic CRC on transmit
void radio_enable_CRC();
//Set radio channel (must be: 11 < channel < 26)
//See P.123 of the datasheet for how everything translates
void radio_set_channel(uint8_t channel);
//Set the radio address
void radio_set_address(uint16_t address);
//Set the radio pan_id
void radio_set_pan_id(uint16_t pan_id);
//Set the radio mode
//Not all modes work with this function
//If something breaks, this function will HANG.
//No errors are currently returned
void radio_set_mode(uint8_t newMode);
//Actually transmit a radio frame, returning to RX_ON mode when done
void radio_transmit();
void radio_setup();

#endif