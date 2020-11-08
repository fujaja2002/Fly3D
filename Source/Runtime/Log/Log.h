#pragma once

#include "Runtime/Platform/Platform.h"

#include <stdio.h>

#if FLY_DEBUG
	#define LOGD(...)  do { fprintf(stdout, "%s", "(Fly3D) [ DEBUG ]:"); fprintf(stdout, __VA_ARGS__); } while (0)
	#define LOGI(...)  do { fprintf(stdout, "%s", "(Fly3D) [ INFO  ]:"); fprintf(stdout, __VA_ARGS__); } while (0)
#else
	#define LOGD(...)  do { } while (0)
	#define LOGI(...)  do { } while (0)
#endif

#define LOGW(...)  do { fprintf(stdout, "%s", "(Fly3D) [WARNING]:"); fprintf(stdout, __VA_ARGS__); } while (0)
#define LOGE(...)  do { fprintf(stdout, "%s", "(Fly3D) [ ERROR ]:"); fprintf(stdout, __VA_ARGS__); } while (0)
#define LOGF(...)  do { fprintf(stdout, "%s", "(Fly3D) [ FAULT ]:"); fprintf(stdout, __VA_ARGS__); } while (0)