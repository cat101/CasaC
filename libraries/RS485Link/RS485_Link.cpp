/*
RS485 protocol library.

Devised and written by Nick Gammon.
Date: 14 November 2011
Version: 1.1

Version 1.1 reset the timeout period after getting STX.

Licence: Released for public use.


Can send from 1 to 255 bytes from one node to another with:

* Packet start indicator (STX)
* Each data byte is doubled and inverted to check validity
* Packet end indicator (ETX)
* Packet CRC (checksum)


To allow flexibility with hardware (eg. Serial, SoftwareSerial, I2C)
you provide three "callback" functions which send or receive data. Examples are:

void fWrite (const byte what)
{
Serial.write (what);  
}

int fAvailable ()
{
return Serial.available ();  
}

int fRead ()
{
return Serial.read ();  
}

*/


#include "RS485_Link.h"

const byte STX = '\2';
const byte ETX = '\3';
byte lastRS485Error=0; //Store the last error code
byte lastRS485ProcessedChar=0;

// send a byte complemented, repeated
// only values sent would be (in hex): 
//   0F, 1E, 2D, 3C, 4B, 5A, 69, 78, 87, 96, A5, B4, C3, D2, E1, F0
void sendComplemented (const byte what)
{
	byte c;

	// first nibble
	c = what >> 4;
	RS485SERIAL.write ((c << 4) | (c ^ 0x0F)); 

	// second nibble
	c = what & 0x0F;
	RS485SERIAL.write ((c << 4) | (c ^ 0x0F)); 

}  // end of sendComplemented

// send a message of "length" bytes (max 255) to other end
// put STX at start, ETX at end, and add CRC
byte crc;
void sendMsg (byte targetNode, byte cmd, const byte * data, const byte length, bool closeMsg)
{
	// Packet: STX, TargetNode, Command (255 means fail or NA)
	digitalWrite (RS485_ENABLE_PIN, HIGH);  // enable sending
	// delayMicroseconds(500);  Makes no diference
	RS485SERIAL.write (STX);  // STX
	sendComplemented (targetNode);crc=crc8 (&targetNode, 1);
	sendComplemented (cmd);crc=crc8 (&cmd, 1, crc);
	sendMsgPart (data, length, closeMsg);
}  // end of sendMsg

void sendMsgPart (const byte * data, const byte length, bool closeMsg)
{
	for (byte i = 0; i < length; i++)
		sendComplemented (data [i]);
	crc=crc8 (data, length, crc);
	if(closeMsg){
		RS485SERIAL.write (ETX);  // ETX
		sendComplemented (crc);
		RS485SERIAL.flush(); // Wait for all the bytes to leave the buffer (this is a blocking call until the ISR/UART finishes sending)
		// delayMicroseconds(500);
		digitalWrite (RS485_ENABLE_PIN, LOW);  // disable sending
		DEBUG_BR_PRINT_P("Sent msg (payload size %hu, crc %hu)\n",length,crc);
	}  
}


// receive a message, maximum "length" bytes, timeout after "timeout" milliseconds
// if nothing received, or an error (eg. bad CRC, bad data) return 0
// otherwise, returns length of received data
byte recvMsg (   
	byte * data,            // buffer to receive into
	const byte length,      // maximum buffer size  (this does not include pre or overflow buffers)
	unsigned int timeout,   // milliseconds before timing out
	byte *prefix,			// The first part of the incoming message will be compared against prefix. It there is no match the function 
							// returns 0. If there is a match the first byte is set to 234 (this is used when the expected content length is zero)
							// This functionality should not be used since the remaining part of the message will be left on the buffer and will
							// make subsequent calls fail until all the bytes have been consumed (too costly)
	byte preLen,
	byte *overflowData, const byte overFlowLen,
	const byte nodeAddress  // if nodeAddress!=RS485UNUSED this function will look at the first received byte and return immediately if it 
							// does not match the node ID or broadcast
)
{              
	unsigned long start_time = millis ();
	bool have_stx = false;
	// variables below are set when we get an STX
	bool have_etx=false;
	bool prefix_matched=false;
	byte input_pos=0;
	byte overflow_pos=0;
	byte prefix_pos=0;
	bool first_nibble=false;
	byte current_byte=0;
	// If timeout is zero then the loop will run most likely one time
	while (millis () - start_time < timeout)
	{
		if(RS485SERIAL.available()){
			byte inByte = RS485SERIAL.read ();
			switch (inByte)
			{
				case STX:   // start of text
					have_stx = true;
					have_etx = false;
					input_pos = 0;
					first_nibble = true;
					start_time = millis ();  // reset timeout period
				break;
					
				case ETX:   // end of text
					have_etx = true;   
				break;
					
				default:
					// wait until packet officially starts
					if (!have_stx)
						break;   
					
					// check byte is in valid form (4 bits followed by 4 bits complemented)
					if ((inByte >> 4) != ((inByte & 0x0F) ^ 0x0F) ){
						#ifndef DISABLE_COMM_ERRORS
							DEBUG_B_PRINT_P("Got a Bad character (pos %u, char 0x%02hx, target node %hu)\n",input_pos+overflow_pos,inByte, (preLen>0)?prefix[0]:data[0]);
							logException_P("Bad character");
						#endif
						lastRS485Error=RS485_ERR_BADCHAR;
						lastRS485ProcessedChar=input_pos+overflow_pos;
						return 0;  // bad character
					}
					
					// convert back 
					inByte >>= 4;
					
					// high-order nibble?
					if (first_nibble){
						current_byte = inByte;
						first_nibble = false;
						break;
					}  // end of first nibble
					
					// low-order nibble
					current_byte <<= 4;
					current_byte |= inByte;
					first_nibble = true;
					
					// if we have the ETX this must be the CRC
					if (have_etx){
						//DEBUG_B_PRINT("CRC check (pos %u)\n",input_pos);
						crc=crc8 (prefix, preLen);
						crc=crc8 (data, input_pos, crc);
						if(overflow_pos)
							crc=crc8 (overflowData, overflow_pos, crc);
						if (crc != current_byte){
							DEBUG_B_PRINT_P("Bad CRC check (pos %hu, preLen %hu, target node %hu, computed crc %hu, received crc %hu)\n"
								, input_pos+overflow_pos
								, preLen
								, (preLen>0)?prefix[0]:data[0]
								, crc, current_byte
								);
							logException_P("Bad CRC check");
							lastRS485Error=RS485_ERR_CRC;
							lastRS485ProcessedChar=input_pos+overflow_pos;
							return 0;  // bad crc  
						}
						if(prefix_matched)
							prefix[0]=234; //This value is used to indicate that the prefix matched
						DEBUG_BR_PRINT_P("Got a complete message (pos %u, target node %hu)\n",input_pos+overflow_pos, (preLen>0)?prefix[0]:data[0]);
						return input_pos+overflow_pos;  // return received length
					}  // end if have ETX already
					
					// Match the prefix
					if (prefix_pos<preLen){
						// DEBUG_BR_PRINT_P("recvMsg: Matching prefix %u %u (%u)\n",prefix[prefix_pos],current_byte,prefix_pos);
						if(prefix[prefix_pos++]!=current_byte){
							logException_P("Prefix did not match");
							lastRS485Error=RS485_ERR_PREFIXNOTMATCH;
							lastRS485ProcessedChar=input_pos+overflow_pos;
							return 0;  // the prefix did not match
						}
						if(prefix_pos==preLen){
							DEBUG_BR_PRINT_P("recvMsg: Prefix matched (size %u)\n", preLen);
							prefix_matched=true;
						}
						break;
					}

					// keep adding if not full
					if (input_pos < length){
						//DEBUG_PRINT("input_pos %u len %u\n",input_pos, length);
						if(input_pos==0){
							// DEBUG_PRINT_P("Msg to %hu\n", current_byte);
							if(nodeAddress!=RS485UNUSED)
								if(current_byte!=nodeAddress && current_byte!=RS485BROADCASTID){
									lastRS485Error=RS485_ERR_WRONGNODE;
									lastRS485ProcessedChar=input_pos+overflow_pos;
									return 0; //This package is not addressed for this node					
								}
						}
						data [input_pos++] = current_byte;
					}else if(overflowData!=NULL && overflow_pos < overFlowLen){
						overflowData [overflow_pos++] = current_byte;
					}else{
						logException_P("Msg overflow");
						lastRS485Error=RS485_ERR_BUFOVERFLOW;
						lastRS485ProcessedChar=input_pos+overflow_pos;
						return 0;  // overflow
					}
				break;
			}  // end of switch
		}	
	} // end of while not timed out
	if(have_stx){
		// I was in the middle of a message
		DEBUG_B_PRINT_P("Timed out while receiving msg (got %hu, timeout %u)\n",input_pos+overflow_pos,timeout);
	}else{
		DEBUG_BR_PRINT_P("Timed out while waiting for message to start (got %hu, timeout %u)\n",input_pos+overflow_pos,timeout);
	}
	// if(timeout>100){
	// 	DEBUG_BR_PRINT_P("Timed out while receiving msg (got %hu, timeout %u)\n",input_pos+overflow_pos,timeout);
	// }
	#ifndef DEBUG //Only use this exception for production code
	logException_P("Timed out while receiving msg");
	#endif
	lastRS485Error=RS485_ERR_TIMEOUT;
	lastRS485ProcessedChar=input_pos+overflow_pos;
	return 0;  // timeout
} // end of recvMsg

#if RS485NODEID == 0
	#include "logger.h"
	void dumpMsg(byte * data, const byte dataLen){              
		const char * hex = "0123456789ABCDEF";
		eventLogger.openFile();
		byte c;
		eventLogger.write(hex[(dataLen>>4) & 0xF]);
		eventLogger.write(hex[ dataLen     & 0xF]);
		eventLogger.write(':');
		for(c=0;c<dataLen;c++){
			eventLogger.write(hex[(data[c]>>4) & 0xF]);
			eventLogger.write(hex[ data[c]     & 0xF]);
			eventLogger.write(' ');
		}
		eventLogger.write('\n');
	}
#endif