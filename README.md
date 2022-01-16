# bmw-obd2-display
My BMW F20 116i doesn't have a temperature gauge. So I made one myself.

The hardware shopping list is as follows:
- ELM327 Bluetooth OBD2 adapter
- ESP32 Devkit v1
- Small 128\*64 OLED display
- Some wires, USB micro cable, a case.

Currently is shows the following information:
- Oil temperature
- Coolant temperature
- Boost pressure
- Highest achieved boost pressure
- (When cranking with key-on-engine-off) lowest battery voltage

## To-Do-s
I want to add the following features some time in the future:
- Update to latest ELMduino library.
- Properly check if data has been correctly received before continuing.
- Use ELM327's voltage instead of reading through OBD2.
- Monitor voltage when connection with ECU has not yet been established.
- Speed up display rendering. (Drawing black box over old values, instead of full re-render?)
- Make "kPa" text smaller.
- Show battery voltage in the bottom-right.
- Try to optimize code for faster execution. Readings per second is somewhat slow.
- Disable ELMduino logging. Setting debug-to-serial parameter to false currently causes ELMduino to not be able to connect, it seems. Needs debugging.

## Credits
Massive thanks to Powerbroker2 for the [ELMduino](https://github.com/PowerBroker2/ELMduino) library!
