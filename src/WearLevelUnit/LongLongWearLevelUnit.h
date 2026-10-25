#ifndef _LONG_LONG_WEAR_LEVEL_UNIT_
#define _LONG_LONG_WEAR_LEVEL_UNIT_

#include "BaseWearLevelUnit.h"
#include <WearLevelType.h>

namespace EmbeddedStorage
{
	namespace Internal
	{
		template<const uint16_t Address,
			const uint16_t DataSize,
			const WearLevelLongLong Option,
			const uint32_t Key,
			typename WearLevelBase>
		class LongLongWearLevelUnit : public WearLevelBase
		{
		private:
			using BaseClass = WearLevelBase;
			using BaseClass::Uint8Min;

		private:
			union ATUI64
			{
				uint64_t Value;
				uint8_t Array[sizeof(uint64_t)];
			} ATUI;

			static const int8_t Counts = sizeof(uint64_t) * 8;

		public:
			LongLongWearLevelUnit() : BaseClass()
			{
				this->Initialize();
			}

#if defined(WEAR_LEVEL_DEBUG)
			const uint64_t DebugMask()
			{
				return GetMask();
			}
#endif

		private:
			const uint64_t GetMask()
			{
				ATUI.Array[0] = EmbeddedEEPROM::ReadBlock(Address + 0);
				ATUI.Array[1] = EmbeddedEEPROM::ReadBlock(Address + 1);
				ATUI.Array[2] = EmbeddedEEPROM::ReadBlock(Address + 2);
				ATUI.Array[3] = EmbeddedEEPROM::ReadBlock(Address + 3);
				ATUI.Array[4] = EmbeddedEEPROM::ReadBlock(Address + 4);
				ATUI.Array[5] = EmbeddedEEPROM::ReadBlock(Address + 5);
				ATUI.Array[6] = EmbeddedEEPROM::ReadBlock(Address + 6);
				ATUI.Array[7] = EmbeddedEEPROM::ReadBlock(Address + 7);

				return ATUI.Value;
			}

			const uint64_t GetMask(const uint8_t counter)
			{
				uint64_t mask = UINT64_MAX;
				for (int8_t i = Counts - 1; i >= (Counts - counter); i--)
				{
					// Remove i bit to mask.
					mask &= ~((uint64_t)1 << i);
				}
				return mask;
			}

		protected:
			const uint8_t GetCurrentCounter() final
			{
				const uint64_t mask = GetMask();

				if (mask == UINT64_MAX) {
					return 0;
				}
				else if (mask > 0)
				{
					// Find first Zero bit from left to right.
					for (uint8_t i = 1; i <= Counts; i++)
					{
						if (((uint8_t)(mask >> (Counts - i))) & 1)
						{
							return Uint8Min(i - 1, (uint8_t)Option - 1);
						}
					}
				}

				return (uint8_t)Option - 1;
			}

			/// <summary>
			/// Increments the counter in a flash compatible way.
			/// From 0 to 64 (Option).
			/// </summary>
			/// <returns>Current Counter</returns>
			bool PrepareNextCounter(uint8_t& counter) final
			{
				counter = GetCurrentCounter();
				if (counter + 1 >= (uint8_t)Option)
				{
					counter = 0;

					for (uint8_t i = 0; i < sizeof(uint64_t); i++)
					{
						this->AppendCounterOperation(Address + i, UINT8_MAX, EmbeddedEEPROM::AsyncWriteModeEnum::EraseOnly);
					}
				}
				else
				{
					counter++;
					ATUI.Value = GetMask(counter);

					for (uint8_t i = 0; i < sizeof(uint64_t); i++)
					{
						if (ATUI.Array[i] != EmbeddedEEPROM::ReadBlock(Address + i))
						{
							this->AppendCounterOperation(Address + i, ATUI.Array[i], EmbeddedEEPROM::AsyncWriteModeEnum::ClearBits);
						}
					}
				}

				return true;
			}

			const bool ValidateCounterMask() final
			{
				const uint64_t mask = GetMask();
				bool foundOne = false;
				for (uint8_t i = 1; i <= Counts; i++)
				{
					const bool bitIsOne = ((mask >> (Counts - i)) & 1) != 0;
					if (foundOne && !bitIsOne)
					{
						return false;
					}
					foundOne = foundOne || bitIsOne;
				}

				return true;
			}
		};
	}
}

/// <summary>
/// Synchronous CRC-checked EEPROM storage with a 64-bit wear-level counter.
/// Stores up to Option data records and is intended for a data structure or array.
/// </summary>
/// <typeparam name="Address">Starting EEPROM address.</typeparam>
/// <typeparam name="DataSize">Number of data bytes per record, excluding its CRC.</typeparam>
/// <typeparam name="Option">Number of rotating data records, from 34 to 66.</typeparam>
/// <typeparam name="Key">CRC key. Changing the key invalidates previously written data.</typeparam>
template<const uint16_t Address,
	const uint16_t DataSize,
	const WearLevelLongLong Option = WearLevelLongLong::x34,
	const uint32_t Key = EmbeddedStorage::GetStorageSize(DataSize, Option)>
class LongLongWearLevelUnit
	: public EmbeddedStorage::Internal::LongLongWearLevelUnit<Address, DataSize, Option, Key,
	BaseWearLevelUnit<Address, DataSize, Key, WearLevelLongLong, Option>>
{};

/// <summary>
/// Asynchronous CRC-checked EEPROM storage with a 64-bit wear-level counter.
/// The source buffer must remain valid and unchanged until the write completes.
/// </summary>
/// <typeparam name="Address">Starting EEPROM address.</typeparam>
/// <typeparam name="DataSize">Number of data bytes per record, excluding its CRC.</typeparam>
/// <typeparam name="Option">Number of rotating data records, from 34 to 66.</typeparam>
/// <typeparam name="Key">CRC key. Changing the key invalidates previously written data.</typeparam>
template<const uint16_t Address,
	const uint16_t DataSize,
	const WearLevelLongLong Option = WearLevelLongLong::x34,
	const uint32_t Key = EmbeddedStorage::GetStorageSize(DataSize, Option)>
class AsyncLongLongWearLevelUnit
	: public EmbeddedStorage::Internal::LongLongWearLevelUnit<Address, DataSize, Option, Key,
	AsyncBaseWearLevelUnit<Address, DataSize, Key, WearLevelLongLong, Option>>
{};
#endif