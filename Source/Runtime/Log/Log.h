#pragma once

#include "Runtime/Platform/Platform.h"

#if FLY_DEBUG
	#define LOGD(...)  do { fprintf(stdout, "%s", "LOGD:"); fprintf(stdout, __VA_ARGS__); } while (0)
	#define LOGI(...)  do { fprintf(stdout, "%s", "LOGI:"); fprintf(stdout, __VA_ARGS__); } while (0)
#else
	#define LOGD(...)  do { } while (0)
	#define LOGI(...)  do { } while (0)
#endif

#define LOGW(...)  do { fprintf(stdout, "%s", "LOGW:"); fprintf(stdout, __VA_ARGS__); } while (0)
#define LOGE(...)  do { fprintf(stdout, "%s", "LOGE:"); fprintf(stdout, __VA_ARGS__); } while (0)
#define LOGF(...)  do { fprintf(stdout, "%s", "LOGF:"); fprintf(stdout, __VA_ARGS__); } while (0)