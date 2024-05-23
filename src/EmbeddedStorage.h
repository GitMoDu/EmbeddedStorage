#ifndef _EMBEDDED_STORAGE_
#define _EMBEDDED_STORAGE_

#include <stdint.h>
#include <WearLevelType.h>

class EmbeddedStorage
{
public:
	/// <summary>
	/// 1 Extra block for CRC.
	/// ||Data...|CRC||
	/// </summary>
	/// <param name="dataSize"></param>
	/// <param name="wearLevelOption"></param>
	/// <returns></returns>
	static constexpr uint16_t GetStorageSize(const uint16_t dataSize, const NoWearLevel wearLevelOption = NoWearLevel::x1)
	{
		return GetSize(dataSize, (uint8_t)NoWearLevel::x1);
	}

	/// <summary>
	/// 1 Extra block for CRC for every Data, times WearLevelOption.
	/// || Counter | Data1... | CRC1 || DataN... | CRCN ||
	/// </summary>
	/// <param name="dataSize"></param>
	/// <param name="wearLevelOption"></param>
	/// <returns></returns>
	static constexpr uint16_t GetStorageSize(const uint16_t dataSize, const WearLevelTiny wearLevelOption)
	{
		return GetSize(dataSize, (uint8_t)wearLevelOption);
	}

	/// <summary>
	/// 1 Extra block for CRC for every Data, times WearLevelOption.
	/// || Counter | Data1... | CRC1 || DataN... | CRCN ||
	/// </summary>
	/// <param name="dataSize"></param>
	/// <param name="wearLevelOption"></param>
	/// <returns></returns>
	static constexpr uint16_t GetStorageSize(const uint16_t dataSize, const WearLevelShort wearLevelOption)
	{
		return GetSize(dataSize, (uint8_t)wearLevelOption);
	}

	/// <summary>
	/// 1 Extra block for CRC for every Data, times WearLevelOption.
	/// || Counter | Data1... | CRC1 || DataN... | CRCN ||
	/// </summary>
	/// <param name="dataSize"></param>
	/// <param name="wearLevelOption"></param>
	/// <returns></returns>
	static constexpr uint16_t GetStorageSize(const uint16_t dataSize, const WearLevelLong wearLevelOption)
	{
		return GetSize(dataSize, (uint8_t)wearLevelOption);
	}

	/// <summary>
	/// 1 Extra block for CRC for every Data, times WearLevelOption.
	/// || Counter | Data1... | CRC1 || DataN... | CRCN ||
	/// </summary>
	/// <param name="dataSize"></param>
	/// <param name="wearLevelOption"></param>
	/// <returns></returns>
	static constexpr uint16_t GetStorageSize(const uint16_t dataSize, const WearLevelLongLong wearLevelOption)
	{
		return GetSize(dataSize, (uint8_t)wearLevelOption);
	}

private:
	/// <summary>
	/// Storage be a single data/crc pair.
	/// ||Data...|CRC||
	/// Or it can be N sized data/crc + counter.
	/// ||Counter|Data1...|CRC1||DataN...|CRCN||
	/// </summary>
	/// <param name="dataSize"></param>
	/// <param name="wearLevelOption"></param>
	/// <returns></returns>
	static constexpr uint16_t GetSize(const uint16_t dataSize, const uint8_t wearLevelOption)
	{
		return ((wearLevelOption > (uint8_t)NoWearLevel::x1) * (GetWearLevelCounterSize(wearLevelOption))) +
			((1 + dataSize) * wearLevelOption);
	}

#if defined(WEAR_LEVEL_DEBUG)
public:
#else
private:
#endif
	static constexpr uint16_t GetWearLevelCounterSize(const uint8_t wearLevelOption)
	{
		return ((wearLevelOption <= (uint8_t)WearLevelTiny::x9) * sizeof(uint8_t))
			| (((wearLevelOption > (uint8_t)WearLevelTiny::x9) * (wearLevelOption <= (uint8_t)WearLevelShort::x17)) * sizeof(uint16_t))
			| (((wearLevelOption > (uint8_t)WearLevelShort::x17) * (wearLevelOption <= (uint8_t)WearLevelLong::x33)) * sizeof(uint32_t))
			| (((wearLevelOption > (uint8_t)WearLevelLong::x33) * (wearLevelOption <= (uint8_t)WearLevelLongLong::x65)) * sizeof(uint64_t));
	}
};
#endif