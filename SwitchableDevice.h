#ifndef _SWITCHABLEDEVICE_h
#define _SWITCHABLEDEVICE_h

#include "arduino.h"

class SwitchableDevice
{
private:
	int pin;

public:
	static const byte STATE_ON = 0x20;
	static const byte STATE_OFF = 0x10;
	
	explicit SwitchableDevice(int devicePin);
	
	boolean isOn;

	void turnOn();
	void turnOff();
	void applyState(byte state);
};

#endif

