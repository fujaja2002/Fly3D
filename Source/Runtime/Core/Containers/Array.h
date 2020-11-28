#pragma once

#include "Runtime/Log/Assert.h"

#include "Runtime/Core/Containers/ContainerAllocationPolicies.h"
#include "Runtime/Template/ChooseClass.h"
#include "Runtime/Template/AreTypesEqual.h"
#include "Runtime/Template/TypeTraits.h"
#include "Runtime/Template/MemoryOps.h"
#include "Runtime/Core/HAL/FlyMemory.h"

#include <initializer_list>

enum 
{
	INDEX_NONE = -1
};

template<typename T, typename Allocator = FDefaultAllocator> 
class TArray;

template<typename T> 
using TArray64 = TArray<T, FDefaultAllocator64>;

template< typename ContainerType, typename ElementType, typename SizeType>
class TIndexedContainerIterator
{
public:
	TIndexedContainerIterator(ContainerType& inContainer, SizeType startIndex = 0)
		: m_Container(inContainer)
		, m_Index(startIndex)
	{

	}

	TIndexedContainerIterator& operator++()
	{
		++m_Index;
		return *this;
	}

	TIndexedContainerIterator operator++(int)
	{
		TIndexedContainerIterator tmp(*this);
		++m_Index;
		return tmp;
	}

	TIndexedContainerIterator& operator--()
	{
		--m_Index;
		return *this;
	}

	TIndexedContainerIterator operator--(int)
	{
		TIndexedContainerIterator tmp(*this);
		--m_Index;
		return tmp;
	}

	TIndexedContainerIterator& operator+=(SizeType offset)
	{
		m_Index += offset;
		return *this;
	}

	TIndexedContainerIterator operator+(SizeType offset) const
	{
		TIndexedContainerIterator tmp(*this);
		return tmp += offset;
	}

	TIndexedContainerIterator& operator-=(SizeType offset)
	{
		return *this += -offset;
	}

	TIndexedContainerIterator operator-(SizeType offset) const
	{
		TIndexedContainerIterator tmp(*this);
		return tmp -= offset;
	}

	ElementType& operator*() const
	{
		return m_Container[m_Index];
	}

	ElementType* operator->() const
	{
		return &m_Container[m_Index];
	}

	explicit operator bool() const
	{
		return m_Container.IsValidIndex(m_Index);
	}

	SizeType GetIndex() const
	{
		return m_Index;
	}

	void Reset()
	{
		m_Index = 0;
	}

	void SetToEnd()
	{
		m_Index = m_Container.Num();
	}

	void RemoveCurrent()
	{
		m_Container.RemoveAt(m_Index);
		m_Index--;
	}

	friend bool operator==(const TIndexedContainerIterator& lhs, const TIndexedContainerIterator& rhs) 
	{ 
		return &lhs.m_Container == &rhs.m_Container && lhs.m_Index == rhs.m_Index; 
	}

	friend bool operator!=(const TIndexedContainerIterator& lhs, const TIndexedContainerIterator& rhs) 
	{ 
		return &lhs.m_Container != &rhs.m_Container || lhs.m_Index != rhs.m_Index; 
	}

private:

	ContainerType& m_Container;
	SizeType      m_Index;
};


template <typename ContainerType, typename ElementType, typename SizeType>
FORCE_INLINE TIndexedContainerIterator<ContainerType, ElementType, SizeType> operator+(SizeType offset, TIndexedContainerIterator<ContainerType, ElementType, SizeType> rhs)
{
	return rhs + offset;
}

template <typename ElementType, typename SizeType>
struct TCheckedPointerIterator
{
	explicit TCheckedPointerIterator(const SizeType& inNum, ElementType* inPtr)
		: m_Ptr(inPtr)
		, m_CurrentNum(inNum)
		, m_InitialNum(inNum)
	{

	}

	FORCE_INLINE ElementType& operator*() const
	{
		return *m_Ptr;
	}

	FORCE_INLINE TCheckedPointerIterator& operator++()
	{
		++m_Ptr;
		return *this;
	}

	FORCE_INLINE TCheckedPointerIterator& operator--()
	{
		--m_Ptr;
		return *this;
	}

private:

	FORCE_INLINE friend bool operator!=(const TCheckedPointerIterator& lhs, const TCheckedPointerIterator& rhs)
	{
		return lhs.m_Ptr != rhs.m_Ptr;
	}

private:

	ElementType*    m_Ptr;
	const SizeType& m_CurrentNum;
	SizeType        m_InitialNum;

};

namespace Fly3DPrivateArray
{
	template <typename FromArrayType, typename ToArrayType>
	struct TCanMoveTArrayPointersBetweenArrayTypes
	{
		typedef typename FromArrayType::Allocator   FromAllocatorType;
		typedef typename ToArrayType  ::Allocator   ToAllocatorType;
		typedef typename FromArrayType::ElementType FromElementType;
		typedef typename ToArrayType  ::ElementType ToElementType;

		enum
		{
			Value = 
				TAreTypesEqual<FromAllocatorType, ToAllocatorType>::Value && 
				TContainerTraits<FromArrayType>::MoveWillEmptyContainer &&
				(TAreTypesEqual<ToElementType, FromElementType>::Value || TIsBitwiseConstructible<ToElementType, FromElementType>::Value)
		};
	};
}

template<typename InElementType, typename InAllocator>
class TArray
{
	template <typename OtherInElementType, typename OtherAllocator>
	friend class TArray;

public:

	typedef typename InAllocator::SizeType SizeType;
	typedef InElementType ElementType;
	typedef InAllocator   Allocator;

public:

	TArray()
		: m_ArrayNum(0)
		, m_ArrayMax(0)
	{

	}

	TArray(const ElementType* ptr, SizeType count)
	{
		Assert(ptr != nullptr && count != 0)
		CopyToEmpty(ptr, count, 0, 0);
	}

	TArray(std::initializer_list<InElementType> initList)
	{
		CopyToEmpty(initList.begin(), (SizeType)initList.size(), 0, 0);
	}

	template <typename OtherElementType, typename OtherAllocator>
	explicit TArray(const TArray<OtherElementType, OtherAllocator>& other)
	{
		CopyToEmpty(other.GetData(), other.Num(), 0, 0);
	}

	TArray(const TArray& other)
	{
		CopyToEmpty(other.GetData(), other.Num(), 0, 0);
	}

	TArray(const TArray& other, SizeType extraSlack)
	{
		CopyToEmpty(other.GetData(), other.Num(), 0, extraSlack);
	}

	TArray& operator=(std::initializer_list<InElementType> initList)
	{
		DestructItems(GetData(), m_ArrayNum);
		CopyToEmpty(initList.begin(), (SizeType)initList.size(), m_ArrayMax, 0);
		return *this;
	}

	template<typename OtherAllocator>
	TArray& operator=(const TArray<ElementType, OtherAllocator>& other)
	{
		DestructItems(GetData(), m_ArrayNum);
		CopyToEmpty(other.GetData(), other.Num(), m_ArrayMax, 0);
		return *this;
	}

	TArray& operator=(const TArray& other)
	{
		if (this != &other)
		{
			DestructItems(GetData(), m_ArrayNum);
			CopyToEmpty(other.GetData(), other.Num(), m_ArrayMax, 0);
		}
		return *this;
	}

	TArray(TArray&& other)
	{
		MoveOrCopy(*this, other, 0);
	}

	template <typename OtherElementType, typename OtherAllocator>
	explicit TArray(TArray<OtherElementType, OtherAllocator>&& other)
	{
		MoveOrCopy(*this, other, 0);
	}

	template <typename OtherElementType>
	TArray(TArray<OtherElementType, Allocator>&& other, SizeType extraSlack)
	{
		MoveOrCopyWithSlack(*this, other, 0, extraSlack);
	}

	TArray& operator=(TArray&& other)
	{
		if (this != &other)
		{
			DestructItems(GetData(), m_ArrayNum);
			MoveOrCopy(*this, other, m_ArrayMax);
		}
		return *this;
	}

	~TArray()
	{
		DestructItems(GetData(), m_ArrayNum);
	}

	ElementType* GetData()
	{
		return (ElementType*)m_AllocatorInstance.GetAllocation();
	}

	const ElementType* GetData() const
	{
		return (const ElementType*)m_AllocatorInstance.GetAllocation();
	}

	uint32 GetTypeSize() const
	{
		return sizeof(ElementType);
	}

	size_t GetAllocatedSize(void) const
	{
		return m_AllocatorInstance.GetAllocatedSize(m_ArrayMax, sizeof(ElementType));
	}

	SizeType GetSlack() const
	{
		return m_ArrayMax - m_ArrayNum;
	}

	void CheckInvariants() const
	{
		Assert((m_ArrayNum >= 0) && (m_ArrayMax >= m_ArrayNum));
	}

	void RangeCheck(SizeType index) const
	{
		CheckInvariants();

		if (Allocator::RequireRangeCheck)
		{
			AssertMsg((index >= 0) && (index < m_ArrayNum), "Array index out of bounds: %i from an array of size %i", index, m_ArrayNum); // & for one branch
		}
	}

	bool IsValidIndex(SizeType index) const
	{
		return index >= 0 && index < m_ArrayNum;
	}

	SizeType Num() const
	{
		return m_ArrayNum;
	}

	SizeType Max() const
	{
		return m_ArrayMax;
	}

	ElementType& operator[](SizeType index)
	{
		RangeCheck(index);
		return GetData()[index];
	}

	const ElementType& operator[](SizeType index) const
	{
		RangeCheck(index);
		return GetData()[index];
	}

	ElementType Pop(bool allowShrinking = true)
	{
		RangeCheck(0);
		ElementType result = MoveTempIfPossible(GetData()[m_ArrayNum - 1]);
		RemoveAt(m_ArrayNum - 1, 1, allowShrinking);
		return result;
	}

	void Push(ElementType&& item)
	{
		Add(MoveTempIfPossible(item));
	}

	void Push(const ElementType& item)
	{
		Add(item);
	}

	ElementType& Top()
	{
		return Last();
	}

	const ElementType& Top() const
	{
		return Last();
	}

	ElementType& Last(SizeType indexFromTheEnd = 0)
	{
		RangeCheck(m_ArrayNum - indexFromTheEnd - 1);
		return GetData()[m_ArrayNum - indexFromTheEnd - 1];
	}

	const ElementType& Last(SizeType indexFromTheEnd = 0) const
	{
		RangeCheck(m_ArrayNum - indexFromTheEnd - 1);
		return GetData()[m_ArrayNum - indexFromTheEnd - 1];
	}

	void Shrink()
	{
		CheckInvariants();
		if (m_ArrayMax != m_ArrayNum)
		{
			ResizeTo(m_ArrayNum);
		}
	}

	bool Find(const ElementType& item, SizeType& index) const
	{
		index = this->Find(item);
		return index != INDEX_NONE;
	}

	SizeType Find(const ElementType& item) const
	{
		const ElementType* start   = GetData();
		const ElementType* dataEnd = start + m_ArrayNum;

		for (const ElementType* data = start; data != dataEnd; ++data)
		{
			if (*data == item)
			{
				return static_cast<SizeType>(data - start);
			}
		}

		return INDEX_NONE;
	}

	bool FindLast(const ElementType& item, SizeType& index) const
	{
		index = this->FindLast(item);
		return index != INDEX_NONE;
	}

	SizeType FindLast(const ElementType& item) const
	{
		const ElementType* start = GetData();
		const ElementType* data  = start + m_ArrayNum;

		while (data != start)
		{
			--data;
			if (*data == item)
			{
				return static_cast<SizeType>(data - start);
			}
		}

		return INDEX_NONE;
	}

	template <typename predicate>
	SizeType FindLastByPredicate(predicate pred, SizeType count) const
	{
		Assert(count >= 0 && count <= this->Num());

		const ElementType* start = GetData();
		const ElementType* data  = start + m_ArrayNum;

		while (data != start)
		{
			--data;
			if (pred(*data))
			{
				return static_cast<SizeType>(data - start);
			}
		}

		return INDEX_NONE;
	}

	template <typename predicate>
	FORCE_INLINE SizeType FindLastByPredicate(predicate pred) const
	{
		return FindLastByPredicate(pred, m_ArrayNum);
	}

	template <typename KeyType>
	SizeType IndexOfByKey(const KeyType& key) const
	{
		const ElementType* start   = GetData();
		const ElementType* dataEnd = start + m_ArrayNum;

		for (const ElementType* data = start; data != dataEnd; ++data)
		{
			if (*data == key)
			{
				return static_cast<SizeType>(data - start);
			}
		}

		return INDEX_NONE;
	}

	template <typename predicate>
	SizeType IndexOfByPredicate(predicate pred) const
	{
		const ElementType* start   = GetData();
		const ElementType* dataEnd = start + m_ArrayNum;

		for (const ElementType* data = start; data != dataEnd; ++data)
		{
			if (pred(*data))
			{
				return static_cast<SizeType>(data - start);
			}
		}

		return INDEX_NONE;
	}

	template <typename KeyType>
	FORCE_INLINE const ElementType* FindByKey(const KeyType& key) const
	{
		return const_cast<TArray*>(this)->FindByKey(key);
	}

	template <typename KeyType>
	ElementType* FindByKey(const KeyType& key)
	{
		const ElementType* start   = GetData();
		const ElementType* dataEnd = start + m_ArrayNum;

		for (ElementType* data = start; data != dataEnd; ++data)
		{
			if (*data == key)
			{
				return data;
			}
		}

		return nullptr;
	}

	template <typename predicate>
	FORCE_INLINE const ElementType* FindByPredicate(predicate pred) const
	{
		return const_cast<TArray*>(this)->FindByPredicate(pred);
	}
	
	template <typename predicate>
	ElementType* FindByPredicate(predicate pred)
	{
		const ElementType* start   = GetData();
		const ElementType* dataEnd = start + m_ArrayNum;

		for (ElementType* data = start; data != dataEnd; ++data)
		{
			if (pred(*data))
			{
				return data;
			}
		}

		return nullptr;
	}

	template <typename predicate>
	TArray<ElementType> FilterByPredicate(predicate pred) const
	{
		TArray<ElementType> filterResults;

		const ElementType* start   = GetData();
		const ElementType* dataEnd = start + m_ArrayNum;

		for (const ElementType* data = start; data != dataEnd; ++data)
		{
			if (pred(*data))
			{
				filterResults.Add(*data);
			}
		}

		return filterResults;
	}

	template <typename ComparisonType>
	bool Contains(const ComparisonType& item) const
	{
		const ElementType* start   = GetData();
		const ElementType* dataEnd = start + m_ArrayNum;

		for (const ElementType* data = start; data != dataEnd; ++data)
		{
			if (*data == item)
			{
				return true;
			}
		}

		return false;
	}

	template <typename predicate>
	FORCE_INLINE bool ContainsByPredicate(predicate pred) const
	{
		return FindByPredicate(pred) != nullptr;
	}

	bool operator==(const TArray& otherArray) const
	{
		SizeType count = Num();

		return count == otherArray.Num() && CompareItems(GetData(), otherArray.GetData(), count);
	}

	FORCE_INLINE bool operator!=(const TArray& otherArray) const
	{
		return !(*this == otherArray);
	}

	FORCE_INLINE SizeType AddUninitialized(SizeType count = 1)
	{
		CheckInvariants();
		Assert(count >= 0);

		const SizeType oldNum = m_ArrayNum;
		if ((m_ArrayNum += count) > m_ArrayMax)
		{
			ResizeGrow(oldNum);
		}

		return oldNum;
	}

	FORCE_INLINE void InsertUninitialized(SizeType index, SizeType count = 1)
	{
		InsertUninitializedImpl(index, count);
	}

	void InsertZeroed(SizeType index, SizeType count = 1)
	{
		InsertUninitializedImpl(index, count);
		
		FMemory::Memzero(GetData() + index, count * sizeof(ElementType));
	}

	ElementType& InsertZeroedGetRef(SizeType index)
	{
		InsertUninitializedImpl(index, 1);
		ElementType* ptr = GetData() + index;
		FMemory::Memzero(ptr, sizeof(ElementType));

		return *ptr;
	}

	void InsertDefaulted(SizeType index, SizeType count = 1)
	{
		InsertUninitializedImpl(index, count);
		DefaultConstructItems<ElementType>(GetData() + index, count);
	}

	ElementType& InsertDefaultedGetRef(SizeType index)
	{
		InsertUninitializedImpl(index, 1);
		ElementType* ptr = GetData() + index;
		DefaultConstructItems<ElementType>(ptr, 1);

		return *ptr;
	}

	SizeType Insert(std::initializer_list<ElementType> initList, const SizeType inIndex)
	{
		SizeType numNewElements = (SizeType)initList.size();

		InsertUninitializedImpl(inIndex, numNewElements);
		ConstructItems<ElementType>(GetData() + inIndex, initList.begin(), numNewElements);

		return inIndex;
	}

	template <typename OtherAllocator>
	SizeType Insert(const TArray<ElementType, OtherAllocator>& items, const SizeType inIndex)
	{
		Assert((const void*)(this) != (const void*)(&items));

		auto numNewElements = items.Num();

		InsertUninitializedImpl(inIndex, numNewElements);
		ConstructItems<ElementType>(GetData() + inIndex, items.GetData(), numNewElements);

		return inIndex;
	}

	template <typename OtherAllocator>
	SizeType Insert(TArray<ElementType, OtherAllocator>&& items, const SizeType inIndex)
	{
		Assert((const void*)(this) != (const void*)(&items));

		auto numNewElements = items.Num();

		InsertUninitializedImpl(inIndex, numNewElements);
		RelocateConstructItems<ElementType>(GetData() + inIndex, items.GetData(), numNewElements);

		items.m_ArrayNum = 0;

		return inIndex;
	}

	SizeType Insert(const ElementType* ptr, SizeType count, SizeType index)
	{
		Assert(ptr != nullptr);

		InsertUninitializedImpl(index, count);
		ConstructItems<ElementType>(GetData() + index, ptr, count);

		return index;
	}

	FORCE_INLINE void CheckAddress(const ElementType* addr) const
	{
		AssertMsg(addr < GetData() || addr >= (GetData() + m_ArrayMax), "Attempting to use a container element (%p) which already comes from the container being modified (%p, m_ArrayMax: %d, m_ArrayNum: %d, SizeofElement: %d)!", addr, GetData(), m_ArrayMax, m_ArrayNum, sizeof(ElementType));
	}

	SizeType Insert(ElementType&& item, SizeType index)
	{
		CheckAddress(&item);

		InsertUninitializedImpl(index, 1);
		new(GetData() + index) ElementType(MoveTempIfPossible(item));

		return index;
	}

	SizeType Insert(const ElementType& item, SizeType index)
	{
		CheckAddress(&item);

		InsertUninitializedImpl(index, 1);
		new(GetData() + index) ElementType(item);

		return index;
	}

	ElementType& InsertGetRef(ElementType&& item, SizeType index)
	{
		CheckAddress(&item);

		InsertUninitializedImpl(index, 1);
		ElementType* ptr = GetData() + index;
		new(ptr) ElementType(MoveTempIfPossible(item));

		return *ptr;
	}

	ElementType& InsertGetRef(const ElementType& item, SizeType index)
	{
		CheckAddress(&item);

		InsertUninitializedImpl(index, 1);
		ElementType* ptr = GetData() + index;
		new(ptr) ElementType(item);

		return *ptr;
	}

	FORCE_INLINE void RemoveAt(SizeType index)
	{
		RemoveAtImpl(index, 1, true);
	}

	template <typename CountType>
	FORCE_INLINE void RemoveAt(SizeType index, CountType count, bool allowShrinking = true)
	{
		static_assert(!TAreTypesEqual<CountType, bool>::Value, "TArray::RemoveAt: unexpected bool passed as the count argument");
		RemoveAtImpl(index, count, allowShrinking);
	}

	FORCE_INLINE void RemoveAtSwap(SizeType index)
	{
		RemoveAtSwapImpl(index, 1, true);
	}

	template <typename CountType>
	FORCE_INLINE void RemoveAtSwap(SizeType index, CountType count, bool allowShrinking = true)
	{
		static_assert(!TAreTypesEqual<CountType, bool>::Value, "TArray::RemoveAtSwap: unexpected bool passed as the count argument");
		RemoveAtSwapImpl(index, count, allowShrinking);
	}

	void Reset(SizeType newSize = 0)
	{
		if (newSize <= m_ArrayMax)
		{
			DestructItems(GetData(), m_ArrayNum);
			m_ArrayNum = 0;
		}
		else
		{
			Empty(newSize);
		}
	}

	void Empty(SizeType slack = 0)
	{
		Assert(slack >= 0);

		DestructItems(GetData(), m_ArrayNum);

		m_ArrayNum = 0;

		if (m_ArrayMax != slack)
		{
			ResizeTo(slack);
		}
	}

	void SetNum(SizeType newNum, bool allowShrinking = true)
	{
		if (newNum > Num())
		{
			const SizeType diff  = newNum - m_ArrayNum;
			const SizeType index = AddUninitialized(diff);
			DefaultConstructItems<ElementType>((uint8*)m_AllocatorInstance.GetAllocation() + index * sizeof(ElementType), diff);
		}
		else if (newNum < Num())
		{
			RemoveAt(newNum, Num() - newNum, allowShrinking);
		}
	}

	void SetNumZeroed(SizeType newNum, bool allowShrinking = true)
	{
		if (newNum > Num())
		{
			AddZeroed(newNum - Num());
		}
		else if (newNum < Num())
		{
			RemoveAt(newNum, Num() - newNum, allowShrinking);
		}
	}

	void SetNumUninitialized(SizeType newNum, bool allowShrinking = true)
	{
		if (newNum > Num())
		{
			AddUninitialized(newNum - Num());
		}
		else if (newNum < Num())
		{
			RemoveAt(newNum, Num() - newNum, allowShrinking);
		}
	}

	void SetNumUnsafeInternal(SizeType newNum)
	{
		Assert(newNum <= Num() && newNum >= 0);
		m_ArrayNum = newNum;
	}
	
	template <typename OtherElementType, typename OtherAllocator>
	void Append(const TArray<OtherElementType, OtherAllocator>& source)
	{
		Assert((void*)(this) != (void*)(&source));

		SizeType sourceCount = source.Num();

		if (!sourceCount)
		{
			return;
		}

		Reserve(m_ArrayNum + sourceCount);
		ConstructItems<ElementType>(GetData() + m_ArrayNum, source.GetData(), sourceCount);

		m_ArrayNum += sourceCount;
	}

	template <typename OtherElementType, typename OtherAllocator>
	void Append(TArray<OtherElementType, OtherAllocator>&& source)
	{
		Assert((void*)(this) != (void*)(&source));

		SizeType sourceCount = source.Num();

		if (!sourceCount)
		{
			return;
		}

		Reserve(m_ArrayNum + sourceCount);
		RelocateConstructItems<ElementType>(GetData() + m_ArrayNum, source.GetData(), sourceCount);
		source.m_ArrayNum = 0;

		m_ArrayNum += sourceCount;
	}

	void Append(const ElementType* ptr, SizeType count)
	{
		Assert(ptr != nullptr || count == 0);

		SizeType pos = AddUninitialized(count);
		ConstructItems<ElementType>(GetData() + pos, ptr, count);
	}

	FORCE_INLINE void Append(std::initializer_list<ElementType> initList)
	{
		SizeType count = (SizeType)initList.size();

		SizeType pos = AddUninitialized(count);
		ConstructItems<ElementType>(GetData() + pos, initList.begin(), count);
	}

	TArray& operator+=(TArray&& other)
	{
		Append(MoveTemp(other));
		return *this;
	}

	TArray& operator+=(const TArray& other)
	{
		Append(other);
		return *this;
	}

	TArray& operator+=(std::initializer_list<ElementType> initList)
	{
		Append(initList);
		return *this;
	}

	template <typename... ArgsType>
	FORCE_INLINE SizeType Emplace(ArgsType&&... args)
	{
		const SizeType index = AddUninitialized(1);
		new(GetData() + index) ElementType(Forward<ArgsType>(args)...);
		return index;
	}

	template <typename... ArgsType>
	FORCE_INLINE ElementType& EmplaceGetRef(ArgsType&&... args)
	{
		const SizeType index = AddUninitialized(1);
		ElementType* ptr = GetData() + index;
		new(ptr) ElementType(Forward<ArgsType>(args)...);
		return *ptr;
	}

	template <typename... ArgsType>
	FORCE_INLINE void EmplaceAt(SizeType index, ArgsType&&... args)
	{
		InsertUninitializedImpl(index, 1);
		new(GetData() + index) ElementType(Forward<ArgsType>(args)...);
	}

	template <typename... ArgsType>
	FORCE_INLINE ElementType& EmplaceAtGetRef(SizeType index, ArgsType&&... args)
	{
		InsertUninitializedImpl(index, 1);
		ElementType* ptr = GetData() + index;
		new(ptr) ElementType(Forward<ArgsType>(args)...);
		return *ptr;
	}

	FORCE_INLINE SizeType Add(ElementType&& item)
	{
		CheckAddress(&item);
		return Emplace(MoveTempIfPossible(item));
	}

	FORCE_INLINE SizeType Add(const ElementType& item)
	{
		CheckAddress(&item);
		return Emplace(item);
	}

	FORCE_INLINE ElementType& AddGetRef(ElementType&& item)
	{
		CheckAddress(&item);
		return EmplaceGetRef(MoveTempIfPossible(item));
	}

	FORCE_INLINE ElementType& AddGetRef(const ElementType& item)
	{
		CheckAddress(&item);
		return EmplaceGetRef(item);
	}

	SizeType AddZeroed(SizeType count = 1)
	{
		const SizeType index = AddUninitialized(count);
		FMemory::Memzero((uint8*)m_AllocatorInstance.GetAllocation() + index * sizeof(ElementType), count * sizeof(ElementType));
		return index;
	}

	ElementType& AddZeroedGetRef()
	{
		const SizeType index = AddUninitialized(1);
		ElementType* ptr = GetData() + index;
		FMemory::Memzero(ptr, sizeof(ElementType));
		return *ptr;
	}

	SizeType AddDefaulted(SizeType count = 1)
	{
		const SizeType index = AddUninitialized(count);
		DefaultConstructItems<ElementType>((uint8*)m_AllocatorInstance.GetAllocation() + index * sizeof(ElementType), count);
		return index;
	}

	ElementType& AddDefaultedGetRef()
	{
		const SizeType index = AddUninitialized(1);
		ElementType* ptr = GetData() + index;
		DefaultConstructItems<ElementType>(ptr, 1);
		return *ptr;
	}

	FORCE_INLINE SizeType AddUnique(ElementType&& item) 
	{ 
		return AddUniqueImpl(MoveTempIfPossible(item)); 
	}

	FORCE_INLINE SizeType AddUnique(const ElementType& item) 
	{ 
		return AddUniqueImpl(item); 
	}

	FORCE_INLINE void Reserve(SizeType number)
	{
		Assert(number >= 0);

		if (number > m_ArrayMax)
		{
			ResizeTo(number);
		}
	}

	void Init(const ElementType& element, SizeType number)
	{
		Empty(number);

		for (SizeType index = 0; index < number; ++index)
		{
			new(*this) ElementType(element);
		}
	}

	SizeType RemoveSingle(const ElementType& item)
	{
		SizeType index = Find(item);
		if (index == INDEX_NONE)
		{
			return 0;
		}

		auto* removePtr = GetData() + index;

		DestructItems(removePtr, 1);

		const SizeType nextIndex = index + 1;
		RelocateConstructItems<ElementType>(removePtr, removePtr + 1, m_ArrayNum - (index + 1));

		--m_ArrayNum;

		return 1;
	}

	SizeType Remove(const ElementType& item)
	{
		CheckAddress(&item);

		return RemoveAll(
			[&item](ElementType& element) 
			{ 
				return element == item; 
			}
		);
	}

	template <class PREDICATE_CLASS>
	SizeType RemoveAll(const PREDICATE_CLASS& predicate)
	{
		const SizeType originalNum = m_ArrayNum;
		if (!originalNum)
		{
			return 0;
		}

		SizeType writeIndex = 0;
		SizeType readIndex  = 0;

		bool notMatch = !predicate(GetData()[readIndex]);

		do
		{
			SizeType runStartIndex = readIndex++;
			while (readIndex < originalNum && notMatch == !predicate(GetData()[readIndex]))
			{
				readIndex++;
			}

			SizeType runLength = readIndex - runStartIndex;
			Assert(runLength > 0);

			if (notMatch)
			{
				if (writeIndex != runStartIndex)
				{
					FMemory::Memmove(&GetData()[writeIndex], &GetData()[runStartIndex], sizeof(ElementType)* runLength);
				}
				writeIndex += runLength;
			}
			else
			{
				DestructItems(GetData() + runStartIndex, runLength);
			}

			notMatch = !notMatch;

		} 
		while (readIndex < originalNum);

		m_ArrayNum = writeIndex;

		return originalNum - m_ArrayNum;
	}

	template <class PREDICATE_CLASS>
	void RemoveAllSwap(const PREDICATE_CLASS& predicate, bool allowShrinking = true)
	{
		SizeType itemIndex = 0;

		while (itemIndex < Num())
		{
			if (predicate((*this)[itemIndex]))
			{
				RemoveAtSwap(itemIndex, 1, allowShrinking);
			}
			else
			{
				++itemIndex;
			}
		}
	}

	SizeType RemoveSingleSwap(const ElementType& item, bool allowShrinking = true)
	{
		SizeType index = Find(item);
		if (index == INDEX_NONE)
		{
			return 0;
		}

		RemoveAtSwap(index, 1, allowShrinking);

		return 1;
	}

	SizeType RemoveSwap(const ElementType& item)
	{
		CheckAddress(&item);

		const SizeType originalNum = m_ArrayNum;
		for (SizeType index = 0; index < m_ArrayNum; ++index)
		{
			if ((*this)[index] == item)
			{
				RemoveAtSwap(index--);
			}
		}

		return originalNum - m_ArrayNum;
	}

	FORCE_INLINE void SwapMemory(SizeType firstIndexToSwap, SizeType secondIndexToSwap)
	{
		FMemory::Memswap(
			(uint8*)m_AllocatorInstance.GetAllocation() + (sizeof(ElementType)*firstIndexToSwap),
			(uint8*)m_AllocatorInstance.GetAllocation() + (sizeof(ElementType)*secondIndexToSwap),
			sizeof(ElementType)
		);
	}

	FORCE_INLINE void Swap(SizeType firstIndexToSwap, SizeType secondIndexToSwap)
	{
		Assert((firstIndexToSwap >= 0) && (secondIndexToSwap >= 0));
		Assert((m_ArrayNum > firstIndexToSwap) && (m_ArrayNum > secondIndexToSwap));

		if (firstIndexToSwap != secondIndexToSwap)
		{
			SwapMemory(firstIndexToSwap, secondIndexToSwap);
		}
	}

public:

	// Iterators
	typedef TIndexedContainerIterator<TArray, ElementType, SizeType>				TIterator;
	typedef TIndexedContainerIterator<const TArray, const ElementType, SizeType>	TConstIterator;
	

	TIterator CreateIterator()
	{
		return TIterator(*this);
	}

	TConstIterator CreateConstIterator() const
	{
		return TConstIterator(*this);
	}

public:

	typedef TCheckedPointerIterator<ElementType, SizeType>			RangedForIteratorType;
	typedef TCheckedPointerIterator<const ElementType, SizeType>	RangedForConstIteratorType;

	FORCE_INLINE RangedForIteratorType begin()
	{ 
		return RangedForIteratorType(m_ArrayNum, GetData()); 
	}

	FORCE_INLINE RangedForConstIteratorType begin() const 
	{ 
		return RangedForConstIteratorType(m_ArrayNum, GetData()); 
	}

	FORCE_INLINE RangedForIteratorType end()      
	{ 
		return RangedForIteratorType(m_ArrayNum, GetData() + Num()); 
	}

	FORCE_INLINE RangedForConstIteratorType end() const 
	{ 
		return RangedForConstIteratorType(m_ArrayNum, GetData() + Num());
	}

public:

private:

	template <typename FromArrayType, typename ToArrayType>
	static FORCE_INLINE typename TEnableIf<Fly3DPrivateArray::TCanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value>::Type MoveOrCopy(ToArrayType& toArray, FromArrayType& fromArray, SizeType prevMax)
	{
		toArray.m_AllocatorInstance.MoveToEmpty(fromArray.m_AllocatorInstance);
		toArray.m_ArrayNum = fromArray.m_ArrayNum;
		toArray.m_ArrayMax = fromArray.m_ArrayMax;
		fromArray.m_ArrayNum = 0;
		fromArray.m_ArrayMax = 0;
	}

	template <typename FromArrayType, typename ToArrayType>
	static FORCE_INLINE typename TEnableIf<!Fly3DPrivateArray::TCanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value>::Type MoveOrCopy(ToArrayType& ToArray, FromArrayType& FromArray, SizeType prevMax)
	{
		ToArray.CopyToEmpty(FromArray.GetData(), FromArray.Num(), prevMax, 0);
	}

	template <typename FromArrayType, typename ToArrayType>
	static FORCE_INLINE typename TEnableIf<Fly3DPrivateArray::TCanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value>::Type MoveOrCopyWithSlack(ToArrayType& ToArray, FromArrayType& FromArray, SizeType prevMax, SizeType extraSlack)
	{
		MoveOrCopy(ToArray, FromArray, prevMax);

		ToArray.Reserve(ToArray.m_ArrayNum + extraSlack);
	}

	template <typename FromArrayType, typename ToArrayType>
	static FORCE_INLINE typename TEnableIf<!Fly3DPrivateArray::TCanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value>::Type MoveOrCopyWithSlack(ToArrayType& ToArray, FromArrayType& FromArray, SizeType prevMax, SizeType extraSlack)
	{
		ToArray.CopyToEmpty(FromArray.GetData(), FromArray.Num(), prevMax, extraSlack);
	}

	template <typename OtherSizeType>
	void InsertUninitializedImpl(SizeType index, OtherSizeType count)
	{
		CheckInvariants();
		Assert((count >= 0) & (index >= 0) & (index <= m_ArrayNum));

		SizeType newNum = count;
		AssertMsg((OtherSizeType)newNum == count, TEXT("Invalid number of elements to add to this array type: %llu"), (unsigned long long)newNum);

		const SizeType oldNum = m_ArrayNum;
		if ((m_ArrayNum += count) > m_ArrayMax)
		{
			ResizeGrow(oldNum);
		}

		ElementType* data = GetData() + index;
		RelocateConstructItems<ElementType>(data + count, data, oldNum - index);
	}

	void RemoveAtImpl(SizeType index, SizeType count, bool allowShrinking)
	{
		if (count)
		{
			CheckInvariants();
			Assert((count >= 0) & (index >= 0) & (index + count <= m_ArrayNum));

			DestructItems(GetData() + index, count);

			SizeType numToMove = m_ArrayNum - index - count;

			if (numToMove)
			{
				FMemory::Memmove
				(
					(uint8*)m_AllocatorInstance.GetAllocation() + (index) * sizeof(ElementType),
					(uint8*)m_AllocatorInstance.GetAllocation() + (index + count) * sizeof(ElementType),
					numToMove * sizeof(ElementType)
				);
			}

			m_ArrayNum -= count;

			if (allowShrinking)
			{
				ResizeShrink();
			}
		}
	}

	void RemoveAtSwapImpl(SizeType index, SizeType count = 1, bool allowShrinking = true)
	{
		if (count)
		{
			CheckInvariants();
			Assert((count >= 0) & (index >= 0) & (index + count <= m_ArrayNum));

			DestructItems(GetData() + index, count);

			const SizeType numElementsInHole = count;
			const SizeType numElementsAfterHole = m_ArrayNum - (index + count);
			const SizeType numElementsToMoveIntoHole = FMath::Min(numElementsInHole, numElementsAfterHole);

			if (numElementsToMoveIntoHole)
			{
				FMemory::Memcpy(
					(uint8*)m_AllocatorInstance.GetAllocation() + (index) * sizeof(ElementType),
					(uint8*)m_AllocatorInstance.GetAllocation() + (m_ArrayNum - numElementsToMoveIntoHole) * sizeof(ElementType),
					numElementsToMoveIntoHole * sizeof(ElementType)
				);
			}

			m_ArrayNum -= count;

			if (allowShrinking)
			{
				ResizeShrink();
			}
		}
	}

	template <typename ArgsType>
	SizeType AddUniqueImpl(ArgsType&& args)
	{
		SizeType index;
		if (Find(args, index))
		{
			return index;
		}

		return Add(Forward<ArgsType>(args));
	}

	void ResizeGrow(SizeType oldNum)
	{
		m_ArrayMax = m_AllocatorInstance.CalculateSlackGrow(m_ArrayNum, m_ArrayMax, sizeof(ElementType));
		m_AllocatorInstance.ResizeAllocation(oldNum, m_ArrayMax, sizeof(ElementType));
	}

	void ResizeShrink()
	{
		const SizeType newArrayMax = m_AllocatorInstance.CalculateSlackShrink(m_ArrayNum, m_ArrayMax, sizeof(ElementType));
		if (newArrayMax != m_ArrayMax)
		{
			m_ArrayMax = newArrayMax;
			Assert(m_ArrayMax >= m_ArrayNum);
			m_AllocatorInstance.ResizeAllocation(m_ArrayNum, m_ArrayMax, sizeof(ElementType));
		}
	}

	void ResizeTo(SizeType newMax)
	{
		if (newMax)
		{
			newMax = m_AllocatorInstance.CalculateSlackReserve(newMax, sizeof(ElementType));
		}

		if (newMax != m_ArrayMax)
		{
			m_ArrayMax = newMax;
			m_AllocatorInstance.ResizeAllocation(m_ArrayNum, m_ArrayMax, sizeof(ElementType));
		}
	}

	void ResizeForCopy(SizeType newMax, SizeType prevMax)
	{
		if (newMax)
		{
			newMax = m_AllocatorInstance.CalculateSlackReserve(newMax, sizeof(ElementType));
		}

		if (newMax != prevMax)
		{
			m_AllocatorInstance.ResizeAllocation(0, newMax, sizeof(ElementType));
		}

		m_ArrayMax = newMax;
	}

	template <typename OtherElementType, typename OtherSizeType>
	void CopyToEmpty(const OtherElementType* otherData, OtherSizeType otherNum, SizeType prevMax, SizeType extraSlack)
	{
		SizeType newNum = otherNum;
		AssertMsg((OtherSizeType)newNum == otherNum, TEXT("Invalid number of elements to add to this array type: %llu"), (unsigned long long)newNum);

		checkSlow(extraSlack >= 0);

		m_ArrayNum = newNum;
		if (otherNum || extraSlack || prevMax)
		{
			ResizeForCopy(newNum + extraSlack, prevMax);
			ConstructItems<ElementType>(GetData(), otherData, otherNum);
		}
		else
		{
			m_ArrayMax = 0;
		}
	}

protected:

	typedef typename TChooseClass<
		Allocator::NeedsElementType,
		typename Allocator::template ForElementType<ElementType>,
		typename Allocator::ForAnyElementType
	>::Result ElementAllocatorType;

protected:

	ElementAllocatorType m_AllocatorInstance;
	SizeType             m_ArrayNum;
	SizeType             m_ArrayMax;

};


template <typename InElementType, typename Allocator>
struct TContainerTraits<TArray<InElementType, Allocator>> : public TContainerTraitsBase<TArray<InElementType, Allocator>>
{
	enum 
	{ 
		MoveWillEmptyContainer = TAllocatorTraits<Allocator>::SupportsMove 
	};
};