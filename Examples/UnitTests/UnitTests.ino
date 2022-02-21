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
using TestUnitShort18 = ShortWearLevelUnit<sizeof(uint8_t), WearLevelShort::x17>;

StorageUnit<1> StorageZero(0);

TestUnitTiny2 Tiny2(StorageZero.GetStartAddress() + StorageZero.GetUsedBlockCount());
TestUnitTiny9 Tiny9(Tiny2.GetStartAddress() + Tiny2.GetUsedBlockCount());
TestUnitShort10 Short10(Tiny9.GetStartAddress() + Tiny9.GetUsedBlockCount());
TestUnitShort18 Short17(Short10.GetStartAddress() + Short10.GetUsedBlockCount());

static const uint16_t TestUnitsSize = Short17.GetStartAddress() + Short17.GetUsedBlockCount();


void loop()
{
}

void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);

	Serial.println();
	Serial.println();
	Serial.println(F("Embedded Storage Unit Test Start"));
	Serial.println();
	Serial.println();

	Serial.print(F("\tTest Units size\t"));
	Serial.print(TestUnitsSize);
	Serial.println(F(" bytes"));

	if (TestUnitsSize > EEPROM_RAM_DATA_SIZE)
	{
		Serial.println(F("\tTest Units don't fit in RAM (fake EEPROM)."));
		return;
	}
	Serial.println();


	TestStorageAttributor();

	TestStorageUnit();

	TestUnitWearLevelTiny();

	TestUnitWearLevelShort();
	Serial.println();
	Serial.println(F("Integration tests passed."));

	//TODO: Test Wear level
	// - counter bit fiddling.
	// - counter rolling.
	// - counter range [0;Option].
	// - counter holes.


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
	Serial.println(F("Testing Storage Unit."));

	const uint8_t testValue = 123;
	uint8_t value = testValue;
	StorageZero.WriteData(&value);

	value = 0;
	if (!StorageZero.ReadData(&value) || value != testValue)
	{
		Serial.println(F("\tStorage Unit invalidated."));
		return;
	}

	Serial.println(F("\tStorage Unit validated."));
}

void TestUnitWearLevelTiny()
{
	Serial.println(F("Testing Wear Level Tiny Unit:"));

	const uint8_t testValue = 123;
	uint8_t value = testValue;
	Tiny2.WriteData(&value);
	Tiny9.WriteData(&value);

	for (uint8_t i = 0; i < WearLevelTiny::x2; i++)
	{
		value = 0;
		if (!Tiny2.ReadData(&value) || value != testValue)
		{
			Serial.println(F("\tWear Level Tiny x2 invalidated."));
			return;
		}
	}


	for (uint8_t i = 0; i < WearLevelTiny::x9; i++)
	{
		value = 0;
		if (!Tiny9.ReadData(&value) || value != testValue)
		{
			Serial.println(F("\tWear Level Tiny x9 invalidated."));
			return;
		}
	}

	Serial.println(F("\tWear Level Tiny validated."));
}

void TestUnitWearLevelShort()
{
	Serial.println(F("Testing Wear Level Short Unit:"));

	const uint8_t testValue = 123;
	uint8_t value = testValue;
	Short10.WriteData(&value);
	Short17.WriteData(&value);

	for (uint8_t i = 0; i < WearLevelShort::x10; i++)
	{
		value = 0;
		if (!Short10.ReadData(&value) || value != testValue)
		{
			Serial.println(F("\tWear Level Short x10 invalidated."));
			return;
		}
	}

	for (uint8_t i = 0; i < WearLevelShort::x17; i++)
	{
		value = 0;
		if (!Short17.ReadData(&value) || value != testValue)
		{
			Serial.println(F("\tWear Level Short x17 invalidated."));
			return;
		}
	}

	Serial.println(F("\tWear Level Short validated."));
}

void TestStorageAttributor()
{
	Serial.println(F("Testing Storage Attributor:"));
	if (!Attributor.Validate())
	{
		Serial.println(F("\tAttributor invalidated."));
		return;
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
	}

	if (Attributor.GetTotalSpace() != EEPROM_RAM_DATA_SIZE)
	{
		Serial.println(F("\tAttributor.GetTotalSpace() failed."));
	}

	if (Attributor.GetTotalSpace() - Attributor.GetUsedSpace() != Attributor.GetFreeSpace())
	{
		Serial.println(F("\tAttributor.GetFreeSpace() failed."));
	}
	Serial.println(F("\tAttributor validated."));
	Serial.println();
}