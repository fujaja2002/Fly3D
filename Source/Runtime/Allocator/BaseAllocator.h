#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/Noncopyable.h"
#include "Runtime/Allocator/AllocatorType.h"

#include <stddef.h>

class BaseAllocator : public Noncopyable
{
protected:

	enum { DEFAULT_ALIGN_SIZE = sizeof(size_t) };
	 
public:

	BaseAllocator(bool threadSafe = false);

	virtual ~BaseAllocator();

	virtual uint8* Allocate(uint32 size, int32 align, AllocatorType type, const char* file, int32 line) = 0;

	virtual uint8* Reallocate(uint8* p, uint32 size, AllocatorType type, int32 align, const char* file, int32 line) = 0;

	virtual bool Deallocate(uint8* p) = 0;

	virtual bool Contains(const uint8* p) const = 0;

	virtual bool TryDeallocate(uint8* p);

	virtual uint32 GetAllocatedMemorySize() const 
	{ 
		return m_TotalAllocatedBytes;
	}

	virtual uint32 GetReservedMemorySize() const 
	{ 
		return m_TotalReservedBytes;
	}

	virtual uint32 GetPeakAllocatedMemorySize() const 
	{ 
		return m_PeakAllocatedBytes;
	}

	virtual uint32 GetNumberOfAllocations() const 
	{ 
		return m_NumAllocations;
	}

	virtual bool IsThreadSafe() const
	{
		return m_IsThreadSafe;
	}

protected:

	uint32	m_NumAllocations;
	uint32	m_TotalAllocatedBytes;
	uint32	m_TotalReservedBytes;
	uint32	m_PeakAllocatedBytes;

private:

	bool	m_IsThreadSafe;
};

BaseAllocator* GetAllocator();
