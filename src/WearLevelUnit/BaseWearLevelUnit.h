#ifndef _BASE_WEAR_LEVEL_UNIT_
#define _BASE_WEAR_LEVEL_UNIT_

#include "EmbeddedStorageBase\EmbeddedEEPROM.h"
#include "EmbeddedStorageBase\EmbeddedCrc.h"


/// <summary>
/// Base Wear levelled, CRC checked EEPROM storage unit.
/// Flash overhead: 1 byte for counter, 1 byte for CRC times WearLevelOption.
/// Designed for use with a single data struct or array.
/// </summary>
/// <typeparam name="address">Address (offset) in EEPROM.</typeparam>
/// <param name="DataSize">Data size in bytes.</param>
/// <param name="WearLevelOption">WearLevel option, from 2 to 8.</param>
/// <param name="Key">Storage cryptographic salt key.
///  Changing the key invalidates any previous data.</param>
template<const uint16_t address,
	const uint16_t DataSize,
	const uint32_t Key,
	typename WearLevelType,
	const WearLevelType WearLevelOption>
class BaseWearLevelUnit
{
protected:
	EmbeddedCrc<Key> Crc{};

public:
	static constexpr uint16_t Address()
	{
		return address;
	}

	/// <summary>
	/// Gets the total EEPROM space reserved by this unit, including counters and CRCs.
	/// </summary>
	static constexpr uint16_t Size()
	{
		return EmbeddedStorage::GetStorageSize(DataSize, WearLevelOption);
	}

protected:
	virtual const uint8_t GetCurrentCounter() { return 0; }
	virtual const uint8_t IncrementCounter()
	{
		uint8_t counter = GetCurrentCounter();
		if (!PrepareNextCounter(counter)) { return 0; }
		return counter;
	}
	virtual bool PrepareNextCounter(uint8_t& counter)
	{
		counter = GetCurrentCounter();
		return false;
	}
	virtual const bool ValidateCounterMask() { return false; }
	virtual void AppendCounterOperation(const uint16_t offset, const uint8_t value,
		const EmbeddedEEPROM::AsyncWriteModeEnum mode)
	{
		ApplyCounterOperation(offset, value, mode);
	}

	void ApplyCounterOperation(const uint16_t offset, const uint8_t value,
		const EmbeddedEEPROM::AsyncWriteModeEnum mode)
	{
		if (mode == EmbeddedEEPROM::AsyncWriteModeEnum::Replace)
		{
			EmbeddedEEPROM::WriteBlock(offset, value);
		}
		else if (mode == EmbeddedEEPROM::AsyncWriteModeEnum::ClearBits)
		{
			EmbeddedEEPROM::ProgramZeroBitsToZero(offset, value);
		}
		else
		{
			EmbeddedEEPROM::ClearByteToOnes(offset);
		}
	}

	static constexpr uint16_t GetDataOffset(const uint8_t counter)
	{
		return static_cast<uint16_t>(GetCounterSize()
			+ (static_cast<uint16_t>(counter) * EmbeddedStorage::GetStorageSize(DataSize)));
	}

public:
	BaseWearLevelUnit()
	{}

#if defined(WEAR_LEVEL_DEBUG)
public:
#else
protected:
#endif
	void ResetCounter()
	{
		for (uint8_t i = 0; i < GetCounterSize(); i++)
		{
			EmbeddedEEPROM::ProgramZeroBitsToZero(address + i, 0);
		}
	}

public:
#if defined(WEAR_LEVEL_DEBUG)
	const uint8_t DebugCounter()
	{
		return GetCurrentCounter();
	}

	const uint8_t DebugOption()
	{
		return (uint8_t)WearLevelOption;
	}

	void DebugInitialize()
	{
		Initialize();
	}
#endif

	static constexpr size_t GetCounterSize()
	{
		return EmbeddedStorage::GetWearLevelCounterSize((uint8_t)WearLevelOption);
	}

	/// <summary>
	/// Reads the declared DataSize into target array.
	/// </summary>
	/// <param name="target">Target array.</param>
	/// <returns>True if CRC matches.</returns>
	const bool ReadData(uint8_t* target)
	{
		if (EmbeddedEEPROM::IsAsyncOperationActive()) { return false; }
		const uint8_t counter = GetCurrentCounter();
		const uint16_t offset = GetDataOffset(counter);

		for (uint16_t i = 0; i < DataSize; i++)
		{
			target[i] = EmbeddedEEPROM::ReadBlock(address + offset + i);
		}

		return Crc.GetCrc(target, DataSize, counter) == EmbeddedEEPROM::ReadBlock(address + offset + DataSize);
	}


	/// <summary>
	/// Writes the declared DataSize from source array.
	/// </summary>
	/// <param name="source">Source array.</param>
	void WriteData(const uint8_t* source)
	{
		if (EmbeddedEEPROM::IsAsyncOperationActive()) { return; }
		const uint8_t counter = IncrementCounter();
		const uint16_t offset = GetDataOffset(counter);

		for (uint16_t i = 0; i < DataSize; i++)
		{
			EmbeddedEEPROM::WriteBlock(address + offset + i, source[i]);
		}

		EmbeddedEEPROM::WriteBlock(address + offset + DataSize, Crc.GetCrc(source, DataSize, counter));
	}

	/// <summary>
	/// Writes one byte at an offset from the start of this unit.
	/// </summary>
	/// <param name="offset">Byte offset within this unit.</param>
	/// <param name="value">Value to write.</param>
	void WriteByte(const uint16_t offset, const uint8_t value)
	{
		EmbeddedEEPROM::WriteBlock(address + offset, value);
	}

	/// <summary>
	/// Reads one byte at an offset from the start of this unit.
	/// </summary>
	/// <param name="offset">Byte offset within this unit.</param>
	/// <returns>The byte stored at the specified offset.</returns>
	const uint8_t ReadByte(const uint16_t offset)
	{
		return EmbeddedEEPROM::ReadBlock(address + offset);
	}

protected:
	/// <summary>
	/// Ensure the current counter in this Unit is according to spec.
	/// </summary>
	void Initialize()
	{
		if (!ValidateCounterMask())
		{
			ResetCounter();
		}
	}

	static constexpr uint8_t Uint8Min(const uint8_t a, const uint8_t b)
	{
		return ((a <= b) * a) | ((b < a) * b);
	}
};

/// <summary>
/// Same features as BaseWearLevelUnit, but supports asynchronous EEPROM writes.
/// Starts an asynchronous EEPROM data write using the unit's wear-leveling strategy.
/// </summary>
/// <typeparam name="WearLevelType">Type that represents the wear-leveling mode/options.</typeparam>
/// <typeparam name="address">Base EEPROM address used by the wear-level unit.</typeparam>
/// <typeparam name="DataSize">Number of payload bytes written per record.</typeparam>
/// <typeparam name="Key">Compile-time key used by the base wear-level logic.</typeparam>
/// <typeparam name="WearLevelOption">Selected wear-leveling option value.</typeparam>
template<const uint16_t address,
	const uint16_t DataSize,
	const uint32_t Key,
	typename WearLevelType,
	const WearLevelType WearLevelOption>
class AsyncBaseWearLevelUnit : public BaseWearLevelUnit<address, DataSize, Key, WearLevelType, WearLevelOption>
{
private:
	struct CounterOperation
	{
		uint16_t Address;
		uint8_t Value;
		EmbeddedEEPROM::AsyncWriteModeEnum Mode;
	};
	static constexpr uint8_t CounterOperationCapacity = 8;
	CounterOperation AsyncCounterOperations[CounterOperationCapacity]{};
	uint8_t AsyncCounterOperationCount = 0;
	uint8_t AsyncCounterOperationIndex = 0;
	const uint8_t* AsyncSource = nullptr;
	uint32_t AsyncDataIndex = 0;
	uint16_t AsyncWriteOffset = 0;
	uint8_t AsyncCounter = 0;
	uint8_t AsyncCrc = 0;
	bool AsyncWaitingForByte = false;
	bool AsyncCounterOperationOverflow = false;
	EmbeddedEEPROM::AsyncOperationStateEnum AsyncState = EmbeddedEEPROM::AsyncOperationStateEnum::Idle;

protected:
	void AppendCounterOperation(const uint16_t offset, const uint8_t value,
		const EmbeddedEEPROM::AsyncWriteModeEnum mode) override
	{
		if (AsyncCounterOperationCount >= CounterOperationCapacity)
		{
			AsyncCounterOperationOverflow = true;
			return;
		}
		CounterOperation& operation = AsyncCounterOperations[AsyncCounterOperationCount++];
		operation.Address = offset;
		operation.Value = value;
		operation.Mode = mode;
	}

	const uint8_t IncrementCounter() override
	{
		AsyncCounterOperationCount = 0;
		AsyncCounterOperationOverflow = false;
		uint8_t counter = this->GetCurrentCounter();
		if (!this->PrepareNextCounter(counter) || AsyncCounterOperationOverflow) { return 0; }
		for (uint8_t i = 0; i < AsyncCounterOperationCount; i++)
		{
			const CounterOperation& operation = AsyncCounterOperations[i];
			this->ApplyCounterOperation(operation.Address, operation.Value, operation.Mode);
		}
		return counter;
	}

public:
	/// <summary>
	/// Starts an asynchronous write of the next wear-level record and its CRC.
	/// The source buffer must remain valid and unchanged until polling reports completion.
	/// </summary>
	/// <param name="source">Source data buffer containing DataSize bytes.</param>
	/// <returns>True if the write was started; otherwise, false.</returns>
	bool BeginWriteData(const uint8_t* source)
	{
		if (source == nullptr || AsyncState == EmbeddedEEPROM::AsyncOperationStateEnum::Pending
			|| !EmbeddedEEPROM::TryBeginAsyncOperation())
		{
			return false;
		}

		AsyncCounterOperationCount = 0;
		AsyncCounterOperationOverflow = false;
		if (!this->PrepareNextCounter(AsyncCounter) || AsyncCounterOperationOverflow)
		{
			EmbeddedEEPROM::EndAsyncOperation();
			AsyncState = EmbeddedEEPROM::AsyncOperationStateEnum::Failed;
			return false;
		}

		AsyncSource = source;
		AsyncCounterOperationIndex = 0;
		AsyncDataIndex = 0;
		AsyncWriteOffset = this->GetDataOffset(AsyncCounter);
		AsyncCrc = this->Crc.GetCrc(source, DataSize, AsyncCounter);
		AsyncWaitingForByte = false;
		AsyncState = EmbeddedEEPROM::AsyncOperationStateEnum::Pending;
		return true;
	}

	/// <summary>
	/// Advances the asynchronous write and returns its current state.
	/// Call repeatedly until the state is Complete or Failed.
	/// </summary>
	/// <returns>The current asynchronous write state.</returns>
	EmbeddedEEPROM::AsyncOperationStateEnum PollWriteData()
	{
		if (AsyncState != EmbeddedEEPROM::AsyncOperationStateEnum::Pending) { return AsyncState; }
		if (AsyncWaitingForByte)
		{
			if (EmbeddedEEPROM::IsAsyncByteBusy()) { return AsyncState; }
			AsyncWaitingForByte = false;
		}

		if (AsyncCounterOperationIndex < AsyncCounterOperationCount)
		{
			const CounterOperation& operation = AsyncCounterOperations[AsyncCounterOperationIndex];
			const EmbeddedEEPROM::AsyncByteStatusEnum status = EmbeddedEEPROM::TryStartAsyncByte(
				operation.Address, operation.Value, operation.Mode);
			if (status == EmbeddedEEPROM::AsyncByteStatusEnum::Busy) { return AsyncState; }

			AsyncCounterOperationIndex++;
			AsyncWaitingForByte = status == EmbeddedEEPROM::AsyncByteStatusEnum::Started;
			return AsyncState;
		}

		if (AsyncDataIndex <= DataSize)
		{
			const uint8_t value = AsyncDataIndex < DataSize ? AsyncSource[AsyncDataIndex] : AsyncCrc;
			const EmbeddedEEPROM::AsyncByteStatusEnum status = EmbeddedEEPROM::TryStartAsyncByte(
				static_cast<uint16_t>(address + AsyncWriteOffset + AsyncDataIndex), value,
				EmbeddedEEPROM::AsyncWriteModeEnum::Replace);
			if (status == EmbeddedEEPROM::AsyncByteStatusEnum::Busy) { return AsyncState; }

			AsyncDataIndex++;
			AsyncWaitingForByte = status == EmbeddedEEPROM::AsyncByteStatusEnum::Started;
			return AsyncState;
		}

		EmbeddedEEPROM::EndAsyncOperation();
		AsyncSource = nullptr;
		AsyncState = EmbeddedEEPROM::AsyncOperationStateEnum::Complete;
		return AsyncState;
	}
};

#endif