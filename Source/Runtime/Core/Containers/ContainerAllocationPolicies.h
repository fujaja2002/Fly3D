#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/TypeCompatibleBytes.h"
#include "Runtime/Template/Template.h"
#include "Runtime/Allocator/MemoryMacros.h"
#include "Runtime/Math/Math.h"

template<int IndexSize> 
class TSizedDefaultAllocator;

using FDefaultAllocator   = TSizedDefaultAllocator<32>;
using FDefaultAllocator64 = TSizedDefaultAllocator<64>;

template <typename SizeType>
FORCE_INLINE SizeType DefaultCalculateSlackShrink(SizeType numElements, SizeType numAllocatedElements, size_t bytesPerElement, uint32 alignment = DEFAULT_ALIGNMENT)
{
	const SizeType currentSlackElements = numAllocatedElements - numElements;
	const size_t currentSlackBytes      = currentSlackElements * bytesPerElement;
	const bool tooManySlackBytes        = currentSlackBytes >= 16384;
	const bool tooManySlackElements     = 3 * numElements < 2 * numAllocatedElements;

	if ((tooManySlackBytes || tooManySlackElements) && (currentSlackElements > 64 || !numElements))
	{
		return numElements;
	}
	else
	{
		return numAllocatedElements;
	}
}

template <typename SizeType>
FORCE_INLINE SizeType DefaultCalculateSlackGrow(SizeType numElements, SizeType numAllocatedElements, size_t bytesPerElement, uint32 alignment = DEFAULT_ALIGNMENT)
{
	const size_t firstGrow    = 4;
	const size_t constantGrow = 16;

	size_t grow = firstGrow;
	if (numAllocatedElements || size_t(numElements) > grow)
	{
		grow = size_t(numElements) + 3 * size_t(numElements) / 8 + constantGrow;
	}
	
	return (SizeType)grow;
}

template <typename SizeType>
FORCE_INLINE SizeType DefaultCalculateSlackReserve(SizeType numElements, size_t bytesPerElement, uint32 alignment = DEFAULT_ALIGNMENT)
{
	return numElements;
}

template <typename AllocatorType>
struct TAllocatorTraitsBase
{
	enum 
	{ 
		SupportsMove = false 
	};

	enum 
	{ 
		IsZeroConstruct = false 
	};
};

template <typename AllocatorType>
struct TAllocatorTraits : TAllocatorTraitsBase<AllocatorType>
{

};

template<uint32 Alignment = DEFAULT_ALIGNMENT>
class TAlignedHeapAllocator
{
public:
	using SizeType = int32;

	enum 
	{ 
		NeedsElementType = false 
	};

	enum 
	{ 
		RequireRangeCheck = true 
	};

	class ForAnyElementType
	{
	public:

		ForAnyElementType()
			: m_Data(nullptr)
		{

		}

		/**
		* Moves the state of another allocator into this one.
		*/
		FORCE_INLINE void MoveToEmpty(ForAnyElementType& other)
		{
			if (m_Data)
			{
				FLY3D_FREE(m_Data);
			}

			m_Data = other.m_Data;
			other.m_Data = nullptr;
		}

		FORCE_INLINE ~ForAnyElementType()
		{
			if (m_Data)
			{
				FLY3D_FREE(m_Data);
				m_Data = nullptr;
			}
		}

		FORCE_INLINE void* GetAllocation() const
		{
			return m_Data;
		}

		void ResizeAllocation(SizeType previousNumElements, SizeType numElements, size_t numBytesPerElement)
		{
			if (m_Data == nullptr)
			{
				m_Data = FLY3D_MALLOC_ALIGNED(numElements * numBytesPerElement, Alignment, kMemTypeAlignedHeapAllocator);
			}
			else
			{
				FLY3D_REALLOC_ALIGNED(m_Data, numElements * numBytesPerElement, Alignment, kMemTypeAlignedHeapAllocator);
			}
		}

		FORCE_INLINE SizeType CalculateSlackReserve(SizeType numElements, size_t numBytesPerElement) const
		{
			return DefaultCalculateSlackReserve(numElements, numBytesPerElement, Alignment);
		}

		FORCE_INLINE SizeType CalculateSlackShrink(SizeType numElements, SizeType numAllocatedElements, size_t numBytesPerElement) const
		{
			return DefaultCalculateSlackShrink(numElements, numAllocatedElements, numBytesPerElement, Alignment);
		}

		FORCE_INLINE SizeType CalculateSlackGrow(SizeType numElements, SizeType numAllocatedElements, size_t numBytesPerElement) const
		{
			return DefaultCalculateSlackGrow(numElements, numAllocatedElements, numBytesPerElement, Alignment);
		}

		size_t GetAllocatedSize(SizeType numAllocatedElements, size_t numBytesPerElement) const
		{
			return numAllocatedElements * numBytesPerElement;
		}

		bool HasAllocation() const
		{
			return m_Data != nullptr;
		}

	private:
		ForAnyElementType(const ForAnyElementType& other);

		ForAnyElementType& operator=(const ForAnyElementType& other);

		void* m_Data;
	};

	template<typename ElementType>
	class ForElementType : public ForAnyElementType
	{
	public:

		/** Default constructor. */
		ForElementType()
		{

		}

		FORCE_INLINE ElementType* GetAllocation() const
		{
			return (ElementType*)ForAnyElementType::GetAllocation();
		}
	};
};

template <uint32 Alignment>
struct TAllocatorTraits<TAlignedHeapAllocator<Alignment>> : TAllocatorTraitsBase<TAlignedHeapAllocator<Alignment>>
{
	enum 
	{ 
		SupportsMove = true 
	};

	enum 
	{ 
		IsZeroConstruct = true 
	};
};

template <int IndexSize>
struct TBitsToSizeType
{
	static_assert(IndexSize, "Unsupported allocator index size.");
};

template <> 
struct TBitsToSizeType<8>  
{ 
	using Type = int8; 
};

template <> 
struct TBitsToSizeType<16> 
{ 
	using Type = int16; 
};

template <> 
struct TBitsToSizeType<32> 
{ 
	using Type = int32; 
};

template <> 
struct TBitsToSizeType<64> 
{ 
	using Type = int64; 
};

template <int IndexSize>
class TSizedHeapAllocator
{
public:
	using SizeType = typename TBitsToSizeType<IndexSize>::Type;

	enum 
	{ 
		NeedsElementType = false 
	};

	enum 
	{ 
		RequireRangeCheck = true 
	};

	class ForAnyElementType
	{
	public:

		ForAnyElementType()
			: m_Data(nullptr)
		{

		}

		/**
		* Moves the state of another allocator into this one.
		*/
		FORCE_INLINE void MoveToEmpty(ForAnyElementType& other)
		{
			if (m_Data)
			{
				FLY3D_FREE(m_Data);
			}

			m_Data = other.m_Data;
			other.m_Data = nullptr;
		}

		FORCE_INLINE ~ForAnyElementType()
		{
			if (m_Data)
			{
				FLY3D_FREE(m_Data);
			}
		}

		FORCE_INLINE void* GetAllocation() const
		{
			return m_Data;
		}

		FORCE_INLINE void ResizeAllocation(SizeType previousNumElements, SizeType numElements, size_t numBytesPerElement)
		{
			if (m_Data == nullptr)
			{
				m_Data = FLY3D_MALLOC_ALIGNED(numElements * numBytesPerElement, DEFAULT_ALIGNMENT, kMemTypeSizedHeapAllocator);
			}
			else
			{
				FLY3D_REALLOC_ALIGNED(m_Data, numElements * numBytesPerElement, DEFAULT_ALIGNMENT, kMemTypeSizedHeapAllocator);
			}
		}

		FORCE_INLINE SizeType CalculateSlackReserve(SizeType numElements, size_t numBytesPerElement) const
		{
			return DefaultCalculateSlackReserve(numElements, numBytesPerElement);
		}

		FORCE_INLINE SizeType CalculateSlackShrink(SizeType numElements, SizeType NumAllocatedElements, SizeType numBytesPerElement) const
		{
			return DefaultCalculateSlackShrink(numElements, NumAllocatedElements, numBytesPerElement);
		}

		FORCE_INLINE SizeType CalculateSlackGrow(SizeType numElements, SizeType NumAllocatedElements, SizeType numBytesPerElement) const
		{
			return DefaultCalculateSlackGrow(numElements, NumAllocatedElements, numBytesPerElement);
		}

		size_t GetAllocatedSize(SizeType numAllocatedElements, size_t numBytesPerElement) const
		{
			return numAllocatedElements * numBytesPerElement;
		}

		bool HasAllocation() const
		{
			return m_Data != nullptr;
		}

	private:
		ForAnyElementType(const ForAnyElementType& other);

		ForAnyElementType& operator=(const ForAnyElementType& other);

		void* m_Data;
	};

	template<typename ElementType>
	class ForElementType : public ForAnyElementType
	{
	public:

		ForElementType()
		{

		}

		FORCE_INLINE ElementType* GetAllocation() const
		{
			return (ElementType*)ForAnyElementType::GetAllocation();
		}
	};
};

template <uint8 IndexSize>
struct TAllocatorTraits<TSizedHeapAllocator<IndexSize>> : TAllocatorTraitsBase<TSizedHeapAllocator<IndexSize>>
{
	enum 
	{ 
		SupportsMove = true 
	};

	enum 
	{ 
		IsZeroConstruct = true 
	};
};

template <int IndexSize> 
class TSizedDefaultAllocator : public TSizedHeapAllocator<IndexSize> 
{ 
public: 
	typedef TSizedHeapAllocator<IndexSize> Typedef; 
};

template <int IndexSize> 
struct TAllocatorTraits<TSizedDefaultAllocator<IndexSize>> : TAllocatorTraits<typename TSizedDefaultAllocator<IndexSize>::Typedef> 
{

};

template <> 
struct TAllocatorTraits<FDefaultAllocator> : TAllocatorTraits<typename FDefaultAllocator::Typedef> 
{

};