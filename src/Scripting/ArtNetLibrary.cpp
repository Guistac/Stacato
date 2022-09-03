#include <pch.h>
#include "ArtNetLibrary.h"

#include "LuaLibraryHelper.h"

#include "Networking/ArtNet/ArtnetNode.h"

namespace Scripting::ArtNetLibrary{
	
	LuaSharedPointer<ArtNetNode, "ArtnetNode"> lua_ArtNetNode;
	LuaSharedPointer<DmxUniverse, "DmxUniverse"> lua_DmxUniverse;


	int lua_createNewUniverse(lua_State* L){
		auto dmxNode = lua_ArtNetNode.checkArgument(L, 1);
		int universeNumber = luaL_checknumber(L, 2);
		auto newUniverse = dmxNode->createNewUniverse(universeNumber);
		lua_DmxUniverse.push(L, newUniverse);
		return 1;
	}

	int lua_setChannel(lua_State* L){
		auto dmxUniverse = lua_DmxUniverse.checkArgument(L, 1);
		int dmxChannel = luaL_checknumber(L, 2);
		int value = luaL_checknumber(L, 3);
		dmxUniverse->setChannel(dmxChannel, value);
		return 0;
	}
	
	void openlib(lua_State* L, std::shared_ptr<ArtNetNode> artnetNode){
		lua_ArtNetNode.addMethod("createNewUniverse", lua_createNewUniverse);
		lua_ArtNetNode.declare(L);
		
		lua_DmxUniverse.addMethod("setChannel", lua_setChannel);
		lua_DmxUniverse.declare(L);
		
		lua_ArtNetNode.push(L, artnetNode);
		lua_setglobal(L, "ArtNetNode");
	}
};
