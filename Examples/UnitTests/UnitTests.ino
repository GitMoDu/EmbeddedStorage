/*
	Library Unit Testing Project.
*/

#define SERIAL_BAUD_RATE 9600

#define EEPROM_BOUNDS_CHECK
#define WEAR_LEVEL_DEBUG
//#define EEPROM_MOCK_IN_MEMORY

#include <AsyncStorageUnit.h>
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

using TestUnitStorageMultiByte = StorageUnit<InlineUsed, 4, 100004>;
using TestUnitStorageWrongKey = StorageUnit<InlineUsed, 4, 100005>;
using TestUnitTiny5MultiByte = TinyWearLevelUnit<TestUnitStorageMultiByte::Address() + TestUnitStorageMultiByte::Size(), 4, WearLevelTiny::x5, 100006>;
using TestUnitAsyncStorage = AsyncStorageUnit<TestUnitTiny5MultiByte::Address() + TestUnitTiny5MultiByte::Size(), 4, 100007>;
using TestUnitAsyncTiny5 = AsyncTinyWearLevelUnit<TestUnitTiny5::Address(), sizeof(Storage2Definition::Struct), WearLevelTiny::x5>;
using TestUnitAsyncShort10 = AsyncShortWearLevelUnit<0, sizeof(uint8_t), WearLevelShort::x10>;
using TestUnitAsyncLong18 = AsyncLongWearLevelUnit<0, sizeof(uint8_t), WearLevelLong::x18>;
using TestUnitAsyncLongLong34 = AsyncLongLongWearLevelUnit<0, sizeof(uint8_t), WearLevelLongLong::x34>;

static_assert(sizeof(TestUnitStorage) < sizeof(AsyncStorageUnit<0, sizeof(uint8_t)>), "Synchronous storage must omit async state.");
static_assert(sizeof(TestUnitTiny5) < sizeof(TestUnitAsyncTiny5), "Synchronous wear-level storage must omit async state.");
static_assert(sizeof(ShortWearLevelUnit<0, sizeof(uint8_t), WearLevelShort::x10>) < sizeof(TestUnitAsyncShort10), "Synchronous short wear-level storage must omit async state.");
static_assert(sizeof(LongWearLevelUnit<0, sizeof(uint8_t), WearLevelLong::x18>) < sizeof(TestUnitAsyncLong18), "Synchronous long wear-level storage must omit async state.");
static_assert(sizeof(LongLongWearLevelUnit<0, sizeof(uint8_t), WearLevelLongLong::x34>) < sizeof(TestUnitAsyncLongLong34), "Synchronous long-long wear-level storage must omit async state.");

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

template<typename UnitType>
bool CompleteAsyncWrite(UnitType& unit)
{
	EmbeddedEEPROM::AsyncOperationStateEnum state = unit.PollWriteData();
	while (state == EmbeddedEEPROM::AsyncOperationStateEnum::Pending)
	{
		delay(1);
		state = unit.PollWriteData();
	}
	return state == EmbeddedEEPROM::AsyncOperationStateEnum::Complete;
}

void TestAsyncStorageUnit()
{
	EmbeddedEEPROM::EraseEEPROM();
	TestUnitAsyncStorage unit{};
	const uint8_t expected[] = { 0x12, 0x34, 0x56, 0x78 };
	uint8_t value[sizeof(expected)]{};

	if (unit.BeginWriteData(nullptr) || !unit.BeginWriteData(expected)
		|| unit.BeginWriteData(expected) || !CompleteAsyncWrite(unit) || !unit.ReadData(value))
	{
		Serial.println(F("\tAsync storage write or CRC validation failed."));
		OnFail();
	}
	for (uint8_t i = 0; i < sizeof(expected); i++)
	{
		if (value[i] != expected[i])
		{
			Serial.println(F("\tAsync storage readback mismatch."));
			OnFail();
		}
	}
}

template<typename UnitType>
void TestAsyncWearLevelWrite()
{
	EmbeddedEEPROM::EraseEEPROM();
	UnitType unit{};
	const uint8_t expected = 42;
	uint8_t value = 0;
	if (!unit.BeginWriteData(&expected) || !CompleteAsyncWrite(unit)
		|| unit.DebugCounter() != 1 || !unit.ReadData(&value) || value != expected)
	{
		Serial.println(F("\tAsync wear-level write failed."));
		OnFail();
	}
}

void TestAsyncWearLevelUnit()
{
	EmbeddedEEPROM::EraseEEPROM();
	TestUnitAsyncTiny5 unit{};
	uint8_t value = 0;

	for (uint8_t i = 0; i < 7; i++)
	{
		value = i + 20;
		if (!unit.BeginWriteData(&value) || !CompleteAsyncWrite(unit))
		{
			Serial.println(F("\tAsync wear-level write failed."));
			OnFail();
		}
		const uint8_t expectedCounter = (i + 1) % (uint8_t)WearLevelTiny::x5;
		uint8_t readback = 0;
		if (unit.DebugCounter() != expectedCounter || !unit.ReadData(&readback) || readback != value)
		{
			Serial.println(F("\tAsync wear-level counter or readback mismatch."));
			OnFail();
		}
	}

	TestUnitAsyncTiny5 reloaded{};
	uint8_t readback = 0;
	if (reloaded.DebugCounter() != 2 || !reloaded.ReadData(&readback) || readback != value)
	{
		Serial.println(F("\tAsync wear-level data did not persist after reconstruction."));
		OnFail();
	}
	TestAsyncWearLevelWrite<TestUnitAsyncShort10>();
	TestAsyncWearLevelWrite<TestUnitAsyncLong18>();
	TestAsyncWearLevelWrite<TestUnitAsyncLongLong34>();
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
	TestStorageUnitMultiByte();
	TestUnitWearMultiByte();
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
	TestAsyncStorageUnit();
	TestAsyncWearLevelUnit();

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
	if (StructsAttributor::GetSizeByKey(Storage1Definition::Key) != StructsSizeAttributor::GetSize(0)
		|| StructsAttributor::GetSizeByKey<Storage1Definition>() != StructsSizeAttributor::GetSize(0))
	{
		PrintSizeMismatch(0, StructsAttributor::GetSizeByKey(Storage1Definition::Key), StructsSizeAttributor::GetSize(0), 2);
		OnFail();
	}
	if (StructsAttributor::GetAddressByKey(Storage2Definition::Key) != StructsSizeAttributor::GetAddress(1))
	{
		PrintAddressMismatch(1, StructsAttributor::GetAddressByKey(Storage2Definition::Key), StructsSizeAttributor::GetAddress(1), 1);
		OnFail();
	}
	if (StructsAttributor::GetSizeByKey(Storage2Definition::Key) != StructsSizeAttributor::GetSize(1)
		|| StructsAttributor::GetSizeByKey<Storage2Definition>() != StructsSizeAttributor::GetSize(1))
	{
		PrintSizeMismatch(1, StructsAttributor::GetSizeByKey(Storage2Definition::Key), StructsSizeAttributor::GetSize(1), 2);
		OnFail();
	}
	if (StructsAttributor::GetAddressByKey(Storage3Definition::Key) != StructsSizeAttributor::GetAddress(2))
	{
		PrintAddressMismatch(2, StructsAttributor::GetAddressByKey(Storage3Definition::Key), StructsSizeAttributor::GetAddress(2), 1);
		OnFail();
	}
	if (StructsAttributor::GetSizeByKey(Storage3Definition::Key) != StructsSizeAttributor::GetSize(2)
		|| StructsAttributor::GetSizeByKey<Storage3Definition>() != StructsSizeAttributor::GetSize(2))
	{
		PrintSizeMismatch(2, StructsAttributor::GetSizeByKey(Storage3Definition::Key), StructsSizeAttributor::GetSize(2), 2);
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

void TestStorageUnitMultiByte()
{
	TestUnitStorageMultiByte storage{};
	TestUnitStorageWrongKey wrongKey{};
	const uint8_t expected[] = { 0x12, 0x34, 0x56, 0x78 };
	uint8_t value[sizeof(expected)]{};

	Serial.print(F("Testing Multi-byte Storage Unit\t"));
	Serial.print(TestUnitStorageMultiByte::Address());
	Serial.print(',');
	Serial.println(TestUnitStorageMultiByte::Size());

	EmbeddedEEPROM::EraseEEPROM();
	if (storage.ReadData(value))
	{
		Serial.println(F("\tErased storage was reported valid."));
		OnFail();
	}

	storage.WriteData(expected);
	if (!storage.ReadData(value))
	{
		Serial.println(F("\tMulti-byte storage CRC invalidated after write."));
		OnFail();
	}
	for (uint8_t i = 0; i < sizeof(expected); i++)
	{
		if (value[i] != expected[i])
		{
			Serial.print(F("\tMulti-byte storage mismatch at byte "));
			Serial.println(i);
			OnFail();
		}
	}
	if (wrongKey.ReadData(value))
	{
		Serial.println(F("\tStorage data was accepted with the wrong key."));
		OnFail();
	}

	storage.WriteByte(1, expected[1] ^ 0xFF);
	if (storage.ReadByte(1) != (uint8_t)(expected[1] ^ 0xFF) || storage.ReadData(value))
	{
		Serial.println(F("\tPayload corruption was not rejected."));
		OnFail();
	}

	storage.WriteData(expected);
	const uint16_t crcAddress = TestUnitStorageMultiByte::Address() + sizeof(expected);
	EmbeddedEEPROM::WriteBlock(crcAddress, EmbeddedEEPROM::ReadBlock(crcAddress) ^ 0xFF);
	if (storage.ReadData(value))
	{
		Serial.println(F("\tCRC corruption was not rejected."));
		OnFail();
	}
	storage.WriteData(expected);

	Serial.println(F("\tValidated erased-state, wrong-key, multi-byte, and CRC checks."));
}

void TestUnitWearMultiByte()
{
	const uint8_t expected[][4] =
	{
		{ 0x10, 0x20, 0x30, 0x40 },
		{ 0x11, 0x21, 0x31, 0x41 },
		{ 0x12, 0x22, 0x32, 0x42 },
		{ 0x13, 0x23, 0x33, 0x43 },
		{ 0x14, 0x24, 0x34, 0x44 }
	};
	uint8_t value[sizeof(expected[0])]{};

	Serial.print(F("Testing Multi-byte Wear Level Unit\t"));
	Serial.print(TestUnitTiny5MultiByte::Address());
	Serial.print(',');
	Serial.println(TestUnitTiny5MultiByte::Size());

	EmbeddedEEPROM::EraseEEPROM();
	TestUnitTiny5MultiByte unit{};
	// Start at the final counter so the first write uses slot zero.
	unit.ResetCounter();
	for (uint8_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
	{
		unit.WriteData(expected[i]);
		TestUnitTiny5MultiByte reloaded{};
		if (reloaded.DebugCounter() != i)
		{
			Serial.print(F("\tMulti-byte wear-level counter mismatch after reinitialization: "));
			Serial.print(reloaded.DebugCounter());
			Serial.print(F(" expected "));
			Serial.println(i);
			OnFail();
		}
		if (!reloaded.ReadData(value))
		{
			Serial.println(F("\tMulti-byte wear-level data did not persist after reinitialization."));
			OnFail();
		}
		for (uint8_t j = 0; j < sizeof(value); j++)
		{
			if (value[j] != expected[i][j])
			{
				Serial.print(F("\tMulti-byte wear-level mismatch at byte "));
				Serial.println(j);
				OnFail();
			}
		}
	}

	const uint8_t wrappedValue[] = { 0x55, 0x66, 0x77, 0x88 };
	unit.WriteData(wrappedValue);
	TestUnitTiny5MultiByte reloaded{};
	if (reloaded.DebugCounter() != 0 || !reloaded.ReadData(value))
	{
		Serial.println(F("\tMulti-byte wear-level rollover did not persist."));
		OnFail();
	}
	for (uint8_t i = 0; i < sizeof(value); i++)
	{
		if (value[i] != wrappedValue[i])
		{
			Serial.println(F("\tMulti-byte wear-level rollover data mismatch."));
			OnFail();
		}
	}

	const uint16_t currentDataOffset = reloaded.GetCounterSize()
		+ reloaded.DebugCounter() * EmbeddedStorage::GetStorageSize(sizeof(value));
	reloaded.WriteByte(currentDataOffset + 2, wrappedValue[2] ^ 0xFF);
	if (reloaded.ReadData(value))
	{
		Serial.println(F("\tMulti-byte wear-level corruption was not rejected."));
		OnFail();
	}
	const uint8_t recoveryValue[] = { 0xA5, 0x5A, 0xC3, 0x3C };
	reloaded.WriteData(recoveryValue);
	TestUnitTiny5MultiByte recovered{};
	if (recovered.DebugCounter() != 1 || !recovered.ReadData(value))
	{
		Serial.println(F("\tMulti-byte wear-level recovery write did not persist."));
		OnFail();
	}
	for (uint8_t i = 0; i < sizeof(value); i++)
	{
		if (value[i] != recoveryValue[i])
		{
			Serial.println(F("\tMulti-byte wear-level recovery data mismatch."));
			OnFail();
		}
	}

	Serial.println(F("\tMulti-byte wear-level persistence, rollover, CRC, and recovery validated."));
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

		UnitType reloaded{};
		uint8_t reloadedValue = 0;
		if (reloaded.DebugCounter() != i || !reloaded.ReadData(&reloadedValue) || reloadedValue != value)
		{
			Serial.println(F("\tWear-level data did not persist after reinitialization."));
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

	const uint8_t finalValue = value;
	const uint16_t currentDataOffset = unit.GetCounterSize()
		+ unit.DebugCounter() * EmbeddedStorage::GetStorageSize(sizeof(uint8_t));
	unit.WriteByte(currentDataOffset, finalValue ^ 0xFF);
	value = 0;
	if (unit.ReadData(&value))
	{
		Serial.println(F("\tWear-level payload corruption was not rejected."));
		OnFail();
	}

	unit.WriteData(&finalValue);
	value = 0;
	if (!unit.ReadData(&value) || value != finalValue)
	{
		Serial.println(F("\tWear-level write after corruption was not readable."));
		OnFail();
	}

	Serial.println(F("\tPayload corruption rejected; write recovered."));
	Serial.println(F("\tValidated."));
}
