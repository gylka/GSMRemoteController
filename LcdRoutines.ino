#include "Arduino.h"

byte signalCharFilled[8] = {
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
};
byte signalCharStartEmpty[8] = {
	0b11111,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
    0b11111,	
};
byte signalCharMiddleEmpty[8] = {
	0b11111,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11111,
};
byte signalCharEndEmpty[8] = {
	0b11111,
	0b00001,
	0b00001,
	0b00001,
	0b00001,
	0b00001,
	0b11111,
};

void printMainTemplateOnLcd(LiquidCrystal& lcd) {
	lcd.setCursor(0,0);
	lcd.print(F("GSM="));
	lcd.setCursor(0,1);
	lcd.print(F("Tkotel="));
	lcd.setCursor(15,1);
	lcd.print(F("N="));
	lcd.setCursor(0,2);
	lcd.print(F("Tkimnata="));
	lcd.setCursor(14,3);
	lcd.print(F("Tu="));
}

void drawSignalLevel(int signalLevel) {
	switch (signalLevel) {
		case :
			break;
	}
}