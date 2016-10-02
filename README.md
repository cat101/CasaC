# Installing the dependencies

## MAC OS
* Install arduino.cc IDE
* Using brew add a new tap and install Arduino.mk 
```
$ brew tap sudar/arduino-mk
$ brew install arduino-mk
```
* If you want do serial debugging from arduino-mk, install pyserial (pip may not be installed by default)
```
	$ brew install python    
	$ pip install pyserial
```

# Building 
* Check configuration on Makefile.common
