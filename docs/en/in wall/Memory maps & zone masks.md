## An slave's memory map

The slave memory map is managed by SensorAcq

```
+------------+
|digSamples 1|
|            | <-+ Each bit represents an NO input
|            |
|digSamples i|
+------------+
|senSamples 1|
|            | <-+ The first x bits are maped to the NC alarm sensors
|            | The following bits correspond to the double click sensors
|senSamples j|
+------------+
|errSamples 1|
|            | <-+ Each bit indicates an error state on the x NC alarm sensors
|            |
|errSamples k|
+------------+
|digOutputs 1|
|            | <-+ Each bit represents a digital output
|            |
|digOutputs l|
+------------+
|analogSens 1|
|            | <-+ Each unsigned int (2 bytes) is one analog input
|            | then we map all the external sensors (e.g. DHT)
|analogSens m|
+------------+
```

## The global sensor map

The Sensor arrays are used to store the last readings across all the house. The format is

- NodeID, No. dig+sen+err+out (in bytes), No. analog readings (in bytes), padding to store all the data

```
byte PA_O_sensors[]={PA_O_NODEID,5,6,0,0,0,0,0,0,0,0,0,0,0}; //dig:1 sen:1 err:1 out:2 ana:3 (x2) + 0
byte PA_E_sensors[]={PA_E_NODEID,7,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //dig:1 sen:2 err:2 out:2 ana:4 (x2) + 0
byte *houseSensors[] = {sensors.padding,PA_O_sensors,PA_E_sensors};
```

## The global zone mask set

A house mask flags what events we want to trigger on the master. There is a bit set for every sensor input on the corresponding node's memory map (i.e. sensor array)

![How masks are structured](../uploads/SW%20diagrams/Sensor%20mask%20data%20structure.png)

