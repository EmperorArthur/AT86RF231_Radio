//SPI functions
//Copyright Arthur Moore 2012
//GPL V3
#ifndef SPI_H
#define SPI_H

#define SSDDR DDRB
#define SCK PB5
#define MISO PB4
#define MOSI PB3
#define SS PB2
#define SS_high() PORTB |= _BV(SS)
#define SS_low() PORTB &= ~_BV(SS)
//Every spi transaction is both a read and a write.
//The return is what's read and the "toWrite" is what's written
void spi_setup(){
	//Set input and outputs for pins
	SSDDR |= _BV(SCK) | _BV(MOSI) | _BV(SS);
	SSDDR &= ~_BV(MISO);
	//make sure select is high
	SS_high();
	//Set up SPI (Enable in Master mode with a clock divider of /64)
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(SPR1);
	//			 (Sample on rising edge, setup on falling edge, MSB first)
	SPCR &= ~_BV(CPOL) | ~_BV(CPHA) | ~_BV(DORD);
}
uint8_t SPI_transaction(uint8_t toWrite){
	SPDR = toWrite;
	//Spinlock untill the transaction is complete
	while(!(SPSR & _BV(SPIF)));
	return SPDR;
}


#endif