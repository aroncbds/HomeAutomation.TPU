MCU = atmega328p
AVR_GCC = avr-gcc
AVR_OBJCOPY = avr-objcopy
AVRDUDE = avrdude
F_CPU = 16000000
HEX = firmware.hex
ELF = firmware.elf
OBJ = $(SRC:.c=.o)

SRC = $(wildcard src/*.c)
INCLUDE_DIRS = -I./libs/Arduino_FreeRTOS_Library/src/ -I./libs/DallasTemperature/ -I./libs/Ethernet2/src/ -I./libs/LiquidCrystal_I2C -I./libs/OneWire/

AVR_GCCFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -std=c99
AVR_OBJCOPYFLAGS = -O ihex -R .eeprom

all: $(HEX)

# build the hex-file from the ELF-file
$(HEX): $(ELF)
	$(AVR_OBJCOPY) $(AVR_OBJCOPYFLAGS) $(ELF) $(HEX)

# build the ELF-file from object files
$(ELF): $(OBJ)
	$(AVR_GCC) $(AVR_GCCFLAGS) $(INCLUDE_DIRS) -o $(ELF) $(OBJ)

# compile source files in src/ directory to object files
$(OBJ): src/%.o : src/%.c
	$(AVR_GCC) $(AVR_GCCFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Flash to device (for MCUs having OptiBoot in place)
flash: $(HEX)
	avrdude -c arduino -p m328p -P /dev/ttyACM0 -b 115200 -U flash:w:$(HEX):i

# Clean up generated files
clean:
	rm -f $(OBJ) $(ELF) $(HEX)
