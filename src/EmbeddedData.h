/*

Depends on https://github.com/PRosenb/EEPROMWearLevel.git

*/

#if !defined(_EMBEDDED_DATA_)
#define _EMBEDDED_DATA_

#include <IEmbedded.h>

template<class T>
class TemplateEmbeddedData : public virtual IEmbeddedData
{
private:
	IEmbeddedStorage* Storage = nullptr;

public:
	T Data;

public:
	TemplateEmbeddedData() : IEmbeddedData()
		, Data()
	{
	}

	T* GetData()
	{
		return &Data;
	}

	virtual const uint8_t GetSize()
	{
		return sizeof(T);
	}

	virtual const uint32_t GetKey()
	{
		return T::Key;
	}

	virtual void Load()
	{
		// Fill in to memory from the stored value.
		if (!Storage->GetData(T::Key, (uint8_t*)&Data))
		{
			// No value in Storage, get default from struct declaration.
			T::SetDefaultValue(Data);
			OnDataUpdated();
		}
	}

	// Setup to load the value from storage to memory.
	bool Setup(IEmbeddedStorage* storage)
	{
		Storage = storage;
		if (Storage != nullptr)
		{
			Storage->AddEmbeddedData(this);

			return true;
		}
		else
		{
			return false;
		}
	}

	virtual void OnDataUpdated()
	{
		Commit();
	}

	virtual void Commit()
	{
		Storage->SetData(T::Key, (uint8_t*)&Data);
	}
};

// Extension to enable asynchronous commit of data to EEPROM.
template<class T>
class TemplateAsyncEmbeddedData : public TemplateEmbeddedData<T>
{
private:
	volatile bool WritePending = false;

	IEmbeddedDataAsyncCommit* AsyncCommiter = nullptr;

public:
	TemplateAsyncEmbeddedData(IEmbeddedDataAsyncCommit* asyncCommiter)
		: TemplateEmbeddedData<T>()
		, AsyncCommiter(asyncCommiter)
	{
	}

	virtual bool NeedsCommit()
	{
		return WritePending;
	}

	virtual void OnDataUpdated()
	{
		WritePending = true;
		AsyncCommiter->AsyncUpdate();
	}

	virtual void Commit()
	{
		WritePending = false;
		TemplateEmbeddedData<T>::Commit();
	}
};
#endif