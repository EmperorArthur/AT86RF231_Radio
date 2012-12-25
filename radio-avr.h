//AVR specific Radio functions
//Copyright Arthur Moore 2012
//GPL V3

//This contains all the AVR specific code
#ifndef RADIO_AVR_H
#define RADIO_AVR_H

#include <avr/io.h>
#include <util/delay.h>
#define RADIO_RST() DDRD |= _BV(PD6);PORTD &= ~_BV(PD6);_delay_ms(1000);PORTD |= _BV(PD6);
#define SET_SLP_TR_LOW() DDRB |= _BV(PB0); PORTB &= ~_BV(PB0);

#endif