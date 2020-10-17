#include "Runtime/Allocator/AllocatorType.h"

static const char* g_AllocatorTypeNames[] = 
{
#define SetAllocatorTypeName(Name) "kMemType"#Name ,
#include "AllocatorType.inc"
#undef SetAllocatorTypeName
};

const char* GetAllocatorTypeName(AllocatorType type)
{
	return g_AllocatorTypeNames[type];
}