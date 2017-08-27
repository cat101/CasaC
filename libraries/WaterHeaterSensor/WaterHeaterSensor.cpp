
// Extracted from http://hack4life.pbworks.com/Arduino%20Solar%20Water%20Heater%20Sensor

//#include "Arduino.h"
#include "config.h"
#include "debug.h"

// The input needs to generate I/O interrupts (https://www.arduino.cc/en/Reference/AttachInterrupt)
const int inPin = 21; //This is hard coded for now...it should really be WATER_TANK_SENSOR
const int inInt = digitalPinToInterrupt(inPin); 


//volatile unsigned int count = 0;
volatile char tempWH,levelWH; // temp in Celsius and level goes from 0 to 3


void handleSensorInterrupt();

void initWaterHeaterSensor() {
  attachInterrupt(inInt, handleSensorInterrupt, CHANGE);
}

byte wh_data[5];

void handleSensorInterrupt() {
  /* This routine has been optimized to just capture the part of the packet 
     that has the temp & level
     On averages it uses 550 us per second
  */
  static unsigned int duration;
  static unsigned long lastTime;
  static char c=15; //Current bit
  static bool expectMark;
  unsigned long time = micros();
  duration = time - lastTime;
  if(5000>duration && duration>3000){
    //Match SPACE_HEADER typically 4000us (new packet)
    c=39;
    // memset(data, 0, sizeof(data));
    wh_data[2]=0;
    wh_data[3]=0;
    expectMark=true;
  // }else if(c>0){ // Get the whole packet
  }else if(c>=16){ 
    if(expectMark){
      // Skip MARK
      expectMark=false;
    }else{
      // Decode SPACE
      if(1700>duration && duration>1300){
        // #define SPACE_ONE 1500
        wh_data[c>>3]|=(1<<(c & B111));
        c--;
      }else if(800>duration && duration>400){
        // #define SPACE_ZERO 500
        c--;
      }
      expectMark=true;
      if(c==15){
        // Got all the bits
        tempWH=wh_data[3];
        levelWH=(wh_data[2] & 0x0F);
      }
    }
  }
  lastTime = time;  
//  count+=micros()-time;
}

// #define READ_TIMEOUT 70000UL
// #define HEADER_TIMEOUT 84000UL // READ_TIMEOUT*1.2
// #define TIMEOUT 50000UL
// #define MARK_HEADER 7000
// #define SPACE_HEADER 4000
// #define MARK 500
// #define SPACE_ONE 1500
// #define SPACE_ZERO 500

// int expectPulse(int val){
//   unsigned long t=micros();
//   while(digitalRead(39)==val){
//     if( (micros()-t)>TIMEOUT ) return 0;
//   }
//   return micros()-t;
// }

// // temp in celsious and level goes from 0 to 3
// bool readTempNLevelSensor(char inPin, char &temp, char &level){
//    pinMode(39,INPUT);
//    byte wh_data[5]={0,0,0,0,0};
//    unsigned long val1;
//    unsigned long st=micros();
//    val1 = expectPulse(HIGH);
//    if(val1>MARK_HEADER){
//      val1 = expectPulse(LOW);
//      if(val1>SPACE_HEADER){
//        int c=39;
//        for(;c>-1;c--){
//          val1 = expectPulse(HIGH);
//          if(val1<MARK){
//           val1 = expectPulse(LOW);
//           if(val1==0){
//             //Serial.println("Mark error 0");
//             break; 
//           }
//           if(val1<SPACE_ZERO){
//             //0
//             //wh_data[c>>3]|=(1<<(c & B111));
//           }else if(val1<SPACE_ONE){
//             //1
//             wh_data[c>>3]|=(1<<(c & B111));
//           }else{
//            //Serial.print(val1);Serial.println(" Space error");
//            break; 
//           }
//          }else{
//           //Serial.println("Mark error");
//           break; 
//          }  
//        }
//        // Each reading should not take more than 70ms (use time to detect errors)
//        if(micros()-st<70000){
//          temp=wh_data[3];
//          level=wh_data[2];
//          //Serial.print(wh_data[3]);Serial.print(" ");Serial.println((wh_data[2]));
//          //Serial.print(wh_data[4],HEX);Serial.print(" ");Serial.print(wh_data[3],HEX);Serial.print(" ");Serial.print(wh_data[2],HEX);Serial.print(" ");Serial.print(wh_data[1],HEX);Serial.print(" ");Serial.println(wh_data[0],HEX);
//          return true;
//        }
//     }
//    }  
//    return false;
// }


// unsigned int expectPulse(char inPin, int val){
//   unsigned long t=micros();
//   while(digitalRead(inPin)==val){
//     if( (micros()-t)>TIMEOUT ) return 0;
//   }
//   return (micros()-t);
// }


// // temp in Celsius and level goes from 0 to 3
// bool readTempNLevelSensor(char inPin, char &temp, char &level){
//    byte wh_data[5]={0,0,0,0,0};
//    unsigned int val1;
//    pinMode(inPin,INPUT);
// //   val1 = expectPulse(inPin, HIGH);

//    unsigned long t=micros();
//    unsigned long high_st=t;
//    while( (micros()-high_st) <=MARK_HEADER){
//      high_st=micros();
//      while(digitalRead(inPin)==HIGH){
//        if( (micros()-high_st) > MARK_HEADER) break;
//      }
//      if( (micros()-t)>HEADER_TIMEOUT ){
//       DEBUG_PRINT_P("Timed out (%lu, %lu, %lu, %lu, val1=%lu)\n",t, high_st, micros(),HEADER_TIMEOUT,(micros()-high_st));
//       return false;
//      }
//    }
//    val1=(micros()-high_st);
//    unsigned long st=millis();
//    if(val1>MARK_HEADER){
//      val1 = expectPulse(inPin, LOW);
//      if(val1>SPACE_HEADER){
//        int c=39;
//        for(;c>-1;c--){
//          val1 = expectPulse(inPin, HIGH);
//          if(val1<MARK){
//           val1 = expectPulse(inPin, LOW);
//           if(val1==0){
//             DEBUG_PRINT_P("Mark error 0\n");
//             break; 
//           }
//           if(val1<SPACE_ZERO){
//             //0
//             //wh_data[c>>3]|=(1<<(c & B111));
//           }else if(val1<SPACE_ONE){
//             //1
//             wh_data[c>>3]|=(1<<(c & B111));
//           }else{
//             DEBUG_PRINT_P("Mark error 0 (%lu)\n",val1);
//            break; 
//           }
//          }else{
//           DEBUG_PRINT_P("Mark error\n");
//           break; 
//          }  
//        }
//        // Each reading should not take more than 70ms (use time to detect errors)
//        if(millis()-st<READ_TIMEOUT/1000){
//          temp=wh_data[3];
//          level=wh_data[2];
//          //Serial.print(wh_data[3]);Serial.print(" ");Serial.println((wh_data[2]));
//          //Serial.print(wh_data[4],HEX);Serial.print(" ");Serial.print(wh_data[3],HEX);Serial.print(" ");Serial.print(wh_data[2],HEX);Serial.print(" ");Serial.print(wh_data[1],HEX);Serial.print(" ");Serial.println(wh_data[0],HEX);
//          return true;
//        }
//     }
//    }  
//    DEBUG_PRINT_P("Water Sensor disconnected? (pin=%hd, started %lu, end %lu, val1=%u)\n",inPin,st,millis(),val1);
//    return false;
// }
