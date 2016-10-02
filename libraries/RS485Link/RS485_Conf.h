#ifndef RS485Conf_h
#define RS485Conf_h

#define RS485MASTERID 0
#define RS485BROADCASTID 255
#define RS485UNUSED 254 //Used for internal purposes
// This buffer is used on the receive 
#define RS485_COMMBUFSIZE 100 // The EEPROM config can easily get to 50
// #define RS485_ENABLE_TESTS    //This setting controls whether the speed test code gets included
#define RS485_TESTBUFSIZE 180

// Amount ms needed to receive a request or answer through the RS485 network
// 4ms is around 57 bytes at 115kbps, still the average response time for the uC is 2ms
#define RS485_SERIALBAUD 115200
#define RS485_MSGASSEMBLYTIME 15//*10

// #define RS485_SERIALBAUD 28800
// #define RS485_MSGASSEMBLYTIME 15*4

// #define RS485_SERIALBAUD 9600
// #define RS485_MSGASSEMBLYTIME 15*8

// Commands
#define RS485CMD_COLLECT_SENSOR 4
#define RS485CMD_COLLECT_SENSOR_EXTENDED 5
#define RS485CMD_WRITENLOAD_CONFIG 6
#define RS485CMD_TIMED_OUTPUT 7
#define RS485CMD_SET_MODE 8 // Used to set mode bits (isDark, Service, Debug, etc)
#define RS485CMD_SEND_SURROGATE_CMD 9 // Send a command (text string) to a surrogate via a slave
#define RS485CMD_COMM_CHECK 240
// #define RS485CMD_SERVICE_MODE 241 deprecated by RS485CMD_SET_MODE
#define RS485CMD_NODE_BUSY 242

#endif