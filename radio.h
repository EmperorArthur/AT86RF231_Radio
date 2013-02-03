//Radio functions
//Copyright Arthur Moore 2012
//GPL V3
#ifndef RADIO_H
#define RADIO_H

//Needed for uint8_t
#include <avr/io.h>
#include "Radio_Frame.h"
//This contains all the AVR specific code
#include "radio-avr.h"

//Easily defined stuff not in the registermap
#define IRQ_PLL_LOCK 0
#define IRQ_RX_START 2
#define IRQ_TRX_END 3
#define RX_CRC_VALID 7

uint8_t radio_reg_read(uint8_t address);
uint8_t radio_reg_write(uint8_t address, uint8_t data);
//Write to the frame buffer
void radio_Frame_write(rawFrame &outFrame);
//read from the frame buffer
//Returns the LQI (Link Quality Information)
uint8_t radio_Frame_read(rawFrame &inFrame);
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