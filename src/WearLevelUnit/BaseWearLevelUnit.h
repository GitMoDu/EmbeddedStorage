#ifndef _BASE_WEAR_LEVEL_UNIT_
#define _BASE_WEAR_LEVEL_UNIT_

#include "EmbeddedStorageBase\EmbeddedEEPROM.h"
#include "EmbeddedStorageBase\EmbeddedCrc.h"


/// <summary>
/// Base Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 1 byte for counter, 1 byte for CRC times WearLevelOption.
/// Designed for use with a single data struct or array.
/// </summary>
/// <typeparam name="address">Address (offset) in EEPROM.</typeparam>
/// <param name="DataSize">Data size in bytes.</param>
/// <param name="WearLevelOption">WearLevel option, from 2 to 8.</param>
/// <param name="Key">Storage cryptographic salt key.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t address,
	const uint16_t DataSize,
	const uint32_t Key,
	typename WearLevelType,
	const WearLevelType WearLevelOption>
class BaseWearLevelUnit
{
private:
	EmbeddedCrc<Key> Crc{};

public:
	static constexpr uint16_t Address()
	{
		return address;
	}

	static constexpr uint16_t Size()
	{
		return EmbeddedStorage::GetStorageSize(DataSize, WearLevelOption);
	}

protected:
	virtual const uint8_t GetCurrentCounter() { return 0; }
	virtual const uint8_t IncrementCounter() { return 0; }
	virtual const bool ValidateCounterMask() { return false; }

public:
	BaseWearLevelUnit()
	{
		EEPROM.begin();
		Initialize();
	}

#if defined(WEAR_LEVEL_DEBUG)
public:
#else
protected:
#endif
	void ResetCounter()
	{
		for (uint8_t i = 0; i < GetCounterSize(); i++)
		{
			EmbeddedEEPROM::ProgramZeroBitsToZero(address + i, 0);
		}
	}

public:
#if defined(WEAR_LEVEL_DEBUG)
	const uint8_t DebugCounter()
	{
		return GetCurrentCounter();
	}

	const uint8_t DebugOption()
	{
		return (uint8_t)WearLevelOption;
	}

	void DebugInitialize()
	{
		Initialize();
	}

	static constexpr size_t GetCounterSize()
	{
		return EmbeddedStorage::GetWearLevelCounterSize((uint8_t)WearLevelOption);
	}
#endif

	/// <summary>
	/// Reads the declared DataSize into target array.
	/// </summary>
	/// <param name="target">Target array.</param>
	/// <returns>True if CRC matches.</returns>
	const bool ReadData(uint8_t* target)
	{
		const uint8_t counter = GetCurrentCounter();
		const uint16_t offset = (uint16_t)GetCounterSize() + ((uint16_t)counter * EmbeddedStorage::GetStorageSize(DataSize));

		for (uint16_t i = 0; i < DataSize; i++)
		{
			target[i] = EmbeddedEEPROM::ReadBlock(address + offset + i);
		}

		return Crc.GetCrc(target, DataSize, counter) == EmbeddedEEPROM::ReadBlock(address + offset + DataSize);
	}


	/// <summary>
	/// Writes the declared DataSize from source array.
	/// </summary>
	/// <param name="source">Source array.</param>
	void WriteData(const uint8_t* source)
	{
		const uint8_t counter = IncrementCounter();
		const uint16_t offset = (uint16_t)GetCounterSize() + ((uint16_t)counter * EmbeddedStorage::GetStorageSize(DataSize));

		for (uint16_t i = 0; i < DataSize; i++)
		{
			EmbeddedEEPROM::WriteBlock(address + offset + i, source[i]);
		}

		EmbeddedEEPROM::WriteBlock(address + offset + DataSize, Crc.GetCrc(source, DataSize, counter));
	}

	void WriteByte(const uint16_t offset, const uint8_t value)
	{
		EmbeddedEEPROM::WriteBlock(address + offset, value);
	}

	const uint8_t ReadByte(const uint16_t offset)
	{
		return EmbeddedEEPROM::ReadBlock(address + offset);
	}

private:
	/// <summary>
	/// Ensure the current counter in this Unit is according to spec.
	/// </summary>
	void Initialize()
	{
		if (!ValidateCounterMask())
		{
			ResetCounter();
		}
	}

protected:
	static constexpr uint8_t Uint8Min(const uint8_t a, const uint8_t b)
	{
		return ((a <= b) * a) | ((b < a) * b);
	}
};
#endif