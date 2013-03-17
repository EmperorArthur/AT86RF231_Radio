//Example Radio Program
//Copyright Arthur Moore 2013

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdio.h>
#include "communication.h"
#include "LED.h"
#include "radio.h"
#include "at86rf230_registermap.h"

unsigned int counter = 0;
radioFrame aFrame;

void setup()
{
	//Disable interupts during setup
	cli();
	communication_setup();
	radio_setup();
	//Set up IRQ pin for radio interupt (input, no pull up)
	DDRB &= ~_BV(PB1);
	PORTB &= ~_BV(PB1);
	PCICR |= _BV(PCIE0);
	PCMSK0 |= _BV(PCINT1);
	
	//Set up the radio frame to be sent
	aFrame.fcf.dstAddrMode = SIXTEEN;
	aFrame.fcf.srcAddrMode = SIXTEEN;
	aFrame.dstAddr.pan_id = 0x1234;
	aFrame.dstAddr.address = 0x5678;
	aFrame.srcAddr.pan_id = 0x9ABC;
	aFrame.srcAddr.address = 0xDEF0;
	
	BlinkLED(1000,1);
	printf("Radio Example Initalized\n\r");
	
	//Enable interupts (mainly the timer, and ADC interupts)
	sei();
}

//Transmit a radio frame
void loop(){
	aFrame.sequenceNumber = counter++;
	aFrame.data = "This_is_a_test.\n";
	printf("Sending:  %s\n",aFrame.data.c_str());
	radio_Frame_write(aFrame);
}

//This is in case something went wrong
ISR(BADISR_vect){
	cli();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		printf("Warning:  Uncaught Interupt Detected!!!\n\r");
		BlinkLED(100,20);
	}
}
//This handles if the radio toggled the IRQ line
ISR(PCINT0_vect){
	cli();
	//If IRQ is high
	if(PINB & _BV(PB1)){
		//Read the interupt
		uint8_t radio_interupt_vector = radio_reg_read(RG_IRQ_STATUS);
		//If we have a frame waiting to be read
		if(radio_interupt_vector & _BV(IRQ_TRX_END)){
			//Make sure the CRC is valid
			if(radio_reg_read(RG_PHY_RSSI)&_BV(RX_CRC_VALID)){
				//Read and print it
				radioFrame aFrame;
				radio_Frame_read(aFrame);
				printf("%s",aFrame.data.c_str());
			}else{
				//Read it anyways as raw data
				radioData someData;
				radio_Frame_read(someData);
				//For some odd reason, transmit is causing me to see 0 length frames.
				//Ignore them, but print anything else recieved
				if(someData.size() != 0){
					printf("\nRadio frame with invalid CRC Recieved\n");
					printf("Frame Size:  %u\n",someData.size());
					printf("%s\n",someData.c_str());
				}
			}
		}else{
			//Let the user know
			printf("\nUnhandled radio interupt:  %u\n",radio_interupt_vector);
			printf("IRQ mask is:  %u\n",radio_reg_read(RG_IRQ_MASK));
			printf("Radio mode is:  %u\n",radio_reg_read(RG_TRX_STATUS));
		}
	}
	sei();
}

int main(){
	setup();
	for(;;){
		//loop();
		BlinkLED(500,3);
	}
	return 0;
}
