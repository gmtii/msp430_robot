#
# Makefile - usci_serial
#
# License: Do with this code what you want. However, don't blame
# me if you connect it to a heart pump and it stops.  This source
# is provided as is with no warranties. It probably has bugs!!
# You have been warned!
#
# Author: Rick Kimball
# email: rick@kimballsoftware.com
# Version: 1.00 Initial version 05-12-2011
 
CC=/home/gmtii/msp430/energia-0101E0009/hardware/tools/msp430/bin/msp430-gcc
CXX=/home/gmtii/msp430/energia-0101E0009/hardware/tools/msp430/bin/msp430-g++
MCU=msp430g2553
 
CFLAGS=-mmcu=$(MCU) -Os -g -Wall 
 
APP=main
TARGET=Debug
 
all: $(TARGET)/$(APP).elf
 
$(TARGET)/$(APP).elf: $(TARGET)/$(APP).o
	$(CC) $(CFLAGS) -o $(TARGET)/$(APP).elf $(TARGET)/$(APP).o i2c.c comun.c pid.c 10dof.c -lm
	/home/gmtii/msp430/energia-0101E0009/hardware/tools/msp430/bin/msp430-objdump -DS $(TARGET)/$(APP).elf >$(TARGET)/$(APP).lst
	/home/gmtii/msp430/energia-0101E0009/hardware/tools/msp430/bin/msp430-size $(TARGET)/$(APP).elf
	
$(TARGET)/$(APP).o:	i2c.h comun.h pid.h 10dof.h $(APP).c 
	$(CC) $(CFLAGS) -c -o $(TARGET)/$(APP).o $(APP).c 
	
install:
	/home/gmtii/msp430/energia-0101E0009/hardware/tools/msp430/bin/mspdebug -q --force-reset rf2500 "prog $(TARGET)/$(APP).elf"
 
clean:
	rm -f $(TARGET)/$(APP).o $(TARGET)/$(APP).elf $(TARGET)/$(APP).lst
	mkdir -p $(TARGET)/
