#include "Runtime/Allocator/AllocatorType.h"

static const char* g_AllocatorTypeNames[] = 
{
#define DO_LABEL(Name) "kMemType"#Name,
#include "AllocatorType.inc"
#undef DO_LABEL
};

const char* GetAllocatorTypeName(EAllocatorType type)
{
	return g_AllocatorTypeNames[type];
}