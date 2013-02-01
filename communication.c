//Radio and Serial communication functions
//Copyright Arthur Moore 2012
//GPL V3
#include "communication.h"
#include <avr/io.h>
#include "uart.h"

int com_getchar_f(FILE *stream){
	return (int) com_getchar();
}
unsigned char com_getchar(){
	return (unsigned char) uart_getchar(1);
}
int com_putchar_f(char outChar,FILE *stream){
	com_putchar(outChar);
	return 0;
}
void com_putchar(unsigned char outChar){
	//Serial expects crlf instead of just lf
	if('\n' == outChar){
		com_putchar('\r');
	}
	uart_putchar(outChar);
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
	// UART
	DDRD |= (1<<PD1);
	DDRD |= (1<<PD2);
	uart_setup();
	
	#ifdef __cplusplus
		com_stream = fdevopen(
			com_putchar_f,
			com_getchar_f);
		stdout = stdin = com_stream;
	#else
		stdout = &com_stream;
	#endif
	return 0;
}