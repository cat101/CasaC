#include "config.h"
#include "debug.h"
#include "RFReaderHT12E.h"
#include <EEPROMEx.h>

#include <SimpleTimer.h>
extern SimpleTimer timer;

#include <SensorAcq.h>
extern SensorAcq sensors;
unsigned int lastPacket=0;

RFReaderHT12E::RFReaderHT12E(){
  // Init variables
  //processEvent=callback;
  validRFCodes=NULL;
  noRFCodes=0;
  lastCode=0;
  lastPacketTS=0;
  prevPacket=0;
}

void handleRFInterrupt();


void RFReaderHT12E::begin(){
  attachInterrupt(digitalPinToInterrupt(RFREADER_RADIO_PIN), handleRFInterrupt, CHANGE);
}

int RFReaderHT12E::sizeEECfg(void){
  return sizeof(unsigned long)*10; // We have room for ten codes
}

int RFReaderHT12E::loadEECfg(int address){
  unsigned long codes[10];
  if(address==0)
    return 0;
  int bytes=EEPROMEx.readBlock(address, codes);
  setValidCodes(codes, 10);
  return bytes;
}

void RFReaderHT12E::setValidCodes(unsigned long *validCodes, byte noCodes){
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
  validRFCodes=(unsigned int*)malloc(sizeof(unsigned int)*noRFCodes);
  byte a=0;
  for(c=0;c<noCodes;c++){
    if(validCodes[c]==0UL)
      continue;
    DEBUG_PRINT_P("Loading RF reader code %lx (%hd)\n", validCodes[c], c);
    validRFCodes[a++]=validCodes[c];
  }
}

void RFReaderHT12E::process(){
  if(lastPacket!=0 && (millis()-lastPacketTS)>1000){ //Only take a reading every 1 sec
    if(prevPacket==lastPacket){
      prevPacket=0; 
      lastPacketTS=millis();
      lastCode=lastPacket;
      DEBUG_PRINT_P("RF packet: 0x%04X (addr 0x%03X, data 0x%01X)\n",lastPacket,lastPacket>>2,lastPacket & B11);
      byte c;
      for(c=0;c<noRFCodes;c++){
        if(validRFCodes[c]==lastCode){
          // We found the code
          unlockDoor();
          break;
        }
      }
      if(c==noRFCodes){
        DEBUG_PRINT_P("RFReaderHT12E::invalidCode()\n");
      }
    }else{
      DEBUG_PRINT_P("lastPacket 0x%04X\n",lastPacket);
      prevPacket=lastPacket; //Debounce run      
    }
    lastPacket=0;
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

void RFReaderHT12E::unlockDoor() {
  DEBUG_PRINT_P("RFREADER_OPENDOOR_EV\n");
  setLock(true);
  timer.setTimeout(RFREADER_UNLOCK_TIME, lockDoor);
}

#define PACKET_LEN 12
void handleRFInterrupt() {
  static unsigned long lastTime;
  static unsigned int packet;
  static char c; //Current bit
  unsigned int duration;
  unsigned long time = micros();
  duration = time - lastTime;

  if(12000>duration && duration>9000){  //These are 36 clock-ticks on the HT12E
    //Match a long LOW (packet header)
    c=PACKET_LEN;
    packet=0;
  }else if(digitalRead(RFREADER_RADIO_PIN)==LOW && c!=0){
    //We just finished a high pulse
    if(333>duration && duration>250){ 
      packet = (packet << 1) + 1;         // attach a *1* to the rightmost end of the buffer
      c--;
    }else if(666>duration && duration>500){
      packet = (packet << 1);            // attach a *0* to the rightmost end of the buffer
      c--;
    }else{
      c=0; //Assume noise and wait for the next header
      packet=lastPacket; // Leave last packet untouched
    }
    if(c==0){ 
      // Got the whole packet
      lastPacket=packet;
    }
  }
  lastTime = time;  
}