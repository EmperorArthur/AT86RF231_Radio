//SPI functions
//Copyright Arthur Moore 2012
//GPL V3
#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

#define SSDDR DDRB
#define SCK PB5
#define MISO PB4
#define MOSI PB3
#define SS PB2
#define SS_high() PORTB |= _BV(SS)
#define SS_low() PORTB &= ~_BV(SS)

void spi_setup();
//Every spi transaction is both a read and a write.
//The return is what's read and the "toWrite" is what's written
uint8_t SPI_transaction(uint8_t toWrite);

#endif