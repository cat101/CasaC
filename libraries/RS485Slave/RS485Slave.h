#ifndef RS485Slave_h
#define RS485Slave_h
#include "Arduino.h"
#include "RS485_Link.h"

// This function is called when a command is received. Return false if the buffer could not be processed
typedef bool commandDispatcher(byte *buf,byte received);

class RS485Slave{
private:
#ifdef RS485_ENABLE_TESTS
	byte testBuff[RS485_TESTBUFSIZE];
	unsigned int nextRoundDelay;
#endif
public:
	void begin(void);
	bool processConnection(unsigned long timeout=10);
	
};
#endif //RS485Slave_h