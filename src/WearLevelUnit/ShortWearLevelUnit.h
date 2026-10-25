#ifndef _SHORT_WEAR_LEVEL_UNIT_
#define _SHORT_WEAR_LEVEL_UNIT_

#include "BaseWearLevelUnit.h"
#include <WearLevelType.h>


namespace EmbeddedStorage
{
	namespace Internal
	{
		template<const uint16_t Address,
			const uint16_t DataSize,
			const WearLevelShort Option,
			const uint32_t Key,
			typename WearLevelBase>
		class ShortWearLevelUnit : public WearLevelBase
		{
		private:
			using BaseClass = WearLevelBase;
			using BaseClass::Uint8Min;

		private:
			enum CounterEnum : uint16_t
			{
				c0 = 0b1111111111111111,
				c1 = 0b0111111111111111,
				c2 = 0b0011111111111111,
				c3 = 0b0001111111111111,
				c4 = 0b0000111111111111,
				c5 = 0b0000011111111111,
				c6 = 0b0000001111111111,
				c7 = 0b0000000111111111,
				c8 = 0b0000000011111111,
				c9 = 0b0000000001111111,
				c10 = 0b0000000000111111,
				c11 = 0b0000000000011111,
				c12 = 0b0000000000001111,
				c13 = 0b0000000000000111,
				c14 = 0b0000000000000011,
				c15 = 0b0000000000000001,
				c16 = 0b0000000000000000,
				EnumCount
			};

			union ATUI16
			{
				uint16_t Value;
				uint8_t Array[sizeof(uint16_t)];
			} ATUI;

		public:
			ShortWearLevelUnit() : BaseClass()
			{
				this->Initialize();
			}

#if defined(WEAR_LEVEL_DEBUG)
			const uint16_t DebugMask()
			{
				return GetMask();
			}
#endif

		private:
			const uint16_t GetMask()
			{
				ATUI.Array[0] = EmbeddedEEPROM::ReadBlock(Address + 0);
				ATUI.Array[1] = EmbeddedEEPROM::ReadBlock(Address + 1);

				return ATUI.Value;
			}

			const uint16_t GetMask(const uint8_t counter)
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
				case 9:
					return CounterEnum::c9;
				case 10:
					return CounterEnum::c10;
				case 11:
					return CounterEnum::c11;
				case 12:
					return CounterEnum::c12;
				case 13:
					return CounterEnum::c13;
				case 14:
					return CounterEnum::c14;
				case 15:
					return CounterEnum::c15;
				case 16:
					return CounterEnum::c16;
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
					return 2;
				case CounterEnum::c3:
					return 3;
				case CounterEnum::c4:
					return 4;
				case CounterEnum::c5:
					return 5;
				case CounterEnum::c6:
					return 6;
				case CounterEnum::c7:
					return 7;
				case CounterEnum::c8:
					return 8;
				case CounterEnum::c9:
					return 9;
				case CounterEnum::c10:
					return Uint8Min(10, (uint8_t)Option - 1);
				case CounterEnum::c11:
					return Uint8Min(11, (uint8_t)Option - 1);
				case CounterEnum::c12:
					return Uint8Min(12, (uint8_t)Option - 1);
				case CounterEnum::c13:
					return Uint8Min(13, (uint8_t)Option - 1);
				case CounterEnum::c14:
					return Uint8Min(14, (uint8_t)Option - 1);
				case CounterEnum::c15:
					return Uint8Min(15, (uint8_t)Option - 1);
				case CounterEnum::c16:
					return Uint8Min(16, (uint8_t)Option - 1);
				default:
					return 0;
				}
			}

			/// <summary>
			/// Increments the counter in a flash compatible way.
			/// From 0 to 18 (Option).
			/// </summary>
			/// <returns>Current Counter</returns>
			bool PrepareNextCounter(uint8_t& counter) final
			{
				counter = GetCurrentCounter();
				if (counter + 1 >= (uint8_t)Option)
				{
					counter = 0;
					this->AppendCounterOperation(Address + 0, UINT8_MAX, EmbeddedEEPROM::AsyncWriteModeEnum::EraseOnly);
					this->AppendCounterOperation(Address + 1, UINT8_MAX, EmbeddedEEPROM::AsyncWriteModeEnum::EraseOnly);
				}
				else
				{
					counter++;
					ATUI.Value = GetMask(counter);

					if (ATUI.Array[0] != EmbeddedEEPROM::ReadBlock(Address + 0))
					{
						this->AppendCounterOperation(Address + 0, ATUI.Array[0], EmbeddedEEPROM::AsyncWriteModeEnum::ClearBits);
					}
					if (ATUI.Array[1] != EmbeddedEEPROM::ReadBlock(Address + 1))
					{
						this->AppendCounterOperation(Address + 1, ATUI.Array[1], EmbeddedEEPROM::AsyncWriteModeEnum::ClearBits);
					}
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
				case CounterEnum::c9:
				case CounterEnum::c10:
				case CounterEnum::c11:
				case CounterEnum::c12:
				case CounterEnum::c13:
				case CounterEnum::c14:
				case CounterEnum::c15:
				case CounterEnum::c16:
					return GetCurrentCounter() <= (uint8_t)Option;
				default:
					return false;
				}
			}
		};
	}
}

/// <summary>
/// Synchronous CRC-checked EEPROM storage with a 16-bit wear-level counter.
/// Stores up to Option data records and is intended for a data structure or array.
/// </summary>
/// <typeparam name="Address">Starting EEPROM address.</typeparam>
/// <typeparam name="DataSize">Number of data bytes per record, excluding its CRC.</typeparam>
/// <typeparam name="Option">Number of rotating data records, from 10 to 18.</typeparam>
/// <typeparam name="Key">CRC key. Changing the key invalidates previously written data.</typeparam>
template<const uint16_t Address,
	const uint16_t DataSize,
	const WearLevelShort Option = WearLevelShort::x10,
	const uint32_t Key = EmbeddedStorage::GetStorageSize(DataSize, Option)>
class ShortWearLevelUnit
	: public EmbeddedStorage::Internal::ShortWearLevelUnit<Address, DataSize, Option, Key,
	BaseWearLevelUnit<Address, DataSize, Key, WearLevelShort, Option>>
{};

/// <summary>
/// Asynchronous CRC-checked EEPROM storage with a 16-bit wear-level counter.
/// The source buffer must remain valid and unchanged until the write completes.
/// </summary>
/// <typeparam name="Address">Starting EEPROM address.</typeparam>
/// <typeparam name="DataSize">Number of data bytes per record, excluding its CRC.</typeparam>
/// <typeparam name="Option">Number of rotating data records, from 10 to 18.</typeparam>
/// <typeparam name="Key">CRC key. Changing the key invalidates previously written data.</typeparam>
template<const uint16_t Address,
	const uint16_t DataSize,
	const WearLevelShort Option = WearLevelShort::x10,
	const uint32_t Key = EmbeddedStorage::GetStorageSize(DataSize, Option)>
class AsyncShortWearLevelUnit
	: public EmbeddedStorage::Internal::ShortWearLevelUnit<Address, DataSize, Option, Key,
	AsyncBaseWearLevelUnit<Address, DataSize, Key, WearLevelShort, Option>>
{};
#endif