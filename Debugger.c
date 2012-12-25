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
#include "radio.h"
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

//Possibly put this in radio.h/c
#include <util/delay.h>
#define RADIO_RST() DDRD |= _BV(PD6);PORTD &= ~_BV(PD6);_delay_ms(1000);PORTD |= _BV(PD6);
#define SET_SLP_TR_LOW() DDRB |= _BV(PB0); PORTB &= ~_BV(PB0);

void setup()
{
	//Disable interupts during setup
	cli();
	communication_setup();
	SET_SLP_TR_LOW();
	RADIO_RST();
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
				//Set the radio's channel
				case 'c':
					scanf("%i",&temp);
					radio_set_channel((uint8_t) temp);
					break;
				case 'x':
					RADIO_RST();
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
