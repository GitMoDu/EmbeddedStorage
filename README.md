# EmbeddedStorage

Abstractions for storing, reading and managing data structs and arrays in EEPROM, inspired by Arduino EEPROMWearLevel Library.

## Features:
  - CRC validated data.
  - Templated StorageUnit, with a common interface for startup constants (address and size).
  - Wear levelling options (thank you @PRosenb https://github.com/PRosenb/EEPROMWearLevel).
  - Optional run-time bounds check with EEPROM_BOUNDS_CHECK.
  - Support for ATTiny85.
  - Static setup-time address allocator, for setup with multiple units in one project.

## Dependencies:
  - FastCRC: https://github.com/FrankBoesing/FastCRC
  - Arduino EEPROM
    - https://www.arduino.cc/en/Reference/EEPROM


## References
Special mention for Arduino EEPROMWearLevel Library flash twidling bits.
  - https://github.com/PRosenb/EEPROMWearLevel/blob/master/src/avr/EEPROMWearLevelAvr.cpp


## Library:
  - StorageUnit
    - Templated size and key.
    - Start address is passed on constructor.
    - CRC validated data.
    - CRC seed Key is optional and can be used for versioning. Defaults to Key = size.
    - 1 byte of EEPROM overhead.

  - WearLevelUnit
    - Same base features as StorageUnit.
    - 1 byte of EEPROM overhead per level option, plus counter.
    - 1 to 2 bytes of counter EEPROM overhead.
    - Wear levelling options start at x2. For x1 use StorageUnit.
    - Wear level units:
      - Tiny: from x2 to x8 levels of data. 1 extra byte of EEPROM overhead.
      - Short: from x9 to x16 levels of data. 2 extra bytes of EEPROM overhead.
