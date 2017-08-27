#include "config.h"
#include "debug.h"

#include <CardReader.h>
#include <pins_arduino.h>
#include <EEPROMEx.h>

#include <SimpleTimer.h>
extern SimpleTimer timer;

#include <SensorAcq.h>
extern SensorAcq sensors;

volatile unsigned long wieReader;
volatile int  wieReaderCount;

void wieReaderOne(void);
void wieReaderZero(void);

CardReader::CardReader(){
  // Init variables
  //processEvent=callback;
  validRFIDs=NULL;
  noRFIDs=0;
  validPINs=NULL;
  noPINs=0;
  pinTimeout=0;
  wieReaderCount=0;
  wieReader=0;
  lastEntry=0;
}

void CardReader::begin(){
  // Init IO
  pinMode(CARD_READER_D0, INPUT);
  digitalWrite(CARD_READER_D0, HIGH); // enable internal pull up
  pinMode(CARD_READER_D1, INPUT);
  digitalWrite(CARD_READER_D1, HIGH); // enable internal pull up
#ifdef CARD_READER_LED
  pinMode(CARD_READER_LED, OUTPUT);
  digitalWrite(CARD_READER_LED, HIGH); // The led switches from red to green when set to LOW
#endif
#ifdef CARD_READER_BUZZ  
  pinMode(CARD_READER_BUZZ, OUTPUT);
  digitalWrite(CARD_READER_BUZZ, HIGH); // The buzzer enables on LOW
#endif
// #ifdef CARD_READER_LOCK
//   pinMode(CARD_READER_LOCK, OUTPUT);
//   digitalWrite(CARD_READER_LOCK, LOW); 
// #endif

  // The init code only supports pins 2 & 3
#if CARD_READER_D0!=2 || CARD_READER_D1!=3
  #error Change the interrupt rutines
#endif
  attachInterrupt(0,wieReaderZero, CHANGE);  //Attach the hardware interrupts to the Wiegand data pins
  attachInterrupt(1,wieReaderOne, CHANGE);
}

int CardReader::sizeEECfg(void){
  return sizeof(unsigned long)*10;
}

int CardReader::loadEECfg(int address){
  unsigned long users[10];
  if(address==0)
    return 0;
  int bytes=EEPROMEx.readBlock(address, users);
  setValidUsers(users, 10);
  return bytes;
}

void CardReader::setValidUsers(unsigned long *validUsers, byte noUsers){
  //Split the list into PINs & RFIDs
  byte c;
  if(validRFIDs!=NULL){
      //This is a configuration reload
     free(validRFIDs);noRFIDs=0;
     free(validPINs);noPINs=0;
  }
  for(c=0;c<noUsers;c++){
    if(validUsers[c]==0UL)
      continue;
    if(validUsers[c]<=0xFFFFUL){
      noPINs++;
    }else{
      noRFIDs++;
    }
  }
  //DEBUG_PRINT_P("Allocating %d users and %d cards\n", noPINs, noRFIDs);
  validRFIDs=(unsigned long*)malloc(sizeof(unsigned long)*noRFIDs);
  validPINs=(unsigned int*)malloc(sizeof(unsigned int)*noPINs);
  byte a=0,b=0;
  for(c=0;c<noUsers;c++){
    if(validUsers[c]==0UL)
      continue;
    DEBUG_PRINT_P("Loading card reader user %lx (%hd)\n", validUsers[c], c);
    if(validUsers[c]<=0xFFFFUL){
      validPINs[a++]=validUsers[c];
    }else{
      validRFIDs[b++]=validUsers[c];
    }
  }
}

void CardReader::process(){
  if(wieReaderCount==0){
      return; // No reader activity
  }
  if(pinTimeout==0){
    pinTimeout=millis(); // Start the reader activity
    return;
  }

  //Wait 70ms in case it was a card scan
  if(millis()-pinTimeout>=70){
    if(wieReaderCount == 26){
      DEBUG_PRINT_P("Reader entry %lx (card)\n",wieReader);
      byte c;
      for(c=0;c<noRFIDs;c++){
        if(validRFIDs[c]==wieReader){
        // We found the RFID
          unlockDoor();
          break;
        }
      }
      if(c==noRFIDs)
        invalidCode(); 
      lastEntry=wieReader;
      pinTimeout=wieReaderCount=wieReader=0;
    }else if(wieReaderCount == 20){
      DEBUG_PRINT_P("Reader entry %lx (PIN)\n",wieReader);
      if((wieReader & 0xf) == 0){
        //Is this a request to open the door?
        unsigned int entry=(wieReader>>4);
        byte c;
        for(c=0;c<noPINs;c++){
          if(validPINs[c]==entry){
          // We found the PIN
            unlockDoor();
            break;
          }
        }
        if(c==noPINs)
          invalidCode(); 
      }
      lastEntry=wieReader;
      pinTimeout=wieReaderCount=wieReader=0;
    }else if(millis()-pinTimeout>=CARD_PIN_ENTRYTIME){
      //No valid entry reset
      // DEBUG_PRINT_P("RESET\n");
      pinTimeout=wieReaderCount=wieReader=0;
    }
  }
}

#ifdef CARD_READER_LED
  #define setLED(active) digitalWrite(CARD_READER_LED,(active)?LOW:HIGH); 
  #define toggleLED() *portInputRegister(digitalPinToPort(CARD_READER_LED)) = digitalPinToBitMask(CARD_READER_LED);
#else
  #define setLED(active)
  #define toggleLED()
#endif
#ifdef CARD_READER_LOCK
  #define setLock(active) sensors.digitalWrite(CARD_READER_LOCK,active); 
#else
  #define setLock(active)
#endif

#ifdef CARD_READER_BUZZ  
  #define setBuzzer(active) digitalWrite(CARD_READER_BUZZ,(active)?LOW:HIGH); 
  // The Port Input Pins I/O location is read only. However, writing a logic one to a bit in 
  // the PINx Register, will result in a toggle in the corresponding bit in the Data Register
  #define toggleBuzzer() *portInputRegister(digitalPinToPort(CARD_READER_BUZZ)) = digitalPinToBitMask(CARD_READER_BUZZ);
#else
  #define setBuzzer(active)
  #define toggleBuzzer()
#endif
void lockDoorWrapper(void){
  CardReader::lockDoor(); 
}
void CardReader::lockDoor() {
  setLED(false);
  setLock(false);
  sensors.disableAnalog=false;
}

void CardReader::unlockDoor() {
  DEBUG_PRINT_P("READER_OPENDOOR_EV\n");
  sensors.disableAnalog=true; //Prevent faulty analog readings due to the current drawn by the door strike
  setLED(true);
  setLock(true);
  timer.setTimeout(CARD_DOOR_UNLOCK_TIME, lockDoorWrapper);
}

// byte noBuzz=0;
void buzzError(void){
  toggleBuzzer();
}

void CardReader::invalidCode(void){
  DEBUG_PRINT_P("CardReader::invalidCode()\n");
  timer.setTimer(50, buzzError, CARD_BUZZER_ERROR_DURATION/50);
}

void blink(void){
  toggleLED();
}

int blinkTimer=-1; // -1 is the return value when there are no more timers
void CardReader::blinkLED(bool isOn){
  if(isOn && blinkTimer==-1){
    blinkTimer=timer.setInterval(500, blink);
  }else if(!isOn && blinkTimer!=-1){
    timer.deleteTimer(blinkTimer);
    setLED(false);
    blinkTimer=-1;
  }
}

void wieReaderOne(void) {
  if(digitalRead(CARD_READER_D1) == LOW){
    wieReaderCount++;
    wieReader = wieReader << 1;
    wieReader |= 1;
  }
}

void wieReaderZero(void) {
  if(digitalRead(CARD_READER_D0) == LOW){
    wieReaderCount++;
    wieReader = wieReader << 1;  
  }
}



 
 
