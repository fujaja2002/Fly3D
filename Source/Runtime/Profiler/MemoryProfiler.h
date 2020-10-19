#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Allocator/AllocatorType.h"
#include "Runtime/Allocator/BaseAllocator.h"
#include "Runtime/Template/Noncopyable.h"

#include <unordered_set>

class MemoryProfiler;
struct MemorySalt;

struct MemorySalt
{
#if ENABLE_MEM_PROFILER
	const char*		file;
	int32			line;
#endif
	int32			size;
	AllocatorType	type;
	BaseAllocator*  owner;

	void Fill(int32 inSize, AllocatorType inType, BaseAllocator* inOwner, const char* inFile, int32 inLine)
	{
#if ENABLE_MEM_PROFILER
		file = inFile;
		line = inLine;
#endif
		size  = inSize;
		type  = inType;
		owner = inOwner;
	}
};

#if ENABLE_MEM_PROFILER

class MemoryProfiler : public Noncopyable
{
public:

	MemoryProfiler();

	~MemoryProfiler();

	constexpr static int32 MemorySaltSize()
	{
		return sizeof(MemorySalt);
	}

	const MemorySalt* GetMemorySalt(const uint8* ptr);

	uint32 GetMemoryHeaderSize();

	uint32 GetAllocatedMemorySize();

	bool RegisterAllocation(const MemorySalt* salt);

	bool UnRegisterAllocation(const MemorySalt* salt);

private:

	std::unordered_set<const MemorySalt*> m_Salts;

};

MemoryProfiler* GetMemoryProfiler();

#endif