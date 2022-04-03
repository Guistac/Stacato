#include <pch.h>
#include "LoggingLibrary.h"

#include <ofxLua.h>

namespace Scripting::LoggingLibrary{

	void log(lua_State* L, int logType){
		int argType = lua_type(L, -1);
		switch(argType){
			case LUA_TNIL:
			case LUA_TBOOLEAN:{
				int integer = luaL_checkinteger(L, -1);
				}break;
			case LUA_TNUMBER:{
				double number = luaL_checknumber(L, -1);
				}break;
			case LUA_TSTRING:{
				const char* string = luaL_checkstring(L, -1);
				}break;
			default:
				luaL_argcheck(L, false, -1, "Argument Type unsupported.");
				break;
		}
	}

	void logInfo(lua_State* L){
		log(L, 0);
	}

	void logWarning(lua_State* L){
		log(L, 1);
	}

	void openLib(lua_State* L){
		
	}

};
