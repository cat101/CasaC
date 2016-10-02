#include "Arduino.h"

// calculate 8-bit CRC
// This code seems to be the one for CRC-8-Dallas/Maxim (x8 + x5 + x4 + 1)
// https://en.wikipedia.org/wiki/Polynomial_representations_of_cyclic_redundancy_checks
// The implementation comes from the 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
// Online calculator http://www.datastat.com/sysadminjournal/maximcrc.cgi
byte crc8 (const byte *addr, byte len, byte crc)
{
	while (len--) 
	{
		byte inbyte = *addr++;
		for (byte i = 8; i; i--)
		{
			byte mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) 
			crc ^= 0x8C;
			inbyte >>= 1;
		}  // end of for
	}  // end of while
	return crc;
}  // end of crc8