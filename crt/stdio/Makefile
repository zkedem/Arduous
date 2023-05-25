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
SOURCES :=	stdio.c

INC_PATH=-I./ -I../ -I../service_request/ -I../../


# ---------------------------------------
# ---------- MAKEFILE CODE --------------
# ---------------------------------------

MCUFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)

CFLAGS = $(MCUFLAGS)  $(INC_PATH) -nostartfiles -nodefaultlibs -nostdlib -c $(SOURCES) $(DEFS) -S
LFLAGS = $(MCUFLAGS) -o $(TARGET).elf *.o

# ---------------------------------------
# Optimization choice
# This will avoid not used code and guarantee proper delays
CFLAGS += -ffunction-sections -O1 -mrelax
LFLAGS += -Wl,--gc-sections -Wl,-relax

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS)
#	$(CC) $(LFLAGS)

clean:
	@echo "Cleaning Objects"
	rm -f $(wildcard *.o)
	rm -f $(wildcard *.s)
	rm -rf obj
	rm -rf .dep
