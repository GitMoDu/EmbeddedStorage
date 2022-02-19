#ifndef _STORAGE_UNIT_
#define _STORAGE_UNIT_

#include <EmbeddedEEPROM.h>
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328p__) || defined(__AVR_atmega328p__) || defined(__AVR_ATtiny85__)

#include <EmbeddedCrc.h>

/// <summary>
/// CRC checked EEPROM storage unit.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="SizeBytes">Data size in bytes.</param>
/// <param name="Key">Storage cryptographic salt key.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t SizeBytes, const uint8_t Key = SizeBytes>
class StorageUnit : private EmbeddedEEPROM
{
private:
	EmbeddedCrc<Key> Crc;

public:
	static constexpr uint16_t GetUsedBlockCount()
	{
		// 1 Block count for Data.
		// 1 Extra block for CRC.
		return SizeBytes + 1;
	}

public:
	StorageUnit(const uint16_t startBlockAddress)
		: EmbeddedEEPROM(startBlockAddress)
		, Crc()
	{}

	/// <summary>
	/// Reads the declared SizeBytes into target array.
	/// </summary>
	/// <param name="target">Target array.</param>
	/// <returns>True if CRC matches.</returns>
	const bool ReadData(uint8_t* target)
	{
		for (uint16_t i = 0; i < SizeBytes; i++)
		{
			target[i] = ReadBlock(i);
		}

		return Crc.GetCrc(target, SizeBytes) == ReadBlock(SizeBytes);
	}

	/// <summary>
	/// Writes the declared SizeBytes from source array.
	/// </summary>
	/// <param name="source">Source array.</param>
	void WriteData(const uint8_t* source)
	{
		for (uint16_t i = 0; i < SizeBytes; i++)
		{
			WriteBlock(i, source[i]);
		}

		WriteBlock(SizeBytes, Crc.GetCrc(source, SizeBytes));
	}
};
#endif
#endif