#ifndef _LONG_LONG_WEAR_LEVEL_UNIT_
#define _LONG_LONG_WEAR_LEVEL_UNIT_

#include "BaseWearLevelUnit.h"
#if defined(EMBEDDED_EEPROM_STORAGE)


/// <summary>
/// Wear levelling long options.
/// 8 blocks can count up to 65 with no erasures.
/// For smaller options, use WearLevelUnit<WearLevelLong>.
/// </summary>
enum WearLevelLongLong
{
	x34 = 34,
	x35 = 35,
	x36 = 36,
	x37 = 37,
	x38 = 38,
	x39 = 39,
	x40 = 40,
	x41 = 41,
	x42 = 42,
	x43 = 43,
	x44 = 44,
	x45 = 45,
	x46 = 46,
	x47 = 47,
	x48 = 48,
	x49 = 49,
	x50 = 50,
	x51 = 51,
	x52 = 52,
	x53 = 53,
	x54 = 54,
	x55 = 55,
	x56 = 56,
	x57 = 57,
	x58 = 58,
	x59 = 59,
	x60 = 60,
	x61 = 61,
	x62 = 62,
	x63 = 63,
	x64 = 64,
	x65 = 65
};

/// <summary>
/// Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 8 bytes for counter, 1 byte for CRC times Option.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="SizeBytes">Data size in bytes.</param>
/// <param name="Option">WearLevel option, from 34 to 66.</param>
/// <param name="Key">Storage cryptographic salt key. Defaults to SizeBites + Option.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t SizeBytes,
	const WearLevelLongLong Option = WearLevelLongLong::x34,
	const uint8_t Key = SizeBytes + (uint8_t)Option + sizeof(uint64_t)>
	class LongLongWearLevelUnit
	: public BaseWearLevelUnit<SizeBytes, Key, sizeof(uint64_t), (uint8_t)Option>
{
private:
	uint16_t StartAddress;

	union ATUI64
	{
		uint64_t Value;
		uint8_t Array[sizeof(uint64_t)];
	} ATUI;

	static const int8_t Counts = sizeof(uint64_t) * 8;

public:
	LongLongWearLevelUnit(const uint16_t startBlockAddress)
		: BaseWearLevelUnit<SizeBytes, Key, sizeof(uint64_t), (uint8_t)Option>(startBlockAddress)
		, StartAddress(startBlockAddress)
	{
		Initialize();
	}

#if defined(WEAR_LEVEL_DEBUG)
	const uint64_t DebugMask()
	{
		return GetMask();
	}
#endif

private:
	const uint64_t GetMask()
	{
		ATUI.Array[0] = ReadBlock(0);
		ATUI.Array[1] = ReadBlock(1);
		ATUI.Array[2] = ReadBlock(2);
		ATUI.Array[3] = ReadBlock(3);
		ATUI.Array[4] = ReadBlock(4);
		ATUI.Array[5] = ReadBlock(5);
		ATUI.Array[6] = ReadBlock(6);
		ATUI.Array[7] = ReadBlock(7);

		return ATUI.Value;
	}

	const uint64_t GetMask(const uint8_t counter)
	{
		uint64_t mask = UINT64_MAX;
		for (int8_t i = Counts; i >= (Counts - counter); i--)
		{
			// Remove i bit to mask.
			mask &= ~((uint64_t)1 << i);
		}
		return mask;
	}

protected:
	const uint8_t GetCurrentCounter() final
	{
		const uint64_t mask = GetMask();

		if (mask == UINT64_MAX) {
			return 0;
		}
		else if (mask > 0)
		{
			// Find first Zero bit from left to right.
			for (uint8_t i = 0; i <= Counts; i++)
			{
				if (((uint8_t)(mask >> (Counts - i))) & 1)
				{
					return min(i - 1, Option - 1);
				}
			}
		}

		return Option - 1;
	}

	/// <summary>
	/// Increments the counter in a flash compatible way.
	/// From 0 to 64 (Option).
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
			ClearByteToOnes(2);
			ClearByteToOnes(3);
			ClearByteToOnes(4);
			ClearByteToOnes(5);
			ClearByteToOnes(6);
			ClearByteToOnes(7);
		}
		else
		{
			counter++;
			ATUI.Value = GetMask(counter);

			if (ATUI.Array[0] != ReadBlock(0))
			{
				ProgramZeroBitsToZero(0, ATUI.Array[0]);
			}
			if (ATUI.Array[1] != ReadBlock(1))
			{
				ProgramZeroBitsToZero(1, ATUI.Array[1]);
			}
			if (ATUI.Array[2] != ReadBlock(2))
			{
				ProgramZeroBitsToZero(2, ATUI.Array[2]);
			}
			if (ATUI.Array[3] != ReadBlock(3))
			{
				ProgramZeroBitsToZero(3, ATUI.Array[3]);
			}
			if (ATUI.Array[4] != ReadBlock(4))
			{
				ProgramZeroBitsToZero(4, ATUI.Array[4]);
			}
			if (ATUI.Array[5] != ReadBlock(5))
			{
				ProgramZeroBitsToZero(5, ATUI.Array[5]);
			}
			if (ATUI.Array[6] != ReadBlock(6))
			{
				ProgramZeroBitsToZero(6, ATUI.Array[6]);
			}
			if (ATUI.Array[7] != ReadBlock(7))
			{
				ProgramZeroBitsToZero(7, ATUI.Array[7]);
			}
		}

		return counter;
	}

	const bool ValidateCounterMask() final
	{
		const uint64_t mask = GetMask();

		if (mask == UINT64_MAX)
		{
			return true;
		}
		else if (mask > 0)
		{
			// Find first 1 bit from left to right.
			for (uint8_t i = 0; i <= Counts; i++)
			{
				if (((uint8_t)(mask >> (Counts - i))) & 1)
				{
					// Make sure all bits to the right are a 1.
					for (int8_t j = i; j > 0; j--)
					{
						if (~((uint8_t)(mask >> j) & 1))
						{
							return false;
						}
					}

					return true;
				}
			}
		}

		return true;
	}
};
#endif
#endif