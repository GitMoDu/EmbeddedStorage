# ![Embedded Storage](https://github.com/GitMoDu/EmbeddedStorage/blob/master/media/banner.svg)

Read, write, and manage CRC-validated data in EEPROM on supported Arduino AVR
boards. Inspired by the Arduino EEPROMWearLevel library.

## Features

- CRC-validated storage for a data structure or byte array.
- Synchronous storage units and asynchronous write variants.
- Optional wear leveling with 2–65 rotating data records.
- Compile-time storage sizing and layout helpers.
- Optional runtime EEPROM bounds checking.
- Optional in-memory EEPROM mock for testing.

## Supported hardware

The EEPROM backend is enabled for AVR builds targeting ATmega328P or ATtiny85.
The library uses Arduino EEPROM.

## Dependencies

- Arduino EEPROM
- [CRC](https://github.com/RobTillaart/CRC) by Rob Tillaart

## Storage units

`StorageUnit` stores one data record followed by one CRC byte. Its starting
EEPROM address, payload size, and optional CRC key are template parameters:

```cpp
#include <StorageUnit.h>

struct Settings
{
    uint8_t mode;
    uint16_t interval;
};

using SettingsStorage =
    StorageUnit<0, sizeof(Settings), 1001>;

SettingsStorage settingsStorage;
```

Read and write the object as bytes:

```cpp
Settings settings{1, 250};

settingsStorage.WriteData(
    reinterpret_cast<const uint8_t*>(&settings));

Settings loaded{};
const bool valid = settingsStorage.ReadData(
    reinterpret_cast<uint8_t*>(&loaded));
```

`ReadData()` returns `true` when the stored CRC matches the data. The optional
`Key` template parameter is included in CRC calculation; changing it makes
previously stored data fail validation. It is a CRC input, not encryption or
authentication. If omitted, the key defaults to `DataSize`.

Use `StorageUnit::Size()` to get the total EEPROM space reserved by a unit,
including its CRC byte. `StorageUnit::Address()` returns its starting address.

## Wear-level units

Wear-level units rotate writes across multiple data records. Each record
contains the payload and its own CRC byte. The unit also stores a counter that
tracks the current record.

| Unit type | Options | Counter storage |
| --- | ---: | ---: |
| `TinyWearLevelUnit` | x2–x9 | 1 byte |
| `ShortWearLevelUnit` | x10–x17 | 2 bytes |
| `LongWearLevelUnit` | x18–x33 | 4 bytes |
| `LongLongWearLevelUnit` | x34–x65 | 8 bytes |

Each record uses `DataSize` bytes plus one CRC byte. The total reserved space,
including the counter, is available from the unit’s `Size()` method.

For example:

```cpp
#include <WearLevelUnit.h>

using SettingsWearLevel =
    TinyWearLevelUnit<0, sizeof(Settings), WearLevelTiny::x5>;

SettingsWearLevel settingsWearLevel;
```

The wear-level types are declared in `WearLevelType.h`: `WearLevelTiny`,
`WearLevelShort`, `WearLevelLong`, and `WearLevelLongLong`.

### How the wear-level counter works

The counter is not stored as an ordinary binary number. It uses a
thermometer-style bit mask: each step clears one additional bit. EEPROM can
program those bits from 1 to 0 without first erasing the counter byte.

For a tiny unit configured with three records, the counter progresses like
this. Each write advances the counter and selects the corresponding slot:

```text
Write             Counter byte       Selected record
Initial state     11111111           (slot 0)
1                 01111111           slot 1
2                 00111111           slot 2
3                 11111111           slot 0  <- erase counter on rollover
4                 01111111           slot 1
...               ...                ...
```

The counter byte is at a fixed address. It is **not** rotated with the data
slots. The bit-mask representation means the counter can advance by clearing
bits, without erasing that byte on every write. When the configured record
count rolls over, the counter storage is erased and counting starts again.

Meanwhile, the payload writes rotate through the data slots:

```text
Write 1       Write 2       Write 3       Write 4
  slot 1  →     slot 2  →     slot 0  →     slot 1  →  ...
```

Thus each payload slot is reused only after writes have cycled through the
other slots. The counter still has its own wear; this scheme avoids erasing it
on each increment, but does not eliminate EEPROM wear.

The CRC is calculated using the counter as a salt, so the record is checked in
the context of its slot. A slot may contain an older value until a later write
reuses it; wear leveling is not a history API.

## Asynchronous writes

`AsyncStorageUnit` and the `Async*WearLevelUnit` variants provide
non-blocking writes. Start a write with `BeginWriteData()`, then call
`PollWriteData()` repeatedly until it returns `Complete` or `Failed`.

```cpp
#include <AsyncStorageUnit.h>

using AsyncSettingsStorage =
    AsyncStorageUnit<0, sizeof(Settings), 1001>;

AsyncSettingsStorage asyncSettingsStorage;
Settings settings{1, 250};

if (asyncSettingsStorage.BeginWriteData(
        reinterpret_cast<const uint8_t*>(&settings)))
{
    EmbeddedEEPROM::AsyncOperationStateEnum state;
    do
    {
        state = asyncSettingsStorage.PollWriteData();
    }
    while (state == EmbeddedEEPROM::AsyncOperationStateEnum::Pending);

    if (state == EmbeddedEEPROM::AsyncOperationStateEnum::Complete)
    {
        // The write completed.
    }
}
```

The source buffer must remain valid and unchanged until polling reports
completion. Only one asynchronous EEPROM operation can be active at a time.
Do not start another EEPROM operation until the asynchronous write completes.

Include `WearLevelUnit.h` for the asynchronous wear-level types:
`AsyncTinyWearLevelUnit`, `AsyncShortWearLevelUnit`,
`AsyncLongWearLevelUnit`, and `AsyncLongLongWearLevelUnit`.

## Compile-time layout helpers

`TemplateSizeAttributor` calculates addresses and sizes from a list of EEPROM
reservation sizes. `TemplateStorageAttributor` calculates them from a sequence
of storage types. Each type supplies its payload `Size`, `WearLevelOption`,
and `Key`.

The attributor places units consecutively in the order provided:

```text
EEPROM addresses increase →

+------------------+----------------------+-----------------------+
| Storage 1        | Storage 2            | Storage 3             |
| address 0        | address 2            | address 18            |
| reserved 2 B     | reserved 16 B        | reserved 52 B         |
+------------------+----------------------+-----------------------+
|<--------------------------- 70 bytes total -------------------->|
```

For example, these reservations correspond to a 1-byte non-wear-level
payload, a 2-byte payload using `WearLevelTiny::x5`, and a 4-byte payload
using `WearLevelShort::x10`. The reserved sizes include CRC bytes and any
wear-level counter.

Each storage type supplies its own `Key`; the attributor does not generate or
assign keys. It can use a key to look up the matching unit’s address or size.
Use unique keys when looking up units by key. See
`Examples/UnitTests/UnitTests.ino` for a layout example.

## Configuration and testing

Define `EEPROM_BOUNDS_CHECK` before including the library to enable runtime
EEPROM address checks.

Define `EEPROM_MOCK_IN_MEMORY` before including the library to use an
in-memory EEPROM mock instead of hardware EEPROM. This is intended for testing.

See the `Examples` directory for storage, wear-leveling, and unit-test
examples.

## References and acknowledgments

The EEPROM wear-leveling approach was inspired by
[Arduino EEPROMWearLevel](https://github.com/PRosenb/EEPROMWearLevel).
In particular, this project references its AVR EEPROM bit-manipulation
implementation:

- [EEPROMWearLevelAvr.cpp](https://github.com/PRosenb/EEPROMWearLevel/blob/master/src/avr/EEPROMWearLevelAvr.cpp)
