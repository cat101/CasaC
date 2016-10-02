#define D_MODE_AUTO B000 //= Fully Automatic (temp 25)
#define D_MODE_COOL B011 //= Cool
#define D_MODE_HEAT B100 //= Heat (temp 23)
#define D_MODE_DRY  B010 //= DRY (temp 0xc0 = 96 degrees c)
#define D_MODE_FAN  B110 //= FAN (temp not shown, but 25)
#define D_FAN_1BAR   0x3 // = 1 bar
#define D_FAN_2BAR   0x4 // = 2 bar
#define D_FAN_3BAR   0x5 // = 3 bar
#define D_FAN_4BAR   0x6 // = 4 bar
#define D_FAN_5BAR   0x7 // = 5 bar
#define D_FAN_AUTO   0xa // = Auto
#define D_FAN_NIGHT  0xb // = Not auto, moon + tree
#define D_AUX_NORM   0
#define D_AUX_POWER  1   //This may be quiet...
#define D_AUX_QUIET  16
void airController_on();
void airController_off();
void airController_setAux(uint8_t aux);
void airController_setTemp(uint8_t temp);
void airController_setFan(uint8_t fan);
void airController_setMode(uint8_t mode);
void airController_setSwing(bool horizontal, bool vertical);
void airController_setIEyeOn();
void airController_send();



