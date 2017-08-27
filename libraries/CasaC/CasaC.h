#include "config.h"
#include "debug.h"

#include <EEPROMEx.h>

#include <SensorAcq.h>
SensorAcq sensors;

unsigned int houseMode=0; //Status pushed by the master
#define needCourtesyLightsInside(x)	(x & B1000)
#define isSleepTime(x)	(x & B10)
#define isMainsDown(x)	(x & B1000000)

#include "RS485Slave.h"
RS485Slave RS485comm;

unsigned long nextSample;   // Absolute time when the next sampling period should start
unsigned long pass2Due=0;   // Absolute time when pass 2 of the sampling process should start

#ifdef USE_SIMPLE_TIMER
#include <SimpleTimer.h>
SimpleTimer timer;
#endif

void initTasks(void);

void setup(){
	#ifdef DEBUG
	initDebugger(115200);
//    pinMode(6, OUTPUT);  
	#endif

	// Load configuration from EEPROM
	int address=loadEECfg();

	/* Initialize sensor acquisition */
	sensors.begin();

	RS485comm.begin();

	nextSample=millis(); // Force an immediate sample

	initTasks(); //Init all scheduled tasks

 	#if !defined(DEBUGSERIAL) || ARDUINO_TYPE == ARD_MEGA
		// Blink the LED to indicate liveness
		pinMode(STATUS_LED, OUTPUT); 
	#endif
	DEBUG_PRINT_P("Node %d started (config size %d)\n", RS485NODEID, address);
}

extern void p2loop();
//static bool aquiring=false;
void p1loop(){
	unsigned long t_entry=0;
	// This is the high priority loop. Takes care of time sensitive tasks like sensor sampling, timers & serial comm
	// The interrupt driven serial code implements a 64 byte buffer for Megas (this buffer gets filled up in 4.4ms at 115kbps)
	// Extended polling all the 7 nodes requires 150bytes RX + the commands (15 bytes?) --> 11ms are spent per round listening to serial bytes (out of 250ms per round)
	// I could call the processing routine once there are more than 1 byte but this could cause whole packets to get queued until we hit the threshold
	// if(RS485SERIAL.available()>50){
	// 	DEBUG_PRINT_P("Comm buffer close to full %d/63 bytes\n",RS485SERIAL.available());
	// }
	while(RS485SERIAL.available())
	{
		// We have data waiting on the RS485 comms
		// Check if it is the start of message
		#define STX '\2'
		if(RS485SERIAL.peek()==STX){  // THIS IS A VIOLATION OF THE COMM LAYERS
			t_entry=millis();
			// digitalWrite(6, HIGH);
			DEBUG_BR_PRINT_P("Got a packet start (buffered %d)\n",RS485SERIAL.available());
			//Allow at least 4m (RS485_MSGASSEMBLYTIME) to collect the whole package (if we finish faster the function will return)
			if(RS485comm.processConnection(RS485_MSGASSEMBLYTIME))	
				nextSample=0; //Force a sample upon being polled. This tries to sync the cycles with the master so that slaves can be ready to answer on the next round  
			// digitalWrite(6, LOW);
		}else{
			RS485SERIAL.read(); //Dump the char
		}

	}
  //   if(aquiring){
  //   	//Serial.print('.');
		// // DEBUG_PRINT(".");
  //   	return; //Prevent re-entrance	
  //   } 
#ifdef USE_SIMPLE_TIMER
	timer.run();
#endif
	unsigned long currentMillis=millis();
	if(sensors.dblClickTimeOut!=0 && currentMillis >=sensors.dblClickTimeOut) { //Reset double click timers
		sensors.dblClickClean();
	}
	if(currentMillis >= nextSample) { //Take a sample
		if(!pass2Due){
		 	#if !defined(DEBUGSERIAL) || ARDUINO_TYPE == ARD_MEGA
				// Blink the LED to indicate liveness
				digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
			#endif
			sensors.acquirePass1();
			pass2Due=currentMillis+ACQ_DEBOUNCE_TIME;  
		}else if(currentMillis>=pass2Due){
			sensors.acquirePass2();  
			DEBUG_BR_PRINT_P("Node %u: Dig=0x%02hx, Sensors=0x%02hx, Err=0x%02hx, Output=0x%02hx, Analog=%03u, DblClick=0x%02hx\n"
			  ,RS485NODEID
			  ,sensors.digSamples[0]
			  ,sensors.senSamples[0]
			  ,sensors.errSamples[0]
			  ,sensors.digOutput[0]
			  ,sensors.analogSensors[0]
			  ,sensors.dblClickSamples[0]);		
			unsigned long t_acq=millis();
		    // aquiring=true;
			sensors.acquireSensors(); //Sample the rest of the analog sensors
		    // aquiring=false;
		    if(millis()-t_acq>200){ 
		    	// If we burnt more than 200ms to acquire sensors we should flag it (too close to the budget)
		    	logException_P("Acq time %lums",millis()-t_acq);
				DEBUG_PRINT_P("Comm & sample time %lu, extras sensors sample time %lu (budget 250ms)\n",t_acq-t_entry, millis()-t_acq);
		    }
			// Schedule the next sample
			pass2Due=0;
			nextSample = currentMillis + ACQ_SAMPLE_WATCHDOG;    
		}
	}
}
void loop(){
	p1loop();
	p2loop();
}

/*
        REQUEST               RESPONSE                                                                               
                                                                                                                     
   8 7 6 5 4 3 2 1 0      8 7 6 5 4 3 2 1 0                                                                          
   +---------------+      +---------------+                                                                          
 0 |    Node Id    |    0 |  0(to master) |                                                                          
   +---------------+      +---------------+                                                                          
 1 |   Cmd = 4/5   |    1 |    Cmd = 4    |                                                                          
   +---------------+      +---------------+                                                                          
                        2 |   Digital In  | <-+                                                                      
   Cmd 4:                 +---------------+   |                                                                      
    Check status        . |    Sensors    |   |                                                                      
                          +---------------+   +--+ The no. bytes depends                                             
                        . |    Errors     |   |    on the controller                                                 
   Cmd 5:                 +---------------+   |                                                                      
    Check extended      i |  Digital Out  |   |                                                                      
    status                +---------------+ <-+                                                                      
                          |Extended status|                                                                          
                          +---------------+                                                                          
                                                                                                                     
   +---------------+  i+1 +---------------+                                                                          
 1 |Ev|S/R|Out (6) |      |Type=1 |Size=4 | <----+ Optional frame with discrete events (size does not include header)
   +---------------+      +---------------+            Type 1:RFID Event (size 4 bytes)                                            
                          | entry (31|24) | <-+        Type 2:Exception (followed by string)                         
   Ev 1:Trigger event     +---------------+   |        Type 5:Comm Debug (followed by Node ID and other data)        
  S/R 1:Set or reset      | entry (23|16) |   |        Type 6:RF packet received (size 4 bytes)                                                              
  Out 6:bit to set        +---------------+   +--+ 5 digits BCD or                                                   
                          | entry (15| 8) |   |    26 bit RFID                                                       
                          +---------------+   |    32 bit RF code                                                       
                      i+5 | entry ( 7| 0) |   |                                                                      
                          +---------------+ <-+                                                                      

		                 http://www.asciiflow.com/
*/


void sendEventsToMaster_MainLoop(){ // Called from RS485Slave::processConnection
  // Check if there is an outstanding exception to piggy back
  if(lastException!=0){
    byte c=strlen(exceptionText);
    if(c>16) c=16; //Cap the string size
    byte exceptionEv[]={(byte)(0x20 + c)}; // The extra frame format is Frametype (4bits), Framesize(4bits)
    sendMsgPart (exceptionEv, sizeof(exceptionEv), false);
    sendMsgPart ((byte *)exceptionText, c, false);
    lastException=0; // Reset exception time
  }

}

