#ifndef _BASE_WEAR_LEVEL_UNIT_
#define _BASE_WEAR_LEVEL_UNIT_

#include "EmbeddedStorageBase\EmbeddedEEPROM.h"
#if defined(EMBEDDED_EEPROM_STORAGE)
#include "EmbeddedStorageBase\EmbeddedCrc.h"


//#define WEAR_LEVEL_DEBUG

/// <summary>
/// Base Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 1 byte for counter, 1 byte for CRC times Option.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="SizeBytes">Data size in bytes.</param>
/// <param name="Option">WearLevel option, from 2 to 8.</param>
/// <param name="Key">Storage cryptographic salt key.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t SizeBytes,
	const uint8_t Key,
	const uint8_t CounterSize,
	const uint8_t Option>
	class BaseWearLevelUnit : protected EmbeddedEEPROM
{
private:
	EmbeddedCrc<Key> Crc;

	static const uint8_t CrcSize = 1;

public:
	static constexpr uint16_t GetUsedBlockCount()
	{
		// CounterSize bytes for counter, 1 byte for CRC times Option.
		return CounterSize + ((uint16_t)Option * (SizeBytes + 1));
	}

protected:
	virtual const uint8_t GetCurrentCounter() { return 0; }
	virtual const uint8_t IncrementCounter() { return 0; }
	virtual const bool ValidateCounterMask() { return false; }

public:
	BaseWearLevelUnit(const uint16_t startBlockAddress)
		: EmbeddedEEPROM(startBlockAddress)
		, Crc()
	{}

protected:
	void Initialize()
	{
		if (!ValidateCounterMask())
		{
			ResetCounter();
		}
	}

#if defined(WEAR_LEVEL_DEBUG)
public:
#else
protected:
#endif
	void ResetCounter()
	{
		for (uint8_t i = 0; i < CounterSize; i++)
		{
			ProgramZeroBitsToZero(i, 0);
		}
	}

public:
#if defined(WEAR_LEVEL_DEBUG)
	const uint8_t DebugCounter()
	{
		return GetCurrentCounter();
	}
#endif

#if defined(EEPROM_BOUNDS_CHECK)
	const uint16_t GetStartAddress()
	{
		return StartBlockAddress;
	}
#endif

	/// <summary>
	/// Reads the declared SizeBytes into target array.
	/// </summary>
	/// <param name="target">Target array.</param>
	/// <returns>True if CRC matches.</returns>
	const bool ReadData(uint8_t* target)
	{
		const uint8_t counter = GetCurrentCounter();
		const uint16_t offset = (uint16_t)CounterSize + ((uint16_t)counter * (SizeBytes + CrcSize));

		for (uint16_t i = 0; i < SizeBytes; i++)
		{
			target[i] = ReadBlock(offset + i);
		}

		return Crc.GetCrc(target, SizeBytes, counter) == ReadBlock(offset + SizeBytes);
	}


	/// <summary>
	/// Writes the declared SizeBytes from source array.
	/// </summary>
	/// <param name="source">Source array.</param>
	void WriteData(const uint8_t* source)
	{
		const uint8_t counter = IncrementCounter();
		const uint16_t offset = (uint16_t)CounterSize + ((uint16_t)counter * (SizeBytes + CrcSize));

		for (uint16_t i = 0; i < SizeBytes; i++)
		{
			WriteBlock(offset + i, source[i]);
		}

		WriteBlock(offset + SizeBytes, Crc.GetCrc(source, SizeBytes, counter));
	}
};
#endif
#endif