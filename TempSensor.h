#ifndef _TEMPSENSOR_h
#define _TEMPSENSOR_h

#include "arduino.h"
#include "OneWire.h"

struct Resolution;

class TempSensor
{
private:
	// time in milliseconds for which if sensor is not responding it will still be marked as working. Filters small connectivity issues
	static const unsigned long SENSOR_IS_WORKING_FLAG_DELAY_MS = 5000;

	OneWire* DS18B20;
//	boolean isPresent; // true if 1-wire device answering with Present pulse
//	byte romCode[8];
	unsigned long millisStoppedWorking;
	unsigned long convertCallTimeMillis;		// time of the call of ConverT command
	HardwareSerial *serial;

public:
	// resolution (6th and 7th bit define 9..12-bit resolution)
//	static Resolution _9BIT_RESOLUTION;     // conversion time 93.75ms
//	static Resolution _10BIT_RESOLUTION;    // conversion time 187.5ms
	static Resolution _11BIT_RESOLUTION;    // conversion time 375ms
//	static Resolution _12BIT_RESOLUTION;    // conversion time 750ms

	static const int ERROR_TEMPERATURE_VALUE = -500;

	TempSensor(OneWire& device, Resolution& resolutionConfig, HardwareSerial& debuggerSerial);

	float temperatureValue;
	Resolution *resolution;
	boolean isWorking; // true if device executes commands or not answering for less than MILLIS_NOT_WORKING_DELAY
	boolean isConvertCalled;
	boolean isConvertFinished;
	boolean isResolutionConfigured;

	boolean isPresent; // true if 1-wire device answering with Present pulse
	byte romCode[8];

	boolean readRomCode();
	boolean setResolution(Resolution& resolutionConfig);
	boolean callConvert();
	boolean updateTemperatureValue();
	void updateFlags();

};

struct Resolution
{
	byte configByte;
	unsigned int conversionTime;
};
#endif

