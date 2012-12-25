//Radio functions
//Copyright Arthur Moore 2012
//GPL V3
#ifndef RADIO_H
#define RADIO_H

//Needed for uint8_t
#include <avr/io.h>
//This contains all the AVR specific code
#include "radio-avr.h"

//This is the frame to send to the radio
//Warning, changing the frame size deletes everything in data
struct radioFrame {
	public:
		radioFrame();
		radioFrame(uint8_t newSize);
		~radioFrame();
		void setSize(uint8_t newSize);
		uint8_t getSize();
		uint8_t size();
		void setDataPoint(uint8_t location,uint8_t newDataPoint);
		uint8_t getDataPoint(uint8_t location);
	private:
		uint8_t mySize; //Max size is 127 bytes
		uint8_t * data;
};


uint8_t radio_reg_read(uint8_t address);
uint8_t radio_reg_write(uint8_t address, uint8_t data);
//Write to the frame buffer
void radio_Frame_write(radioFrame outFrame);
//read from the frame buffer
//Returns the LQI (Link Quality Information)
uint8_t radio_Frame_read(radioFrame inFrame);
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
void radio_setup();

#endif