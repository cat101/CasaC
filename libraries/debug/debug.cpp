#include <avr/pgmspace.h>
#include "Arduino.h"
#include "config.h"
#include "debug.h"

// we need fundamental FILE definitions and printf declarations
#include <stdio.h>
#ifdef DEBUG
#ifdef DEBUG_RS232_TX_PIN
	#include <SoftwareSerial.h>
	SoftwareSerial DEBUGSERIAL (DEBUG_RS232_RX_PIN, DEBUG_RS232_TX_PIN);  // receive pin, transmit pin
#endif
// #include <RTClib.h>
// extern RTC_DS1307 RTC;

bool debugEnabled=true;

// create a FILE structure to reference our UART output function
static FILE debug_uartout = {0} ;
static int debug_uart_putchar (char c, FILE *stream)
{
    DEBUGSERIAL.write(c) ;
    return 0 ;
}

void initDebugger(unsigned long bauds)
{
   // Start the UART
   DEBUGSERIAL.begin(bauds) ;

   // fill in the UART file descriptor with pointer to writer.
   fdev_setup_stream (&debug_uartout, debug_uart_putchar, NULL, _FDEV_SETUP_WRITE);

   // The uart is the standard output device STDOUT.
   stdout = &debug_uartout ;
}


// void serialPrintP(const char *data)
// {
//   while ( pgm_read_byte ( data ) != 0 )
//     DEBUGSERIAL.write(pgm_read_byte ( data ++));
//   DEBUGSERIAL.write('\n');
// }

// uint8_t * heapptr, * stackptr;
// int availableMemory() {
//   stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
//   heapptr = stackptr;                     // save value of heap pointer
//   free(stackptr);      // free up the memory again (sets stackptr to 0)
//   stackptr =  (uint8_t *)(SP);           // save value of stack pointer
//   return 2048-((int) heapptr);
// }
extern int __heap_start, *__brkval;
int availableMemory() {
  // Read http://blog.wickeddevice.com/?p=359
  DEBUG_PRINT_P("0 %d %d %d %d (0, BSS, Heap start, Heap end, Stack pointer, Total RAM)\n"
    ,(int) __heap_start
    ,(int) __brkval
    ,(int) SP
    , RAMEND);
  return 0;
}


#endif /*This is the optional code neded for debuging*/

#if RS485NODEID == 0
  extern uint32_t getUnixTime(void);
  #include "logger.h"
#endif
uint32_t lastException=0;
char exceptionText[EXCEPTION_BUFF];

void logException(PGM_P fmt, ... )
{
// #ifdef DISABLE_COMM_ERRORS This is broken
//   // EXCEPTION: 1410212176, Node 7:Bad character
//   // EXCEPTION: 1410212177, Bad character
//   if(exceptionText[7+0]=='B' && exceptionText[7+4]=='c') //This matches the first case
//     return;
// #endif
#if RS485NODEID == 0
  lastException=getUnixTime();
#else
  lastException=millis();
#endif
  va_list args;
  va_start (args, fmt );
  int c=vsnprintf_P(exceptionText, EXCEPTION_BUFF, fmt, args);
  va_end (args);
#if RS485NODEID != 0
  // Only 16 bytes can be sent to master
  if(c>16){
    DEBUG_PRINT_P("Exception text too long to be sent to master %d\n",c);
  }
  lastException=millis();
#endif
  DEBUG_PRINT_P("EXCEPTION: %lu, %s\n",lastException,exceptionText);
#if RS485NODEID == 0 && !defined(DISABLE_EXCEPTIONS_TO_SD) // On the master we log exceptions to the SD card
  //the event logger also prints to the serial console so we temporarly disable it to prevent double printing
  // if(debugEnabled){
  //   debugEnabled=false;
  //   eventLogger.log(exceptionText);  
  //   debugEnabled=true;
  // }else{
    eventLogger.log_NS(exceptionText);      
  // }
#endif
}