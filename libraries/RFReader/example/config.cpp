/*
	To compile do
		move to /src
	Then restore
*/


#include "config.h"
#include "debug.h"
#ifdef DEBUG_RS232_TX_PIN
#include <SoftwareSerial.h>
#endif



#include "RFReader.h"
RFReader reader;

unsigned long nextSample;

void setup(){
	#ifdef DEBUG
	initDebugger(115200);
//    pinMode(6, OUTPUT);  
	#endif
    reader.begin();
	DEBUG_PRINT_P("Started\n");
}

void loop(){
  reader.process();
}

