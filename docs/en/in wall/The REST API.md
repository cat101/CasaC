## Get and set sensors & actuators

- Get the memory map (in binary)
  - curl "192.168.1.80/io"
  - curl "192.168.1.80/io" |hexdump.exe –C

- Set/reset actuators
  - curl "192.168.14.80/io?set=0604" ActuactorId must be in HEX (i.e. HS\_O\_)
  - curl "192.168.1.80/io?reset=0604" ActuactorId must be in HEX (i.e. HS\_O\_)

- Trigger events
  - curl "192.168.1.80/io?open=0502" EventId must be in HEX (i.e. HS\_) - use for \_S & \_B since there is no zone parameter
  - curl "192.168.1.80/io?close=0502" EventId must be in HEX (i.e. HS\_)

## Play audio files through the PA system

- curl "192.168.1.80/playaudio?play=5"
- curl "192.168.1.80/playaudio?stop"

## Filesystem access

See [The SD file system](in wall/The SD file system.md))

## Arm/disarm the burglar alarm

- curl "192.168.14.80/alarm"
- curl "192.168.14.80/alarm?arm\_stay"
- curl "192.168.14.80/alarm?arm\_away"
- curl "192.168.14.80/alarm?disarm"

## RTC

- curl "192.168.1.80/clock"
- curl "192.168.1.80/clock?set=UNIXTIME"
  - Online converter [http://www.epochconverter.com/](http://www.epochconverter.com/)

## Update the master's config file

The master gets its non-slave parameter directly from the SD card. To change the configuration we have to upload a new file and then restart.

1. curl -H "Expect:" --upload-file m.cfg "192.168.1.80/fs/"
2. curl "192.168.1.80/restart?node=0" (RESETING SLAVES IS NOT IMPLEMENTED)

## Update a local node config file

This command updates the EEPROM configuration data. It applies to the slaves and the slave functionality present on the master. The new config parameters are also reloaded into memory

1. curl -H "Expect:" --upload-file 1.cfg "192.168.1.80/fs/"
2. curl "192.168.1.80/reloadconfig?node=1"

## Debug & Test calls

The ##setMode## command allows to enable and disable selective features

- curl "192.168.14.80/setMode?node=4&set=5"      Set service mode. In production service mode disables the IO write operation on the sensoracq class (i.e. light won't turn on or off)
- curl "192.168.14.80/setMode?node=4&reset=5"    Reset service mode
- curl "192.168.14.80/setMode?node=255&set=3"      Set isDark
- curl "192.168.14.80/setMode?node=255&reset=3"    Reset isDark
- curl "192.168.14.80/setMode?node=255&set=4"      Set debugEnabled
- curl "192.168.14.80/setMode?node=255&reset=4"    Reset debugEnabled
- curl "192.168.14.80/setMode?node=0&reset=100"    Disables the RuleEvaluator (i.e. all time of day related rules)
- curl "192.168.14.80/setMode?node=0&set=100"      Enables the RuleEvaluator
- curl "192.168.14.80/setMode?node=0&set=101"      Enables the water heater logic
- curl "192.168.14.80/setMode?node=0&set=102"      Enables ioTimers behind courtesy lights (excludes stairs)
- curl "192.168.14.80/setMode?node=0&reset=102"    Disables ioTimers
- curl "192.168.14.80/setMode?node=0&set=103"      Enables a level & heat boost cycle for the water heater
- curl "192.168.14.80/setMode?node=0&set=104"      Enables winter mode for the burglar alarm
  
- curl "192.168.1.80/stresstest?node=1&nextRoundDelay=3&paddingLen=10"

Return the last exception and a set of vital stats regarding resource usage

- curl "192.168.1.80/health"

Set calibration parameters for the energy management surrogate

- curl "192.168.14.80/transferswitch?vc=216.80&c1c=16.70&c1v=1.80&c2c=14.10&c2v=2.20&c3c=14.10&c3v=2.20&c4c=14.10&c4v=2.20&nh=20&to=2000&mfv=100.0&bfv=11.0&bvc=6.90"
- curl "192.168.14.80/transferswitch?c3c=15.05"
- curl "192.168.14.80/transferswitch?c3v=1.5"
- curl "192.168.14.80/transferswitch?c2v=1.80&c3v=1.80&c4v=1.80"

## Enable callbacks to the rest proxy/building management system

- curl "192.168.14.80/setcb" return enabled callbacks
- curl "192.168.14.80/setcb?set=1" Enable callbacks for zone 1
- curl "192.168.14.80/setcb?reset=1"

## TO-DO Add the rest proxy API
