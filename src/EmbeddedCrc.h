#ifndef _EMBEDDED_CRC_
#define _EMBEDDED_CRC_

#include <FastCRC.h>


/// <summary>
/// Depends on https://github.com/FrankBoesing/FastCRC
/// </summary>
template<const uint8_t Key = 0>
class EmbeddedCrc
{
private:
	FastCRC8 SmBus;
	uint8_t Seed;

public:
	EmbeddedCrc() {}

	const uint16_t GetCrc(const uint8_t* data, const handle_t length, const uint8_t salt = 0)
	{
		SmBus.smbus(data, (size_t)length);

		Seed = Key;
		SmBus.smbus_upd(&Seed, (size_t)sizeof(uint8_t));

		return SmBus.smbus_upd(&salt, (size_t)sizeof(uint8_t));
	}
};
#endif