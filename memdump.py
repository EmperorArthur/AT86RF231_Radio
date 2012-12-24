import serial
ser = serial.Serial(7)	#COM 8
ser.setTimeout(1)		#1 second timeout

initString = bytes(b"Debugger Initalized, awaiting commands\r\n")	#The string saying the debugger is running

#Wait for initalization
serIn = ser.readline()
while (serIn != initString):
	serIn = ser.readline()

#Read a memory address
def read(address):
	ser.write(bytes(b"m"))
	ser.write(bytes(b"r"))
	ser.write(bytes(str(address),"ascii"))
	ser.write(bytes(b"\n"))
	#The first just reads junk data, the second is the important one
	ser.readline()
	serIn = ser.readline()
	return int(serIn[0:2],16)

#Write to an address in memory
def write(address,value):
	ser.write(bytes(b"m"))
	ser.write(bytes(b"w"))
	ser.write(bytes(str(address),"ascii"))
	ser.write(bytes(b"\n"))
	ser.write(bytes(str(value),"ascii"))
	ser.write(bytes(b"\n"))
	#The first and thrid just read junk data, the second is the important one
	ser.readline()
	serIn = ser.readline()
	ser.readline()
	return int(serIn[0:2],16)

#Write to SPI, returning the output (need to controll SS seperately)
def spi(value):
	ser.write(bytes(b"s"))
	ser.write(bytes(b"s"))
	ser.write(bytes(str(value),"ascii"))
	ser.write(bytes(b"\n"))
	#The first just reads junk data, the second is the important one
	ser.readline()
	serIn = ser.readline()
	return int(serIn[0:2],16)

#Use the spi debugging feature to read a radio register
def spi_reg_read(address):
	address = (address & 0x7F) | 0b10000000
	ser.write(bytes(b"s"))
	ser.write(bytes(b"l"))
	spi(address)
	output = spi(0)
	ser.write(bytes(b"s"))
	ser.write(bytes(b"h"))
	return output

#Use the spi debugging feature to write to a radio register
def spi_reg_write(address,vale):
	address = (address & 0x3F) | 0b11000000
	ser.write(bytes(b"s"))
	ser.write(bytes(b"l"))
	spi(address)
	output = spi(vale)
	ser.write(bytes(b"s"))
	ser.write(bytes(b"h"))
	return output

#Use the radio functions to read a radio register
def radio_reg_read(address):
	ser.write(bytes(b"r"))
	ser.write(bytes(b"r"))
	ser.write(bytes(str(address),"ascii"))
	ser.write(bytes(b"\n"))
	#The first just reads junk data, the second is the important one
	ser.readline()
	serIn = ser.readline()
	return int(serIn[0:2],16)

#Read the first (size) bytes of memory (Max 255)
def mem_read(size):
	memory = []
	for addr in range(size):
		memory.append(read(addr))
		print(str(addr) + "," + str(memory[addr]))

#Turn off the LED
def LED_off():
	print("Turning the LED off")
	#43 is PORTD
	write(43,0)
	print("OK")
	print(str(43) + "," + str(read(43)))


def radio_reg_check():
	print("Reading 0x00")
	print(radio_reg_read(0))
	print("Reading 0x1C (Should be 3)")
	print(radio_reg_read(0x1C))
	print("Reading 0x1D (Should be 2)")
	print(radio_reg_read(0x1D))
	print("Reading 0x1E (Should be 31)")
	print(radio_reg_read(0x1E))
	print("Reading 0x1F (Should be 0)")
	print(radio_reg_read(0x1F))
	print("Reading all Radio Registers twice")
	radio = []
	for addr in range(0x2F):
		radio.append(radio_reg_read(addr))
	radio1 = []
	for addr in range(0x2F):
		radio1.append(spi_reg_read(addr))
	print("Comparing the two reads for differences")
	difference = False
	for addr in range(0x2F):
		if radio1[addr] != radio[addr]:
			print(str(addr) + "," + str(radio[addr]) + "," + str(radio1[addr]))
			difference = True
	if False == difference:
		print("No differences detected")

#Read a frame from memory using SPI debug
def spi_read_frame():
	frame = []
	ser.write(bytes(b"s"))
	ser.write(bytes(b"l"))
	spi(0b00100000)
	size = spi(0)
	print("Frame size is:  "+str(size))
	for i in range(size):
		frame.append(spi(0))
		print(str(i) + "," + str(frame[i]))
	ser.write(bytes(b"s"))
	ser.write(bytes(b"h"))
	return (size,frame)

#Write a frame to memory using SPI debug
def spi_write_frame(size):
	frame = []
	ser.write(bytes(b"s"))
	ser.write(bytes(b"l"))
	spi(0b01100000)
	spi(size)
	for i in range(size):
		frame.append(spi(size-i))
		print(str(i) + "," + str(frame[i]))
	ser.write(bytes(b"s"))
	ser.write(bytes(b"h"))

#LED_off()
#spi_write_frame(30)
#(inSize,inFrame) = spi_read_frame()

list1 = []
list2 = []
currentList = []
currentList = list1
while True:
	cmdIn = input("What would you like to do?  ")
	cmdIn = cmdIn[0:len(cmdIn)-1]
	if "srf" == cmdIn:
		print("Using the spi debug function to read the radio's frame buffer")
		(inSize,currentList) = spi_read_frame()
	if "rrr" == cmdIn:
		print("Reading radio memory, using builtin function")
		del currentList[0:len(currentList)]
		for addr in range(0x2F):
			currentList.append(radio_reg_read(addr))
	if "srr" == cmdIn:
		print("Reading radio memory, using spi function")
		del currentList[0:len(currentList)]
		for addr in range(0x2F):
			currentList.append(radio_reg_read(addr))
	if "compare" == cmdIn:
		print("Comparing the two lists for differences")
		difference = False
		if len(list1) != len(list2):
			print("Warning: List's are not the same size, not comparing")
		else:
			for addr in range(len(list1)):
				if list1[addr] != list2[addr]:
					print(str(addr) + "," + str(list2[addr]) + "," + str(list1[addr]))
					difference = True
			if False == difference:
				print("No differences detected")
	if "setlist1" == cmdIn:
		currentList = list1
		if currentList is list1:
			print("currentList is now list1")
	if "setlist2" == cmdIn:
		currentList = list2
		if currentList is list2:
			print("currentList is now list2")
	if "print" == cmdIn:
		print("Outputting currentList")
		for addr in range(len(currentList)):
			print(str(addr) + "," + str(currentList[addr]))
	if "help" == cmdIn:
		print("This is a command interpreter to interface with the microcontroller.")
		print("There are two lists, which hold the incoming data.")
		print("Command list:")
		print("setlist1")
		print("setlist2")
		print("compare:		Compares list1 and list2 for differences")
		print("print:		Prints the currently selected list")
		print("rrr:			Use the builtin function to read the radio's registers to the currently selected list")
		print("srr:			Use the spi debug function to read the radio's registers to the currently selected list")
		print("srf:			Use the spi debug function to read the radio's frame buffer to the currently selected list")
