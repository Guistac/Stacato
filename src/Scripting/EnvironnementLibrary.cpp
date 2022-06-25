#include <pch.h>
#include "EnvironnementLibrary.h"

#include "LuaLibrary.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

#include <iostream>


//luaL_argcheck(L, index >= 1 && index <= getThingCount(), -1, "getThing() array index out of range");


namespace Scripting::EnvironnementLibrary{

	//————————————————— LIBRARY TYPES ——————————————————

	LuaLibrary lua_Environnement;
	LuaObject<Machine> lua_Machine("Machine");
	LuaObject<Animatable> lua_Animatable("Animatable");
	LuaEnumerator(AnimatableType, "AnimatableType");

	//————————————————— ENVIRONNEMENT ——————————————————

	namespace Lua_Environnement{

		int getMachineCount(lua_State* L){
			if(lua_gettop(L) != 0) luaL_error(L, "Too many arguments provided to Environnement.getMachineCount(), expected 0");
			int machineCount = Environnement::getMachines().size();
			lua_pushnumber(L, machineCount);
			return 1;
		}

		int hasMachine(lua_State* L){
			if(lua_gettop(L) != 1) luaL_error(L, "Too many arguments provoded to Environnement.hasMachine(), expected 1");
			const char* machineName = luaL_checkstring(L, -1);
			for(auto& machine : Environnement::getMachines()){
				if(strcmp(machine->getName(), machineName) == 0){
					lua_pushboolean(L, true);
					return 1;
				}
			}
			lua_pushboolean(L, false);
			return 1;
		}

		int getMachine(lua_State* L){
			if(lua_gettop(L) != 1) luaL_error(L, "Incorrect argument count provided to Environnement.getMachine(), expected 1");
			const char* machineName = luaL_checkstring(L, -1);
			for(auto& machine : Environnement::getMachines()){
				if(strcmp(machine->getName(), machineName) == 0){

					lua_Machine.push(L, machine.get());
					return 1;
				}
			}
			lua_pushnil(L);
			return 1;
		}
	
		int getMachines(lua_State* L){
			if(lua_gettop(L) != 0) luaL_error(L, "Incorrect argument count provided to Environnement.getMachines(), expected 0");
			auto& machines = Environnement::getMachines();
			lua_createtable(L, machines.size(), 1);
			for(int i = 0; i < machines.size(); i++){
				lua_pushinteger(L, i);
				lua_Machine.push(L, machines[i].get());
				lua_settable(L, -3);
			}
			return 1;
		}

	}



	//————————————————— MACHINE ——————————————————

	namespace Lua_Machine{

		int getName(lua_State* L){
			Machine* machine = lua_Machine.checkArgument(L, 1);
			lua_pushstring(L, machine->getName());
			return 1;
		}

		int hasAnimatable(lua_State* L){
			auto machine = lua_Machine.checkArgument(L, 1);
			const char* animatableName = luaL_checkstring(L, 2);
			for(auto& animatable : machine->animatables){
				if(strcmp(animatable->getName(), animatableName) == 0){
					lua_pushboolean(L, true);
					return 1;
				}
			}
			lua_pushboolean(L, false);
			return 1;
		}

		int getAnimatable(lua_State* L){
			auto machine = lua_Machine.checkArgument(L, 1);
			const char* animatableName = luaL_checkstring(L, 2);
			for(auto& animatable : machine->animatables){
				if(strcmp(animatable->getName(), animatableName) == 0){
					lua_Animatable.push(L, animatable.get());
					return 1;
				}
			}
			lua_pushnil(L);
			return 1;
		}
	
		int getAnimatables(lua_State* L){
			auto machine = lua_Machine.checkArgument(L, 1);
			auto& animatables = machine->animatables;
			lua_createtable(L, animatables.size(), 1);
			for(int i = 0; i < animatables.size(); i++){
				lua_pushinteger(L, i);
				lua_Animatable.push(L, animatables[i].get());
				lua_settable(L, -3);
			}
			return 1;
		}

	}



	//————————————————— ANIMATABLE ——————————————————

	namespace Lua_Animatable{

		int getName(lua_State* L){
			auto animatable = lua_Animatable.checkArgument(L, 1);
			lua_pushstring(L, animatable->getName());
			return 1;
		}

		int getType(lua_State* L){
			auto animatable = lua_Animatable.checkArgument(L, 1);
			AnimatableType type = animatable->getType();
			LuaEnumerator_AnimatableType::push(L, type);
			return 1;
		}
	
	}
	

	//————————————————— LIBRARY ——————————————————
	
	void openlib(lua_State* L){
		
		LuaEnumerator_AnimatableType::declare(L);
		
		lua_Machine.addMethod("getName", Lua_Machine::getName);
		lua_Machine.addMethod("hasAnimatable", Lua_Machine::hasAnimatable);
		lua_Machine.addMethod("getAnimatable", Lua_Machine::getAnimatable);
		lua_Machine.addMethod("getAnimatables", Lua_Machine::getAnimatables);
		lua_Machine.declare(L);
		
		lua_Animatable.addMethod("getName", Lua_Animatable::getName);
		lua_Animatable.addMethod("getType", Lua_Animatable::getType);
		lua_Animatable.declare(L);
		
		lua_Environnement.addFunction("getMachineCount", Lua_Environnement::getMachineCount);
		lua_Environnement.addFunction("hasMachine", Lua_Environnement::hasMachine);
		lua_Environnement.addFunction("getMachine", Lua_Environnement::getMachine);
		lua_Environnement.addFunction("getMachines", Lua_Environnement::getMachines);
		lua_Environnement.declare(L, "Environnement");
		
		
	}



};









