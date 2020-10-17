#include "Runtime/Profiler/MemoryProfiler.h"

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
	if (m_Salts.find(salt) != m_Salts.end())
	{
		return false;
	}

	m_Salts.insert(salt);

	return true;
}

bool MemoryProfiler::UnRegisterAllocation(const MemorySalt* salt)
{
	auto it = m_Salts.find(salt);

	if (it == m_Salts.end())
	{
		return false;
	}

	m_Salts.erase(it);

	return true;
}

#endif