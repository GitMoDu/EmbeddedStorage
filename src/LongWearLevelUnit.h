#ifndef _LONG_WEAR_LEVEL_UNIT_
#define _LONG_WEAR_LEVEL_UNIT_

#include <EmbeddedEEPROM.h>
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328p__) || defined(__AVR_atmega328p__) || defined(__AVR_ATtiny85__)

#include <EmbeddedCrc.h>


//#define WEAR_LEVEL_DEBUG


/// <summary>
/// Wear levelling long options.
/// 2 blocks can count up to 17 with no erasures.
/// For smaller options, use WearLevelUnit<WearLevelShort>.
/// </summary>
enum WearLevelLong
{
	x10 = 10,
	x11 = 11,
	x12 = 12,
	x13 = 13,
	x14 = 14,
	x15 = 15,
	x16 = 16,
	x17 = 17
};

/// <summary>
/// Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 2 bytes for counter, 1 byte for CRC times Option.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="SizeBytes">Data size in bytes.</param>
/// <param name="Option">WearLevel option, from 10 to 18.</param>
/// <param name="Key">Storage cryptographic salt key.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t SizeBytes, const WearLevelLong Option = WearLevelLong::x10, const uint8_t Key = SizeBytes>
class LongWearLevelUnit : private EmbeddedEEPROM
{
private:
	EmbeddedCrc<Key> Crc;

	static const uint8_t CounterSize = sizeof(uint16_t);

	enum CounterEnum
	{
		c0 = 0b1111111111111111,
		c1 = 0b0111111111111111,
		c2 = 0b0011111111111111,
		c3 = 0b0001111111111111,
		c4 = 0b0000111111111111,
		c5 = 0b0000011111111111,
		c6 = 0b0000001111111111,
		c7 = 0b0000000111111111,
		c8 = 0b0000000011111111,
		c9 = 0b0000000001111111,
		c10 = 0b0000000000111111,
		c11 = 0b0000000000011111,
		c12 = 0b0000000000001111,
		c13 = 0b0000000000000111,
		c14 = 0b0000000000000011,
		c15 = 0b0000000000000001,
		c16 = 0b0000000000000000
	};

public:
	static constexpr GetUsedBlockCount()
	{
		// 2 bytes for counter, 1 byte for CRC times Option.
		return 2 + ((int)Option * (SizeBytes + 1));
	}

public:
	LongWearLevelUnit(const uint16_t startBlockAddress)
		: EmbeddedEEPROM(startBlockAddress)
		, Crc()
	{
		if (!ValidateCounterMask())
		{
			ClearByteToOnes(0);
			ClearByteToOnes(1);
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

private:
	const uint16_t GetMask(const uint8_t counter)
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
		case 9:
			return CounterEnum::c9;
		case 10:
			return CounterEnum::c10;
		case 11:
			return CounterEnum::c11;
		case 12:
			return CounterEnum::c12;
		case 13:
			return CounterEnum::c13;
		case 14:
			return CounterEnum::c14;
		case 15:
			return CounterEnum::c15;
		case 16:
			return CounterEnum::c16;
		default:
			return CounterEnum::c0;
		}
	}

	const uint8_t GetCurrentCounter()
	{
		uint16_t mask = ReadBlock(0) << 8;
		mask += ReadBlock(1);

		switch (mask)
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
		case CounterEnum::c9:
			return 9;
		case CounterEnum::c10:
			return 10;
		case CounterEnum::c11:
			return 11;
		case CounterEnum::c12:
			return 12;
		case CounterEnum::c13:
			return 13;
		case CounterEnum::c14:
			return 14;
		case CounterEnum::c15:
			return 15;
		case CounterEnum::c16:
			return 16;
		default:
			return 0;
		}
	}

	/// <summary>
	/// Increments the counter in a flash compatible way.
	/// From 0 to 18 (Option).
	/// </summary>
	/// <returns>Current Counter</returns>
	const uint8_t IncrementCounter()
	{
		uint8_t counter = GetCurrentCounter();
		if (counter + 1 >= Option)
		{
			counter = 0;
			ClearByteToOnes(0);
			ClearByteToOnes(1);
		}
		else
		{
			counter++;
			uint16_t mask = GetMask(counter);
			ProgramZeroBitsToZero(0, mask >> 8);
			ProgramZeroBitsToZero(1, mask);
		}

		return counter;
	}

	const bool ValidateCounterMask()
	{
		uint16_t mask = ReadBlock(0) << 8;
		mask += ReadBlock(1);

		switch (mask)
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
		case CounterEnum::c9:
		case CounterEnum::c10:
		case CounterEnum::c11:
		case CounterEnum::c12:
		case CounterEnum::c13:
		case CounterEnum::c14:
		case CounterEnum::c15:
		case CounterEnum::c16:
			return true;
		default:
			return false;
		}
	}
};
#endif
#endif