# MCU settings
BOARD = arduino:avr:uno
PORT = /dev/ttyUSB0

# Debugging settings
DEBUGGER = avarice
GDB = avr-gdb
SIMULATOR = simavr

# Directories
BUILD_DIR = build
SRC_DIR = src
LIB_DIR = lib

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17 -g
CXXFLAGS += -I$(LIB_DIR)
CXXFLAGS += -DF_CPU=16000000UL

# Object files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

# Arduino CLI path (inside Docker)
ARDUINO_CLI = arduino-cli

# Targets
all: $(BUILD_DIR)/firmware.hex

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	avr-g++ $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/firmware.elf: $(OBJECTS)
	avr-g++ $(CXXFLAGS) $^ -o $@ -lm

$(BUILD_DIR)/firmware.hex: $(BUILD_DIR)/firmware.elf
	avr-objcopy -O ihex -R .eeprom $< $@

upload: $(BUILD_DIR)/firmware.hex
	$(ARDUINO_CLI) upload -p $(PORT) --fqbn $(BOARD) --input-file $(BUILD_DIR)/firmware.hex

clean:
	rm -rf $(BUILD_DIR)

debug: $(BUILD_DIR)/firmware.elf
	$(GDB) -ex "target remote localhost:4242" $(BUILD_DIR)/firmware.elf

simulate: $(BUILD_DIR)/firmware.elf
	$(SIMULATOR) -g -m atmega328p $(BUILD_DIR)/firmware.elf
