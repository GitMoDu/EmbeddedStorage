#ifndef _STORAGE_ATTRIBUTOR_h
#define _STORAGE_ATTRIBUTOR_h

#include "EmbeddedStorageBase\EmbeddedEEPROM.h"
#if defined(EMBEDDED_EEPROM_STORAGE)

/// <summary>
/// Base attributor class with minimal memory footprint.
/// A simple way to normalize EEPROM address attribuions.
/// </summary>
class StorageAttributor
{
protected:
	virtual const uint8_t GetPartitionsCount() { return 0; }
	virtual const uint16_t GetPartitionSize(const uint8_t partition) { return 0; }

public:
	StorageAttributor() {}

	const bool Validate()
	{
		return GetHighestBlockSize() <= EEPROM_SIZE;
	}

	const uint16_t GetUsedSpace()
	{
		return GetHighestBlockSize();
	}

	const int16_t GetFreeSpace()
	{
		return (int32_t)EEPROM_SIZE - GetHighestBlockSize();
	}

	const uint16_t GetTotalSpace()
	{
		return EEPROM_SIZE;
	}

	const uint16_t GetUnitStartAddress(const uint8_t unitIndex)
	{
		uint16_t blockAddress = 0;
		for (uint8_t i = 0; i < unitIndex; i++)
		{
			blockAddress += GetPartitionSize(i);
		}

		return blockAddress;
	}
private:
	const uint16_t GetHighestBlockSize()
	{
		uint16_t blockAddress = 0;
		for (uint8_t i = 0; i < GetPartitionsCount(); i++)
		{
			blockAddress += GetPartitionSize(i);
		}

		return blockAddress;
	}
};
#endif
#endif