# MCU Type (Mega 2560)
MCU = atmega2560

# CPU Speed
F_CPU = 16000000UL

# Compiler and flags
CC = avr-gcc
CXX = avr-g++
OBJCOPY = avr-objcopy
CFLAGS = -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -std=gnu99
CXXFLAGS = -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -std=gnu++11

# Include paths
INCLUDE_DIRS = -I./include -I./src/FreeRTOS -I./src/libraries

# Source files
SRC = $(wildcard src/*.c) $(wildcard src/libraries/*.c) $(wildcard src/FreeRTOS/*.c)

# Output files
OBJ = $(SRC:.c=.o)
TARGET = my_project.elf
HEX = my_project.hex

# AVRDUDE (for flashing the Arduino)
AVRDUDE = avrdude
AVRDUDE_FLAGS = -p $(MCU) -c wiring -P /dev/ttyUSB0 -b 115200 -U flash:w:$(HEX):i

# Build the project
all: $(HEX)

$(HEX): $(TARGET)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJ) $(TARGET) $(HEX)

# Flash firmware to Arduino
flash: $(HEX)
	$(AVRDUDE) $(AVRDUDE_FLAGS)

# Monitor Serial Output
monitor:
	minicom -b 115200 -D /dev/ttyUSB0
