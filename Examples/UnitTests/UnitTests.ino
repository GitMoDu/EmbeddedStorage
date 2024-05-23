/*
	Library Unit Testing Project.
*/

#define SERIAL_BAUD_RATE 9600

#define EEPROM_BOUNDS_CHECK
#define WEAR_LEVEL_DEBUG
//#define EEPROM_MOCK_IN_MEMORY

#include <StorageUnit.h>
#include <WearLevelUnit.h>
#include <StorageAttributor.h>

struct Storage1Definition
{
	struct Struct
	{
		uint8_t Value;
	};

	static constexpr uint16_t Size = sizeof(Struct);
	static constexpr uint32_t Key = 100001;
	static constexpr NoWearLevel WearLevelOption = NoWearLevel::x1;
	static constexpr uint16_t EeepromSize = EmbeddedStorage::GetStorageSize(Size /*,WearLevelOption*/);
};

struct Storage2Definition
{
	struct Struct
	{
		uint16_t Value;
	};

	static constexpr uint16_t Size = sizeof(Struct);
	static constexpr uint32_t Key = 100002;
	static constexpr WearLevelTiny WearLevelOption = WearLevelTiny::x5;
	static constexpr uint16_t EeepromSize = EmbeddedStorage::GetStorageSize(Size, WearLevelOption);
};

struct Storage3Definition
{
	struct Struct
	{
		uint32_t Value;
	};

	static constexpr uint16_t Size = sizeof(Struct);
	static constexpr uint32_t Key = 100003;
	static constexpr WearLevelShort WearLevelOption = WearLevelShort::x10;
	static constexpr uint16_t EeepromSize = EmbeddedStorage::GetStorageSize(Size, WearLevelOption);
};

using StructsSizeAttributor = TemplateSizeAttributor<Storage1Definition::EeepromSize, Storage2Definition::EeepromSize, Storage3Definition::EeepromSize>;
using StructsAttributor = TemplateStorageAttributor<Storage1Definition, Storage2Definition, Storage3Definition>;

using TestUnitStorage = StorageUnit<0, sizeof(Storage1Definition::Struct)>;
using TestUnitTiny5 = TinyWearLevelUnit<TestUnitStorage::Address() + TestUnitStorage::Size(), sizeof(Storage2Definition::Struct), Storage2Definition::WearLevelOption>;
using TestUnitShort10 = ShortWearLevelUnit<TestUnitTiny5::Address() + TestUnitTiny5::Size(), sizeof(Storage3Definition::Struct), Storage3Definition::WearLevelOption>;
static constexpr size_t InlineUsed = TestUnitShort10::Address() + TestUnitShort10::Size();

using TestUnitShort17 = ShortWearLevelUnit<0, sizeof(uint8_t), WearLevelShort::x17>;
using TestUnitLong18 = LongWearLevelUnit<0, sizeof(uint8_t), WearLevelLong::x18>;
using TestUnitLong33 = LongWearLevelUnit<0, sizeof(uint8_t), WearLevelLong::x33>;
using TestUnitLongLong34 = LongLongWearLevelUnit<0, sizeof(uint8_t), WearLevelLongLong::x34>;
using TestUnitLongLong65 = LongLongWearLevelUnit<0, sizeof(uint8_t), WearLevelLongLong::x65>;


void loop()
{
}

void PrintStorage(const uint16_t index, const uint16_t address, const uint16_t size)
{
	Serial.print(F("\tStorage"));
	Serial.print(index + 1);
	Serial.print(F(" @ "));
	Serial.print(address);
	Serial.print(F("\t("));
	Serial.print(size);
	Serial.print(F(" bytes)"));
	Serial.println();
}

void OnFail()
{
	Serial.println();
	Serial.println();
	Serial.println();
	Serial.println();
	Serial.println(F("--------------------"));

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


	Serial.print(F("Attribution ("));
	Serial.print(StructsAttributor::GetUsed());
	Serial.println(F(" bytes)"));
	Serial.println(F("\tInline Size Attribution"));
	PrintStorage(0, 0, TestUnitStorage::Size());
	PrintStorage(1, TestUnitStorage::Address() + TestUnitStorage::Size(), TestUnitTiny5::Size());
	PrintStorage(2, TestUnitTiny5::Address() + TestUnitTiny5::Size(), TestUnitShort10::Size());
	Serial.println();

	Serial.println(F("\tConstexpr Size Attribution"));
	for (size_t i = 0; i < StructsAttributor::GetCount(); i++)
	{
		PrintStorage(i, StructsAttributor::GetAddress(i), StructsAttributor::GetSize(i));
	}

	TestStorageAttributor();
	Serial.println();

	TestStorageUnit<TestUnitStorage>();
#if defined(ARDUINO_AVR_ATTINYX5)
	TestUnitWear<TestUnitTiny5>("Tiny2");
	TestUnitWear<TestUnitShort10>("Short10");
#else
	TestUnitWear<TestUnitTiny5>("Tiny2");
	TestUnitWear<TestUnitShort10>("Short10");
	TestUnitWear<TestUnitShort17>("Short17");
	TestUnitWear<TestUnitLong18>("Long18");
	TestUnitWear<TestUnitLong33>("Long33");
	TestUnitWear<TestUnitLongLong34>("LongLong34");
	TestUnitWear<TestUnitLongLong65>("LongLong65");
#endif

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

void TestStorageAttributor()
{
	if (StructsAttributor::GetCount() != StructsSizeAttributor::GetCount()
		|| StructsAttributor::GetCount() != 3)
	{
		Serial.println(F("Attributor. GetCount() failed."));
		Serial.println(StructsAttributor::GetCount());
		Serial.println(F(" expected "));
		Serial.println(3);
		OnFail();
	}

	if (StructsAttributor::GetUsed() > EmbeddedEEPROM::Size())
	{
		Serial.println(F("Attributor Validate() failed."));
		OnFail();
	}

	if (StructsSizeAttributor::GetUsed() > EmbeddedEEPROM::Size())
	{
		Serial.println(F("SizeAttributor Validate() failed."));
		OnFail();
	}

	if (StructsAttributor::GetUsed() != StructsSizeAttributor::GetUsed())
	{
		Serial.println(F("Attributor. GetUsed() failed."));
		Serial.println(StructsAttributor::GetUsed());
		Serial.println(F(" expected "));
		Serial.println(StructsSizeAttributor::GetUsed());
		OnFail();
	}

	if (StructsAttributor::GetUsed() != InlineUsed)
	{
		Serial.println(F("Attributor. GetUsed() failed."));
		Serial.println(StructsAttributor::GetUsed());
		Serial.println(F(" expected "));
		Serial.println(InlineUsed);
		OnFail();
	}

	for (size_t i = 0; i < StructsAttributor::GetCount(); i++)
	{
		if (StructsAttributor::GetAddress(i) != StructsSizeAttributor::GetAddress(i))
		{
			PrintAddressMismatch(i, StructsAttributor::GetAddress(i), StructsSizeAttributor::GetAddress(i), 0);
			OnFail();
		}

		if (StructsAttributor::GetSize(i) != StructsSizeAttributor::GetSize(i))
		{
			PrintSizeMismatch(i, StructsAttributor::GetSize(i), StructsSizeAttributor::GetSize(i), 0);
			OnFail();
		}
	}

	if (StructsAttributor::GetAddressByKey(Storage1Definition::Key) != StructsSizeAttributor::GetAddress(0))
	{
		PrintAddressMismatch(0, StructsAttributor::GetAddressByKey(Storage1Definition::Key), StructsSizeAttributor::GetAddress(0), 1);
		OnFail();
	}
	if (StructsAttributor::GetAddressByKey(Storage2Definition::Key) != StructsSizeAttributor::GetAddress(1))
	{
		PrintAddressMismatch(1, StructsAttributor::GetAddressByKey(Storage2Definition::Key), StructsSizeAttributor::GetAddress(1), 1);
		OnFail();
	}
	if (StructsAttributor::GetAddressByKey(Storage3Definition::Key) != StructsSizeAttributor::GetAddress(2))
	{
		PrintAddressMismatch(2, StructsAttributor::GetAddressByKey(Storage3Definition::Key), StructsSizeAttributor::GetAddress(2), 1);
		OnFail();
	}

	if (StructsAttributor::GetAddress(0) != TestUnitStorage::Address())
	{
		PrintAddressMismatch(0, StructsAttributor::GetAddress(0), TestUnitStorage::Address(), 2);
		OnFail();
	}

	if (StructsAttributor::GetAddress(1) != TestUnitTiny5::Address())
	{
		PrintAddressMismatch(1, StructsAttributor::GetAddress(1), TestUnitTiny5::Address(), 2);
		OnFail();
	}

	if (StructsAttributor::GetAddress(2) != TestUnitShort10::Address())
	{
		PrintAddressMismatch(2, StructsAttributor::GetAddress(2), TestUnitShort10::Address(), 2);

		OnFail();
	}

	if (StructsAttributor::GetSize(0) != TestUnitStorage::Size())
	{
		PrintSizeMismatch(0, StructsAttributor::GetSize(0), TestUnitStorage::Size(), 1);
		OnFail();
	}
	if (StructsAttributor::GetSize(1) != TestUnitTiny5::Size())
	{
		PrintSizeMismatch(1, StructsAttributor::GetSize(1), TestUnitTiny5::Size(), 1);
		OnFail();
	}
	if (StructsAttributor::GetSize(2) != TestUnitShort10::Size())
	{
		PrintSizeMismatch(2, StructsAttributor::GetSize(2), TestUnitShort10::Size(), 1);
		OnFail();
	}

	Serial.println(F("\tValidated."));
	Serial.println();
}

void PrintAddressMismatch(const uint8_t index, const uint16_t address, const uint16_t expected, const uint8_t errorCode)
{
	Serial.print(F("Storage"));
	Serial.print(index + 1);
	Serial.print(F(" address mistmatch ["));
	Serial.print(errorCode);
	Serial.print(F("] "));
	Serial.print(address);
	Serial.print(F(" expected "));
	Serial.println(expected);
}

void PrintSizeMismatch(const uint8_t index, const uint16_t size, const uint16_t expected, const uint8_t errorCode)
{
	Serial.print(F("Storage"));
	Serial.print(index + 1);
	Serial.print(F(" size mistmatch ["));
	Serial.print(errorCode);
	Serial.print(F("] "));
	Serial.print(size);
	Serial.print(F(" expected "));
	Serial.println(expected);
}

template<typename StorageType>
void TestStorageUnit()
{
	StorageType storage{};
	Serial.print(F("Testing Storage Unit\t"));
	Serial.print(StorageType::Address());
	Serial.print(',');
	Serial.println(StorageType::Size());

	const uint8_t testValue = 123;
	uint8_t value = testValue;
	storage.WriteData(&value);

	value = 0;
	if (!storage.ReadData(&value) || value != testValue)
	{
		Serial.println(F("\tStorage Unit invalidated."));
		OnFail();
	}

	Serial.println(F("\tValidated."));
}

template<class UnitType>
void PrintWearMask(const uint8_t size, UnitType unit)
{
	const uint64_t mask = unit.DebugMask();
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
void TestUnitWear(String name)
{
	UnitType unit{};

	const uint8_t option = unit.DebugOption();
	Serial.print(F("Testing Wear Level "));
	Serial.print(name);
	Serial.print(F(" Unit\t"));
	Serial.print(UnitType::Address());
	Serial.print(',');
	Serial.println(UnitType::Size());

	EmbeddedEEPROM::EraseEEPROM();
	Serial.print(F("\tErase"));
	PrintWearMask<UnitType>(unit.GetCounterSize(), unit);
	if (unit.DebugCounter() != 0)
	{
		Serial.print(F("\tCounter erased EEPROM invalidated: "));
		Serial.println(unit.DebugCounter());
		OnFail();
	}


	EmbeddedEEPROM::WriteBlock(unit.Address(), (1 << 4));
	Serial.print(F("\tCorrupt"));
	PrintWearMask<UnitType>(unit.GetCounterSize(), unit);

	unit.DebugInitialize();
	Serial.print(F("\tInitialized"));
	PrintWearMask<UnitType>(unit.GetCounterSize(), unit);
	if (unit.DebugCounter() != (option - 1))
	{
		Serial.print(F("\tCounter initialize invalidated: "));
		Serial.println(unit.DebugCounter());
		OnFail();
	}

	unit.ResetCounter();
	Serial.print(F("\tReset"));
	PrintWearMask<UnitType>(unit.GetCounterSize(), unit);
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
		PrintWearMask<UnitType>(unit.GetCounterSize(), unit);

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
	Serial.print(F("\tEnd"));
	PrintWearMask<UnitType>(unit.GetCounterSize(), unit);
	if (unit.DebugCounter() != 0)
	{
		Serial.print(F("\tCounter end invalidated:"));
		Serial.print(unit.DebugCounter());
		OnFail();
	}

	Serial.println(F("\tValidated."));
}
