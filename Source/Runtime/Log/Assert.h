#pragma once

#include "Runtime/Log/Log.h"
#include "Runtime/Platform/Platform.h"

#include <assert.h>

#if ENABLE_ASSERTIONS
	#define ASSERT_IMP(test, ...) do { if (!test) { LOGE("%s(%d) : ", __FILE__, __LINE__); LOGE(##__VA_ARGS__); } assert(test); } while (0)
#else
	#define ASSERT_IMP(test, ...)
#endif

#define Assert(test)			ASSERT_IMP((test), "Assertion failed on expression: '" #test "'")
#define AssertMsg(test, ...)	ASSERT_IMP((test), ##__VA_ARGS__)