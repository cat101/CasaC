/*---

 HT12E.h
 HT12E Support for Arduino
 The purpose of this library is to enable your Arduino to receive commands remotely
 from a HT12E-driven device or control.  
 Note  : make sure HT12E is operating at 3~4kHz clock range
 Author: Marcelo Shiniti Uchimura
 Date  : Sep '13
 
---*/

#ifndef HT12E_h
#define HT12E_h

#define MAXTRIES 13
#define DATASIZE 12
 
#include "Arduino.h"

class HT12E 
{
 public:
                HT12E(int pin, unsigned int addrMask); // this is the constructor
   int          read();                                // this is the main method
   int          error();                               // for error handling purpose
   int          available();                           // used when there's potentially significant data
 private:
   uint8_t      _pin;      // this is Arduino input pin
   unsigned int _data;     // this is data
   unsigned int _mask;     // this is the address mask
   uint8_t      _flags;    // handles error() (bit 0, 1, 2, and 3) flag
   unsigned long _dur;     // pulse duration
   uint8_t      pilotStreamAvailable();
   uint8_t      syncBitGone();
   uint8_t      dataStreamAvailable();
   uint8_t      maskMatchesIncomingData();
};

#endif

/*---

 HT12E.cpp
 HT12E Support for Arduino
 The purpose of this library is to enable your Arduino to receive commands remotely
 from a HT12E-driven device or control.  
 Note  : make sure HT12E is operating at 3~4kHz clock range
 Author: Marcelo Shiniti Uchimura
 Date  : Sep '13
 
---*/

//#include "HT12E.h"

HT12E::HT12E(int pin, unsigned int addrMask)
{
 _pin = pin;
 pinMode(_pin, INPUT);
 _data = 0;
 _mask = addrMask << 4;  // the HT12E basic word is a stream with an 8-bit address
                         // followed by 4-bit data. I left shift the
                         // address mask 4 bits so I can match it to the entire word
}

int HT12E::read()
{
 return _data;
}

int HT12E::error()
{
 uint8_t temp = _flags;
 _flags = 0;
 return temp & B00001111;   // LSB of _flags is the error flag
}

uint8_t HT12E::pilotStreamAvailable()
{
 /* look for HT12E basic word's pilot stream */
 for(uint8_t ctr = 0; ctr < MAXTRIES; ++ctr)
 {
   while(digitalRead(_pin) == LOW);                // wait for the signal to go HIGH
   
   _dur = pulseIn(_pin, LOW);

   if(_dur > 9000 && _dur < 12000) 
     return 1;          // 36x(clock tick interval)
 }
 
 _flags |= B0001;
 return 0;
}
 
uint8_t HT12E::syncBitGone()
{
 /* now wait until sync bit is gone */
 for(uint8_t ctr = 0; ctr < MAXTRIES; ++ctr)
 {
    if(digitalRead(_pin) == LOW) 
      return 1;
    delayMicroseconds(80);
 }
 
 _flags |= B0010;
 return 0;
}

uint8_t HT12E::dataStreamAvailable()
{
 uint8_t ctr;
 
 /* let's get the address+data bits now */
 for(_data = 0, ctr = 0; ctr < DATASIZE; ++ctr)
 {
   _dur = pulseIn(_pin, HIGH);
   if(_dur > 250 && _dur < 333)        // if pulse width is between 1/4000 and 1/3000 secs
     {
     _data = (_data << 1) + 1;         // attach a *1* to the rightmost end of the buffer
   }
   else if(_dur > 500 && _dur < 666)   // if pulse width is between 2/4000 and 2/3000 secs
   {
     _data = (_data << 1);             // attach a *0* to the rightmost end of the buffer
   }
   else
   {
     _flags |= B0100;
     return 0;
   }
 }
 
 return 1;
}

uint8_t HT12E::maskMatchesIncomingData()
{
 if((_data & _mask) < _mask)
 {
   _flags |= B1000;
   return 0;
 }
 return 1;
}

int HT12E::available()
{
 _flags = 0; // initializes error handling reference
 
 if (pilotStreamAvailable()
   &&  syncBitGone()
   &&  dataStreamAvailable()
   &&  maskMatchesIncomingData())
 {  
   _data ^= _mask;
   return 1;
 }
 
 _data = 0;
 return 0;
}

HT12E remoteControl(2, B01010101); /* the incoming stream is tied to pin 7, and the address mask is the binary 01111111 */

#define RADIO_DATA_PIN 2  //Needs to support interrupts
#define PACKET_LEN 12
unsigned int lastPacket;
void handleRFInterrupt();

void setup()
{
 Serial.begin(115200);
 attachInterrupt(digitalPinToInterrupt(RADIO_DATA_PIN), handleRFInterrupt, CHANGE);

}

void loop()
{
  if(lastPacket!=0){
    Serial.println("I have just received the following data stream: ");
    Serial.println(lastPacket, BIN);    
    lastPacket=0;
  }
//  if (remoteControl.available() > 0)
//  {
//    unsigned int data;
//    data = remoteControl.read();
//    Serial.println("I have just received the following data stream: ");
//    Serial.println(data, BIN);
//  }
//  else
//  {
//    switch (remoteControl.error())
//    {
//      case 0:
//        break;
//      case 1:
// //       Serial.println("Failure when waiting for a pilot stream");
//        break;
//      case 2:
//        Serial.println("Failure when waiting for sync bit to go away");
//        break;
//      case 4:
// //       Serial.println("Failure when waiting for a data stream");
//        break;
//      case 8:
//        Serial.println("Data mask does not match predefined address mask");
//        break;
//      default:
//        Serial.println("Unknown error occurred");
//        break;
//    } 
//  }
}


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
  }else if(digitalRead(RADIO_DATA_PIN)==LOW && c!=0){
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


