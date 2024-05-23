#ifndef _STORAGE_UNIT_
#define _STORAGE_UNIT_

#include "EmbeddedStorageBase\EmbeddedEEPROM.h"
#include "EmbeddedStorageBase\EmbeddedCrc.h"
#include <EmbeddedStorage.h>

/// <summary>
/// CRC checked EEPROM storage unit.
/// Designed for use with a single data struct or array.
/// </summary>
/// <param name="DataSize">Data size in bytes.</param>
/// <param name="Key">Storage cryptographic salt key.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t address,
	const uint16_t DataSize,
	const uint32_t Key = DataSize>
class StorageUnit
{
private:
	EmbeddedCrc<Key> Crc{};

public:
	static constexpr uint16_t Address()
	{
		return address;
	}

	static constexpr uint16_t Size()
	{
		return EmbeddedStorage::GetStorageSize(DataSize);
	}

public:
	StorageUnit()
	{
		EEPROM.begin();
	}

	/// <summary>
	/// Reads the declared DataSize into target array.
	/// </summary>
	/// <param name="target">Target array.</param>
	/// <returns>True if CRC matches.</returns>
	const bool ReadData(uint8_t* target)
	{
		for (uint16_t i = 0; i < DataSize; i++)
		{
			target[i] = EmbeddedEEPROM::ReadBlock(address + i);
		}

		return Crc.GetCrc(target, DataSize) == EmbeddedEEPROM::ReadBlock(address + DataSize);
	}

	/// <summary>
	/// Writes the declared DataSize from source array.
	/// </summary>
	/// <param name="source">Source array.</param>
	void WriteData(const uint8_t* source)
	{
		for (uint16_t i = 0; i < DataSize; i++)
		{
			EmbeddedEEPROM::WriteBlock(address + i, source[i]);
		}

		EmbeddedEEPROM::WriteBlock(address + DataSize, Crc.GetCrc(source, DataSize));
	}

	void WriteByte(const uint16_t offset, const uint8_t value)
	{
		EmbeddedEEPROM::WriteBlock(address + offset, value);
	}

	const uint8_t ReadByte(const uint16_t offset)
	{
		return EmbeddedEEPROM::ReadBlock(address + offset);
	}
};
#endif