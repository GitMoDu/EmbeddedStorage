/*

Depends on https://github.com/GitMoDu/EEPROMWearLevel

*/

#if !defined(_EMBEDDED_STORAGE_)
#define _EMBEDDED_STORAGE_

#include <stdint.h>
#include <EEPROMWearLevel.h>
#include <EmbeddedData.h>

template<const uint8_t BaseVersionCode, const uint8_t MaxPartitions = 5>
class EmbeddedStorage : public virtual IEmbeddedStorage
{
public:
	static const uint8_t MaxDataSize = INT8_MAX;

private:
	EEPROMWearLevel Storage;

	struct PartitionStruct
	{
		IEmbeddedData* Data = nullptr;
		uint8_t Index = 0;
	};

	PartitionStruct Partitions[MaxPartitions];
	uint8_t Count = 0;

public:
	EmbeddedStorage()
		: IEmbeddedStorage()
		, Storage()
	{
	}

	virtual bool AddEmbeddedData(IEmbeddedData* data)
	{
		if (Count >= MaxPartitions || data->GetSize() > MaxDataSize)
		{
			return false;
		}
		else
		{
			// Validate key doesn't already exist.
			for (uint8_t i = 0; i < Count; i++)
			{
				if (Partitions[i].Data->GetKey() == data->GetKey())
				{
					return false;
				}
			}

			// 2 Bytes for control are reserved.
			//TODO: Generalize.
			Partitions[Count].Data = data;
			Partitions[Count].Index = Count;
			Count++;

			return true;
		}
	}

	// Version code automatically changes if the partition list and keys don't match.
	// Keys are large
	uint8_t GetVersionCode()
	{
		uint32_t rollingKey = BaseVersionCode;

		for (uint8_t i = 0; i < Count; i++)
		{
			rollingKey += Partitions[i].Data->GetKey();
		}

		// Pseudo hash to use all bits.
		uint8_t version = rollingKey & 0xFF;
		version += (rollingKey >> 8) & 0xFF;
		version += (rollingKey >> 16) & 0xFF;
		version += (rollingKey >> 24) & 0xFF;

		return version;
	}

	bool SetData(const uint32_t key, const uint8_t* data)
	{
		for (uint8_t i = 0; i < Count; i++)
		{
			if (Partitions[i].Data->GetKey() == key)
			{
				eeprom_busy_wait();

				Storage.putArray(Partitions[i].Index, data, Partitions[i].Data->GetSize());

				return true;
			}
		}

		return false;
	}

	bool GetData(const uint32_t key, uint8_t* data)
	{
		for (uint8_t i = 0; i < Count; i++)
		{
			if (Partitions[i].Data->GetKey() == key)
			{
				eeprom_busy_wait();

				return Storage.getArray(Partitions[i].Index, data, Partitions[i].Data->GetSize());
			}
		}

		return false;
	}

	// To be called after all EmbeddedData objects have been added and set-up.
	bool Setup()
	{
		int lengths[Count];
		uint16_t totalLength = 0;

		for (uint8_t i = 0; i < Count; i++)
		{
			lengths[i] = Partitions[i].Data->GetSize() + 4;
			totalLength += lengths[i];
		}

		Storage.begin(GetVersionCode(), lengths, Count);

		eeprom_busy_wait();


		for (uint8_t i = 0; i < Count; i++)
		{
			Partitions[i].Data->Load();
		}

		return true;
	}
};

#endif