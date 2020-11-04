#pragma once

#include "Runtime/Platform/Platform.h"

template<int32 Size, uint32 Alignment>
struct TAlignedBytes;

template<int32 Size>
struct TAlignedBytes<Size, 1>
{
	uint8 Pad[Size];
};

template<typename ElementType>
struct TTypeCompatibleBytes : public TAlignedBytes<sizeof(ElementType), alignof(ElementType)>
{
	ElementType* GetTypedPtr()		
	{ 
		return (ElementType*)this;  
	}

	const ElementType* GetTypedPtr() const	
	{ 
		return (const ElementType*)this; 
	}
};