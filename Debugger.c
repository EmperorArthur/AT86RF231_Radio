//Wireless In home Power mOnitoring Datalogger
//Copyright Arthur Moore 2012

//macros to turn these #defines into strings when needed (the precompiler is weird)
#define STRINGIFY(str) TOSTR(str)
#define TOSTR(str) #str

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "communication.h"
#include "LED.h"

void setup()
{
	//Disable interupts during setup
	cli();
	//Set up communication
	communication_setup();
  
	BlinkLED(1000,1);

	//Enable interupts (mainly the timer, and ADC interupts)
	sei();
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
		//loop();
		sprint("It works\n\r");
		BlinkLED(500,3);
	}
	return 0;
}
