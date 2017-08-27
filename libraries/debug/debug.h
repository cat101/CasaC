#ifndef debug_h
#define debug_h
#include <avr/pgmspace.h>
/*
  Sample uses
    DEBUG_PRINT("--> idx %u",idx);
*/


// Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
// -Wno-uninitialized

#ifdef DEBUG
// This is a printf like command
#define DEBUG_PRINT(...) if(debugEnabled){printf(__VA_ARGS__);}
// This is a printf like command that stores the format store in ROM
#define DEBUG_PRINT_P(fmt,...) if(debugEnabled){printf_P(PSTR(fmt),##__VA_ARGS__);}
// This is just to print literals (stored in ROM)
// #define DEBUG_PRINT_PL(x) if(debugEnabled){printf_P(PSTR(x));}
#ifdef DEBUG_RS232_TX_PIN
    #define DEBUGSERIAL softSerial
#else
    #define DEBUGSERIAL Serial
#endif
void serialPrintP(const char *data);
void initDebugger(unsigned long bauds);
extern bool debugEnabled;

#ifdef DEBUG_RS232_TX_PIN
	#include <SoftwareSerial.h>
	extern SoftwareSerial DEBUGSERIAL;
#endif
#define DEBUG_SERIAL(x) DEBUGSERIAL.x

#else

#define DEBUG_PRINT(...)
#define DEBUG_PRINT_P(...)
// #define DEBUG_PRINT_PL(x)
#define DEBUG_SERIAL(x)


#endif

// The debug_b functions are used to "park" debugging code in the background
#ifdef DEBUG_B
#define DEBUG_B_PRINT(...) if(debugEnabled){printf(__VA_ARGS__);}
#define DEBUG_B_PRINT_P(fmt,...) if(debugEnabled){printf_P(PSTR(fmt),##__VA_ARGS__);}
// #define DEBUG_B_PRINT_P(x) if(debugEnabled){printf_P(PSTR(x));}
#define DEBUG_B_SERIAL(x) DEBUGSERIAL.x
#else
#define DEBUG_B_PRINT(...)
#define DEBUG_B_PRINT_P(...)
// #define DEBUG_B_PRINT_PL(x)
#define DEBUG_B_SERIAL(x)
#endif

// The debug_br functions are used for "parked" info the prints recurrently (i.e noisy)
#ifdef DEBUG_BR
#define DEBUG_BR_PRINT(...) if(debugEnabled){printf(__VA_ARGS__);}
#define DEBUG_BR_PRINT_P(fmt,...) if(debugEnabled){printf_P(PSTR(fmt),##__VA_ARGS__);}
// #define DEBUG_BR_PRINT_P(x) if(debugEnabled){printf_P(PSTR(x));}
#define DEBUG_BR_SERIAL(x) DEBUGSERIAL.x
#else
#define DEBUG_BR_PRINT(...)
#define DEBUG_BR_PRINT_P(...)
// #define DEBUG_BR_PRINT_PL(x)
#define DEBUG_BR_SERIAL(x)
#endif

int availableMemory(); 
#define EXCEPTION_BUFF 40  //This is the max size for exception text on the master. At the slaves it gets capped at 16
extern uint32_t lastException;
extern char exceptionText[EXCEPTION_BUFF];

// Log exception prints to seril and stores the last exception in RAM together with a timestamp. 
// On the master it also logs an event to SD
#define logException_P(fmt,...) logException(PSTR(fmt),##__VA_ARGS__)
void logException(PGM_P fmt, ... );

// Log an event to the SD and print to the serial output
#define logEvent_P(fmt,...) eventLogger.log_P(PSTR(fmt),##__VA_ARGS__) 

#endif

