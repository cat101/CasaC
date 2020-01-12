## Reader usage

<!-- ![](../../images/RFID%20Keypad.jpg?50%) -->
<img src="../../uploads/images/RFID%20Keypad.jpg" alt="keypad" style="width: 300px;"/> 

- Commands
  - pins are four digits and cannot end with 0 
  - RFID card opens the door directly (does not affect arm/disarmed state)
  - armed 
    - pin (4 digit) + # or \* (disarm) 
    - pin (4 digit) + repeat last digit (duress)

  - disarmed 
    - pin (4 digit) + # (stay) 
    - pin (4 digit) + \* (leave) 
    - pin (4 digit) + 0 (open door)

## Adding a reader to a uC

- Wire the reader to 12V
- Add to config.cpp

```
#include "CardReader.h"
CardReader reader;

void initTasks(void){ // Called by setup()
  reader.begin();
}

void p2loop(){ // Called from loop
  reader.process();
}

void sendEventsToMaster(){ // Called from RS485Slave::processConnection
  // Check if there is an outstanding reader event to piggy back
  if(reader.lastEntry){
    byte cardReaderEv[]={0x14}; // The extra frame format is Frametype (4bits), Framesize(4bits)
    sendMsgPart (cardReaderEv, sizeof(cardReaderEv), false);
    sendMsgPart ((byte *)&reader.lastEntry, sizeof(reader.lastEntry), false);
    reader.lastEntry=0;
  }
}

int loadEECfg(void){
  EEPROM_SIZE_CFG(reader);
    ...
  EEPROM_READ_CFG(reader);
}
```

- On the PIN Usage sheet add 
  - PA-Oeste D02 CARD\_READER\_D0 Green wire
  - PA-Oeste D03 CARD\_READER\_D1 White wire
  - PA-Oeste D06 CARD\_READER\_LED Blue wire
  - PA-Oeste D07 CARD\_READER\_BUZZ Yellow wire

- Add an output to open a door (circuitos & automation sheet). This will add the users to the config file
  - PA\_O01\_D Pestillo puerta simulada PA-Oeste O03 Door strike

- On the local device set the output for the door strike and other constants (config.h)

```
// Config the card reader
#define CARD_READER_LOCK PA_O01_D 
#define CARD_PIN_ENTRYTIME 5000 //ms
#define CARD_DOOR_UNLOCK_TIME 5000
#define CARD_BUZZER_ERROR_DURATION 2500
```

- Upload the user PINs & Keycards
  - Check config.pl and run ResourcePreprocesor.pl
  - curl -H "Expect:" --upload-file 1.cfg "192.168.1.80/fs/"
  - curl "192.168.1.80/reloadconfig?node=1"

