//AVR Specific serail communication functions
//Copyright Arthur Moore 2013
//GPL V3

#ifndef SERIAL_AVR_H
#define SERIAL_AVR_H

void serialSetup(uint16_t speed = 9600);
void serialPutChar(char aChar);
char serialGetChar();

#endif

