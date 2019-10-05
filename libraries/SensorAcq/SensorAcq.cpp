#include "config.h"
#include "debug.h"
#include <SensorAcq.h>
#include <EEPROMEx.h>

void SensorAcq::begin(void){
#if RS485NODEID == 0
	// Add padding on the master
	padding[0]=RS485NODEID;
	padding[1]=sizeof(digSamples)+sizeof(senSamples)+sizeof(errSamples)+sizeof(digOutput);
	padding[2]=sizeof(analogSensors);
#endif	

	memset(senSamples,0,sizeof(senSamples));
	memset(digSamples,0,sizeof(digSamples));
	memset(dblClickSamples,0,sizeof(dblClickSamples));
	memset(digOutput,0,sizeof(digOutput));
	
	analogSamples=(int *)analogSensors;

	serviceMode=false;
	disableAnalog=false;
	digitalWriteCallback=NULL;

	// Configure all the output pins
	byte c;
	for(c=ACQ_OUT_RANGE_START;c<=ACQ_OUT_RANGE_STOP;c++){
		pinMode(c, OUTPUT);  
	}
	dblClickTimeOut=0;
	
	lastSensor=0;
	lastSensorSample=0;
	initSensors();
}

int SensorAcq::sizeEECfg(void){
	return sizeof(senLimits);
}

int SensorAcq::loadEECfg(int address){
	int read=0;
	if(address==0){
		for(byte c=0; c < 1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START;c++){
			senLimits[c].s2open=200; //Over this value the s2 open condition is triggered
			senLimits[c].s1open=610;
			senLimits[c].bothclosed=800;
			senLimits[c].shortc=900; //Over this value the condition is triggered
		}
	}else{
		read=EEPROMEx.readBlock(address, senLimits);
		#ifdef DEBUG
		DEBUG_PRINT_P("Loading sensor limits from EEPROM\n");
		for(byte c=0; c < 1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START;c++){
			DEBUG_PRINT_P(" [%d] %d %d %d %d\n",c
				,senLimits[c].s2open
				,senLimits[c].s1open
				,senLimits[c].bothclosed
				,senLimits[c].shortc);
		}
		#endif
	}
	return read;
}


void SensorAcq::digitalWrite(byte pin, byte val){
	//Check if we have a callback set
	if(digitalWriteCallback!=NULL){
		if(!digitalWriteCallback(pin, val)){
			return; //If the callback returns false then we block the IO
		}
	}
	if(!serviceMode){
		// Update the internal variable that caches the output state
		if(val==HIGH){
			BITOP(digOutput, pin - ACQ_OUT_RANGE_START, BIT_SET);
		}else{
			BITOP(digOutput, pin - ACQ_OUT_RANGE_START, BIT_CLEAR);
		}
		DEBUG_B_PRINT_P("SensorAcq::digitalWrite %hu %hu\n"
					,pin
					,val);
		::digitalWrite(pin, val); //Call the Arduino library to change the output
	}
}

byte SensorAcq::digitalRead(byte pin){
	return (BITOP(digOutput, pin, BIT_TEST))?HIGH:LOW;
}

boolean SensorAcq::acquirePass1(void){
	byte c,i,j,k;
	if(!disableAnalog){
		// Sample all the analog inputs and process the sensors
		memset(tmpSSamples,0,sizeof(tmpSSamples));
		for(c=ACQ_ANALOG_RANGE_START,i=j=k=0; c<=ACQ_ANALOG_RANGE_STOP ; c++,i++){
			tmpASamples[i]=analogRead(c);
			//On the sensor bitmap we map as [s2(3k3),s1(6k8)]
			if(tmpASamples[i]<senLimits[i].s2open){
				//Both bits are already cero (assume both open or wire cut)
			}else if(tmpASamples[i]<senLimits[i].s1open){
				tmpSSamples[k]|= (B01 << j); // Set s2=0 s1=1
			}else if(tmpASamples[i]<senLimits[i].bothclosed){
				tmpSSamples[k]|= (B10 << j); // Set s2=1 s1=0
			}else if(tmpASamples[i]<senLimits[i].shortc){
				tmpSSamples[k]|= (B11 << j); // Set s2=1 s1=1
			}else {
				// set the error
			}
			if(j==6){
				k++;
				j=0;
			}else{
				j+=2;
			}
		}
		// DEBUG_B_PRINT_P("A1 prev %u %hu %hX\n", tmpASamples[1],(tmpSSamples[0]>>2) & B00000011, tmpSSamples[0]);
	}
	// Sample all the digital inputs
	memset(tmpDSamples,0,sizeof(tmpDSamples));
	for(c=ACQ_DIG_RANGE_START,i=0,j=0; c<=ACQ_DIG_RANGE_STOP ; c++,j++){
		if(j==8){
			i++;
			j=0;
		}		
		bitWrite(tmpDSamples[i],j,::digitalRead(c));
		//DEBUG_BR_PRINT_P("Reading digital %d value %d\n",c,::digitalRead(c));
	}
	return true;
}

/* Sample all the analog inputs, debounce, process double clicks, and trigger events for digital inputs and sensors.
	On power up we will dispatch events for all the closed sensors

*/
boolean SensorAcq::acquirePass2(void){
	byte c; // Index into the analog ports (units)
	byte i; // Index into the analogSample array (uints)
	byte k,j; // Index into the temporary sensor sample (bit array where k is rows and j is columns)
	if(!disableAnalog){
		memset(errSamples,0,sizeof(errSamples));
		for(c=ACQ_ANALOG_RANGE_START,i=j=k=0; c<=ACQ_ANALOG_RANGE_STOP ; c++,i++){
			unsigned int reading=analogRead(c);
			// Ignore samples that are changing a lot during one sampling period (i.e. not stable)
			if(abs((signed int)(reading - tmpASamples[i]) )>ACQ_MAX_SAMPLE_DELTA){ //delta is 50
				DEBUG_BR_PRINT_P("SensorAcq::acquirePass2 sample A%hu ignored %u %u\n", i, reading, tmpASamples[i]);		
			}else{
				analogSamples[i]=reading;		
				byte tmpSSample=(tmpSSamples[k]>>j) & B00000011;
				byte tmpxor=(senSamples[k] ^ tmpSSamples[k])>>j;
				// if(i==1)
				// 	DEBUG_B_PRINT_P("A%hu curr %u prev %u %hu %hu j=%hu k=%hu %hX\n", i, reading,tmpASamples[i],tmpSSample,tmpxor & B11,j,k,tmpSSamples[k]);
				// DEBUG_PRINT_P("senSamples[k]=0x%02hx tmpSSamples[k]=0x%02hx tmpSSample=0x%02hx tmpxor=0x%02hx i=%hu j=%hu k=%hu\n"
				// 	,senSamples[k],tmpSSamples[k]
				// 	,tmpSSample,tmpxor,i,j,k);
				//On the sensor bitmap we map as [s2(3k3),s1(6k8)]
				if(reading<senLimits[i].s2open){
					//Both bits are already zero (assume both open or wire cut)
					if(tmpSSample == B00){ //Check that there is no bounce
						//Trigger events if the status of sensors change
						if(tmpxor & B01)
							dispatchSensorEvents(SEN_IN_BASE+k*8+j,SN_OPEN); 
						if(tmpxor & B10)
							dispatchSensorEvents(SEN_IN_BASE+k*8+j+1,SN_OPEN);
						senSamples[k] &= ~(B11<<j); 
					}
				}
				else if(reading<senLimits[i].s1open){
					if(tmpSSample == B01){ //Check that there is no bounce
						//Trigger events if the status of sensors change
						if(tmpxor & B01)
							dispatchSensorEvents(SEN_IN_BASE+k*8+j,SN_CLOSED); 
						if(tmpxor & B10)
							dispatchSensorEvents(SEN_IN_BASE+k*8+j+1,SN_OPEN);
						senSamples[k] |=  (B01<<j); //Set s2=0 s1=1
						senSamples[k] &= ~(B10<<j); 
					}
				}
				else if(reading<senLimits[i].bothclosed){
					if(tmpSSample == B10){ //Check that there is no bounce
						//Trigger events if the status of sensors change
						if(tmpxor & B01)
							dispatchSensorEvents(SEN_IN_BASE+k*8+j,SN_OPEN); 
						if(tmpxor & B10)
							dispatchSensorEvents(SEN_IN_BASE+k*8+j+1,SN_CLOSED);
						senSamples[k] &= ~(B01<<j); //Set  s2=1 s1=0
						senSamples[k] |=  (B10<<j); 
					}
				}
				else if(reading<senLimits[i].shortc){
					if(tmpSSample == B11){ //Check that there is no bounce
						//Trigger events if the status of sensors change
						if(tmpxor & B01)
							dispatchSensorEvents(SEN_IN_BASE+k*8+j,SN_CLOSED); 
						if(tmpxor & B10)
							dispatchSensorEvents(SEN_IN_BASE+k*8+j+1,SN_CLOSED);
						senSamples[k] |= (B11<<j);  //Set  s2=1 s1=1
					}
				}
				else {
					// The value is out of bound (potential short circuit)
					if(tmpSSample == B00) //This is not proper debouncing
					errSamples[k] |= (B11<<j);  //Set  s2=1 s1=1
				}
			}
			if(j==6){
				k++;
				j=0;
			}
			else{
				j+=2;
			}
		}
	}
	// Sample all the digital inputs
	for(c=ACQ_DIG_RANGE_START,i=0,j=0; c<=ACQ_DIG_RANGE_STOP ; c++,j++){
		if(j==8){
			i++;
			j=0;
		}		
		byte val=::digitalRead(c);
		//DEBUG_PRINT_P("In (%d) =  %hu\n",c, val); 
		// if(c>=28){
		// 	// DEBUG_B_PRINT_P("Reading digital %hu value %hu\n",c,val);
		// 	if(val==HIGH){
		// 		DEBUG_B_PRINT_P("Triggered digital %hu value %hu\n",c,val);
		// 	}
		// }
		if(bitRead(tmpDSamples[i],j)==val){
			//If the value is the same then propagate the result
			if(bitRead(digSamples[i],j) != val){ //Detect whether there was a change with the previous one
				bitWrite(digSamples[i],j,val);
				if(c<=ACQ_DIG_DBLCLICK_STOP){
					// This is a double click enabled digital input (detect double click)
					if(val==1){
						if(bitRead(dblClickSamples[i],j)==1){
							// Switch:1 transition to IN=1, DB=1, SE=1 
							// There was a click already within the double click period
							byte idx=((c-ACQ_DIG_RANGE_START)+(1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START)*2); // Compute the sensor positions (in bits or count)
							//DEBUG_PRINT("--> idx %u",idx);
							bitSet(senSamples[(idx>>3)],(idx&7));
							dispatchSensorEvents(idx+SEN_IN_BASE,SN_CLOSED); //Enable the double click sensor
						}else{
							// Switch:0 transition to IN=1, DB=1 
							// This is the first click
							bitSet(dblClickSamples[i],j);
							dispatchSensorEvents(c-ACQ_DIG_RANGE_START+DIG_IN_BASE,SN_CLOSED); // Turn on the digital input bit
							dblClickTimeOut=millis()+ACQ_DOUBLECLICK_DELAY; // Set a timer to check for double clicks							
						}
					}else{
						byte idx=((c-ACQ_DIG_RANGE_START)+(1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START)*2); // Compute the sensor positions (in bits or count)
						if(bitRead(senSamples[(idx>>3)],(idx&7))==1){
							// This is the off after a successful double click
							// Switch:0 transition to IN=0, DB=?, SE=0 
							bitClear(senSamples[(idx>>3)],(idx&7));
							// Dispatch both the sensor and the input events
							dispatchSensorEvents(idx+SEN_IN_BASE,SN_OPEN); //Enable the double click sensor
							dispatchSensorEvents(c-ACQ_DIG_RANGE_START+DIG_IN_BASE,SN_OPEN); // Turn on the digital input bit
						}else if(bitRead(dblClickSamples[i],j)==0){
							// There was no double click, just turn the 1 click light off
							// Switch:0 transition to IN=0, DB=0, SE=0 
							dispatchSensorEvents(c-ACQ_DIG_RANGE_START+DIG_IN_BASE,SN_OPEN);
						}
					}
				}else{
					// This is not a double click capable input
					dispatchSensorEvents(c-ACQ_DIG_RANGE_START+DIG_IN_BASE,val);
				}
			}
		}
	}
	return true;
}
void SensorAcq::dblClickClean(void){
	// Clear all the double click flags
	for(byte c=ACQ_DIG_RANGE_START,i=0,j=0; c<=ACQ_DIG_DBLCLICK_STOP ; c++,j++){
		if(j==8){
			i++;
			j=0;
		}		
		if(bitRead(dblClickSamples[i],j)==1){
			if(bitRead(digSamples[i],j) == 0){ 
				//We delayed off until we ensure that there was no double click. No dispatchSensorEvents the off
				dispatchSensorEvents(c-ACQ_DIG_RANGE_START+DIG_IN_BASE,0);
			}
			bitClear(dblClickSamples[i],j);
		}
	}			
	dblClickTimeOut=0;	
	return;
}


// This function schedules the sampling of all the sensors (no light switches or alarm loops) over the configured period
void SensorAcq::acquireSensors(void){
	if(millis()-lastSensorSample>(ACQ_SENSORSAMPLE_PERIOD/ACQ_NO_ANALOG_SENSORS)){
		lastSensorSample=millis();
		acquireSensor(lastSensor++); // Call the local hook one sensor at a time 
		if(lastSensor==ACQ_NO_ANALOG_SENSORS){
			lastSensor=0;
		}
	}else{
		acquireSensor(255); // Call the local hook to use an idle slot in between the analog sensors
	}
}

// During board assembly and testing this function maps inputs to outputs. In production this functions does nothing so that the I/O is ignored
void SensorAcq::executeServiceMode(byte signal,byte value){
#ifdef ENABLE_SERVICEMODE_BENCHTEST	
	byte out;
	DEBUG_PRINT_P("%hu %hu %hu\n", signal,S00,ACQ_OUT_RANGE_START);
	if(signal<S00){
		// It is a switch event
		out=signal; //Map the input to output (I00 -> O00)
	}else{
		// It is a loop/sensor event
		//out=(int)(((int)signal-(int)S00) + (int)ACQ_OUT_RANGE_START); //Map the input to output (S00 -> O00)
		out=signal;
		DEBUG_PRINT_P("%hu\n", out);
		out-=S00;
		DEBUG_PRINT_P("%hu\n", out);
	}
	// Wrap around the output
	out=out % (1+ ACQ_OUT_RANGE_STOP - ACQ_OUT_RANGE_START);
	out+=ACQ_OUT_RANGE_START;
	DEBUG_PRINT_P("Testing O%02hu\n", out-ACQ_OUT_RANGE_START);
	if(out<=ACQ_OUT_RANGE_STOP){
		digitalWrite(out, value);
	}
#endif	
}