# Make file adapted from the one based on 
# https://github.com/peakhunt/freertos_atmega328p/blob/master/Makefile
# and modified by Tiago Lobao
# and subsequently modified by Ziv Kedem
#
# 
# make (default) = compile project
#
# make clean = delete all binaries
#
# make program = flash through AVRDUDE
#

# ---------------------------------------
# -------- MAKEFILE USER DEFINES --------
# ---------------------------------------

# ---------------------------------------
# Microcontroller specific
MCU=atmega328p
F_CPU=16000000UL

# ---------------------------------------
# Target file name options
TARGET=main

# ---------------------------------------
# Output directory options
# OUTPUT_DIR=./Debug

# ---------------------------------------
# compiler / programmer options
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

# ---------------------------------------
# SD card interface options
SD_CS_PORT    = PORTB # Data Register of the SD CS pin
SD_CS_DDR     = DDRB # Data Direction Register of the SD CS pin
SD_CS_BIT     = 2 # Bit of the SD CS pin
DEFS        = -DSD_CS_PORT=$(SD_CS_PORT) -DSD_CS_DDR=$(SD_CS_DDR) -DSD_CS_BIT=$(SD_CS_BIT)

# ---------------------------------------
# Sources/includes to be used
SOURCES :=	crt1.c

INC_PATH=-I./ -I../ -I../../


# ---------------------------------------
# ---------- MAKEFILE CODE --------------
# ---------------------------------------

MCUFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)

CFLAGS = $(MCUFLAGS)  $(INC_PATH) -nostartfiles -nodefaultlibs -nostdlib -c $(SOURCES) $(DEFS) -S
LFLAGS = $(MCUFLAGS) -o $(TARGET).elf *.o

# ---------------------------------------
# Optimization choice
# This will avoid not used code and guarantee proper delays
CFLAGS += -ffunction-sections -O0 -mrelax
LFLAGS += -Wl,--gc-sections -Wl,-relax

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS)
#	$(CC) $(LFLAGS)
	C:\Program Files\nodejs\node.exe "C:\avr-fix\avr-fix.js" crt1.s
	$(MAKE) -C "service_request" all
	$(MAKE) -C "stdlib" all
	$(MAKE) -C "stdio" all

clean:
	@echo "Cleaning Objects"
	rm -f $(wildcard *.o)
	rm -f $(wildcard *.s)
	rm -f $(wildcard *.og)
	rm -rf obj
	rm -rf .dep
	$(MAKE) -C "service_request" clean
	$(MAKE) -C "stdlib" clean
	$(MAKE) -C "stdio" clean
