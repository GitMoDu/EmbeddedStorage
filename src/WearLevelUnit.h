#ifndef _WEAR_LEVEL_UNIT_
#define _WEAR_LEVEL_UNIT_

#include <EmbeddedEEPROM.h>
#include <EmbeddedCrc.h>

#define ARDUINO_ARCH_AVR

#if defined(ARDUINO_ARCH_AVR)
#define Handle uint16_t
#elif defined(__AVR_ATtiny85__)
#define Handle uint8_t
#else
#error Unsuported board.
#endif



/// <summary>
/// Wear levelling options.
/// A single block can count up to 8 with no erasures.
/// x1 option doesn't make sense, use StorageUnit instead.
/// </summary>
enum WearLevelOption
{
	x2 = 2,
	x3 = 3,
	x4 = 4,
	x5 = 5,
	x6 = 6,
	x7 = 7,
	x8 = 8
};

template<const handle_t SizeBytes, const uint16_t Key = 0, WearLevelOption Option = WearLevelOption::x2>
class WearLevelUnit : private EmbeddedEEPROM
{
private:
	EmbeddedCrc<Key> Crc;

	static const uint8_t CounterSize = sizeof(uint8_t);

public:
	static constexpr GetUsedBlockCount()
	{
		// 1 Block count for Data.
		// 1 Extra block for CRC per Data copy.
		// 1 Extra block for Counter.
		return ((GetDataBlockCount() + 1) * (int)Option) + 1;
	}

private:
	static constexpr uint16_t GetDataBlockCount()
	{
		return EmbeddedEEPROM::GetDataBlockCount(SizeBytes);
	}

public:
	WearLevelUnit(const handle_t startBlockAddress)
		: EmbeddedEEPROM(startBlockAddress)
		, Crc()
	{
		if (!ValidateCounter())
		{
			EraseAll();
		}
	}

	/// <summary>
	/// Erase whole unit and start again.
	/// </summary>
	void EraseAll()
	{
		for (handle_t i = 0; i < GetUsedBlockCount(); i++)
		{
			EraseBlock(i);
		}
	}

	/// <summary>
	/// Reads the declared SizeBytes into target array.
	/// </summary>
	/// <param name="target">Target array.</param>
	/// <returns>True if CRC matches.</returns>
	const bool ReadData(const uint8_t* target)
	{
		// Get the current counter mask.
		uint16_t counter = ReadBlock(0);

		// Re-using the counter bitmask to store the actual value.
		counter = GetCounterValue(counter);

		uint16_t offset = 1 + (counter * (GetDataBlockCount() + 1));

		for (handle_t i = 0; i < GetDataBlockCount(); i++)
		{
			target[i] = ReadBlock(offset + i);
		}

		return Crc.GetCrc(target, SizeBytes, counter) == ReadBlock(offset + GetDataBlockCount());
	}


	/// <summary>
	/// Writes the declared SizeBytes from source array.
	/// </summary>
	/// <param name="source">Source array.</param>
	void WriteData(const uint8_t* source)
	{
		// Get the current counter mask.
		uint8_t counter = ReadBlock(0);

		if (IncrementCounterMask(counter))
		{
			// Erase whole unit and start again.
			EraseAll();
		}

		// Re-using the counter bitmask to store the actual value.
		counter = GetCounterValue(counter);
		WriteUpdateBlock(0, counter);

		uint16_t offset = 1 + (counter * (GetDataBlockCount() + 1));

		for (handle_t i = 0; i < SizeBytes; i++)
		{
			WriteBlock(i, source[i]);
		}

		WriteBlock(GetDataBlockCount(), Crc.GetCrc(source, SizeBytes, counter));
	}

private:
	/// <summary>
	/// Increments the counter in a flash compatible way.
	/// From 0 to 8 (CounterSize).
	/// </summary>
	/// <param name="counter"></param>
	/// <returns>True on counter rollover.</returns>
	const bool IncrementCounterMask(uint8_t& counter)
	{
		for (uint8_t i = 0; i < CounterSize; i++)
		{
			if (!((counter >> i) & 0x01))
			{
				counter |= 1 << i + 1;
				return false;
			}
		}

		counter = 0;
		return true;
	}

	const uint8_t GetCounterValue(const uint8_t counter)
	{
		for (uint8_t i = 0; i < CounterSize; i++)
		{
			if (!((counter >> i) & 0x01))
			{
				return i;
			}
		}

		return CounterSize;
	}

	const bool ValidateCounter()
	{
		uint8_t counter = ReadBlock(0);

		bool high = (counter & 0x01);
		for (uint8_t i = 0; i < CounterSize; i++)
		{
			// Check if first bit is zero.
			if (((counter >> i) & 0x01))
			{
				if (high)
				{
					// All good, skip.
				}
				else
				{
					// Fist high bit must be at zero.
					return false;
				}
			}
			else
			{
				// All subsequent bits should be zero.
				for (uint8_t j = i + 1; j < CounterSize; j++)
				{
					if (((counter >> i) & 0x01))
					{
						return false;
					}
				}
				return true;
			}

			return true;
		}

		return true;
	}
};
#endif