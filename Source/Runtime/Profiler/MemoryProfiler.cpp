#include "Runtime/Profiler/MemoryProfiler.h"
#include "Runtime/Log/Assert.h"

#if ENABLE_MEM_PROFILER

static MemoryProfiler g_MemoryProfiler;

MemoryProfiler* GetMemoryProfiler()
{
	return &g_MemoryProfiler;
}

MemoryProfiler::MemoryProfiler()
	: m_Salts()
{

}

MemoryProfiler::~MemoryProfiler()
{

}

const MemorySalt* MemoryProfiler::GetMemorySalt(const uint8* ptr)
{
	if (ptr == nullptr)
	{
		return nullptr;
	}

	const MemorySalt* salt = (const MemorySalt*)(ptr - MemorySaltSize());

	if (m_Salts.find(salt) != m_Salts.end())
	{
		return salt;
	}
	
	return nullptr;
}

uint32 MemoryProfiler::GetMemoryHeaderSize()
{
	return m_Salts.size() * MemorySaltSize();
}

uint32 MemoryProfiler::GetAllocatedMemorySize()
{
	uint32 size = 0;

	for (auto it = m_Salts.begin(); it != m_Salts.end(); ++it)
	{
		size += (*it)->size;
	}

	return size;
}

bool MemoryProfiler::RegisterAllocation(const MemorySalt* salt)
{
	Assert(salt);

	if (m_Salts.find(salt) != m_Salts.end())
	{
		return false;
	}

	m_Salts.insert(salt);

	return true;
}

bool MemoryProfiler::UnRegisterAllocation(const MemorySalt* salt)
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