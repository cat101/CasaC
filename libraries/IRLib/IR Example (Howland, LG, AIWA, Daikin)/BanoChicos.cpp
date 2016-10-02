#include "config.h"
#include "debug.h"
#ifdef DEBUG_RS232_TX_PIN
#include <SoftwareSerial.h>
#endif

#include <EEPROMEx.h>

#include <dht.h>
dht DHT;


#include <IRLib.h>
#include <IRDaikin.h>

IRsendRaw IRRawSender;
IRsendNEC IRNECSender;
IRsendAIWA IRAIWASender;
// Surrey AC off 
// PROGMEM unsigned int irCode_SurreyAC_off[]={4437 ,4416 ,597 ,1579 ,640 ,469 ,683 ,1472 ,619 ,1557 ,683 ,384 ,661 ,427 ,619 ,1557 ,640 ,427 ,597 ,512 ,619 ,1557 ,683 ,405 ,661 ,427 ,619 ,1557 ,640 ,1515 ,619 ,491 ,619 ,1536 ,661 ,448 ,640 ,1493 ,619 ,1557 ,619 ,1536 ,576 ,1600 ,619 ,469 ,683 ,1472 ,619 ,1557 ,683 ,1472 ,619 ,469 ,619 ,491 ,640 ,448 ,576 ,533 ,661 ,1515 ,619 ,469 ,640 ,427 ,619 ,1557 ,619 ,1557 ,640 ,1493 ,619 ,491 ,619 ,469 ,619 ,469 ,640 ,448 ,597 ,512 ,619 ,469 ,619 ,469 ,619 ,491 ,683 ,1472 ,640 ,1536 ,683 ,1472 ,619 ,1557 ,661 ,1493 ,619 ,4715 ,4501 ,4416 ,704 ,1472 ,576 ,512 ,619 ,1536 ,683 ,1472 ,619 ,469 ,619 ,469 ,640 ,1493 ,640 ,469 ,619 ,469 ,619 ,1536 ,576 ,512 ,619 ,469 ,619 ,1536 ,661 ,1493 ,619 ,469 ,640 ,1493 ,619 ,491 ,661 ,1493 ,619 ,1536 ,661 ,1493 ,619 ,1536 ,661 ,427 ,640 ,1536 ,640 ,1493 ,619 ,1557 ,640 ,427 ,597 ,533 ,619 ,491 ,661 ,405 ,683 ,1493 ,619 ,491 ,640 ,448 ,619 ,1557 ,619 ,1557 ,683 ,1472 ,619 ,491 ,661 ,405 ,661 ,448 ,597 ,512 ,619 ,491 ,619 ,469 ,640 ,427 ,661 ,427 ,619 ,1557 ,640 ,1493 ,619 ,1557 ,640 ,1493 ,619 ,1557 ,683};
//unsigned int *rawIRCodes[]={irCode_Daikin_Off};
// #define NELEMS(x)  (sizeof(x) / sizeof(x[0]))
// #define MAX_CMD NELEMS(irCode_SurreyAC_off)


unsigned long nextSample;

void setup(){
	#ifdef DEBUG
	initDebugger(115200);
//    pinMode(6, OUTPUT);  
	#endif
	RS232Serial.begin(115200);

	nextSample=millis(); // Force an inmediate sample
//	DEBUG_PRINT_P("Node %d started (config size %d)\n", RS485NODEID, address);
	RS232Serial.println("Started\n");
}
#define METHANE_SENSOR_PIN A4   
#define SMOKE_SENSOR_PIN   7   
#define DHT11_DATA_PIN     5
#define BUZZER_PIN         6
#define FAN_PIN            4

void loop(){
	unsigned long currentMillis=millis();
	if(currentMillis >= nextSample) { //Take a sample
		// sensors.acquirePass1();
		// delay(ACQ_DEBOUNCE_TIME);  
		// sensors.acquirePass2();  
		// DEBUG_BR_PRINT_P("Node %u: Dig=0x%02hx, Sensors=0x%02hx, Err=0x%02hx, Output=0x%02hx, Analog=%03u, DblClick=0x%02hx\n"
		//   ,RS485NODEID
		//   ,sensors.digSamples[0]
		//   ,sensors.senSamples[0]
		//   ,sensors.errSamples[0]
		//   ,sensors.digOutput[0]
		//   ,sensors.analogSensors[0]
		//   ,sensors.dblClickSamples[0]);		
		// sensors.acquireSensors(); //Sample the rest of the analog sensors
		// // Schedule the next sample
		nextSample = currentMillis + ACQ_SAMPLE_SPEED;    
	}
	//     // read MQ value
 //    int sensorValue = analogRead(METHANE_SENSOR_PIN);  
	// RS232Serial.println(sensorValue);
 //    unsigned int isSmokeAlarmFired = digitalRead(SMOKE_SENSOR_PIN);  
	// RS232Serial.println(isSmokeAlarmFired);
	// // Acquire temperature & humidity
 //    DHT.read11(DHT11_DATA_PIN);
	// RS232Serial.println(DHT.temperature);
	// RS232Serial.println(DHT.humidity);
 //    delay(1000);

}


// Serial event is only called if there is something to read
#if ARDUINO_TYPE == ARD_MEGA
void serialEvent1()
#else
void serialEvent()
#endif
{
	// unsigned int buf[MAX_CMD];
	char inChar = (char)RS232Serial.read(); 
	switch (inChar){
	    case '1': //Turn Surrey AC off
	  //   	memcpy_P(buf, irCode_SurreyAC_off, sizeof(irCode_SurreyAC_off));
			// IRRawSender.send(buf,NELEMS(irCode_SurreyAC_off),38);
			// // RS232Serial.println("Sent irCode_SurreyAC_off\n");
			//DEBUG_PRINT_P("Sent irCode_SurreyAC_off\n");			
		break;
	    case '2': //Turn NEC projector on/off
		    IRRawSender.sendGeneric(0xF20A40BF, 32, 564*13, 564*8, 564, 564, 564*3, 564, 38, true);
			// RS232Serial.println("Sent NEC protocol off\n");
			//DEBUG_PRINT_P("Sent NEC protocol off\n");
		break;
	    case '3': //Turn Caloventor Howland on/off
	    	// The space heater uses a CS5104CP chip by Semic. It seems that the signal for every button is repeated at least 4 times
		    IRRawSender.sendGeneric(0x00000D90, 12, 0, 0, 1000, 425, 425, 1000, 38, false);
		    delay(8);
		    IRRawSender.sendGeneric(0x00000D90, 12, 0, 0, 1000, 425, 425, 1000, 38, false);
	    break;
	    case '4': //Turn Caloventor Howland potencia
		    IRRawSender.sendGeneric(0x00000D88, 12, 0, 0, 1000, 425, 425, 1000, 38, false);
		    delay(8);
		    IRRawSender.sendGeneric(0x00000D88, 12, 0, 0, 1000, 425, 425, 1000, 38, false);
		break;
	    case '5': //Turn Caloventor Howland timer
		    IRRawSender.sendGeneric(0x00000D84, 12, 0, 0, 1000, 425, 425, 1000, 38, false);
		    delay(8);
		    IRRawSender.sendGeneric(0x00000D84, 12, 0, 0, 1000, 425, 425, 1000, 38, false);
		break;
	    case '6': //Turn Daikin AC on
	    	airController_off();
	    	airController_setTemp(22);
			airController_setFan(D_FAN_AUTO);
			airController_setMode(D_MODE_COOL);
			airController_send();
		break;
	    case '7': 
			// SEND LG TV POWER 0x20DF10EF
			// Channel Up  20DF00FF
			// Channel Down 20DF807F
			// Volume Up 20DF40BF
			// Volume Down 20DFC03F
			// Mute 20DF906F
			IRNECSender.send(0x20DF10EF);
		break;
	    case 'y': 
			// Volume Down 20DFC03F
			IRNECSender.send(0x20DFC03F);
		break;
	    case '8': 
		    //http://lirc.sourceforge.net/remotes/aiwa/RC-TN330
			IRAIWASender.send(0x00FF); //Turn on
	    case '9': 
		    //http://lirc.sourceforge.net/remotes/aiwa/RC-TN330
			IRAIWASender.send(0xB24D); //Vol up
		break;

	}
	if(inChar!='\n'){
		RS232Serial.print("code sent ");
		RS232Serial.println(inChar);
	}
}






