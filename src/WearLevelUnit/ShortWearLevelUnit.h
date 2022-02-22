#ifndef _SHORT_WEAR_LEVEL_UNIT_
#define _SHORT_WEAR_LEVEL_UNIT_

#include "BaseWearLevelUnit.h"
#if defined(EMBEDDED_EEPROM_STORAGE)

/// <summary>
/// Wear levelling short options.
/// 2 blocks can count up to 17 with no erasures.
/// For smaller options, use WearLevelUnit<WearLevelShort>.
/// </summary>
enum WearLevelShort
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
/// <param name="Key">Storage cryptographic salt key. Defaults to SizeBites + Option.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t SizeBytes,
	const WearLevelShort Option = WearLevelShort::x10,
	const uint8_t Key = SizeBytes + (uint8_t)Option + sizeof(uint16_t)>
	class ShortWearLevelUnit
	: public BaseWearLevelUnit<SizeBytes, Key, sizeof(uint16_t), (uint8_t)Option>
{
private:
	enum CounterEnum : uint16_t
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
	ShortWearLevelUnit(const uint16_t startBlockAddress)
		: BaseWearLevelUnit<SizeBytes, Key, sizeof(uint16_t), (uint8_t)Option>(startBlockAddress)
	{
		Initialize();
	}

private:
	const uint16_t GetMask()
	{
		uint16_t mask = ReadBlock(0);
		mask <<= 8;
		mask += ReadBlock(1);

		return mask;
	}

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

protected:
	const uint8_t GetCurrentCounter() final
	{
		switch (GetMask())
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
			return min(10, Option - 1);
		case CounterEnum::c11:
			return min(11, Option - 1);
		case CounterEnum::c12:
			return min(12, Option - 1);
		case CounterEnum::c13:
			return min(13, Option - 1);
		case CounterEnum::c14:
			return min(14, Option - 1);
		case CounterEnum::c15:
			return min(15, Option - 1);
		case CounterEnum::c16:
			return min(16, Option - 1);
		default:
			return 0;
		}
	}

	/// <summary>
	/// Increments the counter in a flash compatible way.
	/// From 0 to 18 (Option).
	/// </summary>
	/// <returns>Current Counter</returns>
	const uint8_t IncrementCounter() final
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
			const uint16_t mask = GetMask(counter);
			ProgramZeroBitsToZero(0, mask >> 8);
			ProgramZeroBitsToZero(1, mask);
		}

		return counter;
	}

	const bool ValidateCounterMask() final
	{
		switch (GetMask())
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
			return GetCurrentCounter() <= Option;
		default:
			return false;
		}
	}
};
#endif
#endif