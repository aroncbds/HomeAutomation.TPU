MCU = atmega2560

F_CPU = 16000000UL

CC = avr-gcc
CXX = avr-g++
OBJCOPY = avr-objcopy
CFLAGS = -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -std=gnu99
CXXFLAGS = -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -std=gnu++11

INCLUDE_DIRS = -I./include -I./src/FreeRTOS -I./src/libraries

SRC = $(wildcard src/*.c) $(wildcard src/libraries/*.c) $(wildcard src/FreeRTOS/*.c)

OBJ = $(SRC:.c=.o)
TARGET = my_project.elf
HEX = my_project.hex

AVRDUDE = avrdude
AVRDUDE_FLAGS = -p $(MCU) -c wiring -P /dev/ttyUSB0 -b 115200 -U flash:w:$(HEX):i

all: $(HEX)

$(HEX): $(TARGET)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(HEX)

# Flash firmware to Arduino
flash: $(HEX)
	$(AVRDUDE) $(AVRDUDE_FLAGS)

# Monitor Serial Output
monitor:
	minicom -b 115200 -D /dev/ttyUSB0
