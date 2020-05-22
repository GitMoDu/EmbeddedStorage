#if !defined(_IEMBEDDED_)
#define _IEMBEDDED_

#include <stdint.h>

class IEmbeddedData
{
public:
	virtual void Load() {}

	virtual const uint8_t GetSize() { return 0; }

	virtual const uint32_t GetKey() { return 0; }
};

class IEmbeddedStorage
{
public:
	virtual bool AddEmbeddedData(IEmbeddedData* data) { return false; }

	virtual bool SetData(const uint32_t key, const uint8_t* data)
	{
		return false;
	}

	virtual bool GetData(const uint32_t key, uint8_t* data)
	{
		return false;
	}
};
#endif