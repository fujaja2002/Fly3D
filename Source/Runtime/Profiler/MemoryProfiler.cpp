#include "Runtime/Profiler/MemoryProfiler.h"
#include "Runtime/Log/Assert.h"

#if ENABLE_MEM_PROFILER

static FMemoryProfiler g_MemoryProfiler;

FMemoryProfiler* GetMemoryProfiler()
{
	return &g_MemoryProfiler;
}

FMemoryProfiler::FMemoryProfiler()
	: m_Salts()
{

}

FMemoryProfiler::~FMemoryProfiler()
{

}

const FMemorySalt* FMemoryProfiler::GetMemorySalt(const uint8* ptr)
{
	if (ptr == nullptr)
	{
		return nullptr;
	}

	const FMemorySalt* salt = (const FMemorySalt*)(ptr - MemorySaltSize());

	if (m_Salts.find(salt) != m_Salts.end())
	{
		return salt;
	}
	
	return nullptr;
}

uint32 FMemoryProfiler::GetMemoryHeaderSize()
{
	return static_cast<uint32>(m_Salts.size()) * MemorySaltSize();
}

uint32 FMemoryProfiler::GetAllocatedMemorySize()
{
	uint32 size = 0;

	for (auto it = m_Salts.begin(); it != m_Salts.end(); ++it)
	{
		size += (*it)->size;
	}

	return size;
}

bool FMemoryProfiler::RegisterAllocation(const FMemorySalt* salt)
{
	Assert(salt);

	if (m_Salts.find(salt) != m_Salts.end())
	{
		return false;
	}

	m_Salts.insert(salt);

	return true;
}

bool FMemoryProfiler::UnRegisterAllocation(const FMemorySalt* salt)
{
	Assert(salt);

	auto it = m_Salts.find(salt);

	if (it == m_Salts.end())
	{
		return false;
	}

	m_Salts.erase(it);

	return true;
}

#endif