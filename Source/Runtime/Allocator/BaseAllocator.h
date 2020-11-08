#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/Noncopyable.h"
#include "Runtime/Allocator/AllocatorType.h"

#include <stddef.h>

class FBaseAllocator : public Noncopyable
{
public:

	enum 
	{ 
		DEFAULT_ALIGN_SIZE = sizeof(size_t) 
	};
	 
public:

	FBaseAllocator(bool threadSafe = false);

	virtual ~FBaseAllocator();

	virtual void* Allocate(uint32 size, uint32 align, EAllocatorType type, const char* file, int32 line) = 0;

	virtual void* Reallocate(void* p, uint32 size, uint32 align, EAllocatorType type, const char* file, int32 line) = 0;

	virtual bool Deallocate(const void* p) = 0;

	virtual bool Contains(const void* p) const = 0;

	virtual bool TryDeallocate(const void* p);

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

FBaseAllocator* GetAllocator();
