#pragma once

enum EAllocatorType
{
#define DO_LABEL(Name) kMemType##Name,
#include "AllocatorType.inc"
#undef DO_LABEL
	kMemTypeCout
};

const char* GetAllocatorTypeName(EAllocatorType type);