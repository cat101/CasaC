#ifndef RS485_Link_h
#define RS485_Link_h
#include "Arduino.h"
#include "config.h"
#include "debug.h"
#include "RS485_Conf.h"
#include "crc.h"


void sendMsg (byte targetNode, byte cmd, const byte * data, const byte length, bool closeMsg=true);
void sendMsgPart (const byte * data, const byte length, bool closeMsg);

byte recvMsg (
              byte *data, const byte length, 
              unsigned int timeout = 500, //Time out is in ms
			  byte *prefix=NULL, byte preLen=0,
              byte *overflowData=NULL, const byte overFlowLen=0,
              const byte nodeAddress = RS485UNUSED 
              );

void dumpMsg(byte * data, const byte dataLen);
void emptyRxChannel();

#define RS485_ERR_BADCHAR 0
#define RS485_ERR_CRC 1
#define RS485_ERR_PREFIXNOTMATCH 2
#define RS485_ERR_WRONGNODE 3
#define RS485_ERR_BUFOVERFLOW 4
#define RS485_ERR_TIMEOUT 5
extern byte lastRS485Error;
extern byte lastRS485ProcessedChar;
#endif