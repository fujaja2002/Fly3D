#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Allocator/AllocatorType.h"

namespace Fly3DPrivateMemory
{
	uint8* Allocate(uint32 size, int32 align, EAllocatorType type, const char* file, int32 line);

	uint8* Reallocate(uint8* p, uint32 size, int32 align, EAllocatorType type, const char* file, int32 line);

	bool Deallocate(uint8* p, EAllocatorType type);
}

void* operator new(size_t size, size_t align, EAllocatorType type, const char* file, int line);
void* operator new[](size_t size, size_t align, EAllocatorType type, const char* file, int line);

void operator delete(void* p, size_t align, EAllocatorType type, const char* file, int line);
void operator delete[](void* p, size_t align, EAllocatorType type, const char* file, int line);

#define FLY3D_NEW(type, label)                   new (label, alignof(type), __FILE__, __LINE__) type
#define FLY3D_NEW_ALIGNED(type, label, align)    new (label, align, __FILE__, __LINE__) type
#define FLY3D_DELETE(ptr, label)                 { Fly3DPrivateMemory::Deallocate(ptr, label); ptr = NULL; }