//Radio and Serial communication functions
//Copyright Arthur Moore 2012
//GPL V3
#include "communication.h"
#include <avr/io.h>
#include "uart.h"

#ifdef __cplusplus
extern "C"{
	FILE * com_stream;
}
#else
static FILE com_stream = FDEV_SETUP_STREAM(
	uart_putchar_f,
	uart_getchar_f,
	_FDEV_SETUP_RW
);
#endif



//Set up both the UART and the Radio
int communication_setup(){
	// UART
	DDRD |= (1<<PD1);
	DDRD |= (1<<PD2);
	uart_setup();
	
	#ifdef __cplusplus
	com_stream = fdevopen(
		uart_putchar_f,
		uart_getchar_f);
	stdout = stdin = com_stream;
	#else
	stdout = &com_stream;
	#endif
	return 0;
}