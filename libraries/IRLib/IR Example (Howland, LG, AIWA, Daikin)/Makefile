# third-party libraries
ARDUINO_LIBS = EEPROM SoftwareSerial debug EEPROMEx DHTlib IRLib

### BOARD_TAG
BOARD_TAG   = nano
BOARD_SUB   = atmega328

# MONITOR_PORT = /dev/tty.usbserial-A9U9LZFR
#/dev/cu.usbserial-A9U9LZFR
MONITOR_PORT = /dev/cu.usbserial-A9QP5BZV
# AVRDUDE_OPTS = -v
# AVRDUDE_ARD_BAUDRATE = 115200

### -------------- END OF SKETCH SPECIFIC CONF -------------- ###

### Get general config info and captur sketch path
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
include ../Makefile.common
CPPFLAGS = -I$(ARDUINO_SKETCHBOOK)/$(current_dir) $(CSTYLE) $(COPTIMIZE) 

### Path to where the your project's libraries are stored.
ARDUINO_SKETCHBOOK := $(BASE_PATH)/src
include /usr/local/opt/arduino-mk/Arduino.mk
