# Configuration options for the Microcontroller

MCU = atmega328
F_CPU = 16000000UL
AVRDUDE_PART = m328p
PROGRAMMER = arduino
EEPROM_CONFIG_OFFSET = 0x00
COMPORT = COM8
#COMPORT = /dev/ttyUSB0
ISPSPEED = 57600

# Fuse bits

HFUSE=0xDA
LFUSE=0xFF

# Build tools (defaults assume crosspack, winavr, etc. is in your PATH)

CC = avr-g++
OBJCOPY = avr-objcopy
AR = avr-ar
AVR_SIZE = avr-size
AVRDUDE = avrdude

# Application configuration

APP = Debugger
APP_OBJECTS = Debugger.o communication.o LED.o radio.o spi.o serial-avr.o

# Build tool configuration

CFLAGS=-mmcu=$(MCU) -Wall -DF_CPU=$(F_CPU) -I. -funsigned-char -funsigned-bitfields \
	-fpack-struct -fshort-enums -fno-strict-aliasing -Os \
	-DEEPROM_CONFIG_OFFSET=$(EEPROM_CONFIG_OFFSET)
LDFLAGS=-mmcu=$(MCU) 

.PHONY: clean app all fuses

all: app

# Rules for building the application

app: $(APP).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(APP).elf: $(APP_OBJECTS) 
	$(CC) $(LDFLAGS) -o $@ $^
	$(AVR_SIZE) -C --mcu=$(MCU) $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(APP).hex
	$(AVRDUDE) -p $(AVRDUDE_PART) -c $(PROGRAMMER) -P $(COMPORT) -b $(ISPSPEED) -e -U flash:w:$<

fuses: 
	$(AVRDUDE) -p $(AVRDUDE_PART) -c $(PROGRAMMER) -P $(COMPORT) -b $(ISPSPEED) -e -U lfuse:w:$(LFUSE):m \
		-U hfuse:w:$(HFUSE):m 

clean:
	rm -f *.o
	rm -f $(APP).{hex,elf}
