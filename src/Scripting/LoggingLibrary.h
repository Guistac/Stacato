#pragma once

class LuaScript;
struct lua_State;

namespace Scripting::LogLibrary{

	void openLib(lua_State* L, LuaScript* script);

}
