#pragma once

#include "Runtime/Allocator/BaseAllocator.h"
#include "Runtime/Profiler/MemoryProfiler.h"

class TLSFAllocator : public BaseAllocator
{
	enum 
	{
		TLSF_Pool_Size  = 512 * 1024 * 1024,
		TLSF_Pool_Count = 128
	};

public:

	TLSFAllocator();

	virtual ~TLSFAllocator();

	int32 GetPoolSize();

	int32 GetPoolCount();

	virtual uint8* Allocate(uint32 size, int32 align, AllocatorType type, const char* file, int32 line) override;

	virtual uint8* Reallocate(uint8* p, uint32 size, AllocatorType type, int32 align, const char* file, int32 line) override;

	virtual bool Deallocate(uint8* p) override;

	virtual bool Contains(const uint8* p) const override;

	const MemorySalt* GetMemorySalt(const uint8* p) const;

private:

	void* MallocBlock();

private:

	void* m_Tlsf;
	int32 m_PoolNum;
	void* m_Pools[TLSF_Pool_Count];
};