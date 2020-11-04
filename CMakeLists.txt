cmake_minimum_required(VERSION 3.12.0)

set(ENGINE_NAME Fly3D)

project(${ENGINE_NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DNOMINMAX=1")

if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_WINDOWS=1")
elseif (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_LINUX=1")
endif ()

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

include_directories(
	Source/
)

add_subdirectory(Source)

set(ALL_LIBS
	${ALL_LIBS}
	FlyCore
)

add_executable(${ENGINE_NAME} Source/main.cpp)
target_link_libraries(${ENGINE_NAME} ${ALL_LIBS})
set_target_properties(${ENGINE_NAME} PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)