## Collect sensor data

- This command is periodically sent by the master to poll slaves (4 times per second)
- Constants 
  - RS485CMD\_COLLECT\_SENSOR: 4
  - RS485CMD\_COLLECT\_SENSOR\_EXTENDED: 5

- Packet format [libraries/CasaC/CasaC.h:122](https://github.com/cat101/CasaC/blob/master/libraries/CasaC/CasaC.h#L122)

## Timed output

- This command turns an output on for an specific period of time and then resets it. If the output was already on then it does nothing. This action is useful for courtesy lights and door locks
- Constants 
  - RS485CMD\_TIMED\_OUTPUT:7

## Push EEPROM config data

- This command send a binary config file that gets written as is on the uC flash
- Constants 
  - RS485CMD\_WRITENLOAD\_CONFIG:6

## Set mode/status

- This command is used to set & share mode bits with single nodes or the whole house. Nodes won't sent an acknowledge when receiving this command
- The commands pushes 2 mode bytes. Description of the individual bits can be found on [libraries/RS485Slave/RS485Slave.cpp:142](https://github.com/cat101/CasaC/blob/master/libraries/RS485Slave/RS485Slave.cpp#L142)
- Constants 
  - RS485CMD\_SET\_MODE:8

## Send command to surrogate

- This command is used to send a command (text string) to a surrogate via a slave. No return value/acknowledgment is expected
- Constants 
  - RS485CMD\_SEND\_SURROGATE\_CMD:9

## RS485CMD\_COMM\_CHECK (code 240)

## RS485CMD\_SERVICE\_MODE (code 241) --- Deprecated. See: RS485CMD\_SET\_MODE

This mode is used to check new boards and verify the inputs and outputs are working as expected. Service mode disables the local and remote event dispatchers for the target board. It also triggers an "in sequence" activation of all the outputs. While in service mode the light switches are mapped directly to output. The same happens with the sensor loops

Params

- isEnabled (bool)
