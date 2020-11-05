#include "Runtime/Platform/Platform.h"
#include "Runtime/TLSF/tlsf.h"
#include "Runtime/Allocator/TLSFAllocator.h"
#include "Runtime/Utilities/Align.h"
#include "Runtime/Profiler/MemoryProfiler.h"

#include <string>

static FTLSFAllocator g_TLSFAllocator;

FBaseAllocator* GetAllocator()
{
	return &g_TLSFAllocator;
}

FTLSFAllocator::FTLSFAllocator()
	: FBaseAllocator(true)
	, m_Tlsf(nullptr)
	, m_PoolNum(0)
{

}

FTLSFAllocator::~FTLSFAllocator()
{
	if (m_Tlsf != nullptr)
	{
		tlsf_destroy(m_Tlsf);
		m_Tlsf = nullptr;
	}

	for (int32 i = 0; i < m_PoolNum; ++i)
	{
		free(m_Pools[i]);
		m_Pools[i] = nullptr;
	}

	m_PoolNum = 0;
}

int32 FTLSFAllocator::GetPoolSize()
{
	return TLSF_Pool_Size * m_PoolNum;
}

int32 FTLSFAllocator::GetPoolCount()
{
	return m_PoolNum;
}

void* FTLSFAllocator::MallocBlock()
{
	Assert(m_PoolNum < static_cast<int32>(TLSF_Pool_Count));
	void* block = malloc(TLSF_Pool_Size);
	Assert(block);

	m_Pools[m_PoolNum++]  = block;
	m_TotalReservedBytes += TLSF_Pool_Size;

	return block;
}

const FMemorySalt* FTLSFAllocator::GetMemorySalt(const uint8* p) const
{
	FMemorySalt* salt = (FMemorySalt*)(p - sizeof(FMemorySalt));
	return salt->owner == this ? salt : nullptr;
}

uint8* FTLSFAllocator::Allocate(uint32 reqSize, int32 align, EAllocatorType type, const char* file, int32 line)
{
	Assert(reqSize < TLSF_Pool_Size - sizeof(FMemorySalt));
	
	if (m_Tlsf == nullptr)
	{
		m_Tlsf = tlsf_create_with_pool(MallocBlock(), TLSF_Pool_Size);
	}

	size_t realSize = AlignUp(reqSize + sizeof(FMemorySalt), align);
	void* mem = tlsf_malloc(m_Tlsf, realSize);

	if (!mem)
	{
		tlsf_add_pool(m_Tlsf, MallocBlock(), TLSF_Pool_Size);
		mem = tlsf_malloc(m_Tlsf, realSize);
	}

	FMemorySalt* salt = (FMemorySalt*)mem;
	salt->Fill(static_cast<uint32>(realSize), type, this, file, line);

	m_NumAllocations      += 1;
	m_TotalAllocatedBytes += static_cast<uint32>(realSize);
	m_PeakAllocatedBytes  += static_cast<uint32>(realSize);

#if ENABLE_MEM_PROFILER
	GetMemoryProfiler()->RegisterAllocation(salt);
#endif

	return (uint8*)mem + sizeof(FMemorySalt);
}

uint8* FTLSFAllocator::Reallocate(uint8* p, uint32 reqSize, int32 align, EAllocatorType type, const char* file, int32 line)
{
	const FMemorySalt* temp = GetMemorySalt(p);
	Assert(temp);

#if ENABLE_MEM_PROFILER
	GetMemoryProfiler()->UnRegisterAllocation(temp);
#endif

	m_TotalAllocatedBytes -= temp->size;
	m_PeakAllocatedBytes  -= temp->size;

	size_t realSize = AlignUp(reqSize + sizeof(FMemorySalt), align);
	void* mem = tlsf_realloc(m_Tlsf, (void*)temp, realSize);
	if (!mem)
	{
		tlsf_add_pool(m_Tlsf, MallocBlock(), TLSF_Pool_Size);
		mem = tlsf_realloc(m_Tlsf, (void*)temp, realSize);
	}

	FMemorySalt* salt = (FMemorySalt*)mem;
	salt->Fill(static_cast<uint32>(realSize), type, this, file, line);

	m_TotalAllocatedBytes += salt->size;
	m_PeakAllocatedBytes  += salt->size;

#if ENABLE_MEM_PROFILER
	GetMemoryProfiler()->RegisterAllocation(salt);
#endif

	return (uint8*)mem;
}

bool FTLSFAllocator::Deallocate(uint8* p)
{
	const FMemorySalt* salt = GetMemorySalt(p);
	Assert(salt);

	if (salt->owner != this)
	{
		return false;
	}

	m_NumAllocations      -= 1;
	m_TotalAllocatedBytes -= salt->size;

#if ENABLE_MEM_PROFILER
	GetMemoryProfiler()->UnRegisterAllocation(salt);
#endif

	tlsf_free(m_Tlsf, (void*)salt);
	return true;
}

bool FTLSFAllocator::Contains(const uint8* p) const
{
	const FMemorySalt* salt = GetMemorySalt(p);
	if (!salt)
	{
		return false;
	}

	return salt->owner == this;
}