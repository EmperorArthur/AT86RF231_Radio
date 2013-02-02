This is a radio framework for use with the Atmel AT86RF231 radio module.

All of this is copyright Arthur Moore 2012 under the GPL V3 license.
If this doesn't suit your neads contact me at Arthur.Moore@cd-net.net

This will let an atmel microcontroller interface with the radio using spi.

Why I did this when Atmel already produced a radio interface:
	There are a couple reasons I started on this project.
	First was space:
		The provided radio driver was so huge it wouldn't work with my application, so I had to shave it down.
	Second was complexity:
		Atmel doesn't make their radio framework easy to use or understand.  Almost every command used raw SPI writes.
		There didn't even seem to be seperate spi functions, or common functions like radio_register_write(...)
		Worse, the documentation was useless, and the example code didn't work without serious modification.
		A user shouldn't have to delve through all the source files to understand how to write a simple radio frame.
	Third was portability:
		It may not seem like it at first glance, but I've tried to partition all the AVR specific code into their own files.
		I do a great deal of work with Atmel chips, but sometimes AVR is not what the job needed, and I have a bunch of these radios.

File breakdown:
at86rf230_registermap.h:	This is a file provided by Atmel.  It contains most constants I use to access the radio.
spi.h/c:			This has all the SPI functions (to change what pins are used edit spi.h)
radio.h/c:			This has all the radio functions, and the radio frame format
radio-avr.h:		This has all the avr specific radio functions (to change what pins are used edit this file)
serial-avr.h/c:		See next line.		(tx and rx pins are fixed, this uses USART0)
communication.h/c:	This and serial-avr allow the use of cin, cout, and cerr via a serial console.
LED.h/c:			A simple LED controller for visual feedback.	(to change what pins are used edit LED.h)
Debugger.c:			The main radio testing interface. This is what I'm using to make sure the radio is working.
memdump.py:			A python script to help with radio testing.  It doesn't do everything, but it will let you read all the registers.

Notes:
	Recieving data from either the serail console or the radio requires interupts to be enabled.

To Do:
	Move the interupt handler from Debugger.c to radio-avr.h
		If I did that, then the easiest way to recieve data is for the main program to have "extern radioFrame recvFrame;"
	Consider moving all pin declarations into one file.
		Either the makefile, or make a file called pins.h
	Make a propper radioFrame that is easy to use.
		The whole reason I started this proj