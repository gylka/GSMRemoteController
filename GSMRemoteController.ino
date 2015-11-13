#include "Sim900.h"
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <OneWire.h>

#include <TempSensor.h>
#include <ControllerLcd.h>
#include <Button.h>
#include <SwitchableDevice.h>


// Pins
#define GSM_SW_PIN         9
#define BOILER_TEMP_PIN    11
#define ROOM_TEMP_PIN      12
#define UNIT_TEMP_PIN      13
#define BOILER_SW_ON       A0
#define PUMP_SW_ON         A1
#define BOILER_ON_BTN      A2
#define BOILER_OFF_BTN     A3

// Constants
const int BOILER_STATUS_EEPROM_ADDRESS = 7;                       // Address in EEPROM where status of boiler is stored  
const byte STATE_ON = 0x20;
const byte STATE_OFF = 0x10;
const int PUMP_TURN_ON_TEMPERATURE = 70;
const int PUMP_TURN_ON_HYSTERESIS = 2;

// Global variables
LiquidCrystal lcdDevice(10, 6, 5, 4, 3, 2);
SoftwareSerial gsmModuleDevice(7, 8);
OneWire boilerTempDevice(BOILER_TEMP_PIN);
OneWire roomTempDevice(ROOM_TEMP_PIN);
OneWire unitTempDevice(UNIT_TEMP_PIN);

ControllerLcd lcd(lcdDevice);
TempSensor boilerTempSensor(boilerTempDevice, TempSensor::_10BIT_RESOLUTION, Serial);
TempSensor roomTempSensor(roomTempDevice, TempSensor::_10BIT_RESOLUTION, Serial);
TempSensor unitTempSensor(unitTempDevice, TempSensor::_10BIT_RESOLUTION, Serial);
Button boilerButtonOn(BOILER_ON_BTN, Serial);
Button boilerButtonOff(BOILER_OFF_BTN, Serial);
SwitchableDevice boiler(BOILER_SW_ON);
SwitchableDevice pump(PUMP_SW_ON);

void initializePins();
void printshit();
void updateBoilderStateToEeprom();
void processBoilerTempSensor();
void processRoomTempSensor();
void processUnitTempSensor();

void setup() {
	lcd.init();
	lcd.showSplashScreen();
	initializePins();
	Serial.begin(19200);
	//	gsmModule.begin(19200);                               // SIM900 module earlier should be configured to 19200 (AT+IPR command)
	delay(1000);
	boiler.applyState(EEPROM.read(BOILER_STATUS_EEPROM_ADDRESS));
	lcd.printBlankTemplate();

}

void loop() {
	if (boilerButtonOn.isButtonPressed()) { boiler.turnOn(); }
	if (boilerButtonOff.isButtonPressed()) { boiler.turnOff(); }
	updateBoilderStateToEeprom();
	lcd.printPumpState(pump.isOn);

	processBoilerTempSensor();
	processRoomTempSensor();
	processUnitTempSensor();

}

void initializePins() {
	pinMode(GSM_SW_PIN, OUTPUT);
	digitalWrite(GSM_SW_PIN, LOW);
	pinMode(BOILER_SW_ON, OUTPUT);
	digitalWrite(BOILER_SW_ON, LOW);
	pinMode(PUMP_SW_ON, OUTPUT);
	digitalWrite(PUMP_SW_ON, LOW);
	pinMode(BOILER_ON_BTN, INPUT);
	pinMode(BOILER_OFF_BTN, INPUT);
}

void updateBoilderStateToEeprom()
{
	if (boiler.isOn) { EEPROM.update(BOILER_STATUS_EEPROM_ADDRESS, STATE_ON); }
	else { EEPROM.update(BOILER_STATUS_EEPROM_ADDRESS, STATE_OFF); }
}

void processBoilerTempSensor()
{
	if (boilerTempSensor.isWorking)
	{
		if (boilerTempSensor.isConvertCalled && boilerTempSensor.isConvertFinished) { boilerTempSensor.updateTemperatureValue(); }
		if (!boilerTempSensor.isConvertCalled) { boilerTempSensor.callConvert(); }
		lcd.printBoilerTemperature(boilerTempSensor.temperatureValue);
		if (boilerTempSensor.temperatureValue > PUMP_TURN_ON_TEMPERATURE) { pump.turnOn(); }
		if (boilerTempSensor.temperatureValue < PUMP_TURN_ON_TEMPERATURE - PUMP_TURN_ON_HYSTERESIS) { pump.turnOff(); }
	}
	else
	{
		if (boilerTempSensor.readRomCode()) { boilerTempSensor.setResolution(*boilerTempSensor.resolution); }
		lcd.printBoilerTemperature(TempSensor::ERROR_TEMPERATURE_VALUE);
		pump.turnOff();
	}
	boilerTempSensor.updateFlags();
}

void processRoomTempSensor()
{
	if (roomTempSensor.isWorking)
	{
		if (roomTempSensor.isConvertCalled && roomTempSensor.isConvertFinished) { roomTempSensor.updateTemperatureValue(); }
		if (!roomTempSensor.isConvertCalled) { roomTempSensor.callConvert(); }
		lcd.printRoomTemperature(roomTempSensor.temperatureValue);
	}
	else
	{
		if (roomTempSensor.readRomCode()) { roomTempSensor.setResolution(*roomTempSensor.resolution); }
		lcd.printRoomTemperature(TempSensor::ERROR_TEMPERATURE_VALUE);
	}
	roomTempSensor.updateFlags();
}

void processUnitTempSensor()
{
	if (unitTempSensor.isWorking)
	{
		if (unitTempSensor.isConvertCalled && unitTempSensor.isConvertFinished) { unitTempSensor.updateTemperatureValue(); }
		if (!unitTempSensor.isConvertCalled) { unitTempSensor.callConvert(); }
		lcd.printUnitTemperature(unitTempSensor.temperatureValue);
	}
	else
	{
		if (unitTempSensor.readRomCode()) { unitTempSensor.setResolution(*unitTempSensor.resolution); }
		lcd.printUnitTemperature(TempSensor::ERROR_TEMPERATURE_VALUE);
	}
	unitTempSensor.updateFlags();
}