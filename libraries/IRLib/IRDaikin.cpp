/*
	Daikin AC map (https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/blob/master/Daikin-AC-remote.pde & https://github.com/lowflyerUK/aircon-raspi-xrf/blob/master/PIC/main.c)
	byte 7= checksum of the first part (and last byte before a 29ms pause)
	byte 13=mode
		b7 = 0
		b6+b5+b4 = Mode
			Modes: b6+b5+b4
			011 = Cool
			100 = Heat (temp 23)
			110 = FAN (temp not shown, but 25)
			000 = Fully Automatic (temp 25)
			010 = DRY (temp 0xc0 = 96 degrees c)
		b3 = 0
		b2 = OFF timer set
		b1 = ON timer set
		b0 = Air Conditioner ON
	byte 14=temp*2   (Temp should be between 18 - 32)
	byte 16=Fan
		FAN control
		b7+b6+b5+b4 = Fan speed
			Fan: b7+b6+b5+b4
			0×30 = 1 bar
			0×40 = 2 bar
			0×50 = 3 bar
			0×60 = 4 bar
			0×70 = 5 bar
			0xa0 = Auto
			0xb0 = Not auto, moon + tree (aka fan night)
		b3+b2+b1+b0 = Swing control up/down
			Swing control up/down:
			0000 = Swing up/down off
			1111 = Swing up/down on
	byte 17
			Swing control left/right:
			0000 = Swing left/right off
			1111 = Swing left/right on
	byte 21=Aux  -> Powerful (bit 1), Silent (bit 5)
	byte 24=Aux2 -> Intelligent eye on (bit 1)
	byte 26= checksum of the second part 
*/
#include <IRLib.h>

// # of bytes per command
#define COMMAND_LENGTH 27
unsigned char daikin[COMMAND_LENGTH]={ 
0x11,0xDA,0x27,0xF0,0x00,0x00,0x00,0x02,    //Preamble (64 bits + 1 stop bit)
//0    1    2   3    4    5     6   7
0x11,0xDA,0x27,0x00,0x00,0x41,0x1E,0x00,	//Payload (152 bits + 1 stop bit)
//8    9   10   11   12    13   14   15
0xB0,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0xE1 };
//16  17    18  19   20    21   22  23   24   25   26

uint8_t airController_checksum()
{
	uint8_t sum = 0;
	uint8_t i;
	for(i = 0; i <= 6; i++){
		sum += daikin[i];
	}
    daikin[7] = sum & 0xFF;
    sum=0;
	for(i = 8; i <= 25; i++){
		sum += daikin[i];
    }
    daikin[26] = sum & 0xFF;
}

void airController_on(){
	//state = ON;
	daikin[13] |= 0x01;
}

void airController_off(){
	//state = OFF;
	daikin[13] &= 0xFE;
}

void airController_setAux(uint8_t aux){
	//Set aux functions (Powerful, Silent, Normal operation)
	daikin[21] = aux;
}


void airController_setTemp(uint8_t temp)
{
	daikin[14] = temp<<1;
}


void airController_setFan(uint8_t fan)
{
	daikin[16] = fan<<4 | (daikin[16] & 0x0F);
}

void airController_setIEyeOn()
{
	daikin[24] = 0x02;
}

void airController_setMode(uint8_t mode)
{
	daikin[13]=mode<<4 | ((daikin[13])&0x01);
}
void airController_setSwing(bool horizontal, bool vertical)
{
	if(vertical)		
		daikin[16]=((daikin[16])&0xF0) | B1111;
	else
		daikin[16]=((daikin[16])&0xF0);
	if(horizontal)		
		daikin[17]=B1111;
	else
		daikin[17]=0;
}

IRsendDaikin IRDaikinSender;

void airController_send(){
  	airController_checksum();  
	IRDaikinSender.send(daikin, 8,0); // Data, Len, Start
	delay(29);
	IRDaikinSender.send(daikin, 19,8); 
}


