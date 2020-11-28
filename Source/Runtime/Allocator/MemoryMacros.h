#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Allocator/AllocatorType.h"
#include "Runtime/Allocator/BaseAllocator.h"

enum
{
	DEFAULT_ALIGNMENT = 8,
	MIN_ALIGNMENT = 8,
};

namespace Fly3DPrivateMemory
{
	void* Allocate(uint32 size, uint32 align, EAllocatorType type, const char* file, int32 line);

	void* Reallocate(void* p, uint32 size, uint32 align, EAllocatorType type, const char* file, int32 line);

	bool Deallocate(const void* p);

	template<typename T>
	FORCE_INLINE void Delete(T* ptr)
	{
		if (ptr)
		{
			ptr->~T();
			Deallocate(ptr);
		}
	}
}

void* operator new(size_t size, size_t align, EAllocatorType type, const char* file, int32 line);
void* operator new[](size_t size, size_t align, EAllocatorType type, const char* file, int32 line);

void operator delete(void* p, size_t align, EAllocatorType type, const char* file, int32 line);
void operator delete[](void* p, size_t align, EAllocatorType type, const char* file, int32 line);

#define FLY3D_NEW(type, label)							new (alignof(type), label, __FILE__, __LINE__) type
#define FLY3D_NEW_ALIGNED(type, label, align)			new (align,         label, __FILE__, __LINE__) type
#define FLY3D_DELETE(ptr)								do { Fly3DPrivateMemory::Delete(ptr); ptr = nullptr; } while(0)

#define FLY3D_MALLOC(size, label)						Fly3DPrivateMemory::Allocate((uint32)(size), FBaseAllocator::DEFAULT_ALIGN_SIZE, label, __FILE__, __LINE__)
#define FLY3D_MALLOC_ALIGNED(size, align, label)		Fly3DPrivateMemory::Allocate((uint32)(size), (uint32)(align), label, __FILE__, __LINE__)
#define FLY3D_REALLOC(ptr, size, label)					Fly3DPrivateMemory::Reallocate(ptr, (uint32)(size), FBaseAllocator::DEFAULT_ALIGN_SIZE, label, __FILE__, __LINE__)
#define FLY3D_REALLOC_ALIGNED(ptr, size, align, label)	Fly3DPrivateMemory::Reallocate(ptr, (uint32)(size), (uint32)(align), label, __FILE__, __LINE__)
#define FLY3D_FREE(ptr)									Fly3DPrivateMemory::Deallocate(ptr)