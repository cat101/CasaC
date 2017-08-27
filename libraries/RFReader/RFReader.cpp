#include "config.h"
#include "debug.h"
#include "RFReader.h"
#include <EEPROMEx.h>
#include "RCSwitch.h"

RCSwitch mySwitch = RCSwitch();

#ifdef RFREADER_LOCK
  #include <SimpleTimer.h>
  extern SimpleTimer timer;
  #include <SensorAcq.h>
  extern SensorAcq sensors;
#endif
// byte lastLen;


RFReader::RFReader(){
  // Init variables
  validRFCodes=NULL;
  noRFCodes=0;
  lastCode=0;
  lastPacketTS=0;
}

void handleRFInterrupt();


void RFReader::begin(){
  mySwitch.enableReceive(digitalPinToInterrupt(RFREADER_RADIO_PIN));
}

int RFReader::sizeEECfg(void){
  return sizeof(unsigned long)*10; // We have room for ten codes
}

int RFReader::loadEECfg(int address){
  unsigned long codes[10];
  if(address==0)
    return 0;
  int bytes=EEPROMEx.readBlock(address, codes);
  setValidCodes(codes, 10);
  return bytes;
}

void RFReader::setValidCodes(unsigned long *validCodes, byte noCodes){
  //Split the list into PINs & RFIDs
  byte c;
  if(validRFCodes!=NULL){
      //This is a configuration reload
     free(validRFCodes);noRFCodes=0;
  }
  // Count how many codes we received
  for(c=0;c<noCodes;c++){
    if(validCodes[c]!=0UL)
      noRFCodes++;
  }
  //DEBUG_PRINT_P("Allocating %d users and %d cards\n", noPINs, noRFIDs);
  validRFCodes=(unsigned long*)malloc(sizeof(unsigned long)*noRFCodes);
  byte a=0;
  for(c=0;c<noCodes;c++){
    if(validCodes[c]==0UL)
      continue;
    DEBUG_PRINT_P("Loading RF reader code %lx (%hd)\n", validCodes[c], c);
    validRFCodes[a++]=validCodes[c];
  }
}

void RFReader::process(){
  if(mySwitch.available()) {
    lastCode=mySwitch.getReceivedValue();
    if((millis()-lastPacketTS)>1000){ //Only take a reading every 1 sec
      lastPacketTS=millis();
      DEBUG_PRINT_P("RF code: 0x%08lX (%hubits, protocol %hu)\n",lastCode,mySwitch.getReceivedBitlength(),mySwitch.getReceivedProtocol());
      byte c;
      for(c=0;c<noRFCodes;c++){
        if(validRFCodes[c]==lastCode){
          // We found the code
          unlockDoor();
          break;
        }
      }
      if(c==noRFCodes){
        DEBUG_PRINT_P("RFReader::invalidCode()\n");
      }
    }
    mySwitch.resetAvailable();
  }
}

#ifdef RFREADER_LOCK
  #define setLock(active) sensors.digitalWrite(RFREADER_LOCK,active); 
#else
  #define setLock(active)
#endif

void lockDoor(void){
  setLock(false);
}

void RFReader::unlockDoor() {
  DEBUG_PRINT_P("RFREADER_OPENDOOR_EV\n");
  setLock(true);
  #ifdef RFREADER_LOCK
    timer.setTimeout(RFREADER_UNLOCK_TIME, lockDoor);
  #endif
}