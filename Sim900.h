#ifndef _SIM900_h
#define _SIM900_h

#include "arduino.h"
#include <BigBufferSoftwareSerial.h>
#include <SwitchableDevice.h>
#include <TempSensor.h>

/*
	SIM900 GSM module should be pre-configured:
	1. Baud rate 19200 8-N-1. Usually new modules come with pre-configured
	automatic baud rate detection (up to 115000 baud) that sets baud rate 
	automatically by detecting "AT" (all caps) command.
	AT+IPR=19200
	2. SMS message format is "text"
	AT+CMGF=1
	3. Send contents of new SMS to serial port
	AT+CNMI=2,2,0,0,0
*/

struct DateTime
{
	byte year;
	byte month;
	byte day;
	byte hour;
	byte minute;
};

class Sim900
{
private:
	static const byte BUFFER_SIZE = 128;
	static const byte AT_COMMAND_BUFFER = 128;
	static const long GSM_SIGNAL_LEVEL_CHECK_INTERVAL_MS = 30000;          // each 10 seconds
	static const long RTC_DATE_TIME_CHECK_INTERVAL_MS = 67000;           // each 20 minutes  1200000
	static const long EXPECTED_ANSWER_TIME_MS = 5000;
	static const byte NUMBER_OF_ALLOWED_PHONE_NUMBERS = 6;

	static const byte COMMAND_ON = 10;
	static const byte COMMAND_OFF = 11;
	static const byte COMMAND_STATUS = 12;
							
	static char allowedPhoneNumbers[NUMBER_OF_ALLOWED_PHONE_NUMBERS][14];

	int switchPin;
	BigBufferSoftwareSerial *gsmSerial;
	SwitchableDevice *boiler;
	SwitchableDevice *pump;
	TempSensor *boilerSensor;
	TempSensor *roomSensor;
	TempSensor *unitSensor;
	char buffer[BUFFER_SIZE];
	HardwareSerial *serial;

	boolean isSignalLevelCommandSent;
	unsigned long millisLastTimeSignalChecked;
	boolean isDateTimeCommandSent;
	unsigned long millisLastTimeDateTimeChecked;
	unsigned long millisLastAnswerRecieved;
	DateTime startingDateTime;

	inline void clearBuffer();
	inline boolean isCharArraysEqual(char* array1, char* array2, byte startIndexOfArray1, byte length);
	inline byte isSmsSenderPhoneNumberAllowed();
	inline void stringToLowerCase(char* array, byte startingIndex, byte length);
	void switchState();
	void sendSignalCheckPeriodically();
	void sendDateTimeCheckPeriodically();
	boolean readModuleDataToBufferIfAvailiable();
	void parseBufferAndExecute();
	void sendSmsResponse(byte indexOfSmsSender, byte command);

public:
	DateTime currentDateTime;
	boolean isOn;
	byte signalLevel;

	Sim900(int gsmModuleSwitchPin, BigBufferSoftwareSerial& serialDevice, TempSensor& boilerSensor_, TempSensor& roomSensor_,
		TempSensor& unitSensor_, SwitchableDevice& boilerDevice_, SwitchableDevice& pumpDevice_, HardwareSerial& debuggerSerial);
	
	void run();

};

#endif

