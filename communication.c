//Radio and Serial communication functions
//Copyright Arthur Moore 2013
//GPL V3
#include "communication.h"
#include "serial-avr.h"

ringBuff::ringBuff(){
	writePosition = &buffer[0];
	readPosition = &buffer[0];
	spaceUsed = 0;
}
int ringBuff::push(char aChar){
	if(spaceUsed < size){
		*writePosition = aChar;
		writePosition++;
		if(writePosition == &buffer[size-1]){
			writePosition = &buffer[0];
		}
		spaceUsed++;
		return 0;
	}else{
		return -1;
	}
}
char ringBuff::pull(){
	char outChar;
	while( 0 == spaceUsed);		//Spinlock untill buffer has data
	outChar = *readPosition;
	readPosition++;
	if(readPosition == &buffer[size-1]){
			readPosition = &buffer[0];
	}
	spaceUsed--;
	return outChar;
}
int ringBuff::getUsed(){
	return spaceUsed;
}

int com_getchar_f(FILE *stream){
	return (int) com_getchar();
}
unsigned char com_getchar(){
	return (unsigned char) serialGetChar();
}
int com_putchar_f(char outChar,FILE *stream){
	com_putchar(outChar);
	return 0;
}
void com_putchar(unsigned char outChar){
	//Serial expects crlf instead of just lf
	if('\n' == outChar){
		serialPutChar('\r');
	}
	serialPutChar(outChar);
}

#ifdef __cplusplus
	extern "C"{
		FILE * com_stream;
	}
#else
	static FILE com_stream = FDEV_SETUP_STREAM(
		com_putchar_f,
		com_getchar_f,
		_FDEV_SETUP_RW
	);
#endif

//Set up the serial connection
int communication_setup(){
	serialSetup();
	
	#ifdef __cplusplus
		com_stream = fdevopen(
			com_putchar_f,
			com_getchar_f);
		stdout = stdin = stderr = com_stream;
	#else
		stdout = stderr = &com_stream;
	#endif
	return 0;
}