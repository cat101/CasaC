#ifndef _WEATHERWS2300_H_ 
#define _WEATHERWS2300_H_  

#include <HardwareSerial.h>
#include <Arduino.h>
class WeatherStationWS2300{
private:
  HardwareSerial &serdevice;
  int read_device(unsigned char *buffer, int size);
  int write_device(unsigned char *buffer, int size);
  int read_data(int address, int number, unsigned char *readdata, unsigned char *commanddata);
  int write_data(int address, int number, unsigned char encode_constant, unsigned char *writedata, unsigned char *commanddata);
  int read_safe(int address, int number, unsigned char *readdata, unsigned char *commanddata);
  int write_safe(int address, int number, unsigned char encode_constant, unsigned char *writedata, unsigned char *commanddata);
public:  

  WeatherStationWS2300(HardwareSerial &dev) : serdevice(dev) {};
  void begin(void);
  void reset_06(void);
  double temperature_indoor(int temperature_conv);
  double temperature_outdoor(int temperature_conv);
  int humidity_indoor(void);
  int humidity_outdoor(void);
  double wind_current(double wind_speed_conv_factor, double *winddir);
  double rain_1h(double rain_conv_factor);
  double rain_24h(double rain_conv_factor);
  double abs_pressure(double pressure_conv_factor);
  bool tendency_forecast(char *tendency, char *forecast);

  struct WeatherStatus{
    int tempIndoorC; //Internal temperature in Celsius (fixed precision arithmetic. Divide by 100)
    int tempOutdoorC; //External temperature in Celsius (fixed precision arithmetic. Divide by 100)
    unsigned char humIndoor; //Internal humidity as a percentage
    unsigned char humOutdoor; //External humidity as a percentage
    unsigned int rainLast1hr; //MMs of rain on the last 1 hour (fixed precision arithmetic. Divide by 100)
    unsigned int rainLast24hr; //MMs of rain on the last 24 hours (fixed precision arithmetic. Divide by 100)
    unsigned int windDir;     //instantaneous wind direction in degrees clockwise from North
    unsigned int windSpeed;  //instantaneous wind speed in km/h (fixed precision arithmetic. Divide by 100)
    unsigned int pressHPa;  //Barometric pressure in hPa (fixed precision arithmetic. Divide by 10)
    char tendency; //The initial of { "Steady", "Rising", "Falling" }
    char forecast; //The initial of { "Rainy", "Cloudy", "Sunny" }
  };
  void getStatus(WeatherStatus &ws);
};
#endif