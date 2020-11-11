set(Runtime_Allocator_HDRS
    Runtime/Allocator/AllocatorType.h
    Runtime/Allocator/BaseAllocator.h
    Runtime/Allocator/MemoryMacros.h
    Runtime/Allocator/TLSFAllocator.h
)
set(Runtime_Allocator_SRCS
    Runtime/Allocator/AllocatorType.cpp
    Runtime/Allocator/BaseAllocator.cpp
    Runtime/Allocator/MemoryMacros.cpp
    Runtime/Allocator/TLSFAllocator.cpp
)

set(Runtime_Platform_HDRS
    Runtime/Platform/Configure.h
    Runtime/Platform/Platform.h
)
set(Runtime_Platform_SRCS
)

set(Runtime_Profiler_HDRS
    Runtime/Profiler/MemoryProfiler.h
)
set(Runtime_Profiler_SRCS
    Runtime/Profiler/MemoryProfiler.cpp
)

set(Runtime_Platform_GenericPlatform_HDRS
    Runtime/Platform/GenericPlatform/GenericPlatform.h
)
set(Runtime_Platform_GenericPlatform_SRCS
)

set(Runtime_Loop_HDRS
    Runtime/Loop/EngineLoop.h
)
set(Runtime_Loop_SRCS
    Runtime/Loop/EngineLoop.cpp
)

set(Runtime_Template_HDRS
    Runtime/Template/AndOrNot.h
    Runtime/Template/AreTypesEqual.h
    Runtime/Template/ChooseClass.h
    Runtime/Template/CopyQualifiersFromTo.h
    Runtime/Template/Decay.h
    Runtime/Template/EnableIf.h
    Runtime/Template/Function.h
    Runtime/Template/Greater.h
    Runtime/Template/Invoke.h
    Runtime/Template/IsArithmetic.h
    Runtime/Template/IsConstructible.h
    Runtime/Template/IsEnum.h
    Runtime/Template/IsEnumClass.h
    Runtime/Template/IsFloatingPoint.h
    Runtime/Template/IsIntegral.h
    Runtime/Template/IsInvocable.h
    Runtime/Template/IsMemberPointer.h
    Runtime/Template/IsPODType.h
    Runtime/Template/IsPointer.h
    Runtime/Template/IsTriviallyDestructible.h
    Runtime/Template/MemoryOps.h
    Runtime/Template/Noncopyable.h
    Runtime/Template/PointerIsConvertibleFromTo.h
    Runtime/Template/RemoveCV.h
    Runtime/Template/RemoveExtent.h
    Runtime/Template/RemovePointer.h
    Runtime/Template/RemoveReference.h
    Runtime/Template/SharedPointer.h
    Runtime/Template/Template.h
    Runtime/Template/TypeCompatibleBytes.h
    Runtime/Template/TypeTraits.h
)
set(Runtime_Template_SRCS
)

set(Runtime_Log_HDRS
    Runtime/Log/Assert.h
    Runtime/Log/Log.h
)
set(Runtime_Log_SRCS
)

set(Runtime_Windows_HDRS
    Runtime/Windows/WindowDefinition.h
    Runtime/Windows/WindowsApplication.h
    Runtime/Windows/WindowsMisc.h
    Runtime/Windows/WindowsWindow.h
)
set(Runtime_Windows_SRCS
    Runtime/Windows/WindowsApplication.cpp
    Runtime/Windows/WindowsMisc.cpp
    Runtime/Windows/WindowsWindow.cpp
)

set(Runtime_TLSF_HDRS
    Runtime/TLSF/tlsf.h
)
set(Runtime_TLSF_SRCS
    Runtime/TLSF/tlsf.c
)

set(Runtime_Core_HDRS
    Runtime/Core/Globals.h
    Runtime/Core/PlatformAtomics.h
    Runtime/Core/PlatformMemory.h
)
set(Runtime_Core_SRCS
    Runtime/Core/Globals.cpp
)

set(Runtime_Utilities_HDRS
    Runtime/Utilities/Align.h
)
set(Runtime_Utilities_SRCS
)

set(Runtime_Math_HDRS
    Runtime/Math/Color.h
    Runtime/Math/Math.h
    Runtime/Math/Vector2D.h
)
set(Runtime_Math_SRCS
    Runtime/Math/Color.cpp
    Runtime/Math/Vector2D.cpp
)

add_library(FlyCore STATIC
    ${Runtime_Allocator_HDRS}
    ${Runtime_Allocator_SRCS}

    ${Runtime_Platform_HDRS}
    ${Runtime_Platform_SRCS}

    ${Runtime_Profiler_HDRS}
    ${Runtime_Profiler_SRCS}

    ${Runtime_Platform_GenericPlatform_HDRS}
    ${Runtime_Platform_GenericPlatform_SRCS}

    ${Runtime_Loop_HDRS}
    ${Runtime_Loop_SRCS}

    ${Runtime_Template_HDRS}
    ${Runtime_Template_SRCS}

    ${Runtime_Log_HDRS}
    ${Runtime_Log_SRCS}

    ${Runtime_Windows_HDRS}
    ${Runtime_Windows_SRCS}

    ${Runtime_TLSF_HDRS}
    ${Runtime_TLSF_SRCS}

    ${Runtime_Core_HDRS}
    ${Runtime_Core_SRCS}

    ${Runtime_Utilities_HDRS}
    ${Runtime_Utilities_SRCS}

    ${Runtime_Math_HDRS}
    ${Runtime_Math_SRCS}

)

source_group(Runtime\\Allocator FILES ${Runtime_Allocator_HDRS} ${Runtime_Allocator_SRCS})
source_group(Runtime\\Platform FILES ${Runtime_Platform_HDRS} ${Runtime_Platform_SRCS})
source_group(Runtime\\Profiler FILES ${Runtime_Profiler_HDRS} ${Runtime_Profiler_SRCS})
source_group(Runtime\\Platform\\GenericPlatform FILES ${Runtime_Platform_GenericPlatform_HDRS} ${Runtime_Platform_GenericPlatform_SRCS})
source_group(Runtime\\Loop FILES ${Runtime_Loop_HDRS} ${Runtime_Loop_SRCS})
source_group(Runtime\\Template FILES ${Runtime_Template_HDRS} ${Runtime_Template_SRCS})
source_group(Runtime\\Log FILES ${Runtime_Log_HDRS} ${Runtime_Log_SRCS})
source_group(Runtime\\Windows FILES ${Runtime_Windows_HDRS} ${Runtime_Windows_SRCS})
source_group(Runtime\\TLSF FILES ${Runtime_TLSF_HDRS} ${Runtime_TLSF_SRCS})
source_group(Runtime\\Core FILES ${Runtime_Core_HDRS} ${Runtime_Core_SRCS})
source_group(Runtime\\Utilities FILES ${Runtime_Utilities_HDRS} ${Runtime_Utilities_SRCS})
source_group(Runtime\\Math FILES ${Runtime_Math_HDRS} ${Runtime_Math_SRCS})
