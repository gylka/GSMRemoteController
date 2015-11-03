#include "Arduino.h"

void turnOnGsmModule(int gsmModuleButtonPin) {
	digitalWrite(gsmModuleButtonPin, LOW);
	delay(1000);
	digitalWrite(gsmModuleButtonPin, HIGH);
	delay(2000);
	digitalWrite(gsmModuleButtonPin, LOW);
}

