#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Allocator/AllocatorType.h"
#include "Runtime/Allocator/BaseAllocator.h"
#include "Runtime/Template/Noncopyable.h"

#include <unordered_set>

class FMemoryProfiler;
struct FMemorySalt;

struct FMemorySalt
{
#if ENABLE_MEM_PROFILER
	const char*			file;
	int32				line;
#endif
	uint32				size;
	EAllocatorType		type;
	FBaseAllocator*		owner;

	void Fill(uint32 inSize, EAllocatorType inType, FBaseAllocator* inOwner, const char* inFile, int32 inLine)
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

class FMemoryProfiler : public Noncopyable
{
public:

	FMemoryProfiler();

	~FMemoryProfiler();

	constexpr static int32 MemorySaltSize()
	{
		return sizeof(FMemorySalt);
	}

	const FMemorySalt* GetMemorySalt(const uint8* ptr);

	uint32 GetMemoryHeaderSize();

	uint32 GetAllocatedMemorySize();

	bool RegisterAllocation(const FMemorySalt* salt);

	bool UnRegisterAllocation(const FMemorySalt* salt);

private:

	std::unordered_set<const FMemorySalt*> m_Salts;

};

FMemoryProfiler* GetMemoryProfiler();

#endif