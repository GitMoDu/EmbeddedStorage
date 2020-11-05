/*
	Name:
*/

#define SERIAL_BAUD_RATE 115200



#include <EmbeddedStorage.h>


static const uint32_t BaseVersionCode = 56; // Random value < UINT8_MAX.


struct TestStruct
{
	static const uint32_t Key = 5435782; // Random key < UINT32_MAX.
	static void SetDefaultValue(TestStruct& value)
	{
		value.Code = 640;
	}

	uint32_t Code;
	uint32_t Code2;
};


TemplateEmbeddedData<TestStruct> EmbeddedData;

EmbeddedStorage<BaseVersionCode> Storage;

void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);

	uint32_t Start = micros();

	EmbeddedData.Setup(&Storage);

	Storage.Setup();

	Start = micros() - Start;
	Serial.print(F("Setup took "));
	Serial.print(Start);
	Serial.println(F(" us"));


	TestStruct* Data = EmbeddedData.GetData();
	delay(100);

	Serial.print(F("Value on Startup: "));
	Serial.println(Data->Code);
	delay(100);

	Data->Code = 1377;


	Start = micros();
	EmbeddedData.OnDataUpdated();
	Start = micros() - Start;
	Serial.print(F("Update took "));
	Serial.print(Start);
	Serial.println(F(" us"));
}

void loop()
{
}
