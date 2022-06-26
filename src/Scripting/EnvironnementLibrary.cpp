#include <pch.h>
#include "EnvironnementLibrary.h"

#include "LuaLibrary.h"

#include "Environnement/Environnement.h"
#include "Animation/AnimationValue.h"
#include "Machine/Machine.h"

#include <iostream>


//luaL_argcheck(L, index >= 1 && index <= getThingCount(), -1, "getThing() array index out of range");


namespace Scripting::EnvironnementLibrary{

	//————————————————— TYPES ——————————————————

	LuaEnumerator(AnimatableType, "AnimatableType");
	LuaShared(Machine, "Machine");
	LuaShared(Animatable, "Animatable");
	LuaShared(AnimationValue, "AnimationValue");

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
					LuaShared_Machine::push(L, machine);
					return 1;
				}
			}
			lua_pushnil(L);
			return 1;
		}
	
		int getMachines(lua_State* L){
			if(lua_gettop(L) != 0) luaL_error(L, "Incorrect argument count provided to Environnement.getMachines(), expected 0");
			auto& machines = Environnement::getMachines();
			LuaTable machinesTable(L);
			machinesTable.begin(machines.size());
			for(auto& machine : machines){
				machinesTable.addShared(machine, LuaShared_Machine::push);
			}
			return 1;
		}

	}



	//————————————————— MACHINE ——————————————————

	namespace Lua_Machine{

		int getName(lua_State* L){
			auto machine = LuaShared_Machine::checkArgument(L, 1);
			lua_pushstring(L, machine->getName());
			return 1;
		}

		int hasAnimatable(lua_State* L){
			auto machine = LuaShared_Machine::checkArgument(L, 1);
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
			auto machine = LuaShared_Machine::checkArgument(L, 1);
			const char* animatableName = luaL_checkstring(L, 2);
			for(auto& animatable : machine->animatables){
				if(strcmp(animatable->getName(), animatableName) == 0){
					LuaShared_Animatable::push(L, animatable);
					return 1;
				}
			}
			lua_pushnil(L);
			return 1;
		}
	
		int getAnimatables(lua_State* L){
			auto machine = LuaShared_Machine::checkArgument(L, 1);
			auto& animatables = machine->animatables;
			lua_createtable(L, animatables.size(), 1);
			for(int i = 0; i < animatables.size(); i++){
				lua_pushinteger(L, i);
				LuaShared_Animatable::push(L, animatables[i]);
				lua_settable(L, -3);
			}
			return 1;
		}
	
		int toString(lua_State* L){
			auto machine = LuaShared_Machine::checkArgument(L, 1);
			lua_pushstring(L, machine->getName());
			return 1;
		}

	}



	//————————————————— ANIMATABLE ——————————————————

	namespace Lua_Animatable{

		int getName(lua_State* L){
			auto animatable = LuaShared_Animatable::checkArgument(L, 1);
			lua_pushstring(L, animatable->getName());
			return 1;
		}

		int getType(lua_State* L){
			auto animatable = LuaShared_Animatable::checkArgument(L, 1);
			AnimatableType type = animatable->getType();
			LuaEnumerator_AnimatableType::push(L, type);
			return 1;
		}
	
		int getActualValue(lua_State* L){
			auto animatable = LuaShared_Animatable::checkArgument(L, 1);
			auto value = animatable->getActualValue();
			LuaShared_AnimationValue::push(L, value);
			return 1;
		}
	
		int getAnimationValue(lua_State* L){
			auto animatable = LuaShared_Animatable::checkArgument(L, 1);
			auto value = animatable->getAnimationValue();
			LuaShared_AnimationValue::push(L, value);
			return 1;
		}
	
	};


	namespace Lua_AnimationValue{
	
		int getType(lua_State* L){
			auto animationValue = LuaShared_AnimationValue::checkArgument(L, 1);
			AnimatableType type = animationValue->getType();
			LuaEnumerator_AnimatableType::push(L, type);
			return 1;
		}
	
	};
	

	//————————————————— LIBRARY ——————————————————
	
	void openlib(lua_State* L){
		
		LuaEnumerator_AnimatableType::declare(L);
		
		LuaShared_Machine::addMethod("getName", Lua_Machine::getName);
		LuaShared_Machine::addMethod("hasAnimatable", Lua_Machine::hasAnimatable);
		LuaShared_Machine::addMethod("getAnimatable", Lua_Machine::getAnimatable);
		LuaShared_Machine::addMethod("getAnimatables", Lua_Machine::getAnimatables);
		LuaShared_Machine::setToStringMethod(Lua_Machine::toString);
		LuaShared_Machine::declare(L);
		
		LuaShared_Animatable::addMethod("getName", Lua_Animatable::getName);
		LuaShared_Animatable::addMethod("getType", Lua_Animatable::getType);
		LuaShared_Animatable::addMethod("getActualValue", Lua_Animatable::getActualValue);
		LuaShared_Animatable::addMethod("getAnimationValue", Lua_Animatable::getAnimationValue);
		LuaShared_Animatable::declare(L);
		
		
		LuaShared_AnimationValue::addMethod("getType", Lua_AnimationValue::getType);
		LuaShared_AnimationValue::declare(L);
		 
		LuaTable environnementLibrary(L);
		environnementLibrary.begin()
		.addFunction(Lua_Environnement::getMachineCount, "getMachineCount")
		.addFunction(Lua_Environnement::hasMachine, "hasMachine")
		.addFunction(Lua_Environnement::getMachine, "getMachine")
		.addFunction(Lua_Environnement::getMachines, "getMachines")
		.setGlobal("Environnement");
		
	}

};









