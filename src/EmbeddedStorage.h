/*

Depends on https://github.com/PRosenb/EEPROMWearLevel.git

*/

#if !defined(_EMBEDDED_STORAGE_)
#define _EMBEDDED_STORAGE_

#include <stdint.h>
#include <EEPROMWearLevel.h>
#include "EmbeddedData.h"

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

	bool SetData(const uint32_t key, const uint8_t* data)
	{
		for (uint8_t i = 0; i < Count; i++)
		{
			if (Partitions[i].Data->GetKey() == key)
			{
				Storage.putArray(Partitions[i].Index, data, Partitions[i].Data->GetSize());

				eeprom_busy_wait();


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

		Storage.begin(BaseVersionCode, lengths, Count);

		eeprom_busy_wait();


		for (uint8_t i = 0; i < Count; i++)
		{
			Partitions[i].Data->Load();
		}

		return true;
	}
};

#endif