# Welcome to La CasaC
La CasaC is a comfortable, energy efficient, low maintenance and sustainable house. It is an ongoing project me and my wife started back in 2012. After three years of design and planning and two years of construction we moved in.

When I started planning La CasaC I had two options: use commercial products like home automation controllers, security lighting, burglar alarm, etc. and then spend my time trying to integrate proprietary protocols or use Open Source hardware and software to build/integrate everything from scratch. Having suffered the commercial/closed approach with my previous house I decided to try the latter.

Building the system from scratch was a significant challenge given the scale of the project. The “initial specs” based on the house blueprints and family expectations included:

- 40 Wall switches to turn lights on/off (i.e. over 45 Light channels to control 136 lamps!)
- 2 Garage doors and 2 doors with an electric lock
- 2 Mechanized awnings that had to follow the sun on summer to keep the living room cool
- 55 sensors (doors switches, PIR sensors, water flood, pool alarm)
- A weather station 
- A solar water heater
- 7 indoor air conditioning units
- 2 bathroom heaters and 2 bathroom fans
- A kitchen range
- 8 smoke detectors/air quality sensors
- A four channel energy meter with an automatic transfer switch to a backup power generator
- A whole house PA system for TTS cues
- 2 RFID keypads for access control
- 4 433Mhz RF receivers

Today I have all these features implemented using a tiered Arduino & Raspberry Pi architecture. This repository contains the public parts of the source tree. I have not yet opened all the code because I'm concerned of divulging PII information. If you are interested in access to the full code contact me on github.

In the meantime you can also check the following links that document other aspects of the project

* [Wiki page for the automation software & hardware](https://cat101.bitbucket.io/en/#!index.md)
* [Wiki page for the house construction (in spanish)](https://cat101.bitbucket.io/sp/#!index.md)
* [Photos of the whole project](https://goo.gl/photos/PSqoa4BDfdnn28Vv8)
* [RF to REST bridge](https://github.com/cat101/rf433-to-rest)
* [Arduino Solar Water Heater Sensor](http://hack4life.pbworks.com/w/page/75653090/Arduino%20Solar%20Water%20Heater%20Sensor)


# Building the code
## Installing the dependencies
### MAC OS
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
## Building the source tree
Check configuration on Makefile.common
