#include "Runtime/Log/Log.h"
#include "Runtime/Allocator/MemoryMacros.h"
#include "Runtime/Allocator/BaseAllocator.h"

namespace Fly3DPrivateMemory
{
	void* Allocate(uint32 size, uint32 align, EAllocatorType type, const char* file, int32 line)
	{
		return GetAllocator()->Allocate(size, align, type, file, line);
	}

	void* Reallocate(void* p, uint32 size, uint32 align, EAllocatorType type, const char* file, int32 line)
	{
		return GetAllocator()->Reallocate(p, size, align, type, file, line);
	}

	bool Deallocate(const void* p)
	{
		return GetAllocator()->Deallocate(p);
	}
}

void* operator new(size_t size, size_t align, EAllocatorType type, const char* file, int32 line)
{
	return (void*)Fly3DPrivateMemory::Allocate((uint32)size, (uint32)align, type, file, line);
}

void* operator new[](size_t size, size_t align, EAllocatorType type, const char* file, int32 line)
{
	return (void*)Fly3DPrivateMemory::Allocate((uint32)size, (uint32)align, type, file, line);
}

void operator delete(void* p, size_t align, EAllocatorType type, const char* file, int32 line)
{
	Fly3DPrivateMemory::Deallocate((uint8*)p);
}

void operator delete[](void* p, size_t align, EAllocatorType type, const char* file, int32 line)
{
	Fly3DPrivateMemory::Deallocate((uint8*)p);
}