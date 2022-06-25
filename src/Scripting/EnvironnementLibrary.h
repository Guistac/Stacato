#pragma once

struct lua_State;

namespace CoolContainer{

	namespace Lua{
		int openlib(lua_State* L);
	}

}


namespace Scripting::EnvironnementLibrary{
	void openlib(lua_State* L);
};
