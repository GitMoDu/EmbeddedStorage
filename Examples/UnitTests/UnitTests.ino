/*
	Library Testing Project.
*/

#define SERIAL_BAUD_RATE 9600


#define EEPROM_RAM_DATA_SIZE 203
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


class MockStorageAttributor : public StorageAttributor
{
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
} MockAttributor;

static const uint16_t MockUnitsSize = StorageUnitSizes::Struct1Size + StorageUnitSizes::Struct2Size + StorageUnitSizes::Struct3Size;


using TestUnitStorage = StorageUnit<1>;
using TestUnitTiny2 = TinyWearLevelUnit<sizeof(uint8_t), WearLevelTiny::x2>;
using TestUnitTiny9 = TinyWearLevelUnit<sizeof(uint16_t), WearLevelTiny::x9>;
using TestUnitShort10 = ShortWearLevelUnit<sizeof(uint8_t), WearLevelShort::x10>;
using TestUnitShort17 = ShortWearLevelUnit<sizeof(uint8_t), WearLevelShort::x17>;
using TestUnitLong18 = LongWearLevelUnit<sizeof(uint8_t), WearLevelLong::x18>;
using TestUnitLong33 = LongWearLevelUnit<sizeof(uint8_t), WearLevelLong::x33>;

TestUnitStorage StorageZero(0);
TestUnitTiny2 Tiny2(StorageZero.GetStartAddress() + StorageZero.GetUsedBlockCount());
TestUnitTiny9 Tiny9(Tiny2.GetStartAddress() + Tiny2.GetUsedBlockCount());
TestUnitShort10 Short10(Tiny9.GetStartAddress() + Tiny9.GetUsedBlockCount());
TestUnitShort17 Short17(Short10.GetStartAddress() + Short10.GetUsedBlockCount());
TestUnitLong18 Long18(Short17.GetStartAddress() + Short17.GetUsedBlockCount());
TestUnitLong33 Long33(Long18.GetStartAddress() + Long18.GetUsedBlockCount());

static const uint16_t TestUnitsSize = Long33.GetStartAddress() + Long33.GetUsedBlockCount();

class TestUnitsStorageAttributor : public StorageAttributor
{
protected:
	virtual const uint8_t GetPartitionsCount() { return 7; }
	virtual const uint16_t GetPartitionSize(const uint8_t partition)
	{
		switch (partition)
		{
		case 0:
			return TestUnitStorage::GetUsedBlockCount();
		case 1:
			return TestUnitTiny2::GetUsedBlockCount();
		case 2:
			return TestUnitTiny9::GetUsedBlockCount();
		case 3:
			return TestUnitShort10::GetUsedBlockCount();
		case 4:
			return TestUnitShort17::GetUsedBlockCount();
		case 5:
			return TestUnitLong18::GetUsedBlockCount();
		case 6:
			return TestUnitLong33::GetUsedBlockCount();
		default:
			return 0;
		}
	}
} TestAttributor;



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
	delay(1000);
	Serial.begin(SERIAL_BAUD_RATE);

	Serial.println();
	Serial.println();
	Serial.println(F("Embedded Storage Unit Test Start"));
	Serial.println();

	if (TestUnitsSize > EEPROM_SIZE)
	{
		Serial.println(F("\tTest Units don't fit in RAM (fake EEPROM)."));
		Serial.print(F("\tTest Units size\t"));
		Serial.print(TestUnitsSize);
		Serial.println(F(" bytes"));
		OnFail();
	}
	Serial.println();


	TestStorageAttributor("Mock", MockAttributor, MockUnitsSize);

	TestStorageAttributor("TestUnits", TestAttributor, TestUnitsSize);

	TestStorageUnit();

	TestUnitWearGeneric<TestUnitTiny2>("Tiny2", Tiny2);
	TestUnitWearGeneric<TestUnitTiny9>("Tiny9", Tiny9);

	TestUnitWearGeneric<TestUnitShort10>("Short10", Short10);
	TestUnitWearGeneric<TestUnitShort17>("Short17", Short17);

	TestUnitWearGeneric<TestUnitLong18>("Long18", Long18);
	TestUnitWearGeneric<TestUnitLong33>("Long33", Long33);

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
void PrintWearMask(String label, const uint8_t size, UnitType unit)
{
	const uint64_t mask = unit.DebugMask();
	Serial.print('\t');
	Serial.print(label);
	Serial.print(F(" 0b"));
	for (int8_t i = (size * 8) - 1; i >= 0; i--)
	{
		if (mask & ((uint64_t)1 << i))
		{
			Serial.print('1');
		}
		else
		{
			Serial.print('0');
		}

	}
	Serial.println();
}

template<class UnitType>
void TestUnitWearGeneric(String name, UnitType unit)
{
	const uint8_t option = unit.DebugOption();
	Serial.print(F("Testing Wear Level "));
	Serial.print(name);
	Serial.println(F(" Unit:"));

	EmbeddedEEPROM::EraseEEPROM();
	PrintWearMask<UnitType>("Erase", unit.GetCounterSize(), unit);
	if (unit.DebugCounter() != 0)
	{
		Serial.print(F("\tCounter erased EEPROM invalidated: "));
		Serial.println(unit.DebugCounter());
		OnFail();
	}

#if defined(EEPROM_RAM_DATA_SIZE)
	FakeEEPROM[unit.GetStartAddress()] = (1 << 4);
#else
	EEPROM.update(unit.GetStartAddress(), (1 << 4));
#endif
	PrintWearMask<UnitType>("Corrupt", unit.GetCounterSize(), unit);
	unit.DebugInitialize();
	PrintWearMask<UnitType>("Initialized", unit.GetCounterSize(), unit);
	if (unit.DebugCounter() != (option - 1))
	{
		Serial.print(F("\tCounter initialize invalidated: "));
		Serial.println(unit.DebugCounter());
		OnFail();
	}

	unit.ResetCounter();
	PrintWearMask<UnitType>("Reset", unit.GetCounterSize(), unit);
	if (unit.DebugCounter() != (option - 1))
	{
		Serial.print(F("\tCounter reset invalidated: "));
		Serial.println(unit.DebugCounter());
		OnFail();
	}

	const uint8_t testValue = 123;
	uint8_t value = 0;

	for (uint8_t i = 0; i < option; i++)
	{
		value = testValue + i;
		unit.WriteData(&value);
		Serial.print(F("\t"));
		Serial.print(F("\t"));
		Serial.print(i);
		PrintWearMask<UnitType>("", unit.GetCounterSize(), unit);

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
	PrintWearMask<UnitType>("End", unit.GetCounterSize(), unit);
	if (unit.DebugCounter() != 0)
	{
		Serial.print(F("\tCounter end invalidated:"));
		Serial.print(unit.DebugCounter());
		OnFail();
	}

	Serial.println(F("\tValidated."));
}

void TestStorageAttributor(String name, StorageAttributor& at, const uint16_t expectedSize)
{
	Serial.print(F("Testing "));
	Serial.print(name);
	Serial.println(F(" Storage Attributor:"));
	if (!at.Validate())
	{
		Serial.print('\t');
		Serial.print(name);
		Serial.println(F(" Attributor invalidated."));
		OnFail();
	}

	Serial.println(F("\tUsed\tFree\tTotal"));
	Serial.print('\t');
	Serial.print(at.GetUsedSpace());
	Serial.print('\t');
	Serial.print(at.GetFreeSpace());
	Serial.print('\t');
	Serial.println(at.GetTotalSpace());

	if (at.GetUsedSpace() != expectedSize)
	{
		Serial.print('\t');
		Serial.print(name);
		Serial.println(F(" Attributor.GetUsedSpace() failed."));
		OnFail();
	}

	if (at.GetTotalSpace() != EEPROM_SIZE)
	{
		Serial.print('\t');
		Serial.print(name);
		Serial.println(F(" Attributor.GetTotalSpace() failed."));
		OnFail();
	}

	if (at.GetTotalSpace() - at.GetUsedSpace() != at.GetFreeSpace())
	{
		Serial.print('\t');
		Serial.print(name);
		Serial.println(F(" Attributor.GetFreeSpace() failed."));
		OnFail();
	}
	Serial.println(F("\tValidated."));
	Serial.println();
}