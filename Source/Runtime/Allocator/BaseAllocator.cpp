#include "Runtime/Allocator/BaseAllocator.h"

FBaseAllocator::FBaseAllocator(bool threadSafe)
	: m_NumAllocations(0)
	, m_TotalAllocatedBytes(0)
	, m_TotalReservedBytes(0)
	, m_PeakAllocatedBytes(0)
	, m_IsThreadSafe(threadSafe)
{

}

FBaseAllocator::~FBaseAllocator()
{

}

bool FBaseAllocator::TryDeallocate(uint8* p)
{
	if (!Contains(p))
	{
		return false;
	}

	Deallocate(p);

	return true;
}