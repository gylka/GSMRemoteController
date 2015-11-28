#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <OneWire.h>

#include <BigBufferSoftwareSerial.h>
#include <TempSensor.h>
#include <ControllerLcd.h>
#include <Button.h>
#include <SwitchableDevice.h>
#include <Sim900.h>


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
const int BOILER_STATUS_EEPROM_ADDRESS = 1;                       // Address in EEPROM where status of boiler is stored  
const int IS_BOILER_START_SCHEDULED_EEPROM_ADDRESS = 2;				
const int BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS = 3;		//  DateTime of boiler's scheduled start takes at least 5 bytes

const byte STATE_ON = 0x20;
const byte STATE_OFF = 0x10;
const int PUMP_TURN_ON_TEMPERATURE = 70;
const int PUMP_TURN_ON_TEMP_HYSTERESIS = 2;

// Global variables
LiquidCrystal lcdDevice(10, 6, 5, 4, 3, 2);
BigBufferSoftwareSerial gsmModuleDevice(7, 8);
OneWire boilerTempDevice(BOILER_TEMP_PIN);
OneWire roomTempDevice(ROOM_TEMP_PIN);
OneWire unitTempDevice(UNIT_TEMP_PIN);

ControllerLcd lcd(lcdDevice);
TempSensor boilerTempSensor(boilerTempDevice, TempSensor::_11BIT_RESOLUTION, Serial);
TempSensor roomTempSensor(roomTempDevice, TempSensor::_11BIT_RESOLUTION, Serial);
TempSensor unitTempSensor(unitTempDevice, TempSensor::_11BIT_RESOLUTION, Serial);
Button boilerButtonOn(BOILER_ON_BTN);
Button boilerButtonOff(BOILER_OFF_BTN);
SwitchableDevice boiler(BOILER_SW_ON);
SwitchableDevice pump(PUMP_SW_ON);
Sim900 gsmModule(GSM_SW_PIN, gsmModuleDevice, boilerTempSensor, roomTempSensor,
	unitTempSensor, boiler, pump, Serial);

void initializePins();
void printshit();
void updateBoilerStateToEeprom();
void processBoilerTempSensor();
void processRoomTempSensor();
void processUnitTempSensor();

void setup() {
	lcd.init();
	lcd.showSplashScreen();
	initializePins();
	Serial.begin(19200);
	gsmModuleDevice.begin(19200);                               // SIM900 module earlier should be configured to 19200 (AT+IPR command)
	delay(1000);
	boiler.applyState(EEPROM.read(BOILER_STATUS_EEPROM_ADDRESS));

	if (EEPROM.read(IS_BOILER_START_SCHEDULED_EEPROM_ADDRESS) == STATE_ON)
	{
		gsmModule.isBoilerStartScheduled = true;
		gsmModule.boilerScheduledStartDateTime.year = EEPROM.read(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS);
		gsmModule.boilerScheduledStartDateTime.month = EEPROM.read(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS + 1);
		gsmModule.boilerScheduledStartDateTime.day = EEPROM.read(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS + 2);
		gsmModule.boilerScheduledStartDateTime.hour = EEPROM.read(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS + 3);
		gsmModule.boilerScheduledStartDateTime.minute = EEPROM.read(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS + 4);
	}
	if (EEPROM.read(IS_BOILER_START_SCHEDULED_EEPROM_ADDRESS) == STATE_OFF) { gsmModule.isBoilerStartScheduled = false; }

	lcd.printBlankTemplate();
}

void loop() {
	if (boilerButtonOn.isButtonPressed()) { boiler.turnOn(); }
	if (boilerButtonOff.isButtonPressed()) { boiler.turnOff(); }
	updateBoilerStateToEeprom();

	lcd.run();
	lcd.printPumpState(pump.isOn);

	processBoilerTempSensor();
	processRoomTempSensor();
	processUnitTempSensor();

	gsmModule.run();
	lcd.printGsmModuleState(gsmModule.isOn);
	lcd.printGsmSignal(gsmModule.signalLevel);
	if (lcd.isScreenRefreshed && gsmModule.startingDateTime.year != 0)
	{
		lcd.printStartTime(gsmModule.startingDateTime.day, gsmModule.startingDateTime.month, gsmModule.startingDateTime.hour, gsmModule.startingDateTime.minute);
		lcd.isScreenRefreshed = false;
	}
//	delay(100);

	// --------------------- FOR DEBUGGING ------------------------
	// Sending everything sent to HardwareSerial to GSM Module
	while(Serial.available())
	{
		gsmModuleDevice.write(Serial.read());
	}
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

void updateBoilerStateToEeprom()
{
	if (boiler.isOn) { EEPROM.update(BOILER_STATUS_EEPROM_ADDRESS, STATE_ON); }
	else { EEPROM.update(BOILER_STATUS_EEPROM_ADDRESS, STATE_OFF); }
	if (gsmModule.isBoilerStartScheduled)
	{
		EEPROM.update(IS_BOILER_START_SCHEDULED_EEPROM_ADDRESS, STATE_ON);
		EEPROM.update(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS, gsmModule.boilerScheduledStartDateTime.year);
		EEPROM.update(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS + 1, gsmModule.boilerScheduledStartDateTime.month);
		EEPROM.update(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS + 2, gsmModule.boilerScheduledStartDateTime.day);
		EEPROM.update(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS + 3, gsmModule.boilerScheduledStartDateTime.hour);
		EEPROM.update(BOILER_SCHEDULED_START_DATE_TIME_EEPROM_ADDRESS + 4, gsmModule.boilerScheduledStartDateTime.minute);
	}
	else { EEPROM.update(IS_BOILER_START_SCHEDULED_EEPROM_ADDRESS, STATE_OFF); }
}

void processBoilerTempSensor()
{
	if (boilerTempSensor.isWorking)
	{
		if (boilerTempSensor.isConvertCalled && boilerTempSensor.isConvertFinished) { boilerTempSensor.updateTemperatureValue(); }
		if (!boilerTempSensor.isConvertCalled) { boilerTempSensor.callConvert(); }
		lcd.printBoilerTemperature(boilerTempSensor.temperatureValue);
		if (boilerTempSensor.temperatureValue > PUMP_TURN_ON_TEMPERATURE) { pump.turnOn(); }
		if (boilerTempSensor.temperatureValue < PUMP_TURN_ON_TEMPERATURE - PUMP_TURN_ON_TEMP_HYSTERESIS) { pump.turnOff(); }
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