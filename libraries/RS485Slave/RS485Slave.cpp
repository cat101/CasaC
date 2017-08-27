#include "RS485Slave.h"

#include <EEPROMEx.h>
#include <SensorAcq.h>
extern SensorAcq sensors;
extern unsigned long nextSample;
extern unsigned int houseMode;

#ifdef RS485_ENABLE_TESTS
struct SystemStatus {
	unsigned long rounds, send;
} status;
#endif

void RS485Slave::begin(void)
{
	pinMode (RS485_ENABLE_PIN, OUTPUT);  // RS485 driver output enable
	digitalWrite (RS485_ENABLE_PIN, LOW); //Set receive mode
	RS485SERIAL.begin(RS485_SERIALBAUD);
#ifdef RS485_ENABLE_TESTS
	memset(testBuff,'B',RS485_COMMBUFSIZE);
	status.send=status.rounds=0;
#endif
}

/*
           REQUEST               RESPONSE
    
      0 1 2 3 4 5 6 7 8      0 1 2 3 4 5 6 7 8
      +---------------+      +---------------+
    0 |    Node ID    |    0 | 0 (to master) |
      |(255=broadcast)|      +---------------+
      +---------------+    1 |    Command    |
    1 |    Command    |      |  (255=error)  |
      +---------------+      +---------------+
    2 |               |
      |    Payload    |
    n |               |
      +---------------+
		   http://www.asciiflow.com/
*/
void sendEventsToMaster();
void sendEventsToMaster_MainLoop();
void processSurrogateCommands(byte *cmd, byte len);


// bool RS485Slave::delay(unsigned long delay`)
// {
// 	byte buf[RS485_COMMBUFSIZE];
// 	unsigned long startTime=millis();
// 	while((millis()-startTime)<delay){
// 		//recvMsg will ignore messages not addressed to this node and return intermediately if the first byte is not a match
// 		byte received = recvMsg (buf, RS485_COMMBUFSIZE - 1, timeout, NULL, 0, NULL, 0, RS485NODEID);
// 		if (received && buf[0]==RS485NODEID){
// 			//A command was sent to this node (ignore broadcasts since they don't require response)
// 			sendMsg (RS485MASTERID, RS485CMD_NODE_BUSY, NULL, 0); //Reply as being busy
// 		}
// 	}
// }
bool RS485Slave::processConnection(unsigned long timeout)
{
	byte buf[RS485_COMMBUFSIZE];
	byte received = recvMsg (buf, RS485_COMMBUFSIZE - 1, timeout, NULL, 0, NULL, 0, RS485NODEID);
	if (received){
		if (buf [0] != RS485NODEID && buf [0] != RS485BROADCASTID){
			return false;  // not my device
		}
		// The return message points to the master
		// Return: TargetNode, Command being ACK, Status (1->OK, 255->NA)
		DEBUG_BR_PRINT_P("Got message for this node of size %u\n", received);
		// byte *msg = (byte []){
		// RS485MASTERID,0,1            
		// };
		byte cmd=buf[1];
#ifdef RS485_ENABLE_TESTS
		if(cmd==RS485CMD_COMM_CHECK && buf[3]<=RS485_COMMBUFSIZE){
			// CMD=240 -> Stress testing (Args: nextRoundDelay, paddingLen)
			DEBUG_PRINT_P("Got stress test cmd and can serve it\n");
			unsigned char paddingLen=buf[3];
			nextRoundDelay= 1<<buf[2];
			sendMsg (RS485MASTERID, buf[1], (byte *)&status, sizeof(status), false);
			sendMsgPart (testBuff, paddingLen, true);
			delay(nextRoundDelay);nextRoundDelay=0;
		}else
#endif
		if(cmd==RS485CMD_COLLECT_SENSOR || cmd==RS485CMD_COLLECT_SENSOR_EXTENDED){		
			// CMD=4 -> Check status (basic)
		    //digitalWrite(6, HIGH);
		    if(received>2){
		    	// Got some events from the master
		    	for(byte i=2;i<received;i++){
		    		if(!(buf[i] & B10000000)){
				    	// Ignore if bit 8 is not 0. We have not implemented yet the ability to trigger events when an output is changed (part of the protocol)
						DEBUG_PRINT_P("Got event: Output %hu, Value %hu (received %hu)\n", buf[i] & 0x3F, (buf[i] & 0x40)>>6,received-2);
				    	// logException_P("OUT_N %hu %hu", buf[i] & 0x3F, (buf[i] & 0x40)>>6);
				    	// if((buf[i] & 0x3F) == 0)
				    	// 	continue; //Ignore luz tanque
						sensors.digitalWrite(ACQ_OUT_RANGE_START+buf[i] & 0x3F, (buf[i] & 0x40)>>6);
					}
				}
		    }
			sendMsg (RS485MASTERID, cmd, sensors.digSamples, sizeof(sensors.digSamples)
			+sizeof(sensors.senSamples)
			+sizeof(sensors.errSamples)
			+sizeof(sensors.digOutput)
			+((cmd==RS485CMD_COLLECT_SENSOR_EXTENDED)?sizeof(sensors.analogSensors):0),false);
			// Check if there is an outstanding reader event to piggy back
			sendEventsToMaster_MainLoop(); // Send from the slave's main loop (used to push exceptions)
			sendEventsToMaster(); 		   // User defined (specific to this slave)
			#ifdef SEND_COMM_DEBUG_EV
			    byte debugEv[]={0x51,0}; // The extra frame format is Frametype (4bits), Framesize(4bits)
			    debugEv[1]=cmd<<4|RS485NODEID;
			    sendMsgPart (debugEv, sizeof(debugEv), false);
			#endif
			sendMsgPart (NULL, 0, true);
			DEBUG_BR_PRINT_P("Sent data (cmd=%d) size %d\n", cmd,sizeof(sensors.digSamples)
				+sizeof(sensors.senSamples)
				+sizeof(sensors.errSamples)
				+sizeof(sensors.digOutput)
				+((cmd==RS485CMD_COLLECT_SENSOR_EXTENDED)?sizeof(sensors.analogSensors):0));
			nextSample=millis(); // Force an immediate sample
		    //digitalWrite(6, LOW);
		}else 
		// if(cmd==RS485CMD_TIMED_OUTPUT){ NOT IMPLEMENTED...adding a new command out of the polling cycle increases the disturbance to slaves
		// 	// We need to temporarily turn something on and the off
		// 						DEBUG_PRINT_P("Got event: Output %hu, Value %hu (received %hu)\n", buf[i] & 0x3F, (buf[i] & 0x40)>>6,received-2);
		// 			sensors.digitalWrite(ACQ_OUT_RANGE_START+buf[i] & 0x3F, (buf[i] & 0x40)>>6);
		// }else 
		if(cmd==RS485CMD_WRITENLOAD_CONFIG){
			// CMD=6 -> Write EEPROM config
			DEBUG_PRINT_P("Updating EEPROM sent %d\n", received-2);
			int writeCount=EEPROMEx.updateBlock(0, &buf[2], received-2); // I'm writing directly since the crc check will be performed on load
			int read=loadEECfg();
			DEBUG_PRINT_P("Update & reload complete (sent %d updated %d re-read %d)\n", received-2, writeCount,read);
			sendMsg (RS485MASTERID, RS485CMD_WRITENLOAD_CONFIG, NULL, 0);
		} else
		if(cmd==RS485CMD_SET_MODE){
			/*
				CMD=8 -> Set mode bits
				    0:isSunIrradiating=false;
				    1:isSleepTime=false;
				    2:needNightimeLightsOutside=false; //This gets triggered before needCourtesyLightsInside
				    3:needCourtesyLightsInside=false;
				    4:debugEnabled
				    5:serviceModeEnabled
				    6:mainsDown  --> We are running on UPS or backup power
				    7:degradedMode -- NOT IMPLEMENTED --> Some key services are down. All nodes need to be autonomous and operate with its own resoruces (e.g. No time, light, etc)
				    8:alarmOn --> Used by the keypads to show that the alarm is on
			*/
			houseMode=(((unsigned int)buf[3])<<8) + buf[2];		
			DEBUG_B_PRINT_P("RS485CMD_SET_MODE: new mode=%04X\n", houseMode);
			debugEnabled=((buf[2] >> 4) & 0x1);
			if((buf[2] >> 5) & 0x1){  //Service mode?
				DEBUG_PRINT_P("Setting service mode to %hu\n", buf[2]);
				sensors.serviceMode=1;
 	            #ifdef ENABLE_SERVICEMODE_BENCHTEST
					for(byte repeat=0;repeat<2;repeat++){
						for(byte c=ACQ_OUT_RANGE_START;c<=ACQ_OUT_RANGE_STOP;c++){
							DEBUG_PRINT_P("Testing O%02hu\n", c-ACQ_OUT_RANGE_START);
							sensors.digitalWrite(c, HIGH);
							delay(500);
							sensors.digitalWrite(c, LOW);
						}
					}
				#endif	
			}else{
				sensors.serviceMode=0;
			}
			dispatchSensorEvents(EV_MODE_SET,0); // Tell the main loop
			// No reply since it could be a broadcast...  sendMsg (RS485MASTERID, RS485CMD_SET_MODE, NULL, 0);
		} else
		if(cmd==RS485CMD_SEND_SURROGATE_CMD){
			processSurrogateCommands(&buf[2], received-2); // Called from RS485Slave::processConnection to redirect commands to a surrogate
		}
		else{
			DEBUG_PRINT_P("Unknown cmd\n");			
			sendMsg (RS485MASTERID, 255, NULL, 0);
		}
#ifdef RS485_ENABLE_TESTS
		status.send++;
#endif			
		return true;
	}  // end if something received
#ifdef RS485_ENABLE_TESTS
	status.rounds++;
#endif
	return false;
}
