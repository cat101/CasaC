extern volatile char tempWH,levelWH; // temp in Celsius and level goes from 0 to 4
void initWaterHeaterSensor(); //Attaches the interrupt handler

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  initWaterHeaterSensor(); //Attaches the interrupt handler
  Serial.println("Starting");

}

char lastTempWH=0,lastLevelWH=5; // temp in Celsius and level goes from 0 to 4
void loop() {
//  if(lastTempWH!=tempWH || lastLevelWH!=levelWH){
    Serial.print(tempWH,DEC);Serial.print("c - L");Serial.println(levelWH,DEC);
    lastTempWH=tempWH;
    lastLevelWH=levelWH;
//  }
    delay(1000);
}
