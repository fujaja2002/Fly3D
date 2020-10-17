set(Runtime_TLSF_HDRS
    Runtime/TLSF/tlsf.h
)
set(Runtime_TLSF_SRCS
    Runtime/TLSF/tlsf.c
)

set(Runtime_Platform_GenericPlatform_HDRS
    Runtime/Platform/GenericPlatform/GenericPlatform.h
)
set(Runtime_Platform_GenericPlatform_SRCS
)

set(Runtime_Allocator_HDRS
    Runtime/Allocator/AllocatorType.h
    Runtime/Allocator/BaseAllocator.h
    Runtime/Allocator/TLSFAllocator.h
)
set(Runtime_Allocator_SRCS
    Runtime/Allocator/AllocatorType.cpp
    Runtime/Allocator/BaseAllocator.cpp
    Runtime/Allocator/TLSFAllocator.cpp
)

set(Runtime_Utilities_HDRS
    Runtime/Utilities/Align.h
)
set(Runtime_Utilities_SRCS
)

set(Runtime_Template_HDRS
    Runtime/Template/Noncopyable.h
)
set(Runtime_Template_SRCS
)

set(Runtime_Platform_HDRS
    Runtime/Platform/Configure.h
    Runtime/Platform/Platform.h
)
set(Runtime_Platform_SRCS
)

set(Runtime_Log_HDRS
    Runtime/Log/Assert.h
)
set(Runtime_Log_SRCS
)

set(Runtime_Profiler_HDRS
    Runtime/Profiler/MemoryProfiler.h
)
set(Runtime_Profiler_SRCS
    Runtime/Profiler/MemoryProfiler.cpp
)

set(Runtime_Math_HDRS
    Runtime/Math/Math.h
)
set(Runtime_Math_SRCS
)

add_library(FlyCore STATIC
    ${Runtime_TLSF_HDRS}
    ${Runtime_TLSF_SRCS}

    ${Runtime_Platform_GenericPlatform_HDRS}
    ${Runtime_Platform_GenericPlatform_SRCS}

    ${Runtime_Allocator_HDRS}
    ${Runtime_Allocator_SRCS}

    ${Runtime_Utilities_HDRS}
    ${Runtime_Utilities_SRCS}

    ${Runtime_Template_HDRS}
    ${Runtime_Template_SRCS}

    ${Runtime_Platform_HDRS}
    ${Runtime_Platform_SRCS}

    ${Runtime_Log_HDRS}
    ${Runtime_Log_SRCS}

    ${Runtime_Profiler_HDRS}
    ${Runtime_Profiler_SRCS}

    ${Runtime_Math_HDRS}
    ${Runtime_Math_SRCS}

)

source_group(Runtime\\TLSF FILES ${Runtime_TLSF_HDRS} ${Runtime_TLSF_SRCS})
source_group(Runtime\\Platform\\GenericPlatform FILES ${Runtime_Platform_GenericPlatform_HDRS} ${Runtime_Platform_GenericPlatform_SRCS})
source_group(Runtime\\Allocator FILES ${Runtime_Allocator_HDRS} ${Runtime_Allocator_SRCS})
source_group(Runtime\\Utilities FILES ${Runtime_Utilities_HDRS} ${Runtime_Utilities_SRCS})
source_group(Runtime\\Template FILES ${Runtime_Template_HDRS} ${Runtime_Template_SRCS})
source_group(Runtime\\Platform FILES ${Runtime_Platform_HDRS} ${Runtime_Platform_SRCS})
source_group(Runtime\\Log FILES ${Runtime_Log_HDRS} ${Runtime_Log_SRCS})
source_group(Runtime\\Profiler FILES ${Runtime_Profiler_HDRS} ${Runtime_Profiler_SRCS})
source_group(Runtime\\Math FILES ${Runtime_Math_HDRS} ${Runtime_Math_SRCS})
