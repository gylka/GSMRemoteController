// Button.h

#ifndef _BUTTON_h
#define _BUTTON_h

#include "arduino.h"

class Button
{
private:
	static const unsigned long BUTTON_DEBOUNCE_DELAY_MS = 100;
	int pin;
	unsigned long buttonPressedMillisTime;
	boolean isButtonPreviousStateIsOn;

public:
	Button(int buttonPin);
	boolean isButtonPressed();
};

#endif

