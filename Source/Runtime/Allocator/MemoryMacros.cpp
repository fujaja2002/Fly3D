#include "Runtime/Allocator/MemoryMacros.h"
#include "Runtime/Allocator/BaseAllocator.h"

namespace Fly3DPrivateMemory
{
	uint8* Allocate(uint32 size, int32 align, EAllocatorType type, const char* file, int32 line)
	{
		return GetAllocator()->Allocate(size, align, type, file, line);
	}

	uint8* Reallocate(uint8* p, uint32 size, int32 align, EAllocatorType type, const char* file, int32 line)
	{
		return GetAllocator()->Reallocate(p, size, align, type, file, line);
	}

	bool Deallocate(uint8* p, EAllocatorType type)
	{
		return GetAllocator()->Deallocate(p);
	}
}

void* operator new(size_t size, size_t align, EAllocatorType type, const char* file, int line)
{
	return (void*)Fly3DPrivateMemory::Allocate(size, align, type, file, line);
}

void* operator new[](size_t size, size_t align, EAllocatorType type, const char* file, int line)
{
	return (void*)Fly3DPrivateMemory::Allocate(size, align, type, file, line);
}

void operator delete(void* p, size_t align, EAllocatorType type, const char* file, int line)
{
	Fly3DPrivateMemory::Deallocate((uint8*)p, type);
}

void operator delete[](void* p, size_t align, EAllocatorType type, const char* file, int line)
{
	Fly3DPrivateMemory::Deallocate((uint8*)p, type);
}