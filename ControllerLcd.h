#ifndef _CONTROLLERLCD_h
#define _CONTROLLERLCD_h

#include "arduino.h"
#include "LiquidCrystal.h"

class ControllerLcd
{
private:
	static byte SIGNAL_CHAR_FILLED[8];
	static byte SIGNAL_CHAR_START_EMPTY[8];
	static byte SIGNAL_CHAR_MIDDLE_EMPTY[8];
	static byte SIGNAL_CHAR_END_EMPTY[8];
	static byte PUMP_CHAR[8];

	static byte SIGNAL_BAR_NO_SIGNAL[4];
	static byte SIGNAL_BAR_LOW_SIGNAL[4];
	static byte SIGNAL_BAR_MEDIUM_SIGNAL[4];
	static byte SIGNAL_BAR_GOOD_SIGNAL[4];
	static byte SIGNAL_BAR_SUPER_SIGNAL[4];

	LiquidCrystal* lcd;

public:

	explicit ControllerLcd(LiquidCrystal& display);

	void clear() const;
	void init();
	void showSplashScreen();
	void printBlankTemplate();
	void printGsmModuleState(boolean isModuleOn);
	void printGsmSignal(unsigned int signalLevel);
	void printBoilerTemperature(float boilerTemperature);
	void printPumpState(boolean isPumpOn);
	void printRoomTemperature(float roomTemperature);
	void printUpTime(unsigned int months, unsigned int days, unsigned int hours);
	void printUnitTemperature(float unitTemperature);
	LiquidCrystal& getLcd() const;

};

#endif

