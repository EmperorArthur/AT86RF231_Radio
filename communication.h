//Radio and Serial communication functions
//Copyright Arthur Moore 2013
//GPL V3
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>

int communication_setup();				//Set up Serial communication

int com_getchar_f(FILE *stream);
unsigned char com_getchar();
int com_putchar_f(char outChar,FILE *stream);
void com_putchar(unsigned char outChar);

//This uses a ring buffer as a first in first out buffer
//The implementation makes it interupt safe without using atomic blocks or cli();
class ringBuff{
	public:
		ringBuff();
		const static int size = 128;
		int push(char aChar);	//Push a char into the buffer (returns '-1' if full, otherwise 0)
		char pull();			//remove a char from the buffer (spinlocks if the buffer is empty)
		int getUsed();
	private:
		volatile char buffer[size];
		volatile char * writePosition;
		volatile char * readPosition;
		volatile int spaceUsed;
};

#endif