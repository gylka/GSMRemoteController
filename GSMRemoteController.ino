#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <OneWire.h>

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
const static byte TEMPERATURE_SENSOR_RESOLUTION = 0b00111111;            // 10-bit resolution (6th and 7th bit define 9..12-bit resolution)
const static int BOILER_STATUS_EEPROM_ADDRESS = 0;                       // Address in EEPROM where status of boiler stores (1 - ON, 0 - OFF)  
const static int DEBOUNCE_DELAY_MS = 100;                                // Minimal time (milliseconds) the button should be pressed to change value

// Global variables
LiquidCrystal lcd(10, 6, 5, 4, 3, 2);
SoftwareSerial gsmModule(7, 8);
OneWire boilerTempSensor(BOILER_TEMP_PIN);
OneWire roomTempSensor(ROOM_TEMP_PIN);
OneWire unitTempSensor(UNIT_TEMP_PIN);

byte boilerTempSensorRomCode[8];
byte roomTempSensorRomCode[8];
byte unitTempSensorRomCode[8];
float boilerTemperature;
float roomTemperature;
float unitTemerature;  
boolean isBoilerTemperatureSensorPresent;
boolean isRoomTemperatureSensorPresent;
boolean isUnitTemperatureSensorPresent;


boolean isBoilerOn, isPumpOn;

boolean isBoilerOnButtonPreviousStateIsOn, isBoilerOffButtonPreviousStateIsOn;
unsigned long boilerOnButtonPressedTime, boilerOffButtonPressedTime;

void setup() {
	lcd.begin(20, 4);
	lcd.noCursor();
	lcd.setCursor(0,0);
	lcd.print(F("Initializing device"));
	lcd.setCursor(3,2);
	lcd.print(F("please wait..."));
	initializePins();
	initializeTemperatureSensors();
	loadBoilerStateFromEeprom();
	Serial.begin(19200);
	gsmModule.begin(19200);                               // SIM900 module earlier should be configured to 19200 (AT+IPR command)
	
}

void loop() {
	lcd.clear();
	lcd.setCursor(0,0);
	
	
	// Executing "Boiler ON Button" routine and getting rid of button press debouncing
	if (digitalRead(BOILER_ON_BTN) && !isBoilerOn) {
		if ( !isBoilerOnButtonPreviousStateIsOn) {
			isBoilerOnButtonPreviousStateIsOn = true;
			boilerOnButtonPressedTime = millis();
		} else {
			if ( (millis() - boilerOnButtonPressedTime) > DEBOUNCE_DELAY_MS) {
				isBoilerOn = true;
				updateBoilerStateToEeprom();
			}
		}
	} else {
		isBoilerOnButtonPreviousStateIsOn = false;
	}
	
	// Executing "Boiler OFF Button" routine and getting rid of button press debouncing
	if (digitalRead(BOILER_OFF_BTN) && isBoilerOn) {
		if ( !isBoilerOffButtonPreviousStateIsOn) {
			isBoilerOffButtonPreviousStateIsOn = true;
			boilerOffButtonPressedTime = millis();
		} else {
			if ( (millis() - boilerOffButtonPressedTime) > DEBOUNCE_DELAY_MS) {
				isBoilerOn = false;
				updateBoilerStateToEeprom();
			}
		}
	}  else {
		isBoilerOffButtonPreviousStateIsOn = false;
	}
	
	// Switching boiler and pump relays
	if (isBoilerOn) {
		digitalWrite(BOILER_SW_ON, HIGH);
	} else {
		digitalWrite(BOILER_SW_ON, LOW);
	}
	if (isPumpOn) {
		digitalWrite(PUMP_SW_ON, HIGH);
	} else {
		digitalWrite(PUMP_SW_ON, LOW);
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

void initializeTemperatureSensors() {
	if (readTemperatureSensorRomCode(boilerTempSensor, boilerTempSensorRomCode)) {
		isBoilerTemperatureSensorPresent = true;
		setTemperatureSensorResolution(boilerTempSensor, boilerTempSensorRomCode, TEMPERATURE_SENSOR_RESOLUTION);
	} else {
		isBoilerTemperatureSensorPresent = false;
	}
	if (readTemperatureSensorRomCode(roomTempSensor, roomTempSensorRomCode)) {
		isRoomTemperatureSensorPresent = true;
		setTemperatureSensorResolution(roomTempSensor, roomTempSensorRomCode, TEMPERATURE_SENSOR_RESOLUTION);
		} else {
		isRoomTemperatureSensorPresent = false;
	}
	if (readTemperatureSensorRomCode(unitTempSensor, unitTempSensorRomCode)) {
		isUnitTemperatureSensorPresent = true;
		setTemperatureSensorResolution(unitTempSensor, unitTempSensorRomCode, TEMPERATURE_SENSOR_RESOLUTION);
		} else {
		isUnitTemperatureSensorPresent = false;
	}
}

void initializeGSMModule() {
	turnOnGsmModule(GSM_SW_PIN);
}

void loadBoilerStateFromEeprom() {
	isBoilerOn = (boolean)EEPROM.read(BOILER_STATUS_EEPROM_ADDRESS);
}

void updateBoilerStateToEeprom() {
	EEPROM.update(BOILER_STATUS_EEPROM_ADDRESS, (byte)isBoilerOn);
}
