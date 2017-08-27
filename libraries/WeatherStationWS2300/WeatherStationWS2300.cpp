#include "WeatherStationWS2300.h"
#include "config.h"
#include "debug.h"

// #define MAXRETRIES          1//50
// #define MAXWINDRETRIES      1//20
//#define WRITENIB            0x42
#define SETBIT              0x12
#define UNSETBIT            0x32
#define WRITEACK            0x10
#define SETACK              0x04
#define UNSETACK            0x0C
//#define RESET_MIN           0x01
//#define RESET_MAX           0x02




/********************************************************************
 * open_weatherstation, Linux version
 *
 * Input:   devicename (/dev/tty0, /dev/tty1 etc)
 * 
 * Returns: Handle to the weatherstation (type WEATHERSTATION)
 *
 ********************************************************************/
void WeatherStationWS2300::begin(void)
{
  serdevice.begin(2400);
  //serdevice.setTimeout(100); //For read operations in ms
  serdevice.setTimeout(20); //For read operations in ms
  return;
}

// extern void p1loop();
// void yieldFor(unsigned int ms){
//   unsigned long t=millis();
//   while((millis()-t)<ms){
//     //p1loop();
//   } 
// }


/********************************************************************
 * reset_06 WS2300 by sending command 06 (Linux version)
 * 
 * Input:   device number of the already open serial port
 *           
 * Returns: nothing, exits progrsm if failing to reset
 *
 ********************************************************************/
void WeatherStationWS2300::reset_06(void)
{
  unsigned char command = 0x06;
  unsigned char answer;
  // int i=0;

  //for (i = 0; i < 100; i++)
  {

    // Discard any garbage in the input buffer
    serdevice.flush();

    write_device(&command, 1);

    // Occasionally 0, then 2 is returned.  If zero comes back, continue
    // reading as this is more efficient than sending an out-of sync
    // reset and letting the data reads restore synchronization.
    // Occasionally, multiple 2's are returned.  Read with a fast timeout
    // until all data is exhausted, if we got a two back at all, we
    // consider it a success
    
    while (1 == read_device(&answer, 1))
    {
      if (answer == 2)
      {
        return;
      }
    }

    // delay(50 * i);   //we sleep longer and longer for each retry
  }
  DEBUG_PRINT_P("Could not reset\n");
  // Serial.println("Could not reset");  
}

/********************************************************************
 * read_device in the Linux version is identical
 * to the standard Linux read() 
 *
 * Inputs:  serdevice - opened file handle
 *          buffer - pointer to the buffer to read into (unsigned char)
 *          size - number of bytes to read
 *
 * Output:  *buffer - modified on success (pointer to unsigned char)
 * 
 * Returns: number of bytes read
 *
 ********************************************************************/
int WeatherStationWS2300::read_device(unsigned char *buffer, int size)
{
  // unsigned long timeout=millis()+1000;
  // int c=0;
  // while(c<size && timeout>millis()){
  //   int ret=serdevice.read();
  //   if(ret!=-1){
  //     buffer[c++]=ret;
  //   }
  // }
  // if(c!=size)
  //    DEBUG_PRINT_P("Read failed (ask %d got %d)\n",size,c);
  // return c;
  int ret=serdevice.readBytes((char *)buffer, size); // read chars from stream into buffer
  // if(ret!=size)
  //    DEBUG_PRINT_P("Read failed (ask %d got %d)\n",size,ret);
  //p1loop();
  return ret;
}

/********************************************************************
 * write_device in the Linux version is identical
 * to the standard Linux write()
 *
 * Inputs:  serdevice - opened file handle
 *          buffer - pointer to the buffer to write from
 *          size - number of bytes to write
 *
 * Returns: number of bytes written
 *
 ********************************************************************/
int WeatherStationWS2300::write_device(unsigned char *buffer, int size)
{  
  int ret=serdevice.write(buffer, size);
  delay(18*ret);  //This delay was obtained empirically...it does not really match the 3ms per byte expected at 2400bps
  return ret;
}

 /********************************************************************
 * address_encoder converts an 16 bit address to the form needed
 * by the WS-2300 when sending commands.
 *
 * Input:   address_in (interger - 16 bit)
 * 
 * Output:  address_out - Pointer to an unsigned character array.
 *          3 bytes, not zero terminated.
 * 
 * Returns: Nothing.
 *
 ********************************************************************/
void address_encoder(int address_in, unsigned char *address_out)
{
  int i = 0;
  int adrbytes = 4;
  unsigned char nibble;

  for (i = 0; i < adrbytes; i++)
  {
    nibble = (address_in >> (4 * (3 - i))) & 0x0F;
    address_out[i] = (unsigned char) (0x82 + (nibble * 4));
  }

  return;
}


/********************************************************************
 * data_encoder converts up to 15 data bytes to the form needed
 * by the WS-2300 when sending write commands.
 *
 * Input:   number - number of databytes (integer)
 *          encode_constant - unsigned char
 *                            0x12=set bit, 0x32=unset bit, 0x42=write nibble
 *          data_in - char array with up to 15 hex values
 * 
 * Output:  address_out - Pointer to an unsigned character array.
 * 
 * Returns: Nothing.
 *
 ********************************************************************/
void data_encoder(int number, unsigned char encode_constant,
                  unsigned char *data_in, unsigned char *data_out)
{
  int i = 0;

  for (i = 0; i < number; i++)
  {
    data_out[i] = (unsigned char) (encode_constant + (data_in[i] * 4));
  }

  return;
}


/********************************************************************
 * numberof_encoder converts the number of bytes we want to read
 * to the form needed by the WS-2300 when sending commands.
 *
 * Input:   number interger, max value 15
 * 
 * Returns: unsigned char which is the coded number of bytes
 *
 ********************************************************************/
unsigned char numberof_encoder(int number)
{
  int coded_number;

  coded_number = (unsigned char) (0xC2 + number * 4);
  if (coded_number > 0xfe)
    coded_number = 0xfe;

  return coded_number;
}


/********************************************************************
 * command_check0123 calculates the checksum for the first 4
 * commands sent to WS2300.
 *
 * Input:   pointer to char to check
 *          sequence of command - i.e. 0, 1, 2 or 3.
 * 
 * Returns: calculated checksum as unsigned char
 *
 ********************************************************************/
unsigned char command_check0123(unsigned char *command, int sequence)
{
  int response;

  response = sequence * 16 + ((*command) - 0x82) / 4;

  return (unsigned char) response;
}


/********************************************************************
 * command_check4 calculates the checksum for the last command
 * which is sent just before data is received from WS2300
 *
 * Input: number of bytes requested
 * 
 * Returns: expected response from requesting number of bytes
 *
 ********************************************************************/
unsigned char command_check4(int number)
{
  int response;

  response = 0x30 + number;

  return response;
}


/********************************************************************
 * data_checksum calculates the checksum for the data bytes received
 * from the WS2300
 *
 * Input:   pointer to array of data to check
 *          number of bytes in array
 * 
 * Returns: calculated checksum as unsigned char
 *
 ********************************************************************/
unsigned char data_checksum(unsigned char *data, int number)
{
  int checksum = 0;
  int i;

  for (i = 0; i < number; i++)
  {
    checksum += data[i];
  }

  checksum &= 0xFF;

  return (unsigned char) checksum;
}


/********************************************************************
 * read_data reads data from the WS2300 based on a given address,
 * number of data read, and a an already open serial port
 *
 * Inputs:  serdevice - device number of the already open serial port
 *          address (interger - 16 bit)
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 *          commanddata - pointer to an array of chars containing
 *                     the commands that were sent to the station
 * 
 * Returns: number of bytes read, -1 if failed
 *
 ********************************************************************/
int WeatherStationWS2300::read_data(int address, int number,
        unsigned char *readdata, unsigned char *commanddata)
{
  // unsigned long ts1=millis();
  unsigned char answer;
  int i;

  // First 4 bytes are populated with converted address range 0000-13B0
  address_encoder(address, commanddata);
  // Last populate the 5th byte with the converted number of bytes
  commanddata[4] = numberof_encoder(number);

  // DEBUG_PRINT_P("Sending address\n");
  for (i = 0; i < 4; i++)
  {
    if (write_device(commanddata + i, 1) != 1)
      return -1;
    if (read_device(&answer, 1) != 1)
      return -1;
    if (answer != command_check0123(commanddata + i, i))
      return -1;
  }

  // DEBUG_PRINT_P("Sending no bytes\n");
  //Send the final command that asks for 'number' of bytes, check answer
  if (write_device(commanddata + 4, 1) != 1)
    return -1;
  // Serial.print("Send cmd time ");Serial.println(millis()-ts1);

  //  Esta primer parte lleva 90ms... poner un callback a loopp1 y prevenir reentrancia poniendo el polling de la weather en algo como P3
  //p1loop(); // YIELD to the main loop =============================================================

  // ts1=millis();
  if (read_device(&answer, 1) != 1)
    return -1;
  if (answer != command_check4(number))
    return -1;

  // DEBUG_PRINT_P("Geting data\n");
  //Read the data bytes
  for (i = 0; i < number; i++)
  {
    if (read_device(readdata + i, 1) != 1)
      return -1;
  }

  // DEBUG_PRINT_P("Verifying checksum\n");
  //Read and verify checksum
  if (read_device(&answer, 1) != 1)
    return -1;
  if (answer != data_checksum(readdata, number))
    return -1;
  // Serial.print("Get data time ");Serial.println(millis()-ts1);  
  return i;

}


/********************************************************************
 * write_data writes data to the WS2300.
 * It can both write nibbles and set/unset bits
 *
 * Inputs:      ws2300 - device number of the already open serial port
 *              address (interger - 16 bit)
 *              number - number of nibbles to be written/changed
 *                       must 1 for bit modes (SETBIT and UNSETBIT)
 *                       max 80 for nibble mode (WRITENIB)
 *              encode_constant - unsigned char
 *                                (SETBIT, UNSETBIT or WRITENIB)
 *              writedata - pointer to an array of chars containing
 *                          data to write, not zero terminated
 *                          data must be in hex - one digit per byte
 *                          If bit mode value must be 0-3 and only
 *                          the first byte can be used.
 * 
 * Output:      commanddata - pointer to an array of chars containing
 *                            the commands that were sent to the station
 *
 * Returns:     number of bytes written, -1 if failed
 *
 ********************************************************************/
int WeatherStationWS2300::write_data(int address, int number,
         unsigned char encode_constant, unsigned char *writedata,
         unsigned char *commanddata)
{
  unsigned char answer;
  unsigned char encoded_data[80];
  int i = 0;
  unsigned char ack_constant = WRITEACK;
  
  if (encode_constant == SETBIT)
  {
    ack_constant = SETACK;
  }
  else if (encode_constant == UNSETBIT)
  {
    ack_constant = UNSETACK;
  }

  // First 4 bytes are populated with converted address range 0000-13XX
  address_encoder(address, commanddata);
  // populate the encoded_data array
  data_encoder(number, encode_constant, writedata, encoded_data);

  //Write the 4 address bytes
  for (i = 0; i < 4; i++)
  {
    if (write_device(commanddata + i, 1) != 1)
      return -1;
    if (read_device(&answer, 1) != 1)
      return -1;
    if (answer != command_check0123(commanddata + i, i))
      return -1;
  }

  //Write the data nibbles or set/unset the bits
  for (i = 0; i < number; i++)
  {
    if (write_device(encoded_data + i, 1) != 1)
      return -1;
    if (read_device(&answer, 1) != 1)
      return -1;
    if (answer != (writedata[i] + ack_constant))
      return -1;
    commanddata[i + 4] = encoded_data[i];
  }

  return i;
}


/********************************************************************
 * read_safe Read data, retry until success or maxretries
 * Reads data from the WS2300 based on a given address,
 * number of data read, and a an already open serial port
 * Uses the read_data function and has same interface
 *
 * Inputs:  ws2300 - device number of the already open serial port
 *          address (interger - 16 bit)
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 *          commanddata - pointer to an array of chars containing
 *                     the commands that were sent to the station
 * 
 * Returns: number of bytes read, -1 if failed
 *
 ********************************************************************/
int WeatherStationWS2300::read_safe(int address, int number,
        unsigned char *readdata, unsigned char *commanddata)
{
  // DEBUG_PRINT_P("Reseting\n");
  // reset_06();
  
  // Read the data. If expected number of bytes read break out of loop.
  // DEBUG_PRINT_P("Reading\n");
  if (read_data(address, number, readdata, commanddata)==number)
  {
    return number;
  }
  return -1;
}


/********************************************************************
 * write_safe Write data, retry until success or maxretries
 * Writes data to the WS2300 based on a given address,
 * number of data to write, and a an already open serial port
 * Uses the write_data function and has same interface
 *
 * Inputs:      serdevice - device number of the already open serial port
 *              address (interger - 16 bit)
 *              number - number of nibbles to be written/changed
 *                       must 1 for bit modes (SETBIT and UNSETBIT)
 *                       unlimited for nibble mode (WRITENIB)
 *              encode_constant - unsigned char
 *                               (SETBIT, UNSETBIT or WRITENIB)
 *              writedata - pointer to an array of chars containing
 *                          data to write, not zero terminated
 *                          data must be in hex - one digit per byte
 *                          If bit mode value must be 0-3 and only
 *                          the first byte can be used.
 * 
 * Output:      commanddata - pointer to an array of chars containing
 *                            the commands that were sent to the station
 * 
 * Returns: number of bytes written, -1 if failed
 *
 ********************************************************************/
int WeatherStationWS2300::write_safe(int address, int number,
               unsigned char encode_constant, unsigned char *writedata,
               unsigned char *commanddata)
{
  reset_06();

  // Read the data. If expected number of bytes read break out of loop.
  if (write_data(address, number, encode_constant, writedata,
      commanddata)==number)
  {
    return number;
  }

  return -1;

}


/********************************************************************/
/* temperature_indoor
 * Read indoor temperature, current temperature only
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Returns: Temperature (deg C if temperature_conv is 0)
 *                      (deg F if temperature_conv is 1)
 *
 ********************************************************************/
double WeatherStationWS2300::temperature_indoor(int temperature_conv)
{
  unsigned char data[20];
  unsigned char command[25];  //room for write data also
  int address=0x346;
  int bytes=2;

  if (read_safe(address, bytes, data, command) != bytes)
    return -1;

  if (temperature_conv)
    return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
              (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) -
              30.0) * 9 / 5 + 32);
  else
    return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
              (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0));
}

/********************************************************************/
/* temperature_outdoor
 * Read indoor temperature, current temperature only
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Returns: Temperature (deg C if temperature_conv is 0)
 *                      (deg F if temperature_conv is 1)
 *
 ********************************************************************/
double WeatherStationWS2300::temperature_outdoor(int temperature_conv)
{
  unsigned char data[20];
  unsigned char command[25];  //room for write data also
  int address=0x373;
  int bytes=2;

  if (read_safe(address, bytes, data, command) != bytes)
    return -1;

  if (temperature_conv)
    return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
              (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) -
              30.0) * 9 / 5 + 32);
  else
    return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
              (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0));
}
/********************************************************************
 * humidity_indoor
 * Read indoor relative humidity, current value only
 * 
 * Input: Handle to weatherstation
 * Returns: relative humidity in percent (integer)
 * 
 ********************************************************************/
int WeatherStationWS2300::humidity_indoor(void)
{
  unsigned char data[20];
  unsigned char command[25];  //room for write data also
  int address=0x3FB;
  int bytes=1;

  if (read_safe(address, bytes, data, command) != bytes)
    return -1;
  return ((data[0] >> 4) * 10 + (data[0] & 0xF));
}

/********************************************************************
 * humidity_outdoor
 * Read relative humidity, current value only
 * 
 * Input: Handle to weatherstation
 * Returns: relative humidity in percent (integer)
 *
 ********************************************************************/
int WeatherStationWS2300::humidity_outdoor(void)
{
  unsigned char data[20];
  unsigned char command[25];  //room for write data also
  int address=0x419;
  int bytes=1;
  
  if (read_safe(address, bytes, data, command) != bytes)
    return -1;

  return ((data[0] >> 4) * 10 + (data[0] & 0xF));
}

/********************************************************************
 * wind_current
 * Read wind speed, wind direction
 *
 * Input: Handle to weatherstation
 *        wind_speed_conv_factor controlling convertion to other
 *             units than m/s
 *
 * Output: winddir - pointer to double in degrees
 *
 * Returns: Wind speed (double) in the unit given in the loaded config
 *
 ********************************************************************/
double WeatherStationWS2300::wind_current(double wind_speed_conv_factor,
                    double *winddir)
{
  unsigned char data[20];
  unsigned char command[25];  //room for write data also
  int address=0x527; //Windspeed and direction
  int bytes=3;
  
  if (read_safe(address, bytes, data, command)!=bytes) //Wind
    return -1;
  
  if ( (data[0]!=0x00) ||                            //Invalid wind data
      ((data[1]==0xFF) && (((data[2]&0xF)==0)||((data[2]&0xF)==1))) )
  {
    return -1; //Disable retries since we do constant polling
  }
  
  //Calculate wind directions

  *winddir = (data[2]>>4)*22.5;

  //Calculate raw wind speed  - convert from m/s to whatever
  return( (((data[2]&0xF)<<8)+(data[1])) / 10.0 * wind_speed_conv_factor );
}

/********************************************************************
 * rain_1h
 * Read rain last 1 hour, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double WeatherStationWS2300::rain_1h(double rain_conv_factor)
{
  unsigned char data[20];
  unsigned char command[25];  //room for write data also
  int address=0x4B4;
  int bytes=3;

  if (read_safe(address, bytes, data, command) != bytes)
    return -1;

  return ( ((data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 +
            (data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 +
            (data[0] & 0xF) / 100.0 ) / rain_conv_factor);
}

/********************************************************************
 * rain_24h
 * Read rain last 24 hours, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double WeatherStationWS2300::rain_24h(double rain_conv_factor)
{
  unsigned char data[20];
  unsigned char command[25];  //room for write data also
  int address=0x497;
  int bytes=3;
  
  if (read_safe(address, bytes, data, command) != bytes)
    return -1;

  return (((data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 +
           (data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 +
           (data[0] & 0xF) / 100.0) / rain_conv_factor);
}

/********************************************************************
 * abs_pressure
 * Read absolute air pressure, current value only
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Returns: pressure (double) converted to unit given in config
 *
 ********************************************************************/
double WeatherStationWS2300::abs_pressure(double pressure_conv_factor)
{
  unsigned char data[20];
  unsigned char command[25];
  int address=0x5D8;
  int bytes=3;
  
  if (read_safe(address, bytes, data, command) != bytes)
    return -1;


  return (((data[2] & 0xF) * 1000 + (data[1] >> 4) * 100 +
           (data[1] & 0xF) * 10 + (data[0] >> 4) +
           (data[0] & 0xF) / 10.0) / pressure_conv_factor);
}

/********************************************************************
 * tendency_forecast
 * Read Tendency and Forecast
 * 
 * Input: Handle to weatherstation
 *
 * Output: tendency - string Steady, Rising or Falling
 *         forecast - string Rainy, Cloudy or Sunny
 *
 * Returns: nothing
 *
 ********************************************************************/
bool WeatherStationWS2300::tendency_forecast(char *tendency, char *forecast)
{
  unsigned char data[20];
  unsigned char command[25];
  int address=0x26B;
  int bytes=1;
  const char tendency_values[] = { 'S', 'R', 'F' };
  const char forecast_values[] = { 'R', 'C', 'S' };

  if (read_safe(address, bytes, data, command) != bytes)
    return false;

  *tendency=tendency_values[data[0] >> 4];
  *forecast=forecast_values[data[0] & 0xF];

  return true;
}


void WeatherStationWS2300::getStatus(WeatherStatus &ws){
  DEBUG_PRINT_P("Populating Weather Status struct (size: %d)\n",sizeof(struct WeatherStatus));
  ws.tempIndoorC=(temperature_indoor(0)*100);
  DEBUG_PRINT_P("Indoor temp %u (c*100)\n",ws.tempIndoorC);  
  ws.tempOutdoorC=(temperature_outdoor(0)*100); 
  // DEBUG_PRINT_P("Outdoor temp %u (c*100)\n",ws.tempOutdoorC);  
  ws.humIndoor=humidity_indoor(); 
  // DEBUG_PRINT_P("Indoor hum %hu (%%)\n",ws.humIndoor);  
  ws.humOutdoor=humidity_outdoor(); 
  // DEBUG_PRINT_P("Outdoor hum %hu (%%)\n",ws.humOutdoor);  
  ws.rainLast1hr=(rain_1h(1)*100); 
  // DEBUG_PRINT_P("MMs of rain on the last 1 hour %u\n",ws.rainLast1hr);  
  ws.rainLast24hr=(rain_24h(1)*100); 
  // DEBUG_PRINT_P("MMs of rain on the last 24 hour %u\n",ws.rainLast24hr);  
  double winDir;
  ws.windSpeed=(wind_current(3.6,&winDir)*100);
  ws.windDir=winDir;
  // DEBUG_PRINT_P("Wind speed %u (km/h)\n",ws.windSpeed);  
  // DEBUG_PRINT_P("Wind dir %u (degrees from N clockwise)\n",ws.windDir);  
  ws.pressHPa=(abs_pressure(1)*10); 
  // // DEBUG_PRINT_P("Pressure %u (hPa)\n",ws.pressHPa);  
  tendency_forecast(&ws.tendency, &ws.forecast);
  // DEBUG_PRINT_P("Tendency %c, Forecast %c\n",ws.tendency, ws.forecast);  
}

