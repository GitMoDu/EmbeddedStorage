#ifndef _LONG_LONG_WEAR_LEVEL_UNIT_
#define _LONG_LONG_WEAR_LEVEL_UNIT_

#include "BaseWearLevelUnit.h"
#include <WearLevelType.h>

/// <summary>
/// Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 8 bytes for counter, 1 byte for CRC times Option.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="SizeBytes">Data size in bytes.</param>
/// <param name="Option">WearLevel option, from 34 to 66.</param>
/// <param name="Key">Storage cryptographic salt key. Defaults to SizeBites + Option.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t Address,
	const uint16_t DataSize,
	const WearLevelLongLong Option = WearLevelLongLong::x34,
	const uint32_t Key = EmbeddedStorage::GetStorageSize(DataSize, Option)>
class LongLongWearLevelUnit
	: public BaseWearLevelUnit<Address, DataSize, Key, WearLevelLongLong, Option>
{
private:
	using BaseClass = BaseWearLevelUnit<Address, DataSize, Key, WearLevelLongLong, Option>;
	using BaseClass::Uint8Min;

private:
	union ATUI64
	{
		uint64_t Value;
		uint8_t Array[sizeof(uint64_t)];
	} ATUI;

	static const int8_t Counts = sizeof(uint64_t) * 8;

public:
	LongLongWearLevelUnit() : BaseClass()
	{}

#if defined(WEAR_LEVEL_DEBUG)
	const uint64_t DebugMask()
	{
		return GetMask();
	}
#endif

private:
	const uint64_t GetMask()
	{
		ATUI.Array[0] = EmbeddedEEPROM::ReadBlock(Address + 0);
		ATUI.Array[1] = EmbeddedEEPROM::ReadBlock(Address + 1);
		ATUI.Array[2] = EmbeddedEEPROM::ReadBlock(Address + 2);
		ATUI.Array[3] = EmbeddedEEPROM::ReadBlock(Address + 3);
		ATUI.Array[4] = EmbeddedEEPROM::ReadBlock(Address + 4);
		ATUI.Array[5] = EmbeddedEEPROM::ReadBlock(Address + 5);
		ATUI.Array[6] = EmbeddedEEPROM::ReadBlock(Address + 6);
		ATUI.Array[7] = EmbeddedEEPROM::ReadBlock(Address + 7);

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
					return Uint8Min(i - 1, (uint8_t)Option - 1);
				}
			}
		}

		return (uint8_t)Option - 1;
	}

	/// <summary>
	/// Increments the counter in a flash compatible way.
	/// From 0 to 64 (Option).
	/// </summary>
	/// <returns>Current Counter</returns>
	const uint8_t IncrementCounter() final
	{
		uint8_t counter = GetCurrentCounter();
		if (counter + 1 >= (uint8_t)Option)
		{
			counter = 0;

			EmbeddedEEPROM::ClearByteToOnes(Address + 0);
			EmbeddedEEPROM::ClearByteToOnes(Address + 1);
			EmbeddedEEPROM::ClearByteToOnes(Address + 2);
			EmbeddedEEPROM::ClearByteToOnes(Address + 3);
			EmbeddedEEPROM::ClearByteToOnes(Address + 4);
			EmbeddedEEPROM::ClearByteToOnes(Address + 5);
			EmbeddedEEPROM::ClearByteToOnes(Address + 6);
			EmbeddedEEPROM::ClearByteToOnes(Address + 7);
		}
		else
		{
			counter++;
			ATUI.Value = GetMask(counter);

			if (ATUI.Array[0] != EmbeddedEEPROM::ReadBlock(Address + 0))
			{
				EmbeddedEEPROM::ProgramZeroBitsToZero(Address + 0, ATUI.Array[0]);
			}
			if (ATUI.Array[1] != EmbeddedEEPROM::ReadBlock(Address + 1))
			{
				EmbeddedEEPROM::ProgramZeroBitsToZero(Address + 1, ATUI.Array[1]);
			}
			if (ATUI.Array[2] != EmbeddedEEPROM::ReadBlock(Address + 2))
			{
				EmbeddedEEPROM::ProgramZeroBitsToZero(Address + 2, ATUI.Array[2]);
			}
			if (ATUI.Array[3] != EmbeddedEEPROM::ReadBlock(Address + 3))
			{
				EmbeddedEEPROM::ProgramZeroBitsToZero(Address + 3, ATUI.Array[3]);
			}
			if (ATUI.Array[4] != EmbeddedEEPROM::ReadBlock(Address + 4))
			{
				EmbeddedEEPROM::ProgramZeroBitsToZero(Address + 4, ATUI.Array[4]);
			}
			if (ATUI.Array[5] != EmbeddedEEPROM::ReadBlock(Address + 5))
			{
				EmbeddedEEPROM::ProgramZeroBitsToZero(Address + 5, ATUI.Array[5]);
			}
			if (ATUI.Array[6] != EmbeddedEEPROM::ReadBlock(Address + 6))
			{
				EmbeddedEEPROM::ProgramZeroBitsToZero(Address + 6, ATUI.Array[6]);
			}
			if (ATUI.Array[7] != EmbeddedEEPROM::ReadBlock(Address + 7))
			{
				EmbeddedEEPROM::ProgramZeroBitsToZero(Address + 7, ATUI.Array[7]);
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
						if (~((uint32_t)(mask >> j) & 1))
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