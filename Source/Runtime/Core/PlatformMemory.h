#pragma once

#include "Runtime/Platform/Platform.h"

#include <memory>

class FPlatformMemory
{
public:

	static FORCE_INLINE void* Memmove(void* dest, const void* src, size_t count)
	{
		return memmove(dest, src, count);
	}

	static FORCE_INLINE int32 Memcmp(const void* buf1, const void* buf2, size_t count)
	{
		return memcmp(buf1, buf2, count);
	}

	static FORCE_INLINE void* Memset(void* dest, uint8 Char, size_t count)
	{
		return memset(dest, Char, count);
	}

	static FORCE_INLINE void* Memzero(void* dest, size_t count)
	{
		return memset(dest, 0, count);
	}

	static FORCE_INLINE void* Memcpy(void* dest, const void* src, size_t count)
	{
		return memcpy(dest, src, count);
	}

	static FORCE_INLINE void* BigBlockMemcpy(void* dest, const void* src, size_t count)
	{
		return memcpy(dest, src, count);
	}

	static FORCE_INLINE void* StreamingMemcpy(void* dest, const void* src, size_t count)
	{
		return memcpy(dest, src, count);
	}

	template <typename T>
	static FORCE_INLINE void Valswap(T& a, T& b)
	{
		T tmp = a;
		a = b;
		b = tmp;
	}

	static FORCE_INLINE void Memswap(void* ptr1, void* ptr2, size_t size)
	{
		switch (size)
		{
			case 0:
				break;

			case 1:
				Valswap(*(uint8*)ptr1, *(uint8*)ptr2);
				break;

			case 2:
				Valswap(*(uint16*)ptr1, *(uint16*)ptr2);
				break;

			case 3:
				Valswap(*((uint16*&)ptr1)++, *((uint16*&)ptr2)++);
				Valswap(*(uint8*)ptr1, *(uint8*)ptr2);
				break;

			case 4:
				Valswap(*(uint32*)ptr1, *(uint32*)ptr2);
				break;

			case 5:
				Valswap(*((uint32*&)ptr1)++, *((uint32*&)ptr2)++);
				Valswap(*(uint8*)ptr1, *(uint8*)ptr2);
				break;

			case 6:
				Valswap(*((uint32*&)ptr1)++, *((uint32*&)ptr2)++);
				Valswap(*(uint16*)ptr1, *(uint16*)ptr2);
				break;

			case 7:
				Valswap(*((uint32*&)ptr1)++, *((uint32*&)ptr2)++);
				Valswap(*((uint16*&)ptr1)++, *((uint16*&)ptr2)++);
				Valswap(*(uint8*)ptr1, *(uint8*)ptr2);
				break;

			case 8:
				Valswap(*(uint64*)ptr1, *(uint64*)ptr2);
				break;

			case 16:
				Valswap(((uint64*)ptr1)[0], ((uint64*)ptr2)[0]);
				Valswap(((uint64*)ptr1)[1], ((uint64*)ptr2)[1]);
				break;

			default:
				Assert(false);
				break;
		}
	}

};