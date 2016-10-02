/* Example program for from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.0  January 2013
 * Copyright 2013 by Chris Young http://cyborg5.com
 * Based on original example sketch for IRremote library 
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */
#include <IRLib.h>

//IRsendNECx My_Sender;
IRsendRaw My_Sender;

void setup()
{
  Serial.begin(115200);
}

// off 
unsigned int raw1[]={4437 ,4416 ,597 ,1579 ,640 ,469 ,683 ,1472 ,619 ,1557 ,683 ,384 ,661 ,427 ,619 ,1557 ,640 ,427 ,597 ,512 ,619 ,1557 ,683 ,405 ,661 ,427 ,619 ,1557 ,640 ,1515 ,619 ,491 ,619 ,1536 ,661 ,448 ,640 ,1493 ,619 ,1557 ,619 ,1536 ,576 ,1600 ,619 ,469 ,683 ,1472 ,619 ,1557 ,683 ,1472 ,619 ,469 ,619 ,491 ,640 ,448 ,576 ,533 ,661 ,1515 ,619 ,469 ,640 ,427 ,619 ,1557 ,619 ,1557 ,640 ,1493 ,619 ,491 ,619 ,469 ,619 ,469 ,640 ,448 ,597 ,512 ,619 ,469 ,619 ,469 ,619 ,491 ,683 ,1472 ,640 ,1536 ,683 ,1472 ,619 ,1557 ,661 ,1493 ,619 ,4715 ,4501 ,4416 ,704 ,1472 ,576 ,512 ,619 ,1536 ,683 ,1472 ,619 ,469 ,619 ,469 ,640 ,1493 ,640 ,469 ,619 ,469 ,619 ,1536 ,576 ,512 ,619 ,469 ,619 ,1536 ,661 ,1493 ,619 ,469 ,640 ,1493 ,619 ,491 ,661 ,1493 ,619 ,1536 ,661 ,1493 ,619 ,1536 ,661 ,427 ,640 ,1536 ,640 ,1493 ,619 ,1557 ,640 ,427 ,597 ,533 ,619 ,491 ,661 ,405 ,683 ,1493 ,619 ,491 ,640 ,448 ,619 ,1557 ,619 ,1557 ,683 ,1472 ,619 ,491 ,661 ,405 ,661 ,448 ,597 ,512 ,619 ,491 ,619 ,469 ,640 ,427 ,661 ,427 ,619 ,1557 ,640 ,1493 ,619 ,1557 ,640 ,1493 ,619 ,1557 ,683};
void loop() {
  switch(Serial.read()){
    //send a code  every time a character is received from the serial port
    //Sony DVD power A8BCA
//    My_Sender.send(0xB24D7B84);
    case '\n':
    case '1':
      My_Sender.send(raw1,sizeof(raw1),38);
      Serial.print("Sent 1\n");
      break;
    case '2':
      My_Sender.send(raw2,sizeof(raw2),38);
      Serial.print("Sent 2\n");
      break;
  }
}


