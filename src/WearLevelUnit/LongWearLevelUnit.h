#ifndef _LONG_WEAR_LEVEL_UNIT_
#define _LONG_WEAR_LEVEL_UNIT_

#include "BaseWearLevelUnit.h"
#if defined(EMBEDDED_EEPROM_STORAGE)


/// <summary>
/// Wear levelling long options.
/// 4 blocks can count up to 33 with no erasures.
/// For smaller options, use WearLevelUnit<WearLevelShort>.
/// </summary>
enum WearLevelLong
{
	x18 = 18,
	x19 = 19,
	x20 = 20,
	x21 = 21,
	x22 = 22,
	x23 = 23,
	x24 = 24,
	x25 = 25,
	x26 = 26,
	x27 = 27,
	x28 = 28,
	x29 = 29,
	x30 = 30,
	x31 = 31,
	x32 = 32,
	x33 = 33
};

/// <summary>
/// Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 4 bytes for counter, 1 byte for CRC times Option.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="SizeBytes">Data size in bytes.</param>
/// <param name="Option">WearLevel option, from 18 to 33.</param>
/// <param name="Key">Storage cryptographic salt key. Defaults to SizeBites + Option.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t SizeBytes,
	const WearLevelLong Option = WearLevelLong::x18,
	const uint8_t Key = SizeBytes + (uint8_t)Option + sizeof(uint32_t)>
	class LongWearLevelUnit
	: public BaseWearLevelUnit<SizeBytes, Key, sizeof(uint32_t), (uint8_t)Option>
{
private:
	uint16_t StartAddress;

	union ATUI32
	{
		uint32_t Value;
		uint8_t Array[sizeof(uint32_t)];
	} ATUI;

	static const int8_t Counts = sizeof(uint32_t) * 8;

public:
	LongWearLevelUnit(const uint16_t startBlockAddress)
		: BaseWearLevelUnit<SizeBytes, Key, sizeof(uint32_t), (uint8_t)Option>(startBlockAddress)
		, StartAddress(startBlockAddress)
	{
		Initialize();
	}

#if defined(WEAR_LEVEL_DEBUG)
	const uint32_t DebugMask()
	{
		return GetMask();
	}
#endif

private:
	const uint32_t GetMask()
	{
		ATUI.Array[0] = ReadBlock(0);
		ATUI.Array[1] = ReadBlock(1);
		ATUI.Array[2] = ReadBlock(2);
		ATUI.Array[3] = ReadBlock(3);

		return ATUI.Value;
	}

	const uint32_t GetMask(const uint8_t counter)
	{
		uint32_t mask = UINT32_MAX;
		for (int8_t i = Counts; i >= (Counts - counter); i--)
		{
			// Remove i bit to mask.
			mask &= ~((uint32_t)1 << i);
		}
		return mask;
	}

protected:
	const uint8_t GetCurrentCounter() final
	{
		const uint32_t mask = GetMask();

		if (mask == UINT32_MAX) {
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
	/// From 0 to 32 (Option).
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
		}

		return counter;
	}

	const bool ValidateCounterMask() final
	{
		const uint32_t mask = GetMask();

		if (mask == UINT32_MAX)
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