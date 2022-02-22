/*
	Library Testing Project.
*/

#define SERIAL_BAUD_RATE 9600


#define EEPROM_RAM_DATA_SIZE 200
#define EEPROM_BOUNDS_CHECK
#define WEAR_LEVEL_DEBUG

#include <StorageUnit.h>
#include <WearLevelUnit.h>
#include <StorageAttributor.h>



struct Struct
{
	uint32_t Value;
} TestData;

enum StorageUnits
{
	Struct1,
	Struct2,
	Struct3,
	UnitCount
};

enum StorageUnitSizes
{
	Struct1Size = 3,
	Struct2Size = 4,
	Struct3Size = 10
};


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
			return StorageUnitSizes::Struct1Size;
		case Struct2:
			return StorageUnitSizes::Struct2Size;
		case Struct3:
			return StorageUnitSizes::Struct3Size;
		default:
			return 0;
		}
	}
} Attributor;


using TestUnitTiny2 = TinyWearLevelUnit<sizeof(uint8_t), WearLevelTiny::x2>;
using TestUnitTiny9 = TinyWearLevelUnit<sizeof(uint16_t), WearLevelTiny::x9>;
using TestUnitShort10 = ShortWearLevelUnit<sizeof(uint8_t), WearLevelShort::x10>;
using TestUnitShort17 = ShortWearLevelUnit<sizeof(uint8_t), WearLevelShort::x17>;

StorageUnit<1> StorageZero(0);

TestUnitTiny2 Tiny2(StorageZero.GetStartAddress() + StorageZero.GetUsedBlockCount());
TestUnitTiny9 Tiny9(Tiny2.GetStartAddress() + Tiny2.GetUsedBlockCount());
TestUnitShort10 Short10(Tiny9.GetStartAddress() + Tiny9.GetUsedBlockCount());
TestUnitShort17 Short17(Short10.GetStartAddress() + Short10.GetUsedBlockCount());

static const uint16_t TestUnitsSize = Short17.GetStartAddress() + Short17.GetUsedBlockCount();


void loop()
{
}

void OnFail()
{
	Serial.println();
	Serial.println();
	Serial.println();
	Serial.println();
	Serial.println(F("--------------------"));

#if defined(EEPROM_RAM_DATA_SIZE)
	Serial.println(F("Erasing EEPROM."));
	EmbeddedEEPROM::EraseEEPROM();
#endif
	while (true);;
}

void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);

	Serial.println();
	Serial.println();
	Serial.println(F("Embedded Storage Unit Test Start"));
	Serial.print(F("\tTest Units size\t"));
	Serial.print(TestUnitsSize);
	Serial.println(F(" bytes"));

	if (TestUnitsSize > EEPROM_RAM_DATA_SIZE)
	{
		Serial.println(F("\tTest Units don't fit in RAM (fake EEPROM)."));
		OnFail();
	}
	Serial.println();


	TestStorageAttributor();

	TestStorageUnit();

	TestUnitWearGeneric<TestUnitTiny2>("Tiny2", WearLevelTiny::x2, Tiny2);
	TestUnitWearGeneric<TestUnitTiny9>("Tiny9", WearLevelTiny::x9, Tiny9);

	TestUnitWearGeneric<TestUnitShort10>("Short10", WearLevelShort::x10, Short10);
	TestUnitWearGeneric<TestUnitShort17>("Short17", WearLevelShort::x17, Short17);

	Serial.println();
	Serial.println();
	Serial.println(F("All tests passed."));

	Serial.println();
	Serial.println();
	Serial.println(F(""));
	Serial.println(F("     :"));
	Serial.println(F(" '.  _  .'"));
	Serial.println(F("-=  (~)  =-"));
	Serial.println(F(" .'  #  '."));
	Serial.println();
	Serial.println();
}

void TestStorageUnit()
{
	Serial.println(F("Testing Storage Unit:"));

	EmbeddedEEPROM::EraseEEPROM();

	const uint8_t testValue = 123;
	uint8_t value = testValue;
	StorageZero.WriteData(&value);

	value = 0;
	if (!StorageZero.ReadData(&value) || value != testValue)
	{
		Serial.println(F("\tStorage Unit invalidated."));
		OnFail();
	}

	Serial.println(F("\tValidated."));
}


template<class UnitType>
void TestUnitWearGeneric(String name, const uint8_t option, UnitType unit)
{
	Serial.print(F("Testing Wear Level "));
	Serial.print(name);
	Serial.println(F(" Unit:"));

	EmbeddedEEPROM::EraseEEPROM();
	if (unit.DebugCounter() != 0)
	{
		Serial.print(F("\tCounter erased EEPROM invalidated: "));
		Serial.println(unit.DebugCounter());
		OnFail();
	}

	unit.ResetCounter();
	if (unit.DebugCounter() != (option - 1))
	{
		Serial.print(F("\tCounter initialize invalidated: "));
		Serial.println(unit.DebugCounter());
		OnFail();
	}

	const uint8_t testValue = 123;
	uint8_t value = 0;

	for (uint8_t i = 0; i < option; i++)
	{
		value = testValue + i;
		unit.WriteData(&value);

		if (unit.DebugCounter() != i)
		{
			Serial.print(F("\tCounter iterator ("));
			Serial.print(i);
			Serial.print(F(" != "));
			Serial.print(unit.DebugCounter());
			Serial.println(F(") invalidated."));
			OnFail();
		}

		value = 0;
		if (!unit.ReadData(&value) || value != (testValue + i))
		{
			Serial.println(F("\tReadback invalidated."));
			OnFail();
		}
	}

	// One more write to force to counter to cycle back to zero.
	unit.WriteData(&value);
	if (unit.DebugCounter() != 0)
	{
		Serial.print(F("\tCounter end invalidated:"));
		Serial.print(unit.DebugCounter());
		OnFail();
	}

	Serial.println(F("\tValidated."));
}

void TestStorageAttributor()
{
	Serial.println(F("Testing Storage Attributor:"));
	if (!Attributor.Validate())
	{
		Serial.println(F("\tAttributor invalidated."));
		OnFail();
	}

	Serial.println(F("\tUsed\tFree\tTotal"));
	Serial.print('\t');
	Serial.print(Attributor.GetUsedSpace());
	Serial.print('\t');
	Serial.print(Attributor.GetFreeSpace());
	Serial.print('\t');
	Serial.println(Attributor.GetTotalSpace());

	if (Attributor.GetUsedSpace() != (StorageUnitSizes::Struct1Size + StorageUnitSizes::Struct2Size + StorageUnitSizes::Struct3Size))
	{
		Serial.println(F("\tAttributor.GetUsedSpace() failed."));
		OnFail();
	}

	if (Attributor.GetTotalSpace() != EEPROM_SIZE())
	{
		Serial.println(F("\tAttributor.GetTotalSpace() failed."));
		OnFail();
	}

	if (Attributor.GetTotalSpace() - Attributor.GetUsedSpace() != Attributor.GetFreeSpace())
	{
		Serial.println(F("\tAttributor.GetFreeSpace() failed."));
		OnFail();
	}
	Serial.println(F("\tValidated."));
	Serial.println();
}