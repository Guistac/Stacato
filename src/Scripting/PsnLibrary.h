#pragma once

struct lua_State;
class PsnServer;

namespace Scripting::PsnLibrary{
	void openlib(lua_State* L, std::shared_ptr<PsnServer> server);
};
