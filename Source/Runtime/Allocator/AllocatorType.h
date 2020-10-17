#pragma once

enum AllocatorType
{
#define SetAllocatorTypeName(Name) kMemType##Name ,
#include "AllocatorType.inc"
#undef SetAllocatorTypeName
	kMemTypeCout
};

const char* GetAllocatorTypeName(AllocatorType type);