#pragma once

#include "Runtime/Platform/Platform.h"

template<int32 Size, uint32 Alignment>
struct TAlignedBytes;

template<int32 Size>
struct TAlignedBytes<Size, 1>
{
	uint8 Pad[Size];
};

template<int32 Size>
struct TAlignedBytes<Size, 2>
{
	__declspec(align(2)) 
	struct TPadding
	{
		uint8 Pad[Size];
	};

	TPadding Padding;
};

template<int32 Size>
struct TAlignedBytes<Size, 4>
{
	__declspec(align(4)) 
		struct TPadding
	{
		uint8 Pad[Size];
	};

	TPadding Padding;
};

template<int32 Size>
struct TAlignedBytes<Size, 8>
{
	__declspec(align(8)) 
		struct TPadding
	{
		uint8 Pad[Size];
	};

	TPadding Padding;
};

template<int32 Size>
struct TAlignedBytes<Size, 16>
{
	__declspec(align(16)) 
		struct TPadding
	{
		uint8 Pad[Size];
	};

	TPadding Padding;
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