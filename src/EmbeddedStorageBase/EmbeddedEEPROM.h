#ifndef _EMBEDDED_EEPROM_
#define _EMBEDDED_EEPROM_

#if defined(ARDUINO_ARCH_AVR) && (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328p__) || defined(__AVR_atmega328p__) || defined(__AVR_ATtiny85__))
#include <stdint.h>
#include <EEPROM.h>

// Allocates a memory array of the same size as the EEPROM.
// For testing purposes only.
//#define EEPROM_MOCK_IN_MEMORY


// Checks if the write address is within Unit space, at run time.
// Disabled by default, for performance.
// Enable for validation, and supply optional Macro for error handling. 
// #define EEPROM_BOUNDS_CHECK

#if defined(EEPROM_BOUNDS_CHECK)
#define EEPROM_ON_ERROR(address) Serial.println(F("EEPROM Error"))
#else
#define EEPROM_ON_ERROR(address)
#endif

#if defined(EEPROM_MOCK_IN_MEMORY)
static uint8_t InMemory[E2END + 1]{};
#endif

/// <summary>
/// Interfaces the Arduino EEPROM,
///  with a defined set of operations for use by child classes.
/// </summary>
class EmbeddedEEPROM
{
public:
	static constexpr uint16_t Size() { return E2END + 1; };

#if defined(EEPROM_MOCK_IN_MEMORY)
	static void EraseEEPROM()
	{
		memset(InMemory, UINT8_MAX, sizeof(InMemory));
	}

	static void WriteBlock(const uint16_t offset, const uint8_t block)
	{
#if defined(EEPROM_BOUNDS_CHECK)
		CheckBounds(offset);
#endif
		InMemory[offset] = block;
	}

	static const uint8_t ReadBlock(const uint16_t offset)
	{
#if defined(EEPROM_BOUNDS_CHECK)
		CheckBounds(offset);
#endif
		return InMemory[offset];
	}

	static void ProgramZeroBitsToZero(const uint16_t offset, const uint8_t byteWithZeros)
	{
		InMemory[offset] &= byteWithZeros;
	}

	static void ClearByteToOnes(const uint16_t offset)
	{
		InMemory[offset] = UINT8_MAX;
	}
#else
		/// <summary>
		/// Clears the entire EEPROM memory. Handle with care.
		/// </summary>
		static void EraseEEPROM()
	{
		for (uint16_t i = 0; i < Size(); i++)
		{
			EEPROM.update(i, UINT8_MAX);
		}
	}

	static void WriteBlock(const uint16_t offset, const uint8_t block)
	{
#if defined(EEPROM_BOUNDS_CHECK)
		CheckBounds(offset);
#endif
		EEPROM.update(offset, block);
	}

	static const uint8_t ReadBlock(const uint16_t offset)
	{
#if defined(EEPROM_BOUNDS_CHECK)
		CheckBounds(offset);
#endif
		return EEPROM[offset];
	}


	/// <summary>
	/// Arduino EEPROMWearLevel Library flash twidling bits.
	/// https://github.com/PRosenb/EEPROMWearLevel/blob/master/src/avr/EEPROMWearLevelAvr.cpp
	/// </summary>
	/// <typeparam name="Address"></typeparam>
	/// <param name="offset"></param>
	/// <param name="byteWithZeros"></param>
	static void ProgramZeroBitsToZero(const uint16_t offset, const uint8_t byteWithZeros)
	{
		//// Wait for completion of any pending operations.
		//while (EECR & (1 << EEPE));

		// EEPROM Mode Bits.
		// EEPM1.0 = 0 0 - Mode 0 Erase & Write in one operation.
		// EEPM1.0 = 0 1 - Mode 1 Erase only.
		// EEPM1.0 = 1 0 - Mode 2 Write only.
		EECR |= 1 << EEPM1;
		EECR &= ~(1 << EEPM0);
		// EEPROM Ready Interrupt Enable.
		// EERIE = 0 - Interrupt Disable.
		// EERIE = 1 - Interrupt Enable.
		EECR &= ~(1 << EERIE);

		// Set EEPROM address - 0x000 - 0x3FF.
		EEAR = offset;

		// Data write into EEPROM.
		EEDR = byteWithZeros;

		uint8_t u8SREG = SREG;
		cli();
		// Wait for completion previous write.
		while (EECR & (1 << EEPE));
		// EEMPE = 1 - Master Write Enable.
		EECR |= (1 << EEMPE);
		// EEPE = 1 - Write Enable.
		EECR |= (1 << EEPE);
		SREG = u8SREG;

		// Wait for completion of write.
		while (EECR & (1 << EEPE));
	}

	static void ClearByteToOnes(const uint16_t offset)
	{
		// Wait for completion of any pending operations.
		while (EECR & (1 << EEPE));

		// EEPROM Mode Bits.
		// EEPM1.0 = 0 0 - Mode 0 Erase & Write in one operation.
		// EEPM1.0 = 0 1 - Mode 1 Erase only.
		// EEPM1.0 = 1 0 - Mode 2 Write only.
		// set EEPM0
		EECR |= 1 << EEPM0;
		// clear EEPM1
		EECR &= ~(1 << EEPM1);
		// EEPROM Ready Interrupt Enable.
		// EERIE = 0 - Interrupt Disable.
		// EERIE = 1 - Interrupt Enable.
		EECR &= ~(1 << EERIE);

		// Set EEPROM address - 0x000 - 0x3FF.
		EEAR = offset;

		uint8_t u8SREG = SREG;
		cli();
		// Wait for completion previous write.
		while (EECR & (1 << EEPE));
		// EEMPE = 1 - Master Write Enable.
		EECR |= (1 << EEMPE);
		// EEPE = 1 - Write Enable.
		EECR |= (1 << EEPE);
		SREG = u8SREG;

		// Wait for completion of any pending operations.
		while (EECR & (1 << EEPE));
	}
#endif

private:
#if defined(EEPROM_BOUNDS_CHECK)
	static void CheckBounds(int offset)
	{
		if ((uint32_t)offset >= Size())
		{
			EEPROM_ON_ERROR(offset);
		}
	}
#endif
};
#endif
#endif