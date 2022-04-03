#[[
    ofBindings.cpp is not compatible with std::filesystem by default
    A quick fix is to manually replace every binding containing "boost::filesystem" with "std::filesystem"
]]#

message(STATUS "== Including ofxLua")

set(OFX_LUA_DIRECTORY ${DEPENDENCIES_DIRECTORY}/openframeworksAddons/ofxLua)

set(OFX_LUA_SOURCES
    ${OFX_LUA_DIRECTORY}/src/ofxLua.cpp
    ${OFX_LUA_DIRECTORY}/src/ofxLua.h
    ${OFX_LUA_DIRECTORY}/src/ofxLuaFileWriter.cpp
    ${OFX_LUA_DIRECTORY}/src/ofxLuaFileWriter.h

    ${OFX_LUA_DIRECTORY}/src/bindings/ofBindings.h
    ${OFX_LUA_DIRECTORY}/src/bindings/glmBindings.cpp

    ${OFX_LUA_DIRECTORY}/src/bindings/desktop/ofBindings.cpp

    ${OFX_LUA_DIRECTORY}/libs/lua/lapi.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lapi.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lauxlib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lauxlib.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lbaselib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lbitlib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lcode.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lcode.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lcorolib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lctype.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lctype.h
    ${OFX_LUA_DIRECTORY}/libs/lua/ldblib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/ldebug.c
    ${OFX_LUA_DIRECTORY}/libs/lua/ldebug.h
    ${OFX_LUA_DIRECTORY}/libs/lua/ldo.c
    ${OFX_LUA_DIRECTORY}/libs/lua/ldo.h
    ${OFX_LUA_DIRECTORY}/libs/lua/ldump.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lfunc.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lfunc.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lgc.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lgc.h
    ${OFX_LUA_DIRECTORY}/libs/lua/linit.c
    ${OFX_LUA_DIRECTORY}/libs/lua/liolib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/llex.c
    ${OFX_LUA_DIRECTORY}/libs/lua/llex.h
    ${OFX_LUA_DIRECTORY}/libs/lua/llimits.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lmathlib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lmem.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lmem.h
    ${OFX_LUA_DIRECTORY}/libs/lua/loadlib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lobject.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lobject.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lopcodes.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lopcodes.h
    ${OFX_LUA_DIRECTORY}/libs/lua/loslib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lparser.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lparser.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lprefix.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lstate.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lstate.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lstring.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lstring.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lstrlib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/ltable.c
    ${OFX_LUA_DIRECTORY}/libs/lua/ltable.h
    ${OFX_LUA_DIRECTORY}/libs/lua/ltablib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/ltm.c
    ${OFX_LUA_DIRECTORY}/libs/lua/ltm.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lua.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lua.hpp
    ${OFX_LUA_DIRECTORY}/libs/lua/luaconf.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lualib.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lundump.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lundump.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lutf8lib.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lvm.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lvm.h
    ${OFX_LUA_DIRECTORY}/libs/lua/lzio.c
    ${OFX_LUA_DIRECTORY}/libs/lua/lzio.h
)

source_group(TREE ${OFX_LUA_DIRECTORY} FILES ${OFX_LUA_SOURCES})

add_library(ofxLua STATIC ${OFX_LUA_SOURCES})

target_include_directories(ofxLua PUBLIC
    ${OFX_LUA_DIRECTORY}/src
    ${OFX_LUA_DIRECTORY}/src/bindings
    ${OFX_LUA_DIRECTORY}/libs/lua
    ${OF_CORE_INCLUDE_PATHS}
    ${OF_LIB_INCLUDE_PATHS}
)

target_compile_definitions(ofxLua PUBLIC
    OF_USING_STD_FS
    LUA_USE_MACOSX
)