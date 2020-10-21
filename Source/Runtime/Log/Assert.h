#pragma once

#include "Runtime/Log/Log.h"
#include "Runtime/Platform/Platform.h"

#include <assert.h>

#if ENABLE_ASSERTIONS
	#define ASSERT_IMP(test, msg) do { if (!test) { LOGE("%s(%d) : %s\n", __FILE__, __LINE__, msg); } assert(test); } while (0)
#else
	#define ASSERT_IMP(test, msg)
#endif

#define Assert(test)			ASSERT_IMP(test, "Assertion failed on expression: '" #test "'")
#define AssertMsg(test, msg)	ASSERT_IMP(test, msg)