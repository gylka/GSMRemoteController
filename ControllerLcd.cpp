#include "ControllerLcd.h"

byte ControllerLcd::SIGNAL_CHAR_FILLED[] = {
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
};
byte ControllerLcd::SIGNAL_CHAR_START_EMPTY[] = {
	0b11111,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b11111,
};
byte ControllerLcd::SIGNAL_CHAR_MIDDLE_EMPTY[] = {
	0b11111,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11111,
};
byte ControllerLcd::SIGNAL_CHAR_END_EMPTY[] = {
	0b11111,
	0b00001,
	0b00001,
	0b00001,
	0b00001,
	0b00001,
	0b11111,
};
byte ControllerLcd::PUMP_CHAR[] = {
	0b10000,
	0b11000,
	0b11110,
	0b11111,
	0b11111,
	0b11110,
	0b11000,
	0b10000
};

byte ControllerLcd::SIGNAL_BAR_NO_SIGNAL[4] =  { 1,2,2,3 };
byte ControllerLcd::SIGNAL_BAR_LOW_SIGNAL[4] = { 0,2,2,3 };
byte ControllerLcd::SIGNAL_BAR_MEDIUM_SIGNAL[4] = { 0,0,2,3 };
byte ControllerLcd::SIGNAL_BAR_GOOD_SIGNAL[4] = { 0,0,0,3 };
byte ControllerLcd::SIGNAL_BAR_SUPER_SIGNAL[4] = { 0,0,0,0 };

ControllerLcd::ControllerLcd(LiquidCrystal& display) { lcd = &display; isScreenRefreshed = true; }

void ControllerLcd::init() const
{
	lcd->begin(20,4);
	lcd->clear();
	lcd->noCursor();
	lcd->createChar(0, SIGNAL_CHAR_FILLED);
	lcd->createChar(1, SIGNAL_CHAR_START_EMPTY);
	lcd->createChar(2, SIGNAL_CHAR_MIDDLE_EMPTY);
	lcd->createChar(3, SIGNAL_CHAR_END_EMPTY);
	lcd->createChar(4, PUMP_CHAR);
}

void ControllerLcd::showSplashScreen()
{
	lcd->clear();
	lcd->setCursor(2, 0);
	lcd->print(F("Gylka GSM Remote"));
	lcd->setCursor(4, 1);
	lcd->print(F("Controller"));
	lcd->setCursor(7, 2);
	lcd->print(F("v1.0"));
}


void ControllerLcd::printBlankTemplate() {
	lcd->clear();
	lcd->setCursor(0, 0);
	lcd->print(F("GSM="));
	lcd->setCursor(0, 1);
	lcd->print(F("Tkotel="));
	lcd->setCursor(15, 1);
	lcd->write(4);
	lcd->print(F("="));
	lcd->setCursor(0, 2);
	lcd->print(F("Tkimnata="));
	lcd->setCursor(13, 3);
	lcd->print(F("Tu="));
}

void ControllerLcd::printGsmModuleState(boolean isModuleOn)
{
	lcd->setCursor(4, 0);
	if (isModuleOn) 	{	lcd->print(F("ON "));	} 
	else	{	lcd->print(F("OFF"));	};
}

void ControllerLcd::printGsmSignal(byte signalLevel)
{
	byte *signalChars;
	if (signalLevel == 0 || signalLevel == 99) { signalChars = SIGNAL_BAR_NO_SIGNAL; };
	if (signalLevel >= 1 && signalLevel <= 9) { signalChars = SIGNAL_BAR_LOW_SIGNAL; }
	if (signalLevel >= 10 && signalLevel <= 14) { signalChars = SIGNAL_BAR_MEDIUM_SIGNAL; }
	if (signalLevel >= 15 && signalLevel <= 19) { signalChars = SIGNAL_BAR_GOOD_SIGNAL; }
	if (signalLevel >= 20 && signalLevel <= 31) { signalChars = SIGNAL_BAR_SUPER_SIGNAL; }
	lcd->setCursor(16, 0);
	for (int i = 0; i < 4; i++)
	{
		lcd->write(signalChars[i]);
	}
}

void ControllerLcd::printBoilerTemperature(float boilerTemperature)
{
	lcd->setCursor(7, 1);
	lcd->print(F("        "));
	lcd->setCursor(7, 1);
	if (boilerTemperature < (float)(-100))    { lcd->print(F("ERR"));	}
	else { lcd->print(boilerTemperature); }
}


void ControllerLcd::printPumpState(boolean isPumpOn)
{
	lcd->setCursor(17, 1);
	if (isPumpOn)	{	lcd->print(F("ON "));	}
	else	{	lcd->print(F("OFF"));	};
}

void ControllerLcd::printRoomTemperature(float roomTemperature)
{
	lcd->setCursor(9, 2);
	lcd->print(F("           "));
	lcd->setCursor(9, 2);
	if (roomTemperature < (float)(-100)) { lcd->print(F("ERR")); }
	else { lcd->print(roomTemperature); }
}

void ControllerLcd::printStartTime(byte day, byte month, byte hours, byte minute)
{
	lcd->setCursor(0, 3);
	lcd->print(F("            "));
	lcd->setCursor(0, 3);
	char buffer[12];
	for (byte i = 0; i < 12; i++) { buffer[i] = 0; }
	sprintf(buffer, "%02d/%02d %02d:%02d", day, month, hours, minute);
	lcd->print(buffer);
}

void ControllerLcd::printUnitTemperature(float unitTemperature)
{
	int intTempValue = int(unitTemperature);
	lcd->setCursor(16, 3);
	lcd->print(F("    "));
	lcd->setCursor(16, 3);
	lcd->print(intTempValue);
}

void ControllerLcd::run()
{
	if (millis() - lastResfreshTimeMs > LCD_REFRESH_INTERVAL_MS)
	{
		lcd->clear();
		printBlankTemplate();
		lastResfreshTimeMs = millis();
		isScreenRefreshed = true;
	}
}
