// Extracted from http://hack4life.pbworks.com/Arduino%20Solar%20Water%20Heater%20Sensor

extern volatile char tempWH,levelWH; // temp in Celsius and level goes from 0 to 4
void initWaterHeaterSensor(); //Attaches the interrupt handler
