#ifndef SensorAcq_h
#define SensorAcq_h

#include "Arduino.h"
#include "config.h"

struct sensorThreshold{
	unsigned int s2open, s1open, bothclosed, shortc;
};

#define ACQ_MAX_SAMPLE_DELTA 20 //This parameter controls what is the maximun delta tolerated between to consecutive sampling (i.e. ignore loop readings if they are changing)

#define BITOP(arr,bit,op) ((arr)[(byte)((bit)>>3)] op ((byte)1<<((byte)(bit) & 7)))
#define BIT_SET |=
#define BIT_CLEAR &=~
#define BIT_TEST &
// BITOP(array, 40, |=); /* sets bit 40 */
// BITOP(array, 41, ^=); /* toggles bit 41 */
// if (BITOP(array, 42, &)) return 0; /* tests bit 42 */
// BITOP(array, 43, &=~); /* clears bit 43 */

#define SN_OPEN 0
#define SN_CLOSED 1

class SensorAcq {
public:
#if RS485NODEID == 0
	// Add padding on the master
//	byte padding[]={RS485NODEID,sizeof(digSamples)+sizeof(senSamples)+sizeof(errSamples)+sizeof(digOutput),sizeof(analogSamples)};
	byte padding[3];//={RS485NODEID,0,0};
#endif	
	byte digSamples[1+(1+ACQ_DIG_RANGE_STOP-ACQ_DIG_RANGE_START)/8];
	// The sensor array has 2 bit entries per every analog input and then 1 entry per double click input
#if ACQ_DIG_DBLCLICK_STOP != 0
	byte senSamples[1+((1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START)*2+(1+ACQ_DIG_DBLCLICK_STOP-ACQ_DIG_RANGE_START))/8];  
#else
	byte senSamples[1+((1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START)*2)/8];  
#endif	
	byte errSamples[1+((1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START)*2)/8];  
	byte digOutput[1+(1+ACQ_OUT_RANGE_STOP-ACQ_OUT_RANGE_START)/8]; 
	// The analogSensors is used to store first ADC readouts corresponding to the alarm loops and then sensors values from I2C, DHT and other sensors
	byte analogSensors[(1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START)*2+ACQ_ANALOG_SENSOR_MAPSIZE]; 

#if ACQ_DIG_DBLCLICK_STOP != 0
	byte dblClickSamples[1+(1+ACQ_DIG_DBLCLICK_STOP-ACQ_DIG_RANGE_START)/8];
#else
	byte dblClickSamples[1];
#endif	
	byte tmpDSamples[1+(1+ACQ_DIG_RANGE_STOP-ACQ_DIG_RANGE_START)/8];
	byte tmpSSamples[1+((1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START)*2)/8];
	unsigned int tmpASamples[1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START]; 
	sensorThreshold senLimits[1+ACQ_ANALOG_RANGE_STOP-ACQ_ANALOG_RANGE_START];
	unsigned long dblClickTimeOut;

	bool serviceMode; // If true it disables dispatching
	bool disableAnalog; //If true analog sampling is disabled (used to mask noise)
	void executeServiceMode(byte signal,byte value);

	void begin(void);
	int sizeEECfg(void);
	int loadEECfg(int address);

	boolean acquirePass1(void);
	boolean acquirePass2(void);
	void acquireSensors(void);
	void dblClickClean(void);
	void digitalWrite(byte pin, byte val);
	byte digitalRead(byte pin);

private:
	int *analogSamples; //This is used to easyly manage ADC samplings

	byte lastSensor;
	unsigned long lastSensorSample;
};

#endif
