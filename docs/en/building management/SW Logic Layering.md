## OpenHab
- Manages Internet & house wide context (e.g. winter)
- Orchestrates processes that involve things beyond the Arduinos like calendars, LAN devices, etc.

## REST proxy
- Deal with things that have complex state which cannot be deterministically controlled by the Arduinos
	- Mechanized awnings
	- Things that the state may be affected by external things like an electrical failure (e.g. A/C, space heaters)

## Master Node
- Manages some global context (e.g. night time, need lights, etc.)
- Orchestrates house wide processes
	- Water heater
	- Global lighting schemes 
	- Courtesy lights (these rules could be pushed ot slaves but for flexibility reasons they are on the master)
- The master node includes an slave node that plays by the same rules as everybody else

## Slave Node
- Should deal with all the local automatism
	- Lights
	- RFID access control (accounts are stored locally)

- Does not have an idea of time, state or context
	- A little bit of global context is provided by the master through SetMode (e.g. night time, winter). This is used to change lighting logic

## Surrogate
- No state in memory (may report value of sensors)
- Interaction
	- Slaves poll surrogates and copy their info to the slave's memory (acquired by the master during extended poll)
	- Surrogate state is changed by commands directly from the master (tunneled using the C command)
		- For details on payload see RESTServer.cpp:606
	- The slaves process the surrogate inputs and generate events if needed (no event or constants are assigned by resourceProcesor.pl)