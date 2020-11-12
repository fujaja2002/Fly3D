cmake_minimum_required(VERSION 2.8)

project(lua)

set(LUA_HDRS
    lapi.h
    lauxlib.h
    lcode.h
    lctype.h
    ldebug.h
    ldo.h
    lfunc.h
    lgc.h
    ljumptab.h
    llex.h
    llimits.h
    lmem.h
    lobject.h
    lopcodes.h
    lopnames.h
    lparser.h
    lprefix.h
    lstate.h
    lstring.h
    ltable.h
    ltests.h
    ltm.h
    lua.h
    luaconf.h
    lualib.h
    lundump.h
    lvm.h
    lzio.h
    lua.hpp
)

set(LUA_SRCS
    lapi.c
    lauxlib.c
    lbaselib.c
    lcode.c
    lcorolib.c
    lctype.c
    ldblib.c
    ldebug.c
    ldo.c
    ldump.c
    lfunc.c
    lgc.c
    linit.c
    liolib.c
    llex.c
    lmathlib.c
    lmem.c
    loadlib.c
    lobject.c
    lopcodes.c
    loslib.c
    lparser.c
    lstate.c
    lstring.c
    lstrlib.c
    ltable.c
    ltablib.c
    ltests.c
    ltm.c
    lundump.c
    lutf8lib.c
    lvm.c
    lzio.c
    onelua.c
)

add_library(lua STATIC
	${LUA_HDRS}
	${LUA_SRCS}
)

source_group(src\\ FILES ${LUA_HDRS} ${LUA_SRCS})