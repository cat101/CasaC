#ifndef config_h
#define config_h
#include "Arduino.h"

#define ARD_MEGA 1
#define ARD_NANO 2
#define ARDUINO_TYPE ARD_NANO   // Set to mega or nano


#define DEBUG
// #define DEBUG_B
// #define DEBUG_BR

#ifdef DEBUG
	#define _EEPROMEX_DEBUG         // Enables logging of maximum of writes and out-of-memory (100 by default)
   	#if ARDUINO_TYPE == ARD_NANO
		// This enables softserial (only used no Nano boards)
		#define DEBUG_RS232_RX_PIN 11//3
		#define DEBUG_RS232_TX_PIN 12//2
    #endif
#endif

// Config RS232 networking (Serial for Nano and Serial1 for Mega)
#if ARDUINO_TYPE == ARD_MEGA
#else
	#define RS232Serial Serial
#endif

#define ACQ_SAMPLE_SPEED 10000UL  //Sample every 10 seconds (one sample can take up to 2 seconds) 
#define RS485NODEID 200 //This is needed to disable the use of GetUnixTime on debug.cpp

#endif

