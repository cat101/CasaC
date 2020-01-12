Config section are stored as binary structures. On reset the uC checks the CRC to see if there is a valid configuration. In theory we should be able to push a new config version before updating the uC firmware

The memory map looks as follows

```
 +----------------------+
 | EEPROM Marker (0xFC) |
 +----------------------+
 |     Data block 1     |
 |                      |
 |    e.g Analog Cal    |       crc=crc8(EEPROM_MARKER)
 +----------------------+           + crc8(size of data block 1)
 |         . . .        |           + crc8(       . . .        )
 +----------------------+           + crc8(size of data block n)
 |     Data block n     |
 |                      |
 |    e.g Analog Cal    |
 +----------------------+
                    http://www.asciiflow.com/
```

Each data block is written & read using [EEPROMex](http://playground.arduino.cc/Code/EEPROMex).

Sample data blocks:

- CardReader stores an array of 10 PINs or RFIDs (i.e. unsigned long)
  - [libraries/CardReader/CardReader.cpp:64](https://github.com/cat101/CasaC/blob/master/libraries/CardReader/CardReader.cpp#L64)

- SensorAcq stores an array of struct sensorThreshold (static size defined at compile time)
  - [libraries/SensorAcq/SensorAcq.cpp:39](https://github.com/cat101/CasaC/blob/master/libraries/SensorAcq/SensorAcq.cpp#L39)

