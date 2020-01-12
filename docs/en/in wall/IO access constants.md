#IO access constants
## Constants nomenclature

Every sensor, switch or actuator follows a naming convention. For example `PB_C01_B` refers to a light switch (`B`), located on the ground floor (`PB`), connected to input 01 on the kitchen controller (`C01`). More details below

- Node prefix: PA\_O, SS\_S
- Type suffix:
  - A = ADC input
  - B = push button or switch (NA)
  - D = Door Lock/strike 
  - I = PIR (NC)
  - J = Audio amp control
  - L = light
  - M = Magnetic sensor (NC)
  - O = Other (typically a custom memory map use to store more complex sensors)
  - P = Panic push button (NC)
  - R = Tamper sensor (NC)
  - S = virtual sensor typically created by a double click
  - T = Temperature
  - UI, UO, UA= Unused input, output, analog
  - X = Relay

## Slave constants

### Input/Sensor constants

These constants are used on the slave dispatcher to reference the right event. Sensors represents NC PIRs & Reed switches that are connected in loops to the analog ports.

Inputs and Sensors are not a real IO address, they just represent an event number which the following mapping

- I_nn_ is triggered by the digital port ACQ\_DIG\_RANGE\_START + _nn_
- S00, S01 is set by ACQ\_ANALOG\_RANGE\_START + 0 (analog sensors don't trigger events)
- S_nn - 1_, S_nn_ is set by ACQ\_ANALOG\_RANGE\_STOP (analog sensors don't trigger events)
- S_nn + 1_ is triggered by the first double click on a digital sensor

Example usage on an slave dispatcher

```
case PB_C01_B:sensors.digitalWrite(PB_C01_L, value);break;
case PB_C01_S:sensors.digitalWrite(PB_C07_L, value);break;
```

### Outputs constants

These constants map to the actual port that they represent. Although we don use them directly on digitalWrite and do it through SensorAcq so that we can report the status of the output pins to the master.

For example

```
sensors.digitalWrite(PB_C01_L, value)
```

### Analog sensor/memory constants

These constants are used by slaves to store analog readings from external sensors (e.g. DHT). They are defined on the spreadsheet as named arrays of bytes (e.g uint[2])

For example

```
#define SS_S01_T 0 // Desc:Temp sensor on transfer switch (DHT)
#define SS_S01_T_OFF 4
#define SS_S01_T_LEN 4

void aquireSensor(byte sensorNo){
    case SS_S01_T:
      *((unsigned int *)(&sensors.analogSensors[SS_S01_T_OFF]))=(unsigned int)(transferSwitchReading.temperature*10);
      *((unsigned int *)(&sensors.analogSensors[SS_S01_T_OFF+2]))=(unsigned int)(transferSwitchReading.humidity*10);
}
```
## Master/house constants

### Common nomenclature

- Master prefix: HS\_
- Node ID
```
#define SS_S_NODEID 0
```

Used to reference nodes inside of arrays at the master (e.g. the houseSensors and masks arrays)

### Constants for triggering Sensor/Double click/Input/Output events are used on the master dispatchers

Their format is: 0xAABB where AA is the node id and BB the bit position of the source node memmap

For example

```
#define HS_PA_O00_I 0x0108
#define HS_PA_O02_L 0x0119
#define HS_PB_C02_S 0x041b

case HS_PB_C02_S:RS485comm.addEvent(HS_O_JA_J02_L, value);break;
```

### Constants for controlling actuator/outputs on slave nodes

Their format is: 0xAABB where AA is the node id and BB is a bit field as follows

- Bit 8: 0 means just set the output, 1 trigger an output event on the slave (Not implemented)
- Bit 7: Not Used (later when transmitted used to indicate actuator set or reset) 
- Bit 6-1: bit position of the output part of the target node memmap (6 bits)

The value 0xFFFF is reserved because it is used to indicate the end of the event send queue in RS485Master::addEvent.

For example

```
#define HS_O_PA_O02_L 0x0101

RS485comm.addEvent(HS_O_PA_O02_L, value);
```

### Constants to access analog readings

Analog sensor format: 0xAABB where AA is the node id and BB the byte offset on the analog array

For example, on the master we would do this to read a DHT value

```
#define HS_PA_O01_A 0x0102
#define HS_SS_S01_T 0x0004

node=SS_S_NODEID;
DEBUG_PRINT_P("Extended poll on %u: DHT %u (temp x 10), %u (hum x 10)\n"
       ,houseSensors[node][HS_NODEID]
       ,*((unsigned int *)(&houseSensors[node][HS_DATA]+houseSensors[node][HS_NO_DIG]+(HS_SS_S01_T & 0xFF)))
       ,*((unsigned int *)(&houseSensors[node][HS_DATA]+houseSensors[node][HS_NO_DIG]+(HS_SS_S01_T & 0xFF)+2)));
```
