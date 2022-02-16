#ifndef _EMBEDDED_CRC_
#define _EMBEDDED_CRC_

#include <FastCRC.h>


/// <summary>
/// Depends on https://github.com/FrankBoesing/FastCRC
/// </summary>
template<const uint16_t Key = 0>
class EmbeddedCrc
{
private:
	FastCRC16 ModBus;
	uint8_t Seed;

public:
	EmbeddedCrc() {}

	const uint16_t GetCrc(const uint8_t* data, const handle_t length, const uint16_t salt = 0)
	{
		Seed = Key;
		ModBus.modbus(&Seed, (size_t)sizeof(uint16_t));
		Seed = salt;
		ModBus.modbus_upd(&Seed, (size_t)length);

		return ModBus.modbus_upd(data, (size_t)length);
	}
};
#endif