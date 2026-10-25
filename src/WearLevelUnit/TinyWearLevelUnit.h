#ifndef _TINY_WEAR_LEVEL_UNIT_
#define _TINY_WEAR_LEVEL_UNIT_

#include "BaseWearLevelUnit.h"
#include <WearLevelType.h>

namespace EmbeddedStorage
{
	namespace Internal
	{
		template<const uint16_t Address,
			const uint16_t DataSize,
			const WearLevelTiny Option,
			const uint32_t Key,
			typename WearLevelBase>
		class TinyWearLevelUnit : public WearLevelBase
		{
		private:
			using BaseClass = WearLevelBase;
			using BaseClass::Uint8Min;

		private:
			enum CounterEnum : uint8_t
			{
				c0 = 0b11111111,
				c1 = 0b01111111,
				c2 = 0b00111111,
				c3 = 0b00011111,
				c4 = 0b00001111,
				c5 = 0b00000111,
				c6 = 0b00000011,
				c7 = 0b00000001,
				c8 = 0b00000000,
				EnumCount
			};

		public:
			TinyWearLevelUnit() : BaseClass()
			{
				this->Initialize();
			}

#if defined(WEAR_LEVEL_DEBUG)
			const uint8_t DebugMask()
			{
				return GetMask();
			}
#endif

		private:
			const uint8_t GetMask()
			{
				return EmbeddedEEPROM::ReadBlock(Address + 0);
			}

			const uint8_t GetMask(const uint8_t counter)
			{
				switch (counter)
				{
				case 0:
					return CounterEnum::c0;
				case 1:
					return CounterEnum::c1;
				case 2:
					return CounterEnum::c2;
				case 3:
					return CounterEnum::c3;
				case 4:
					return CounterEnum::c4;
				case 5:
					return CounterEnum::c5;
				case 6:
					return CounterEnum::c6;
				case 7:
					return CounterEnum::c7;
				case 8:
					return CounterEnum::c8;
				default:
					return CounterEnum::c0;
				}
			}

		protected:
			const uint8_t GetCurrentCounter() final
			{
				switch (GetMask())
				{
				case CounterEnum::c0:
					return 0;
				case CounterEnum::c1:
					return 1;
				case CounterEnum::c2:
					return Uint8Min(2, (uint8_t)Option - 1);
				case CounterEnum::c3:
					return Uint8Min(3, (uint8_t)Option - 1);
				case CounterEnum::c4:
					return Uint8Min(4, (uint8_t)Option - 1);
				case CounterEnum::c5:
					return Uint8Min(5, (uint8_t)Option - 1);
				case CounterEnum::c6:
					return Uint8Min(6, (uint8_t)Option - 1);
				case CounterEnum::c7:
					return Uint8Min(7, (uint8_t)Option - 1);
				case CounterEnum::c8:
					return Uint8Min(8, (uint8_t)Option - 1);
				default:
					return 0;
				}
			}

			/// <summary>
			/// Increments the counter in a flash compatible way.
			/// From 0 to 8 (Option).
			/// </summary>
			/// <returns>Current Counter</returns>
			bool PrepareNextCounter(uint8_t& counter) final
			{
				counter = GetCurrentCounter();
				if (counter + 1 >= (uint8_t)Option)
				{
					counter = 0;
					this->AppendCounterOperation(Address + 0, UINT8_MAX, EmbeddedEEPROM::AsyncWriteModeEnum::EraseOnly);
				}
				else
				{
					counter++;
					this->AppendCounterOperation(Address + 0, GetMask(counter), EmbeddedEEPROM::AsyncWriteModeEnum::ClearBits);
				}

				return true;
			}

			const bool ValidateCounterMask() final
			{
				switch (GetMask())
				{
				case CounterEnum::c0:
				case CounterEnum::c1:
				case CounterEnum::c2:
				case CounterEnum::c3:
				case CounterEnum::c4:
				case CounterEnum::c5:
				case CounterEnum::c6:
				case CounterEnum::c7:
				case CounterEnum::c8:
					return GetCurrentCounter() <= (uint8_t)Option;
				default:
					return false;
				}
			}
		};
	}
}

/// <summary>
/// Synchronous CRC-checked EEPROM storage with a compact wear-level counter.
/// Stores up to Option data records and is intended for a data structure or array.
/// </summary>
/// <typeparam name="Address">Starting EEPROM address.</typeparam>
/// <typeparam name="DataSize">Number of data bytes per record, excluding its CRC.</typeparam>
/// <typeparam name="Option">Number of rotating data records, from 2 to 8.</typeparam>
/// <typeparam name="Key">CRC key. Changing the key invalidates previously written data.</typeparam>
template<const uint16_t Address,
	const uint16_t DataSize,
	const WearLevelTiny Option = WearLevelTiny::x2,
	const uint32_t Key = EmbeddedStorage::GetStorageSize(DataSize, Option)>
class TinyWearLevelUnit
	: public EmbeddedStorage::Internal::TinyWearLevelUnit<Address, DataSize, Option, Key,
	BaseWearLevelUnit<Address, DataSize, Key, WearLevelTiny, Option>>
{};

/// <summary>
/// Asynchronous CRC-checked EEPROM storage with a compact wear-level counter.
/// The source buffer must remain valid and unchanged until the write completes.
/// </summary>
/// <typeparam name="Address">Starting EEPROM address.</typeparam>
/// <typeparam name="DataSize">Number of data bytes per record, excluding its CRC.</typeparam>
/// <typeparam name="Option">Number of rotating data records, from 2 to 8.</typeparam>
/// <typeparam name="Key">CRC key. Changing the key invalidates previously written data.</typeparam>
template<const uint16_t Address,
	const uint16_t DataSize,
	const WearLevelTiny Option = WearLevelTiny::x2,
	const uint32_t Key = EmbeddedStorage::GetStorageSize(DataSize, Option)>
class AsyncTinyWearLevelUnit
	: public EmbeddedStorage::Internal::TinyWearLevelUnit<Address, DataSize, Option, Key,
	AsyncBaseWearLevelUnit<Address, DataSize, Key, WearLevelTiny, Option>>
{};
#endif