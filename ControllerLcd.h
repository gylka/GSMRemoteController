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

	inline void clear() const	{	lcd->clear();	};
	void init() const;
	void showSplashScreen();
	void printBlankTemplate();
	void printGsmModuleState(boolean isModuleOn);
	void printGsmSignal(byte signalLevel);
	void printBoilerTemperature(float boilerTemperature);
	void printPumpState(boolean isPumpOn);
	void printRoomTemperature(float roomTemperature);
	void printStartTime(byte days, byte month, byte hours, byte minute);
	void printUnitTemperature(float unitTemperature);

};

#endif

