#include "Runtime/Platform/Platform.h"
#include "Runtime/TLSF/tlsf.h"
#include "Runtime/Allocator/TLSFAllocator.h"
#include "Runtime/Utilities/Align.h"
#include "Runtime/Profiler/MemoryProfiler.h"

#include <string>

static TLSFAllocator g_TLSFAllocator;

BaseAllocator* GetAllocator()
{
	return &g_TLSFAllocator;
}

TLSFAllocator::TLSFAllocator()
	: BaseAllocator(true)
	, m_Tlsf(nullptr)
	, m_PoolNum(0)
{

}

TLSFAllocator::~TLSFAllocator()
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

int32 TLSFAllocator::GetPoolSize()
{
	return TLSF_Pool_Size * m_PoolNum;
}

int32 TLSFAllocator::GetPoolCount()
{
	return m_PoolNum;
}

void* TLSFAllocator::MallocBlock()
{
	void* block = malloc(TLSF_Pool_Size);
	Assert(block);

	m_Pools[m_PoolNum] = block;
	m_PoolNum += 1;

	m_TotalReservedBytes += TLSF_Pool_Size;

	return block;
}

const MemorySalt* TLSFAllocator::GetMemorySalt(const uint8* p) const
{
	MemorySalt* salt = (MemorySalt*)(p - sizeof(MemorySalt));
	return salt->owner == this ? salt : nullptr;
}

uint8* TLSFAllocator::Allocate(uint32 reqSize, int32 align, AllocatorType type, const char* file, int32 line)
{
	Assert(reqSize < TLSF_Pool_Size - sizeof(MemorySalt));
	
	if (m_Tlsf == nullptr)
	{
		m_Tlsf = tlsf_create_with_pool(MallocBlock(), TLSF_Pool_Size);
	}

	uint32 realSize = AlignUp(reqSize + sizeof(MemorySalt), align);
	void* mem = tlsf_malloc(m_Tlsf, realSize);

	if (!mem)
	{
		tlsf_add_pool(m_Tlsf, MallocBlock(), TLSF_Pool_Size);
		mem = tlsf_malloc(m_Tlsf, realSize);
	}

	MemorySalt* salt = (MemorySalt*)mem;
	salt->Fill(realSize, type, this, file, line);

	m_NumAllocations      += 1;
	m_TotalAllocatedBytes += realSize;
	m_PeakAllocatedBytes  += realSize;

#if ENABLE_MEM_PROFILER
	GetMemoryProfiler()->RegisterAllocation(salt);
#endif

	return (uint8*)mem + sizeof(MemorySalt);
}

uint8* TLSFAllocator::Reallocate(uint8* p, uint32 reqSize, AllocatorType type, int32 align, const char* file, int32 line)
{
	const MemorySalt* temp = GetMemorySalt(p);
	Assert(temp);

#if ENABLE_MEM_PROFILER
	GetMemoryProfiler()->UnRegisterAllocation(temp);
#endif

	m_TotalAllocatedBytes -= temp->size;
	m_PeakAllocatedBytes  -= temp->size;

	uint32 realSize = AlignUp(reqSize + sizeof(MemorySalt), align);
	void* mem = tlsf_realloc(m_Tlsf, (void*)temp, realSize);

	MemorySalt* salt = (MemorySalt*)mem;
	salt->Fill(realSize, type, this, file, line);

	m_TotalAllocatedBytes += salt->size;
	m_PeakAllocatedBytes  += salt->size;

#if ENABLE_MEM_PROFILER
	GetMemoryProfiler()->RegisterAllocation(salt);
#endif

	return (uint8*)mem;
}

bool TLSFAllocator::Deallocate(uint8* p)
{
	const MemorySalt* salt = GetMemorySalt(p);
	if (salt->owner != this)
	{
		return false;
	}

#if ENABLE_MEM_PROFILER
	GetMemoryProfiler()->UnRegisterAllocation(salt);
#endif

	tlsf_free(m_Tlsf, (void*)salt);
	return true;
}

bool TLSFAllocator::Contains(const uint8* p) const
{
	const MemorySalt* salt = GetMemorySalt(p);
	return salt->owner == this;
}