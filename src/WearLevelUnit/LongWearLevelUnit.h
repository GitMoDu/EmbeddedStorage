#ifndef _LONG_WEAR_LEVEL_UNIT_
#define _LONG_WEAR_LEVEL_UNIT_

#include "BaseWearLevelUnit.h"
#include <WearLevelType.h>


/// <summary>
/// Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 4 bytes for counter, 1 byte for CRC times Option.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="DataSize">Data size in bytes.</param>
/// <param name="Option">WearLevel option, from 18 to 33.</param>
/// <param name="Key">Storage cryptographic salt key. Defaults to SizeBites + Option.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t Address,
	const uint16_t DataSize,
	const WearLevelLong Option = WearLevelLong::x18,
	const uint32_t Key = EmbeddedStorage::GetStorageSize(DataSize, Option)>
class LongWearLevelUnit
	: public BaseWearLevelUnit<Address, DataSize, Key, WearLevelLong, Option>
{
private:
	using BaseClass = BaseWearLevelUnit<Address, DataSize, Key, WearLevelLong, Option>;
	using BaseClass::Uint8Min;

private:
	union ATUI32
	{
		uint32_t Value;
		uint8_t Array[sizeof(uint32_t)];
	} ATUI;

	static const int8_t Counts = sizeof(uint32_t) * 8;

public:
	LongWearLevelUnit() : BaseClass()
	{}

#if defined(WEAR_LEVEL_DEBUG)
	const uint32_t DebugMask()
	{
		return GetMask();
	}
#endif

private:
	const uint32_t GetMask()
	{
		ATUI.Array[0] = EmbeddedEEPROM::ReadBlock(Address + 0);
		ATUI.Array[1] = EmbeddedEEPROM::ReadBlock(Address + 1);
		ATUI.Array[2] = EmbeddedEEPROM::ReadBlock(Address + 2);
		ATUI.Array[3] = EmbeddedEEPROM::ReadBlock(Address + 3);

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
					return Uint8Min(i - 1, (uint8_t)Option - 1);
				}
			}
		}

		return (uint8_t)Option - 1;
	}

	/// <summary>
	/// Increments the counter in a flash compatible way.
	/// From 0 to 32 (Option).
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