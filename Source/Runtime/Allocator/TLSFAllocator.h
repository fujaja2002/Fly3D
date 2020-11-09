#pragma once

#include "Runtime/Allocator/BaseAllocator.h"
#include "Runtime/Profiler/MemoryProfiler.h"

class FTLSFAllocator : public FBaseAllocator
{
	enum 
	{
		TLSF_Pool_Size  = 512 * 1024 * 1024,
		TLSF_Pool_Count = 128
	};

public:

	FTLSFAllocator();

	virtual ~FTLSFAllocator();

	int32 GetPoolSize();

	int32 GetPoolCount();

	virtual void* Allocate(uint32 size, uint32 align, EAllocatorType type, const char* file, int32 line) override;

	virtual void* Reallocate(void* p, uint32 size, uint32 align, EAllocatorType type, const char* file, int32 line) override;

	virtual bool Deallocate(const void* p) override;

	virtual bool Contains(const void* p) const override;

	const FMemorySalt* GetMemorySalt(const void* p) const;

private:

	void* MallocBlock();

private:

	void* m_Tlsf;
	int32 m_PoolNum;
	void* m_Pools[TLSF_Pool_Count];
};