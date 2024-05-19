#ifndef _EMBEDDED_CRC_
#define _EMBEDDED_CRC_

#include <CRC.h>

#if !defined(CRC_LIB_VERSION)
#pragma Depends on https://github.com/RobTillaart/CRC
#else
/// <summary>
/// Template based abstraction for 8 bit CRC calculation.
/// Depends on https://github.com/RobTillaart/CRC .
/// </summary>
/// <param name="Key">Crypto MAC key.</param>
template<const uint8_t Key = 0>
class EmbeddedCrc
{
private:
	CRC8 Crc8{};

public:
	EmbeddedCrc() {}

	const uint8_t GetCrc(const uint8_t* data, const uint16_t length, const uint8_t salt = 0)
	{
		Crc8.reset();
		Crc8.add(data, (size_t)length);
		Crc8.add(Key);
		Crc8.add(salt);

		return Crc8.calc();
	}
};
#endif
#endif