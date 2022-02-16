#ifndef _STORAGE_UNIT_
#define _STORAGE_UNIT_

#include <EmbeddedEEPROM.h>
#include <EmbeddedCrc.h>

/// <summary>
/// CRC checked EEPROM storage unit.
/// </summary>
/// <param name="SizeBytes">Data size in bytes.</param>
template<const handle_t SizeBytes, const uint8_t Key = 0>
class StorageUnit : private EmbeddedEEPROM
{
private:
	EmbeddedCrc<Key> Crc;

public:
	static constexpr uint16_t GetUsedBlockCount()
	{
		// 1 Block count for Data.
		// 1 Extra block for CRC.
		return GetDataBlockCount() + 1;
	}

private:
	static constexpr uint16_t GetDataBlockCount()
	{
		return EmbeddedEEPROM::GetDataBlockCount(SizeBytes);
	}

public:
	StorageUnit(const handle_t startBlockAddress)
		: EmbeddedEEPROM(startBlockAddress)
		, Crc()
	{}

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
		for (handle_t i = 0; i < GetDataBlockCount(); i++)
		{
			target[i] = ReadBlock(i);
		}

		return Crc.GetCrc(target, SizeBytes) == ReadBlock(GetDataBlockCount());
	}

	/// <summary>
	/// Writes the declared SizeBytes from source array.
	/// </summary>
	/// <param name="source">Source array.</param>
	void WriteData(const uint8_t* source)
	{
		for (handle_t i = 0; i < SizeBytes; i++)
		{
			WriteBlock(i, source[i]);
		}

		WriteBlock(GetDataBlockCount(), Crc.GetCrc(source, SizeBytes));
	}
};
#endif