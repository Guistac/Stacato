#pragma once

struct lua_State;
class ArtNetNode;

namespace Scripting::ArtNetLibrary{
	void openlib(lua_State* L, std::shared_ptr<ArtNetNode> artnetNode);
};
