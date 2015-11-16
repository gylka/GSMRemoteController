#include "Button.h"

Button::Button(int buttonPin): pin(buttonPin), buttonPressedMillisTime(0), isButtonPreviousStateIsOn(false)
{
}

boolean Button::isButtonPressed()
{
	// getting rid of button press debouncing
	boolean result = false;
	if (digitalRead(pin))
	{
		if ( !isButtonPreviousStateIsOn)
		{
			isButtonPreviousStateIsOn = true;
			buttonPressedMillisTime = millis();
		}
		else
		{
			if (millis() - buttonPressedMillisTime > BUTTON_DEBOUNCE_DELAY_MS) { result = true; }
		}
	}
	else
	{
		isButtonPreviousStateIsOn = false;
	}
	return result;
}