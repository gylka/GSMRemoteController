#include "Arduino.h"

boolean readTemperatureSensorRomCode(OneWire& ds18b20, byte romCode[]) {
	// Checking if there's any 1-wire device on the network
	ds18b20.reset_search();
	if ( !ds18b20.search(romCode)) {
		return false;
	}
	// Performing CRC check (8th byte of address ROM)
	if (OneWire::crc8(romCode, 7) != romCode[7]) {
		return false;
	}
	// Checking if address belongs to DS18B20 (1st byte should be 0x28)
	if (romCode[0] != 0x28) {
		return false;
	}
	return true;
}

boolean setTemperatureSensorResolution(OneWire& ds18b20, byte romCode[], byte resolution) {
	if ( !ds18b20.reset()) {
		return false;
	}
	ds18b20.select(romCode);
	ds18b20.write(0x4E);                       // Write 3 bytes to scratchpad
	ds18b20.write(0);                          // 1st byte - TL
	ds18b20.write(0);                          // 2nd byte - TH
	ds18b20.write(resolution);                 // 3rd byte - configuration register (defines resolution)
	ds18b20.write(0x48);                       // Copy scratchpad to EEPROM
	return true;
}

boolean callConvert(OneWire& ds18b20, byte romCode[]){
	if ( !ds18b20.reset()) {
		return false;
	}
	ds18b20.select(romCode);
	ds18b20.write(0x44, 1);                    // Calling conversion command, no pull-up pulse for parasitic with parasite power mode
	return true;
}

 float readTemperatureValue(OneWire& ds18b20, byte romCode[]) {
	if ( !ds18b20.reset()) {                   // If DS18B20 sensor doesn't return present pulse - return error value (10000)
		return 10000;
	}
	ds18b20.select(romCode);
	ds18b20.write(0xBE);                       // Command read scratchpad
	byte scratchpad[8];
	for (int i = 0; i < 8; i++) {
		scratchpad[i] = ds18b20.read();
	}
	unsigned int raw = (scratchpad[1] << 8) | scratchpad[0];
	return (float)raw / 16.0;
}