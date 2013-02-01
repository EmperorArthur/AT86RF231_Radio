//Radio and Serial communication functions
//Copyright Arthur Moore 2012
//GPL V3
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>

int communication_setup();				//Set up Serial communication

int com_getchar_f(FILE *stream);
unsigned char com_getchar();
int com_putchar_f(char outChar,FILE *stream);
void com_putchar(unsigned char outChar);

#endif