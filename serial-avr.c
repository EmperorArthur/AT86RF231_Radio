//AVR Specific serail communication functions
//Copyright Arthur Moore 2013
//GPL V3

#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial-avr.h"
#include "communication.h"	//For ringBuff

//I don't like using globals, but since I'm writing to it using an interupt, I don't really have much choice in the matter
ringBuff readBuffer;

void serialSetup(uint16_t speed){
	//a 12 bit value used for setting the speed
	uint16_t UBRRCount = 0;
	//Enable RX and TX for usart, also enable RX interupt
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
	//Asynchronous, no parity, 1 stop bit, 8 bits (This is the default, but setting it explicitly just to be sure)
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	//Calculate the baud rate
	UBRRCount = (F_CPU/(16UL*speed)) -1;
	//If we're too fast (UBRR is too big)
	if (UBRRCount & 0xF000){
		//Double the clock rate, and recalculate UBRR
		UCSR0A |= _BV(U2X0);
		UBRRCount = (F_CPU/(8UL*speed)) -1;
	}
	//If the speed is too fast (speed is
	//The first 4 bits of UBRR0H need to be 0 (according to the spec sheet)
	UBRR0H = 0x0F & (UBRRCount >> 8);
	//Discard all the upper bits for UBRR0L
	UBRR0L = 0xFF & UBRRCount;
}
void serialPutChar(char aChar){
	//Spin lock untill the transmit buffer is empty
	while( !(UCSR0A & _BV(UDRE0)) );
	//Send char to tx buffer
	UDR0 = aChar;
}
char serialGetChar(){
	return readBuffer.pull();
}

//The isr for recieving a charachter:
ISR(USART_RX_vect){
	//Read the charachter
	char incoming = UDR0;
	//Echo back the charachter
	serialPutChar(incoming);
	//Push the charachter into the buffer
	readBuffer.push(incoming);
}