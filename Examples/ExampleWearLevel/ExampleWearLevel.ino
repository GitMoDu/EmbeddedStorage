/*
	Example Wear Level Storage.
	Uses WearLevelUnit to demonstrate use.
	Also showcases StorageAttributor.
	Type 1 to write a random value to EEPROM, with validation.
	Type 2 to read the current value on EEPROM.
	Type 0 to erase EEPROM (Danger: repeated use may will cause EEPROM wear).

	StorageUnit depends on https://github.com/FrankBoesing/FastCRC.
*/

#define SERIAL_BAUD_RATE 9600


#define WEAR_LEVEL_DEBUG

#include <StorageUnit.h>
#include <WearLevelUnit.h>
#include <StorageAttributor.h>


struct TestStruct1
{
	uint32_t Value1;
	uint32_t Value2;
	uint32_t Value3;
	uint32_t Value4;
	uint64_t Padding1;
	uint64_t Padding2;
	uint64_t Padding3;
	uint64_t Padding4;
} TestData1;

struct TestStruct2
{
	uint32_t Value1;
	int32_t Value2;

} TestData2;


// Define the used Storage units.
enum StorageUnits
{
	Struct1,
	Struct2,
	UnitCount
};

using Struct1Unit = TinyWearLevelUnit<sizeof(TestStruct1), WearLevelTiny::x5>;
using Struct2Unit = ShortWearLevelUnit<sizeof(TestStruct2), WearLevelShort::x17>;


/// <summary>
/// Extend StorageAttributor to fill in the static sizes and count of units.
/// After instatiation, static attributed addresses can be used as a constructor argument for units.
/// </summary>
class ExampleStorageAttributor : public StorageAttributor
{
public:
	ExampleStorageAttributor() : StorageAttributor() {}

protected:
	virtual const uint8_t GetPartitionsCount() { return StorageUnits::UnitCount; }
	virtual const uint16_t GetPartitionSize(const uint8_t partition)
	{
		switch (partition)
		{
		case Struct1:
			return Struct1Unit::GetUsedBlockCount();
		case Struct2:
			return Struct2Unit::GetUsedBlockCount();
		default:
			return 0;
		}
	}
} Attributor;


Struct1Unit EmbeddedStorage(Attributor.GetUnitStartAddress(Struct1));
Struct2Unit EmbeddedStorage2(Attributor.GetUnitStartAddress(Struct2));


void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);

	Serial.println();
	Serial.println();
	Serial.println(F("Example Wear Level Storage Test Start"));


	Serial.println(F("EEPROM"));
	Serial.println(F("Used\tFree\tTotal"));
	Serial.print(Attributor.GetUsedSpace());
	Serial.print('\t');
	Serial.print(Attributor.GetFreeSpace());
	Serial.print('\t');
	Serial.println(Attributor.GetTotalSpace());
	Serial.println();

	Serial.print(F("EEPROM Attributor: "));
	if (Attributor.Validate())
	{
		Serial.println(F("Ok."));
	}
	else
	{
		Serial.println(F("Failed."));
		return;
	}
	Serial.println();

	uint32_t duration = micros();

	if (EmbeddedStorage.ReadData((uint8_t*)&TestData1))
	{
		duration = micros() - duration;
		Serial.print(F("Found valid data on EEPROM (counter: "));
		Serial.print(EmbeddedStorage.DebugCounter());
		Serial.print(F(")\t"));
		Serial.print(TestData1.Value1);
		Serial.print('\t');
		Serial.println(TestData1.Value2);
	}
	else
	{
		duration = micros() - duration;
		Serial.println(F("No valid data on EEPROM."));
	}

	Serial.print(F("Reading TestData1 ("));
	Serial.print(sizeof(TestStruct1));
	Serial.print(F(" bytes) took :\t"));
	Serial.print(duration);
	Serial.println(F(" us"));
}

bool TestValuePending = false;
void loop()
{
	if (TestValuePending)
	{
		TestValuePending = false;
		Serial.print(F("Writing TestData1 to EEPROM:\t"));
		Serial.println(TestData1.Value1);

		uint32_t duration = micros();
		EmbeddedStorage.WriteData((uint8_t*)&TestData1);
		duration = micros() - duration;
		Serial.print(sizeof(TestStruct1));
		Serial.print(F(" bytes took :\t"));
		Serial.print(duration);
		Serial.print(F(" us"));
		Serial.println();

		Serial.print(F("Wrote to counter = "));
		Serial.print(EmbeddedStorage.DebugCounter());
		Serial.println(F(" "));

		if (EmbeddedStorage.ReadData((uint8_t*)&TestData1))
		{
			Serial.println(F("Written data on EEPROM ok."));
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
	uint8_t input = Serial.parseInt();
	switch (input)
	{
	case 0:
		Serial.print(F("Erasing EEPROM..."));
		EmbeddedEEPROM::EraseEEPROM();
		Serial.println(F("Done"));
		Serial.println(F("Storage reinit."));
		EmbeddedStorage.ResetCounter();
		break;
	case 1:
		TestData1.Value1 = random();
		TestData1.Value2 = ~TestData1.Value1;
		TestValuePending = true;
		break;
	case 2:
		Serial.print(F("TestData1 on EEPROM (counter = "));
		Serial.print(EmbeddedStorage.DebugCounter());

		if (EmbeddedStorage.ReadData((uint8_t*)&TestData1))
		{
			Serial.print(F(") is valid:\t"));
			Serial.print(TestData1.Value1);
			Serial.print('\t');
			Serial.println(TestData1.Value2);
		}
		else
		{
			Serial.println(F(") is invalid."));
		}
		break;
	default:
		break;
	}
}