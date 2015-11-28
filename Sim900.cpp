#include "Sim900.h"

char Sim900::allowedPhoneNumbers[Sim900::NUMBER_OF_ALLOWED_PHONE_NUMBERS][14] = {
	// Add allowed phone numbers here
};

Sim900::Sim900(int gsmModuleSwitchPin, BigBufferSoftwareSerial& serialDevice, TempSensor& boilerSensor_, TempSensor& roomSensor_,
	TempSensor& unitSensor_, SwitchableDevice& boilerDevice_, SwitchableDevice& pumpDevice_, HardwareSerial& debuggerSerial)
{
	switchPin = gsmModuleSwitchPin;
	gsmSerial = &serialDevice;
	boilerSensor = &boilerSensor_;
	roomSensor = &roomSensor_;
	unitSensor = &unitSensor_;
	boiler = &boilerDevice_;
	pump = &pumpDevice_;
	serial = &debuggerSerial;
	isOn = false;
	isSignalLevelCommandSent = false;
	millisLastTimeSignalChecked = 0;
	isDateTimeCommandSent = false;
	millisLastTimeDateTimeChecked = 0;
	millisLastAnswerRecieved = 0;
	signalLevel = 0;
	currentDateTime = { 0, 0, 0, 0, 0 };
	startingDateTime = { 0, 0, 0, 0, 0 };
	isBoilerStartScheduled = false;
	boilerScheduledStartDateTime = { 0, 0, 0, 0, 0 };
}

byte Sim900::getNumberOfDaysInMonth (byte year, byte month)
{
	switch (month)
	{
	case 1: return 31;
	case 2: 
	{
		if (year % 4 == 0) { return 29; }
		return 28;
	}
	case 3: return 31;
	case 4: return 30;
	case 5: return 31;
	case 6: return 30;
	case 7: return 31;
	case 8: return 31;
	case 9: return 30;
	case 10: return 31;
	case 11: return 30;
	case 12: return 31;
	}
	return 0;
}

void Sim900::increaseDateTimeByOneDay(DateTime& input, DateTime& output)
{
	if(input.day >= getNumberOfDaysInMonth(input.year, input.month))
	{
		if (input.month == 12)
		{
			output.year = input.year + 1;
			output.month = 1;
		} else
		{
			output.year = input.year;
			output.month = input.month + 1;
		}
		output.day = 1;
	} else
	{
		output.year = input.year;
		output.month = input.month;
		output.day = input.day + 1;
	}
	output.hour = input.hour;
	output.minute = input.minute;
}

boolean Sim900::isLeftDateTimeGreaterThanRight(DateTime& left, DateTime& right)
{
	if (left.year > right.year) { return true;	}
	if (left.year == right.year)
	{
		if (left.month > right.month) { return true; }
		if (left.month == right.month)
		{
			if (left.day > right.day) { return true; }
			if (left.day == right.day)
			{
				if (left.hour > right.hour) { return true; }
				if (left.hour == right.hour)
				{
					if (left.minute > right.minute) { return true; }
				}
			}
		}
	}
	return false;
}

void Sim900::run()
{
	sendSignalCheckPeriodically();
	sendDateTimeCheckPeriodically();
	if (readModuleDataToBufferIfAvailiable())	{	parseBufferAndExecute();   }
	if ((currentDateTime.year != 0) && isBoilerStartScheduled && (boilerScheduledStartDateTime.year == 0))
	{
		if (currentDateTime.hour >= boilerScheduledStartDateTime.hour)
		{
			increaseDateTimeByOneDay(currentDateTime, boilerScheduledStartDateTime);
			boilerScheduledStartDateTime.minute = 0;
		}
		else
		{
			boilerScheduledStartDateTime = { currentDateTime.year, currentDateTime.month, currentDateTime.day, boilerScheduledStartDateTime.hour, 0 };
		}
	}
	if (isBoilerStartScheduled && (boilerScheduledStartDateTime.year != 0))
	{
		if (isLeftDateTimeGreaterThanRight(currentDateTime, boilerScheduledStartDateTime))
		{
			boiler->turnOn();
			isBoilerStartScheduled = false;
		}
	}
}

void Sim900::switchState()
{
	serial->println("Entering switchState");
	digitalWrite(switchPin, LOW);
	delay(1000);
	digitalWrite(switchPin, HIGH);
	delay(2000);
	digitalWrite(switchPin, LOW);
	serial->println("Finished switchState");
}

void Sim900::sendSignalCheckPeriodically()
{
	if (millis() - millisLastTimeSignalChecked > GSM_SIGNAL_LEVEL_CHECK_INTERVAL_MS)
	{
		if (isSignalLevelCommandSent)
		{
			if ((millis() - millisLastTimeSignalChecked > GSM_SIGNAL_LEVEL_CHECK_INTERVAL_MS + EXPECTED_ANSWER_TIME_MS)
				&& (millis() - millisLastAnswerRecieved > EXPECTED_ANSWER_TIME_MS))
			{
				switchState();
				isOn = false;
				isSignalLevelCommandSent = false;
				millisLastTimeSignalChecked = millis();
			}
			if (millis() - millisLastAnswerRecieved < EXPECTED_ANSWER_TIME_MS)
			{
				isOn = true;
				isSignalLevelCommandSent = false;
				millisLastTimeSignalChecked = millis();
			}
		}
		else
		{
			gsmSerial->println(F("AT+CSQ"));
			isSignalLevelCommandSent = true;
		}
	}
}

void Sim900::sendDateTimeCheckPeriodically()
{
	if (millis() - millisLastTimeDateTimeChecked > RTC_DATE_TIME_CHECK_INTERVAL_MS)
	{
		if (isDateTimeCommandSent)
		{
			if (millis() - millisLastTimeDateTimeChecked + RTC_DATE_TIME_CHECK_INTERVAL_MS > RTC_DATE_TIME_CHECK_INTERVAL_MS)
			{
				isDateTimeCommandSent = false;
				millisLastTimeDateTimeChecked = millis();
			}
		}
		else
		{
			gsmSerial->println(F("AT+CCLK?"));
			isDateTimeCommandSent = true;
		}
	}
}

boolean Sim900::readModuleDataToBufferIfAvailiable()
{
	if (gsmSerial->available())
	{
		millisLastAnswerRecieved = millis();
		clearBuffer();
		int count = 0;
		while (gsmSerial->available())
		{
			buffer[count++] = gsmSerial->read();
			if (count == BUFFER_SIZE) { break; }
		}
		serial->write(buffer, count);
		return true;
	}
	return false;
}

void Sim900::parseBufferAndExecute()
{
	char dateTimeSign[9] = "AT+CCLK?";
	char signalLevelSign[7] = "AT+CSQ";
	char smsArriveSign[9] = "\r\n+CMT: ";
	if (isCharArraysEqual(buffer, dateTimeSign, 0, 8))
	{
		if (startingDateTime.year == 0)
		{
			startingDateTime.year = (buffer[20] - 48) * 10 + buffer[21] - 48;
			startingDateTime.month = (buffer[23] - 48) * 10 + buffer[24] - 48;
			startingDateTime.day = (buffer[26] - 48) * 10 + buffer[27] - 48;
			startingDateTime.hour = (buffer[29] - 48) * 10 + buffer[30] - 48;
			startingDateTime.minute = (buffer[32] - 48) * 10 + buffer[33] - 48;
		}
		currentDateTime.year = (buffer[20] - 48) * 10 + buffer[21] - 48;
		currentDateTime.month = (buffer[23] - 48) * 10 + buffer[24] - 48;
		currentDateTime.day = (buffer[26] - 48) * 10 + buffer[27] - 48;
		currentDateTime.hour = (buffer[29] - 48) * 10 + buffer[30] - 48;
		currentDateTime.minute = (buffer[32] - 48) * 10 + buffer[33] - 48;
	}
	if (isCharArraysEqual(buffer, signalLevelSign, 0, 6))
	{
		signalLevel = (buffer[16] - 48) * 10 + buffer[17] - 48;
	}
	if (isCharArraysEqual(buffer, smsArriveSign, 0, 8))
	{
		byte smsSenderIndex = isSmsSenderPhoneNumberAllowed();
		if (smsSenderIndex != BYTE_ERROR_VALUE)
		{
			char onCommand[3] = "on";
			char offCommand[4] = "off";
			char statusCommand[5] = "stat";
			stringToLowerCase(buffer, 51, 4);
			if (isCharArraysEqual(buffer, onCommand, 51, 2))
			{
				byte boilerScheduledStartHour = readScheduledTimeToByte();
				if (boilerScheduledStartHour != BYTE_ERROR_VALUE)		
				{
					isBoilerStartScheduled = true;
					if (currentDateTime.year != 0)                                  // Checking if current time been already set or having initial value
					{
						if (currentDateTime.hour >= boilerScheduledStartHour)
						{
							increaseDateTimeByOneDay(currentDateTime, boilerScheduledStartDateTime);
							boilerScheduledStartDateTime.hour = boilerScheduledStartHour;
							boilerScheduledStartDateTime.minute = 0;
						}
						else
						{
							boilerScheduledStartDateTime = { currentDateTime.year, currentDateTime.month, currentDateTime.day, boilerScheduledStartHour, 0 };
						}
					}
					else
					{
						boilerScheduledStartDateTime = { 0, 0, 0, boilerScheduledStartHour, 0 };
					}
					sendSmsResponse(smsSenderIndex, COMMAND_ON_SCHEDULED);
				} else
				{
					isBoilerStartScheduled = false;
					boiler->turnOn();
					sendSmsResponse(smsSenderIndex, COMMAND_ON);
				}
			}
			if (isCharArraysEqual(buffer, offCommand, 51, 3))
			{
				boiler->turnOff();
				sendSmsResponse(smsSenderIndex, COMMAND_OFF);
			}
			if (isCharArraysEqual(buffer, statusCommand, 51, 4)) { sendSmsResponse(smsSenderIndex, COMMAND_STATUS); }
		}
	}
}

void Sim900::clearBuffer()
{
	for (int i = 0; i < BUFFER_SIZE; i++)     { buffer[i] = NULL; }
}

boolean Sim900::isCharArraysEqual(char* array1, char* array2, byte startIndexOfArray1, byte length)
{
	for (byte i = startIndexOfArray1, k = 0; k < length; i++, k++)
	{
		if (array1[i] != array2[k]) { return false; }
	}
	return true;
}

byte Sim900::readScheduledTimeToByte()
{
	byte returnValue = 0;
	for (int i = 53; i < 55; i++) { if (buffer[i] < 48 || buffer[i] > 57) { return BYTE_ERROR_VALUE; } }
	returnValue = (buffer[53] - 48) * 10 + buffer[54] - 48;
	if (returnValue > 23) { return BYTE_ERROR_VALUE; }
	return returnValue;
}


// Will return index from allowedPhoneNumbers[] or 255 if number not found in allowedPhoneNumbers[]
byte Sim900::isSmsSenderPhoneNumberAllowed()
{
	for (byte i = 0; i < NUMBER_OF_ALLOWED_PHONE_NUMBERS; i++)
	{
		if (isCharArraysEqual(buffer, allowedPhoneNumbers[i], 9, 13)) { return i; }
	}
	return 255;
}

void Sim900::stringToLowerCase(char* array, byte startingIndex, byte length)
{
	for (int i = startingIndex; i < startingIndex + length; i++)
	{
		if (array[i] >= 65 && array[i] <= 90) { array[i] += 32; }
	}
}

void Sim900::sendSmsResponse(byte indexOfSmsSender, byte command)
{
	gsmSerial->print(F("AT+CMGS=\""));
	gsmSerial->print(allowedPhoneNumbers[indexOfSmsSender]);
	gsmSerial->println(F("\""));
	delay(100);
	gsmSerial->print(F("KOMANDA=\""));
	switch (command)
	{
		case COMMAND_ON: gsmSerial->print("ON"); break;
		case COMMAND_ON_SCHEDULED:
		{
			gsmSerial->print("ON");
			gsmSerial->print(boilerScheduledStartDateTime.hour);
			break;
		}
		case COMMAND_OFF: gsmSerial->print("OFF"); break;
		case COMMAND_STATUS: gsmSerial->print("STAT"); break;
	}
	gsmSerial->print(F("\"  KOTEL="));
	if (boiler->isOn) { gsmSerial->print(F("ON")); }
	else { gsmSerial->print(F("OFF")); }
	gsmSerial->print(F("  Tkotel="));
	if (boilerSensor->isWorking) { gsmSerial->print(boilerSensor->temperatureValue); }
	else { gsmSerial->print(F("ERR")); }
	gsmSerial->print(F("  NASOS="));
	if (pump->isOn) { gsmSerial->print(F("ON")); }
	else { gsmSerial->print(F("OFF")); }
	gsmSerial->print(F("  Tkimnata="));
	if (roomSensor->isWorking) { gsmSerial->print(roomSensor->temperatureValue); }
	else { gsmSerial->print(F("ERR")); }
	gsmSerial->print(F("  START=\""));
	char startTimeBuffer[12];
	for (byte i = 0; i < 12; i++) { buffer[i] = 0; }
	sprintf(startTimeBuffer, "%02d/%02d %02d:%02d", startingDateTime.day, startingDateTime.month, startingDateTime.hour, startingDateTime.minute);
	gsmSerial->write(startTimeBuffer, 11);
	gsmSerial->println("\"");
	delay(100);
	gsmSerial->println((char)26);
	delay(100);

}
