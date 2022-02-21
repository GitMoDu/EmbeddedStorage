/*
	Example Wear Level Storage.
	Uses WearLevelUnit to demonstrate use.
	Also showcases StorageAttributor.
	StorageUnit depends on https://github.com/FrankBoesing/FastCRC.
*/

#define SERIAL_BAUD_RATE 9600


#define EEPROM_BOUNDS_CHECK
#define WEAR_LEVEL_DEBUG

#include <WearLevelUnit.h>
#include <StorageAttributor.h>



struct TestStruct1
{
	uint32_t Value1;
	int32_t Value2;

	void SetDefaultValue()
	{
		Value1 = 123;
		Value2 = ~Value1;
	}
} TestData;

struct TestStruct2
{
	uint32_t Value;

	void SetDefaultValue()
	{
		Value = 312;
	}
} TestData2;



// Define the used Storage units.
enum StorageUnits
{
	Struct1 = 0,
	Struct2 = 1,
};
using Struct1ShortWearLevelUnit = ShortWearLevelUnit<sizeof(TestStruct1)>;
using Struct2LongWearLevelUnit = ShortWearLevelUnit<sizeof(TestStruct2)>;


class ExampleStorageAttributor : public StorageAttributor
{
public:
	ExampleStorageAttributor() : StorageAttributor() {}

protected:
	virtual const uint8_t GetPartitionsCount() { return 2; }
	virtual const uint16_t GetPartitionSize(const uint8_t partition)
	{
		switch (partition)
		{
		case Struct1:
			return Struct1ShortWearLevelUnit::GetUsedBlockCount();
		case Struct2:
			return Struct2LongWearLevelUnit::GetUsedBlockCount();
		default:
			return 0;
		}
	}
} Attributor;


Struct1ShortWearLevelUnit EmbeddedStorage(Attributor.GetUnitStartAddress(Struct1));
Struct2LongWearLevelUnit EmbeddedStorage2(Attributor.GetUnitStartAddress(Struct2));


void setup()
{


	Serial.begin(SERIAL_BAUD_RATE);

	Serial.println();
	Serial.println();
	Serial.println(F("Example Storage Test Start"));

	//Serial.print(F("Erasing EEPROM..."));
	//EmbeddedEEPROM::EraseEEPROM();
	//Serial.println(F("Done"));
	//return;

	uint32_t duration = micros();

	if (EmbeddedStorage.ReadData((uint8_t*)&TestData))
	{
		duration = micros() - duration;
		Serial.print(F("Found valid data on EEPROM (counter: "));
		Serial.print(EmbeddedStorage.DebugCounter());
		Serial.print(F(")\t"));
		Serial.print(TestData.Value1);
		Serial.print('\t');
		Serial.println(TestData.Value2);
	}
	else
	{
		TestData.SetDefaultValue();
		duration = micros() - duration;
		Serial.println(F("No valid data on EEPROM."));
	}

	Serial.print(F("Reading "));
	Serial.print(sizeof(TestStruct1));
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
		Serial.println(TestData.Value1);

		uint32_t duration = micros();
		EmbeddedStorage.WriteData((uint8_t*)&TestData);
		duration = micros() - duration;
		Serial.print(F("Writing (counter: "));
		Serial.print(EmbeddedStorage.DebugCounter());
		Serial.print(F(") "));
		Serial.print(sizeof(TestStruct1));
		Serial.print(F(" bytes took :\t"));
		Serial.print(duration);
		Serial.println(F(" us"));

		if (EmbeddedStorage.ReadData((uint8_t*)&TestData))
		{
			Serial.print(F("Written data on EEPROM is valid:\t"));
			Serial.print(TestData.Value1);
			Serial.print('\t');
			Serial.println(TestData.Value2);
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
	TestData.Value1 = Serial.parseInt();
	TestData.Value2 = ~TestData.Value1;

	TestValuePending = true;

}