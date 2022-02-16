#ifndef _EMBEDDED_EEPROM_
#define _EMBEDDED_EEPROM_

#include <stdint.h>
#include <EEPROM.h>


#if defined(ARDUINO_ARCH_AVR)
#define handle_t uint16_t
#if defined(__AVR_ATtiny85__)
#define handle_t uint8_t
#endif
#else
#error Unsuported board.
#endif 

#define EEPROM_BOUNDS_CHECK
#if defined(EEPROM_BOUNDS_CHECK)
#define EEPROM_ON_ERROR Serial.println(F("EEPROM Error"));
#else
#define EEPROM_ON_ERROR
#endif

/// <summary>
/// Interfaces the Arduino EEPROM,
///  with a defined set of operations for use by child classes.
/// </summary>
class EmbeddedEEPROM
{
private:
	const handle_t StartBlockAddress;

public:
	static constexpr  uint16_t GetDataBlockCount(const uint16_t sizeBytes) {
		// 1 Block holds 1 byte of data.
		return sizeBytes;
	}

public:
	EmbeddedEEPROM(const handle_t startBlockAddress)
		: StartBlockAddress(startBlockAddress)
	{}

protected:
	void EraseBlock(const handle_t offset)
	{
		EEPROM[StartBlockAddress + offset] = 0;
	}

	void WriteUpdateBlock(const handle_t offset, const uint8_t block)
	{
		EEPROM.update(StartBlockAddress + offset, block);
	}

	void WriteBlock(const handle_t offset, const uint8_t block)
	{
		EEPROM[StartBlockAddress + offset] = block;
	}

	const uint8_t ReadBlock(const handle_t offset)
	{
		return EEPROM[StartBlockAddress + offset];
	}
};
#endif