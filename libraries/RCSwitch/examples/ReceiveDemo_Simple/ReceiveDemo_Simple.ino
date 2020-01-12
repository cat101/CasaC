/*
  Simple example for receiving
  
  https://github.com/sui77/rc-switch/
*/

#include "RCSwitch.h"

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(115200);
  Serial.print("Started");
  mySwitch.enableReceive(digitalPinToInterrupt(3));
}

void loop() {
  if (mySwitch.available()) {
    
    int value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
    }

    mySwitch.resetAvailable();
  }
}