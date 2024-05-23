#ifndef _STORAGE_ATTRIBUTOR_h
#define _STORAGE_ATTRIBUTOR_h

#include <stdint.h>
#include "VariadicParameters.h"

template<size_t...Sizes>
struct TemplateSizeAttributor
{
	static constexpr uint16_t GetUsed()
	{
		return SizeParameter::Sum(Sizes...);
	}

	static constexpr size_t GetCount()
	{
		return SizeParameter::Count<Sizes...>();
	}

	static constexpr size_t GetAddress(const size_t index)
	{
		return SizeParameter::SumUpTo(index, Sizes...);
	}

	static constexpr size_t GetSize(const size_t index)
	{
		return SizeParameter::Size(index, Sizes...);
	}
};


/// <summary>
/// Assumes StorageTypes have a ::Size static property.
/// Assumes StorageTypes have a ::WearLevelOption static property.
/// </summary>
/// <typeparam name="...StorageTypes"></typeparam>
template<typename... StorageTypes>
struct TemplateStorageAttributor
{
public:
	static constexpr uint16_t GetUsed()
	{
		return StorageParameter::Sum<StorageTypes...>();
	}

	static constexpr size_t GetCount()
	{
		return StorageParameter::Count<StorageTypes...>();
	}

	static constexpr size_t GetAddress(const size_t storageIndex)
	{
		return StorageParameter::SumUpTo<StorageTypes...>(storageIndex);
	}

	static constexpr size_t GetAddressByKey(const uint32_t key)
	{
		return StorageParameter::SumUpToKey<StorageTypes...>(key);
	}

	static constexpr size_t GetSize(const size_t storageIndex)
	{
		return StorageParameter::Size<StorageTypes...>(storageIndex);
	}

	static constexpr size_t GetSizeByKey(const uint32_t key)
	{
		return StorageParameter::SizeByKey<StorageTypes...>(key);
	}

	template<typename StorageType>
	static constexpr size_t GetAddressByKey()
	{
		return GetAddressByKey(StorageType::Key);
	}

	template<typename StorageType>
	static constexpr size_t GetSizeByKey()
	{
		return GetSizeByKey(StorageType::Key);
	}
};

#endif