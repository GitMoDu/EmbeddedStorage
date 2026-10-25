#ifndef _ASYNC_STORAGE_UNIT_
#define _ASYNC_STORAGE_UNIT_

#include <StorageUnit.h>

/// <summary>
/// Asynchronous CRC checked EEPROM storage unit.
/// The source buffer must remain valid and unchanged until the write completes.
/// </summary>
/// <param name="DataSize">Data size in bytes.</param>
/// <param name="Key">Storage cryptographic salt key.</param>
template<const uint16_t address,
	const uint16_t DataSize,
	const uint32_t Key = DataSize>
class AsyncStorageUnit : public StorageUnit<address, DataSize, Key>
{
private:
	const uint8_t* AsyncSource = nullptr;
	uint32_t AsyncIndex = 0;
	uint8_t AsyncCrc = 0;
	bool AsyncWaitingForByte = false;
	EmbeddedEEPROM::AsyncOperationStateEnum AsyncState = EmbeddedEEPROM::AsyncOperationStateEnum::Idle;

public:
	/// <summary>
	/// Starts an asynchronous write of the data and its CRC.
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

		AsyncSource = source;
		AsyncIndex = 0;
		AsyncCrc = this->GetCrc(source, DataSize);
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

		if (AsyncIndex > DataSize)
		{
			EmbeddedEEPROM::EndAsyncOperation();
			AsyncSource = nullptr;
			AsyncState = EmbeddedEEPROM::AsyncOperationStateEnum::Complete;
			return AsyncState;
		}

		const uint8_t value = AsyncIndex < DataSize ? AsyncSource[AsyncIndex] : AsyncCrc;
		const EmbeddedEEPROM::AsyncByteStatusEnum byteStatus = EmbeddedEEPROM::TryStartAsyncByte(
			static_cast<uint16_t>(address + AsyncIndex), value, EmbeddedEEPROM::AsyncWriteModeEnum::Replace);
		if (byteStatus == EmbeddedEEPROM::AsyncByteStatusEnum::Busy) { return AsyncState; }

		AsyncIndex++;
		AsyncWaitingForByte = byteStatus == EmbeddedEEPROM::AsyncByteStatusEnum::Started;
		return AsyncState;
	}
};
#endif