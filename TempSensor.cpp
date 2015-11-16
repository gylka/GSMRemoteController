#include "TempSensor.h"

//Resolution TempSensor::_9BIT_RESOLUTION = { 0b00011111, 100 };     // conversion time 93.75ms
//Resolution TempSensor::_10BIT_RESOLUTION = { 0b00111111, 200 };    // conversion time 187.5ms
Resolution TempSensor::_11BIT_RESOLUTION = { 0b01011111, 400 };    // conversion time 375ms
//Resolution TempSensor::_12BIT_RESOLUTION = { 0b01111111, 800 };    // conversion time 750ms

TempSensor::TempSensor(OneWire& device, Resolution& resolutionConfig, HardwareSerial& debuggerSerial)
{
	DS18B20 = &device;
	serial = &debuggerSerial;
	isWorking = false;
	isPresent = false;
	resolution = &resolutionConfig;
	millisStoppedWorking = 0;
	for (int i = 0; i < 8; i++)	{ romCode[i] = 0; }
	isConvertCalled = false;
	isConvertFinished = false;
	convertCallTimeMillis = 0;
	isResolutionConfigured = false;
	temperatureValue = (float)ERROR_TEMPERATURE_VALUE;
}

boolean TempSensor::readRomCode()
{
	boolean result = false;
	DS18B20->reset_search();
	if (DS18B20->search(romCode) &&                            // Checking if there's any 1-wire device on the network
		(OneWire::crc8(romCode, 7) == romCode[7]) &&           // Performing CRC check (8th byte of address ROM)
		(romCode[0] == 0x28))                                  // Checking if address belongs to DS18B20 (1st byte should be 0x28)
	{
		isPresent = true;
		result = true;
	}
	else
	{
		isPresent = false;
		result = false;
	}
	isConvertCalled = false;
	isConvertFinished = false;
	updateFlags();
	return result;
}

boolean TempSensor::setResolution(Resolution& resolutionConfig)
{
	boolean result = false;
	resolution = &resolutionConfig;
	if (DS18B20->reset())
	{
		isPresent = true;
		result = true;
		DS18B20->select(romCode);
		DS18B20->write(0x4E);                                    // Write 3 bytes to scratchpad
		DS18B20->write(0);                                       // 1st byte - TL
		DS18B20->write(0);                                       // 2nd byte - TH
		DS18B20->write(resolutionConfig.configByte);             // 3rd byte - configuration register (defines resolution)
		DS18B20->write(0x48);                                    // Copy scratchpad to EEPROM
		isResolutionConfigured = true;
	}
	else
	{
		isPresent = false;
		isResolutionConfigured = false;
		result = false;
	}
	updateFlags();
	return result;
}

boolean TempSensor::callConvert()
{
	boolean result = false;
	if (DS18B20->reset())
	{
		isPresent = true;
		result = true;
		DS18B20->select(romCode);
		DS18B20->write(0x44, 1);                      // Calling conversion command, no pull-up pulse for parasitic with parasite power mode
		isConvertCalled = true;
		convertCallTimeMillis = millis();
	}
	else
	{
		isPresent = false;
		result = false;
		isConvertCalled = false;
	}
	updateFlags();
	return result;
}

boolean TempSensor::updateTemperatureValue()
{
	boolean result = false;
	if (DS18B20->reset())
	{
		isPresent = true;
		result = true;
		DS18B20->select(romCode);
		DS18B20->write(0xBE);                                    // Command read scratchpad
		byte scratchpad[8];
		for (int i = 0; i < 8; i++)
		{
			scratchpad[i] = DS18B20->read();
		}
		unsigned int raw = (scratchpad[1] << 8) | scratchpad[0];
		temperatureValue = (float)raw / 16.0;
	}
	else
	{
		isPresent = false;
		result = false;
	}
	isConvertCalled = false;
	updateFlags();
	return result;
}

void TempSensor::updateFlags()
{
	if (!isPresent && isWorking)
	{
		// using millisStoppedWorking as a flag at same time (if == 0 -> then it's the 1st time isPresent turned zero)
		if (millisStoppedWorking == 0)		{ millisStoppedWorking = millis();	}
		if ((millis() - millisStoppedWorking) > SENSOR_IS_WORKING_FLAG_DELAY_MS)
		{
			isWorking = false;
			isConvertCalled = false;
			isConvertFinished = false;
		}
	}
	if (isPresent)
	{
		isWorking = true;
		// setting to zero to zero down the flag
		millisStoppedWorking = 0;
	} 
	if(isConvertCalled && (millis() - convertCallTimeMillis > resolution->conversionTime))
	{
		isConvertFinished = true;
	}
	else { isConvertFinished = false; }
}