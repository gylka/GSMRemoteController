#include "SwitchableDevice.h"

SwitchableDevice::SwitchableDevice(int devicePin)
{
	pin = devicePin;
	isOn = false;
}

void SwitchableDevice::turnOn()
{
	digitalWrite(pin, HIGH);
	isOn = true;
}

void SwitchableDevice::turnOff()
{
	digitalWrite(pin, LOW);
	isOn = false;
}

void SwitchableDevice::applyState(byte state)
{
	switch (state)
	{
	case STATE_ON: { turnOn(); break; }
	case STATE_OFF: { turnOff(); break; }
	};
}

