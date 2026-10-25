#ifndef _STORAGE_UNIT_
#define _STORAGE_UNIT_

#include "EmbeddedStorageBase\EmbeddedEEPROM.h"
#include "EmbeddedStorageBase\EmbeddedCrc.h"
#include <EmbeddedStorage.h>

/// <summary>
/// CRC checked EEPROM storage unit.
/// Designed for use with a single data struct or array.
/// </summary>
/// <typeparam name="address">Starting EEPROM address.</typeparam>
/// <typeparam name="DataSize">Number of data bytes, excluding the stored CRC.</typeparam>
/// <typeparam name="Key">CRC key. Changing the key invalidates previously written data.</typeparam>
template<const uint16_t address,
	const uint16_t DataSize,
	const uint32_t Key = DataSize>
class StorageUnit
{
protected:
	EmbeddedCrc<Key> Crc{};

public:
	/// <summary>
	/// Gets the first EEPROM address used by this unit.
	/// </summary>
	static constexpr uint16_t Address()
	{
		return address;
	}

	/// <summary>
	/// Gets the total EEPROM space reserved by this unit, including its CRC byte.
	/// </summary>
	static constexpr uint16_t Size()
	{
		return EmbeddedStorage::GetStorageSize(DataSize);
	}

public:
	StorageUnit() {}

	/// <summary>
	/// Reads the declared DataSize into target array.
	/// </summary>
	/// <param name="target">Target array.</param>
	/// <returns>True if CRC matches.</returns>
	const bool ReadData(uint8_t* target)
	{
		if (EmbeddedEEPROM::IsAsyncOperationActive()) { return false; }
		for (uint16_t i = 0; i < DataSize; i++)
		{
			target[i] = EmbeddedEEPROM::ReadBlock(address + i);
		}

		return GetCrc(target, DataSize) == EmbeddedEEPROM::ReadBlock(address + DataSize);
	}

	/// <summary>
	/// Writes the declared DataSize from source array.
	/// </summary>
	/// <param name="source">Source array.</param>
	void WriteData(const uint8_t* source)
	{
		if (EmbeddedEEPROM::IsAsyncOperationActive()) { return; }
		for (uint16_t i = 0; i < DataSize; i++)
		{
			EmbeddedEEPROM::WriteBlock(address + i, source[i]);
		}

		EmbeddedEEPROM::WriteBlock(address + DataSize, GetCrc(source, DataSize));
	}

	/// <summary>
	/// Writes one byte at an offset from the start of this unit.
	/// </summary>
	/// <param name="offset">Byte offset within this unit.</param>
	/// <param name="value">Value to write.</param>
	void WriteByte(const uint16_t offset, const uint8_t value)
	{
		EmbeddedEEPROM::WriteBlock(address + offset, value);
	}

	/// <summary>
	/// Reads one byte at an offset from the start of this unit.
	/// </summary>
	/// <param name="offset">Byte offset within this unit.</param>
	/// <returns>The byte stored at the specified offset.</returns>
	const uint8_t ReadByte(const uint16_t offset)
	{
		return EmbeddedEEPROM::ReadBlock(address + offset);
	}

protected:
	const uint8_t GetCrc(const uint8_t* data, const uint16_t length)
	{
		return Crc.GetCrc(data, length);
	}
};
#endif