#ifndef _SHORT_WEAR_LEVEL_UNIT_
#define _SHORT_WEAR_LEVEL_UNIT_

#include <EmbeddedEEPROM.h>
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328p__) || defined(__AVR_atmega328p__) || defined(__AVR_ATtiny85__)

#include <EmbeddedCrc.h>


//#define WEAR_LEVEL_DEBUG


/// <summary>
/// Wear levelling short options.
/// A single block can count up to 9 with no erasures.
/// x1 option doesn't make use of rolling counter, use StorageUnit instead.
/// </summary>
enum WearLevelShort
{
	x2 = 2,
	x3 = 3,
	x4 = 4,
	x5 = 5,
	x6 = 6,
	x7 = 7,
	x8 = 8,
	x9 = 9
};

/// <summary>
/// Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 1 byte for counter, 1 byte for CRC times Option.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="SizeBytes">Data size in bytes.</param>
/// <param name="Option">WearLevel option, from 2 to 8.</param>
/// <param name="Key">Storage cryptographic salt key.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t SizeBytes, const WearLevelShort Option = WearLevelShort::x2, const uint8_t Key = SizeBytes>
class ShortWearLevelUnit : private EmbeddedEEPROM
{
private:
	EmbeddedCrc<Key> Crc;

	static const uint8_t CounterSize = sizeof(uint8_t);

	enum CounterEnum
	{
		c0 = 0b11111111,
		c1 = 0b01111111,
		c2 = 0b00111111,
		c3 = 0b00011111,
		c4 = 0b00001111,
		c5 = 0b00000111,
		c6 = 0b00000011,
		c7 = 0b00000001,
		c8 = 0b00000000
	};

public:
	static constexpr GetUsedBlockCount()
	{
		// 1 byte for counter, 1 byte for CRC times Option.
		return 1 + ((int)Option * (SizeBytes + 1));
	}

public:
	ShortWearLevelUnit(const uint16_t startBlockAddress)
		: EmbeddedEEPROM(startBlockAddress)
		, Crc()
	{
		if (!ValidateCounterMask())
		{
			ClearByteToOnes(0);
		}
	}

#if defined(WEAR_LEVEL_DEBUG)
	const uint8_t DebugCounter()
	{
		return GetCurrentCounter();
	}
#endif

	/// <summary>
	/// Reads the declared SizeBytes into target array.
	/// </summary>
	/// <param name="target">Target array.</param>
	/// <returns>True if CRC matches.</returns>
	const bool ReadData(uint8_t* target)
	{
		uint8_t counter = GetCurrentCounter();
		uint16_t offset = 1 + ((uint16_t)counter * (SizeBytes + 1));

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
		uint8_t counter = IncrementCounter();
		uint16_t offset = 1 + ((uint16_t)counter * (SizeBytes + 1));

		for (uint16_t i = 0; i < SizeBytes; i++)
		{
			WriteBlock(offset + i, source[i]);
		}

		WriteBlock(offset + SizeBytes, Crc.GetCrc(source, SizeBytes, counter));
	}

private:
	const uint8_t GetMask(const uint8_t counter)
	{
		switch (counter)
		{
		case 0:
			return CounterEnum::c0;
		case 1:
			return CounterEnum::c1;
		case 2:
			return CounterEnum::c2;
		case 3:
			return CounterEnum::c3;
		case 4:
			return CounterEnum::c4;
		case 5:
			return CounterEnum::c5;
		case 6:
			return CounterEnum::c6;
		case 7:
			return CounterEnum::c7;
		case 8:
			return CounterEnum::c8;
		default:
			return CounterEnum::c0;
		}
	}

	const uint8_t GetCurrentCounter()
	{
		switch (ReadBlock(0))
		{
		case CounterEnum::c0:
			return 0;
		case CounterEnum::c1:
			return 1;
		case CounterEnum::c2:
			return 2;
		case CounterEnum::c3:
			return 3;
		case CounterEnum::c4:
			return 4;
		case CounterEnum::c5:
			return 5;
		case CounterEnum::c6:
			return 6;
		case CounterEnum::c7:
			return 7;
		case CounterEnum::c8:
			return 8;
		default:
			return 0;
		}
	}

	/// <summary>
	/// Increments the counter in a flash compatible way.
	/// From 0 to 8 (Option).
	/// </summary>
	/// <returns>Current Counter</returns>
	const uint8_t IncrementCounter()
	{
		uint8_t counter = GetCurrentCounter();
		if (counter + 1 >= Option)
		{
			counter = 0;
			ClearByteToOnes(0);
		}
		else
		{
			counter++;
			uint8_t mask = GetMask(counter);
			ProgramZeroBitsToZero(0, mask);
		}

		return counter;
	}

	const bool ValidateCounterMask()
	{
		uint8_t counter = ReadBlock(0);
		switch (counter)
		{
		case CounterEnum::c0:
		case CounterEnum::c1:
		case CounterEnum::c2:
		case CounterEnum::c3:
		case CounterEnum::c4:
		case CounterEnum::c5:
		case CounterEnum::c6:
		case CounterEnum::c7:
		case CounterEnum::c8:
			return true;
		default:
			return false;
		}
	}
};
#endif
#endif