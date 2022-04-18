#include <pch.h>
#include "LoggingLibrary.h"

#include <ofxLua.h>

#include "Script.h"

namespace Scripting::LogLibrary{

	enum class LogLevel{
		INFO,
		WARNING
	};

	void log(lua_State* L, LogLevel level){
		
		Script** userData = (Script**)luaL_checkudata(L, 1, "Logger.Type");
		Script* script = *userData;
		
		char messageBuffer[512];
		messageBuffer[0] = 0;
		int messageSize = 0;
		
		//get number of arguments (argument at stack index 1 is Logger UserData)
		int top = lua_gettop(L);
		if(top <= 1) return; //dont push a message to the console if it contains nothing
		for (int i = 2; i <= top; i++) {
			int type = lua_type(L, i);
			switch (type) {
				case LUA_TSTRING:{
					const char* string = lua_tostring(L, i);
					messageSize += sprintf(messageBuffer + messageSize, "%s ", string);
					break;}
				case LUA_TBOOLEAN:{
					const char* boolString = lua_toboolean(L, i) ? "true" : "false";
					messageSize += sprintf(messageBuffer + messageSize, "%s ", boolString);
					break;}
				case LUA_TNUMBER:{
					double number = lua_tonumber(L, i);
					if(number == (int)number) messageSize += sprintf(messageBuffer + messageSize, "%i ", (int)number);
					else messageSize += sprintf(messageBuffer + messageSize, "%.3f ", number);
					break;}
				default:{
					const char* typeName = lua_typename(L, type);
					messageSize += sprintf(messageBuffer + messageSize, "%s ", typeName);
					break;}
			}
		}
		
		switch(level){
			case LogLevel::INFO:
				script->logInfo(messageBuffer);
				Logger::info("[{}] {}", script->name, messageBuffer);
				break;
			case LogLevel::WARNING:
				script->logWarning(messageBuffer);
				Logger::warn("[{}] {}", script->name, messageBuffer);
				break;
		}
	}

	int logInfo(lua_State* L){
		log(L, LogLevel::INFO);
		return 0;
	}

	int logWarning(lua_State* L){
		log(L, LogLevel::WARNING);
		return 0;
	}

	int clearLog(lua_State* L){
		Script** userData = (Script**)luaL_checkudata(L, 1, "Logger.Type");
		Script* script = *userData;
		script->clearConsole();
		return 0;
	}

	void openLib(lua_State* L, Script* script){
		//metatable for Logger.Type
		luaL_newmetatable(L, "Logger.Type");
		//index table for Logger.Type Metatable
		lua_newtable(L);
		
		//add Logger methods to Logger.Type index table
		lua_pushcfunction(L, logInfo);
		lua_setfield(L, -2, "info");
		lua_pushcfunction(L, logWarning);
		lua_setfield(L, -2, "warn");
		lua_pushcfunction(L, clearLog);
		lua_setfield(L, -2, "clear");
		
		//set index table key
		lua_setfield(L, -2, "__index");
		//pop the CoolThing metatable off the stack
		lua_pop(L, 1);

		//create Logger Object in Lua
		Script** userData = (Script**)lua_newuserdata(L, sizeof(Script*));
		*userData = script;
		
		//assign Logger.Type metatable to Logger object
		luaL_getmetatable(L, "Logger.Type");
		lua_setmetatable(L, -2);
		
		//push Logger Object to script as global called 'Logger'
		lua_setglobal(L, "Logger");
	}


};
