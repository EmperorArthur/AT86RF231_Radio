//Wireless In home Power mOnitoring Datalogger
//Copyright Arthur Moore 2012

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
	BlinkLED(1000,1);

	printf("Debugger Initalized, awaiting commands\n\r");
	
	//Enable interupts (mainly the timer, and ADC interupts)
	sei();
}

unsigned int counter = 33;
	
//This is a quick interpreter that allows for direct memory access by the user
void loop(){
	char controllChar;
	volatile unsigned char * address = 0;
	int temp = 0;
	uint8_t value = 0;
	frameControlField fcf1;
	radioFrame aFrame;
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
					printf("%.2x\n",*address);
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
					printf("%.2x\n",*address);
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
				//Reset the radio
				case 'x':
					RADIO_RST();
					break;
				//Read a radio frame
				case 'f':
					radio_Frame_read(aFrame);
					printf("\nFrame Size:  %i\n",aFrame.size());
					printf("Sequence Number is:  %u\n",aFrame.sequenceNumber);
					printf("%s\n",aFrame.data.c_str());
					printf("CRC is:  %u\n",aFrame.crc16);
					break;
				//Transmit a radio frame
				case 't':
					aFrame.sequenceNumber = counter++;
					aFrame.data = "This_is_a_test.01234567890\n";
					printf("%s\n",aFrame.data.c_str());
					radio_Frame_write(aFrame);
					break;
			}
			break;
	}
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
		uint8_t radio_interupt_vector = radio_reg_read(0x0F);
		//If we have a frame waiting to be read
		if(radio_interupt_vector & 0x08){
			//Read and print it
			radioFrame aFrame;
			radio_Frame_read(aFrame);
			printf("%s",aFrame.data.c_str());
		}else{
			//Let the user know
			printf("\nUnhandled radio interupt:  %u\n",radio_interupt_vector);
			printf("IRQ mask is:  %u\n",radio_reg_read(0x0E));
			printf("Radio mode is:  %u\n",radio_reg_read(0x01));
		}
	}
	sei();
}

int main(){
	setup();
	for(;;){
		loop();
		//This slows register reads to a crawl
		BlinkLED(500,3);
	}
	return 0;
}
