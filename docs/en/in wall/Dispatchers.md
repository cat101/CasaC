## Event routing & zones
* The dispatchers on the master node are invoked only if they are enabled and they receive any event that matches a zone mask
* One event may match multiple masks and therefore be received by a dispatcher multiple times
	* Most of the time this never happens because switches only belong to HouseAutomation24HsMask and sensors are partitioned among the alarm zones (they only belong to one zone)
	* If a sensor does not belong to any mask then no dispatcher will get its event
* The HouseAutomation24Hs zone get automatically generated to trigger when switches that require global coordination at the master level need attention. 
	* The global automatisms get handled on the master's houseAutomationDispatcher using the LIST_OF_GLOBAL_ACTIONS block
	* The HouseAutomation24HsMask only contains switches and the other masks used by the alarm logic contain sensors. 
	* Internally the master fires local events using the HouseAutomation24Hs mask (e.g. EV_SUN_ENERGY_UP, EV_MAINSOK)
	* Note that switches won't trigger any events except for the ones used in house wide automatisms

## Sensor dispatchers (master only)
These dispatcher are primarily invoked by the RS485Master. Some events are injected locally

* houseAutomationDispatcher: This dispatcher never gets disable and it processes the HouseAutomation24Hs zone
* Alarm dispatchers: The disarmDispatcher, stayDispatcher and awayDispatcher selectively handle sensors events depending on the alarm state. The activation (or not) of each dispatcher represents the alarm state
* callbackDispatcher: This dispatcher is used to relay events back to the Raspberry Pi as REST callbacks. It programaticaly filters certain alarm events and it can alternatively relay events for zones selected on callbackZones


## Discrete event dispatcher (master only)
discreteHouseEventDispatcher gets called when the master receives a discrete event from an slave (e.g. RFID, keypad, exception). This dispatcher is just a gateway to sensor dispatchers or to the logging facility

## Local sensor event (slaves only) 
dispatchSensorEvents collect all the local sensor events and executes the local automatisms
