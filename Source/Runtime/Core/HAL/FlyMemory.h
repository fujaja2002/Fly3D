#include "Runtime/Platform/Platform.h"
#include "Runtime/Log/Assert.h"

#include <memory>

struct FMemory
{
	static FORCE_INLINE void* Memmove(void* dst, const void* src, size_t count)
	{
		return memmove(dst, src, count);
	}

	static FORCE_INLINE int32 Memcmp(const void* buf1, const void* buf2, size_t count)
	{
		return memcmp(buf1, buf2, count);
	}

	static FORCE_INLINE void* Memset(void* dst, uint8 Char, size_t count)
	{
		return memset(dst, Char, count);
	}

	static FORCE_INLINE void* Memzero(void* dst, size_t count)
	{
		return memset(dst, 0, count);
	}

	static FORCE_INLINE void* Memcpy(void* dst, const void* src, size_t count)
	{
		return memcpy(dst, src, count);
	}

	static FORCE_INLINE void* BigBlockMemcpy(void* dst, const void* src, size_t count)
	{
		return memcpy(dst, src, count);
	}

	static FORCE_INLINE void* StreamingMemcpy(void* dst, const void* src, size_t count)
	{
		return memcpy( dst, src, count );
	}

};