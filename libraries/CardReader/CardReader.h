#ifndef	_CARDREADER_H_ 
#define	_CARDREADER_H_ 

#include <Arduino.h>


/*
	The card reader abstracts the keypad and the logic to open the door. The list of valid PINs and RFIDs 
	is stored in EEPROM. All the entries (valid & invalid) are reported to be sent to the alarm.
	A valid pin is four digits and a zero at the end
*/


class CardReader{
private:	
	unsigned long pinTimeout;
	unsigned long *validRFIDs;
	byte noRFIDs;
	unsigned int *validPINs;
	byte noPINs;
	void setValidUsers(unsigned long *validUsers, byte noUsers);

public:
	CardReader();
	void begin();
	int sizeEECfg(void);
	int loadEECfg(int address);
	void process();
	static void lockDoor();
	void unlockDoor(void);
 	void invalidCode(void);
 	void blinkLED(bool isOn);

	// Zero means no new entries
	unsigned long lastEntry;
};

#endif

