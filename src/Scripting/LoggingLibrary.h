#pragma once

class Script;
struct lua_State;

namespace Scripting::LogLibrary{

	void openLib(lua_State* L, Script* script);

}
