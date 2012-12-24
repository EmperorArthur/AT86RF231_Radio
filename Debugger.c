//Wireless In home Power mOnitoring Datalogger
//Copyright Arthur Moore 2012

//macros to turn these #defines into strings when needed (the precompiler is weird)
#define STRINGIFY(str) TOSTR(str)
#define TOSTR(str) #str

//Human readable output, or stuff used with my python scripts
//#define HUMAN

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <assert.h>
#include <stdio.h>
#include "communication.h"
#include "LED.h"
#include "spi.h"

//BEGIN Nesicary C++ crap (avr-g++ needs these)
//I did not write these
#include <stdlib.h> 

void * operator new(size_t size); 
void operator delete(void * ptr);

void * operator new(size_t size) 
{ 
  return malloc(size); 
} 

void operator delete(void * ptr) 
{ 
  free(ptr); 
}

__extension__ typedef int __guard __attribute__((mode (__DI__))); 

extern "C" int __cxa_guard_acquire(__guard *); 
extern "C" void __cxa_guard_release (__guard *); 
extern "C" void __cxa_guard_abort (__guard *); 

int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);}; 
void __cxa_guard_release (__guard *g) {*(char *)g = 1;}; 
void __cxa_guard_abort (__guard *) {}; 

extern "C" void __cxa_pure_virtual(void); 
void __cxa_pure_virtual(void) {}; 


using namespace std;
//END Nesicary C++ crap

//BEGIN radio.h/c
//Radio functions
//Copyright Arthur Moore 2012
//GPL V3
#ifndef RADIO_H
#define RADIO_H


//#include "spi.h"
#include "at86rf230_registermap.h"
//I'm using an AT86RF231, so I have some extra features, like some LED Outs
#define PA_EXT_EN	7
#define TRX_CTRL_1	(0x04)

//This is the frame to send to the radio
//Warning, changing the frame size deletes everything in data
struct radioFrame {
	public:
		radioFrame();
		radioFrame(uint8_t newSize);
		void setSize(uint8_t newSize);
		uint8_t getSize();
		uint8_t size();
		void setDataPoint(uint8_t location,uint8_t newDataPoint);
		uint8_t getDataPoint(uint8_t location);
		uint8_t * data;
	private:
		uint8_t mySize; //Max size is 127 bytes
};

#endif
radioFrame::radioFrame(){
	mySize = 0;
	data = NULL;
}
radioFrame::radioFrame(uint8_t newSize){
	mySize = newSize;
	data = (uint8_t *) malloc(mySize);
}
void radioFrame::setSize(uint8_t newSize){
	mySize = newSize;
	if (NULL != data){
		free(data);
	}
	if(0 != newSize){
		data = (uint8_t *) malloc(mySize);
	}
}
uint8_t radioFrame::getSize(){
	return mySize;
}
uint8_t radioFrame::size(){
	return getSize();
}
void radioFrame::setDataPoint(uint8_t location,uint8_t newDataPoint){
	assert(NULL != data);
	assert( (0 < location) && (location < mySize));
	data[location] = newDataPoint;
}
uint8_t radioFrame::getDataPoint(uint8_t location){
	assert(NULL != data);
	assert( (0 < location) && (location < mySize));
	return data[location];
}

uint8_t radio_reg_read(uint8_t address){
	address = (address & 0x7F) | 0b10000000;
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

//Write to the frame buffer
void radio_Frame_write(radioFrame outFrame){
	SS_low();
	//fb write mode
	SPI_transaction(0b01100000);
	//Send the size info (PHR)
	SPI_transaction(outFrame.size());
	//Send the data
	for (int i = 0; i < outFrame.size();i++){
		SPI_transaction(outFrame.getDataPoint(i));
	}
	SS_high();
}

//read from the frame buffer
//Returns the LQI (Link Quality Information)
uint8_t radio_Frame_read(radioFrame inFrame){
	SS_low();
	//fb read mode
	SPI_transaction(0b00100000);
	//Read the size info (PHR)
	inFrame.setSize(SPI_transaction(0));
	//Data is initalized by setSize(...)
	//Read the data
	for (int i = 0; i < inFrame.size();i++){
		inFrame.setDataPoint(i,SPI_transaction(0));
	}
	//Once more to pull the LQI
	uint8_t LQI = SPI_transaction(0);
	SS_high();
	return LQI;
}
void radio_enable_LED(){
	//Enable the LED Outs on the Radio
	uint8_t temp = radio_reg_read(TRX_CTRL_1);
	temp |= _BV(PA_EXT_EN);
	radio_reg_write(TRX_CTRL_1,temp);
}
void radio_setup(){
	spi_setup();
	//radio_enable_LED();
	//Set radio state to trx_off
	//radio_reg_write(RG_TRX_STATE,_BV(CMD_FORCE_TRX_OFF));
	//Wait untill the state is trx_off
	//while( !(radio_reg_read(RG_TRX_STATUS) & TRX_OFF));
	
}

//END radio.h/c
	
void setup()
{
	//Disable interupts during setup
	cli();
	communication_setup();
	radio_setup();
	BlinkLED(1000,1);

	printf("Debugger Initalized, awaiting commands\n\r");
	
	//Enable interupts (mainly the timer, and ADC interupts)
	sei();
}

//This is a quick interpreter that allows for direct memory access by the user
void loop(){
	char controllChar;
	volatile unsigned char * address = 0;
	int temp = 0;
	uint8_t value = 0;
	controllChar = getchar();
	switch (controllChar){
		//Memory Functions
		case 'm':
			controllChar = getchar();
			switch (controllChar){
				//Read from memory
				case 'r':
					scanf("%i",&temp);
					address = (unsigned char *) temp;
					#ifdef HUMAN
						printf("\n\r");
						printf("Memory in \"0x%.2x\" is:  0x%.2x\n\r",address,*address);
					#else
						printf("%.2x\n",*address);
					#endif
					break;
				//Write to memory
				case 'w':
					scanf("%i",&temp);
					address = (unsigned char *) temp;
					scanf("%i",&temp);
					value = (uint8_t) temp;
					//address = (unsigned char *) 43;
					*address = value;
					//Verify write successful
					#ifdef HUMAN
						printf("\n\r");
						printf("Memory in \"0x%.2x\" after write is:   0x%.2x\n\r",address,*address);
					#else
						printf("%.2x\n",*address);
					#endif
					break;
			}
			break;
		//SPI Functions (use 'l' and 'h' to set SS pin, use 's' to preform a transaction)
		case 's':
			controllChar = getchar();
			switch (controllChar){
				case 's':
					scanf("%i",&temp);
					printf("%.2x\n",SPI_transaction((uint8_t) temp));
					break;
				case 'l':
					SS_low();
					break;
				case 'h':
					SS_high();
					break;
			}
			break;
		//Radio functions
		case 'r':
			controllChar = getchar();
			switch (controllChar){
				//Read from a radio register;
				case 'r':
					scanf("%i",&temp);
					value = (uint8_t) temp;
					printf("%.2x\n",radio_reg_read(value));
					break;
				//Write to a radio register
				case 'w':
					scanf("%i",&temp);
					value = (uint8_t) temp;
					scanf("%i",&temp);
					//It's really address, value but I'm being lazy here
					printf("%.2x\n",radio_reg_write(value,(uint8_t) temp));
					break;
				//Test writing and reading a radio frame
				case 't':
					printf("Initiating radio frame test\n");
					radioFrame testFrame1(30);
					for (int i=0;i<testFrame1.size();i++){
						testFrame1.data[i] = testFrame1.size() - i;
					}
					radioFrame testFrame2;
					radio_Frame_write(testFrame1);
					radio_Frame_read(testFrame2);
					if(testFrame1.size() != testFrame2.size()){
						printf("Warning:  Size Mismatch %i, %i\n",testFrame1.size(),testFrame2.size());
					}
					for (int i=0;i<testFrame1.size();i++){
						if(testFrame1.data[i] != testFrame2.data[i]){
							printf("Warning:  Data Mismatch %i, %i\n",testFrame1.data[i],testFrame2.data[i]);
						}
					}
					printf("Radio frame test completed.\n");
					break;
			}
			break;
	}
}

//This is in case something went wrong
ISR(BADISR_vect){
	//cli();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		sprint("Warning:  Uncaught Interupt Detected!!!\n\r");
		BlinkLED(100,20);
	}
}

int main(){
	setup();
	for(;;){
		loop();
		#ifdef HUMAN
			sprint("It works\n\r");
			//BlinkLED(500,3);
		#endif
	}
	return 0;
}
