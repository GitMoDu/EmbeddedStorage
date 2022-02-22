#ifndef _EMBEDDED_EEPROM_
#define _EMBEDDED_EEPROM_



// #define EEPROM_BOUNDS_CHECK
// Checks if the write address is within Unit space, at run time.
// Disabled by default, for performance.
// Enable for validation, and supply optional Macro for error handling. 

// #define EEPROM_RAM_DATA_SIZE 
// Uses memory array to mock EEPROM.
// Disabled by default, used only for validation.

#if defined(EEPROM_BOUNDS_CHECK)
#define EEPROM_ON_ERROR(address) Serial.println(F("EEPROM Error"))
#else
#define EEPROM_ON_ERROR(address)
#endif


#include <stdint.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328p__) || defined(__AVR_atmega328p__) || defined(__AVR_ATtiny85__)
#define EMBEDDED_EEPROM_STORAGE
#if defined(EEPROM_RAM_DATA_SIZE)
static uint8_t FakeEEPROM[EEPROM_RAM_DATA_SIZE];
#define EEPROM_SIZE EEPROM_RAM_DATA_SIZE
#else
#include <EEPROM.h>
#define EEPROM_SIZE EEPROM.length()
#endif

/// <summary>
/// Interfaces the Arduino EEPROM,
///  with a defined set of operations for use by child classes.
/// </summary>
#if defined(ARDUINO_ARCH_AVR)
class EmbeddedEEPROM
{
protected:
	const uint16_t StartBlockAddress;

public:
	EmbeddedEEPROM(const uint16_t startBlockAddress)
		: StartBlockAddress(startBlockAddress)
	{}

	/// <summary>
	/// Clears the entire EEPROM memory. Handle with care.
	/// </summary>
	static void EraseEEPROM()
	{
		for (uint16_t i = 0; i < EEPROM_SIZE; i++)
		{
#if defined(EEPROM_RAM_DATA_SIZE)
			FakeEEPROM[i] = UINT8_MAX;
#else
			EEPROM.update(i, UINT8_MAX);
#endif
		}
	}

#if defined(EEPROM_BOUNDS_CHECK)
private:
	void CheckBounds(int offset)
	{
		if (StartBlockAddress >= EEPROM_SIZE
			|| (StartBlockAddress + offset) >= EEPROM_SIZE)
		{
			EEPROM_ON_ERROR(StartBlockAddress + offset);
		}
	}
#endif

protected:
	void WriteBlock(const uint16_t offset, const uint8_t block)
	{
#if defined(EEPROM_BOUNDS_CHECK)
		CheckBounds(offset);
#endif
#if defined(EEPROM_RAM_DATA_SIZE)
		FakeEEPROM[StartBlockAddress + offset] = block;
#else
		EEPROM.update(StartBlockAddress + offset, block);
#endif
	}

	const uint8_t ReadBlock(const uint16_t offset)
	{
#if defined(EEPROM_BOUNDS_CHECK)
		CheckBounds(offset);
#endif
#if defined(EEPROM_RAM_DATA_SIZE)
		return FakeEEPROM[StartBlockAddress + offset];
#else
		return EEPROM[StartBlockAddress + offset];
#endif
	}

	// Arduino EEPROMWearLevel Library flash twidling bits.
	// https://github.com/PRosenb/EEPROMWearLevel/blob/master/src/avr/EEPROMWearLevelAvr.cpp
	void ProgramZeroBitsToZero(const uint16_t offset, const uint8_t byteWithZeros)
	{
#if defined(EEPROM_RAM_DATA_SIZE)
		FakeEEPROM[StartBlockAddress + offset] = byteWithZeros & FakeEEPROM[StartBlockAddress + offset];
#else
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
		EEAR = StartBlockAddress + offset;

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
#endif
	}

	void ClearByteToOnes(const uint16_t offset)
	{
#if defined(EEPROM_RAM_DATA_SIZE)
		FakeEEPROM[StartBlockAddress + offset] = UINT8_MAX;
#else
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
		EEAR = StartBlockAddress + offset;

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
#endif
	}
};
#endif
#else
#pragma Unsuported board.
#endif 
#endif