/*
	Example Storage.
	Uses StorageUnit to demonstrate use.

	StorageUnit depends on https://github.com/FrankBoesing/FastCRC.
*/

#define SERIAL_BAUD_RATE 9600

#define EEPROM_BOUNDS_CHECK

#include <StorageUnit.h>



struct TestStruct
{
	uint8_t Id;
	uint32_t Value;

	void SetDefaultValue()
	{
		Id = 123;
		Value = random(UINT8_MAX);
	}
} TestData;


StorageUnit<sizeof(TestStruct)> EmbeddedStorage(0);


void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);

	Serial.println();
	Serial.println();
	Serial.println(F("Example Storage Test Start"));

	uint32_t duration = micros();

	if (EmbeddedStorage.ReadData((uint8_t*)&TestData))
	{
		duration = micros() - duration;
		Serial.print(F("Found valid data on EEPROM:"));
		Serial.print('\t');
		Serial.print(TestData.Id);
		Serial.print('\t');
		Serial.println(TestData.Value);
	}
	else
	{
		duration = micros() - duration;
		TestData.SetDefaultValue();
		Serial.println(F("No valid data on EEPROM."));
	}

	Serial.print(F("Reading "));
	Serial.print(sizeof(TestStruct));
	Serial.print(F(" bytes took :\t"));
	Serial.print(duration);
	Serial.println(F(" us"));
}

bool TestValuePending = false;
void loop()
{
	if (TestValuePending)
	{
		TestValuePending = false;
		Serial.print(F("Writing data to EEPROM:\t"));
		Serial.println(TestData.Value);

		uint32_t duration = micros();
		EmbeddedStorage.WriteData((uint8_t*)&TestData);
		duration = micros() - duration;
		Serial.print(F("Writing of "));
		Serial.print(sizeof(TestStruct));
		Serial.print(F(" bytes took :\t"));
		Serial.print(duration);
		Serial.println(F(" us"));


		if (EmbeddedStorage.ReadData((uint8_t*)&TestData))
		{
			Serial.print(F("Written data on EEPROM is valid:\t"));
			Serial.print(TestData.Id);
			Serial.print('\t');
			Serial.println(TestData.Value);
		}
		else
		{
			Serial.println(F("Written data on EEPROM was not correctly read back."));
		}
		Serial.println();
	}
}

void serialEvent()
{
	TestData.Value = Serial.parseInt();
	TestValuePending = true;
}