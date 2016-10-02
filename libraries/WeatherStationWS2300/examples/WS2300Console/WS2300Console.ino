/*
  Open2300 source code here http://www.lavrsen.dk/svn/open2300/trunk/rw2300.c
Copy code from http://www.lavrsen.dk/svn/open2300/trunk/rw2300.c and redo. 
On average each function takes 300ms, when it should no take more than 20ms

*/
#include "config.h"
#include "debug.h"

#include "WeatherStationWS2300.h"



WeatherStationWS2300 ws2300(Serial1);

struct WeatherStatus{
  unsigned int tempIndoorC; //Internal temperature in celcious (fixed prescision arithmetic. Divide by 100)
  unsigned int tempOutdoorC; //External temperature in celcious (fixed prescision arithmetic. Divide by 100)
  unsigned char humIndoor; //Internal humidity as a percentage
  unsigned char humOutdoor; //External humidity as a percentage
  unsigned int rainLast1hr; //MMs of rain on the last 1 hour (fixed prescision arithmetic. Divide by 100)
  unsigned int rainLast24hr; //MMs of rain on the last 24 hours (fixed prescision arithmetic. Divide by 100)
  unsigned int windDir;     //instantaneous wind direction in degrees clockwise from North
  unsigned int windSpeed;  //instantaneous wind speed in km/h (fixed prescision arithmetic. Divide by 100)
  unsigned int pressHPa;  //Barometric preassure in hPa (fixed prescision arithmetic. Divide by 10)
  char tendency; //The initial of { "Steady", "Rising", "Falling" }
  char forecast; //The initial of { "Rainy", "Cloudy", "Sunny" }
} ws;

void setup() {

  // initialize serial communication:
#ifdef DEBUG
  initDebugger(115200);
#endif  
  Serial.println("Hello WS2317");
  //Serial.println("http://www.open-electronics.org/how-to-connect-a-weather-station-ws2355-or-ws2300-to-weather-underground-with-arduino/");
  
  // initialize serial communications to the WS2317:
  ws2300.begin();
}

void loop() {

  int incomingByteSer;      // a variable to read incoming serial data into

  // see if there's incoming serial data:
  if (Serial.available() > 0)
  {
    // read the oldest byte in the serial buffer:
    incomingByteSer = Serial.read();
    //Serial.flush();
    
    if (incomingByteSer == 't') 
    {
      Serial.print("t=Indoor Temperature(C) ");
      Serial.println(ws2300.temperature_indoor(0));
    } 

    if (incomingByteSer == 'T') 
    {
      Serial.print("T=Outdoor Temperature(C) ");
      Serial.println(ws2300.temperature_outdoor(0));
    } 

    if (incomingByteSer == 'h') 
    {
      Serial.print("h=Humidity internal ");
      // String data=getHum(1);
      // Serial.println(data);
    } 

    if (incomingByteSer == 'H') 
    {
      Serial.print("H=Humidity external ");
      // String data=getHum(0);
      // Serial.println(data);
    } 

    if (incomingByteSer == 'p') 
    {
      Serial.print("p=Pressure (hPa) ");
      // String data=getPress(1);
      // Serial.println(data);
    } 

    if (incomingByteSer == 'P') 
    {
      Serial.print("P=Pressure (Hg) ");
      // String data=getPress(0);
      // Serial.println(data);
    } 

    if (incomingByteSer == 'w') 
    {
      double winDir;
      Serial.print("w=Wind Speed (Km/H) ");
      Serial.println(ws2300.wind_current(3.6,&winDir));     
      Serial.print("W=Wind Dir (degrees)");
      Serial.println(winDir);
    } 

    if (incomingByteSer == 'r') 
    {
      Serial.print("r=Rain (mm per 1hr) ");
      // Serial.println(getRainMM(1));
    } 

    if (incomingByteSer == 'R') 
    {
      Serial.print("R=Rain (mm per 24hr) ");
      // Serial.println(getRainMM(0));
    }  

    if (incomingByteSer == 'f') 
    {
      Serial.print("Tendency/Forecast ");
      // byte data=getForecast();
      // Serial.println(data,HEX);
    } 

    if (incomingByteSer == 's') 
    {
      String tempS;
      char tempB[10];
      unsigned long ts1=millis();
      Serial.println("Populating Weather Status struct");
      ws.tempIndoorC=(ws2300.temperature_indoor(0)*100);
      Serial.print("Indoor temp (c*100) ");Serial.print(ws.tempIndoorC);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      ws.tempOutdoorC=(ws2300.temperature_outdoor(0)*100); 
      Serial.print("Outdoor temp (c*100) ");Serial.print(ws.tempOutdoorC);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      ws.humIndoor=ws2300.humidity_indoor(); 
      Serial.print("Indoor hum (perc) ");Serial.print(ws.humIndoor);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      ws.humOutdoor=ws2300.humidity_outdoor(); 
      Serial.print("Outdoor hum (perc) ");Serial.print(ws.humOutdoor);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      ws.rainLast1hr=(ws2300.rain_1h(1)*100); 
      Serial.print("MMs of rain on the last 1 hour ");Serial.print(ws.rainLast1hr);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      ws.rainLast24hr=(ws2300.rain_24h(1)*100); 
      Serial.print("MMs of rain on the last 24 hours ");Serial.print(ws.rainLast24hr);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      double winDir;
      ws.windSpeed=(ws2300.wind_current(3.6,&winDir)*100);
      ws.windDir=winDir;
      Serial.print("Wind speed (km/h) ");Serial.print(ws.windSpeed);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      Serial.print("Wind dir (degrees from N clockwise) ");Serial.print(ws.windDir);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      ws.pressHPa=(ws2300.abs_pressure(1)*10); 
      Serial.print("Pressure (hPa) ");Serial.print(ws.pressHPa);Serial.print(" time(ms) ");Serial.println(millis()-ts1);ts1=millis();
      ws2300.tendency_forecast(&ws.tendency, &ws.forecast);
      DEBUG_PRINT_P("Tendency %c, Forecast %c\n",ws.tendency, ws.forecast);
    } 
  }  //serial available!

}
