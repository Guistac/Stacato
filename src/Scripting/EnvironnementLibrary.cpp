#include <pch.h>
#include "EnvironnementLibrary.h"

#include "LuaLibraryHelper.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

#include "Animation/Animatables/AnimatableBoolean.h"
#include "Animation/Animatables/AnimatableState.h"
#include "Animation/Animatables/AnimatablePosition.h"

namespace Scripting::EnvironnementLibrary{

	//————————————————— TYPES ——————————————————

	LuaEnumerator<AnimatableType, "AnimatableType"> lua_AnimatableType;

	LuaSharedPointer<Machine, "Machine"> lua_Machine;
	LuaSharedPointer<Animatable, "Animatable"> lua_Animatable;

	LuaSharedPointer<AnimatableBoolean, "AnimatableBoolean"> lua_AnimatableBoolean;

	LuaSharedPointer<AnimatableState, "AnimatableState"> lua_AnimatableState;
	LuaPointer<AnimatableStateStruct, "AnimatableStateStruct"> lua_AnimatableStateStruct;

	LuaSharedPointer<AnimatablePosition, "AnimatablePosition"> lua_AnimatablePosition;

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
					lua_Machine.push(L, machine);
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
				machinesTable.addCustom(machine, lua_Machine.push);
			}
			return 1;
		}

	}



	//————————————————— MACHINE ——————————————————

	namespace Lua_Machine{

		int getName(lua_State* L){
			auto machine = lua_Machine.checkArgument(L, 1);
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
					switch(animatable->getType()){
						case AnimatableType::BOOLEAN:
							lua_AnimatableBoolean.push(L, animatable->toBoolean()); return 1;
						case AnimatableType::STATE:
							lua_AnimatableState.push(L, animatable->toState()); return 1;
						case AnimatableType::POSITION:
							lua_AnimatablePosition.push(L, animatable->toPosition()); return 1;
						default:
							lua_pushnil(L);
							return;
					}
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
				lua_Animatable.push(L, animatables[i]);
				lua_settable(L, -3);
			}
			return 1;
		}
	
		int toString(lua_State* L){
			auto machine = lua_Machine.checkArgument(L, 1);
			lua_pushstring(L, machine->getName());
			return 1;
		}

	}



	//————————————————— ANIMATABLE ——————————————————

	namespace Lua_Animatable{
	
		int getName(lua_State* L){
			auto animatable = lua_Animatable.checkDerivedArgument(L, 1);
			lua_pushstring(L, animatable->getName());
			return 1;
		}

		int getType(lua_State* L){
			auto animatable = lua_Animatable.checkDerivedArgument(L, 1);
			AnimatableType type = animatable->getType();
			lua_AnimatableType.push(L, type);
			return 1;
		}
	
		int pushAnimationValue(lua_State* L, std::shared_ptr<AnimationValue> value){
			switch(value->getType()){
				case AnimatableType::BOOLEAN:
					lua_pushboolean(L, value->toBoolean()->value);
					return 1;
				case AnimatableType::STATE:
					lua_AnimatableStateStruct.push(L, value->toState()->value);
					return 1;
				case AnimatableType::POSITION:{
					auto positionValue = value->toPosition();
					LuaTable positionValueTable(L);
					positionValueTable.begin(0, 3)
					.addNumber(positionValue->position, "Position")
					.addNumber(positionValue->velocity, "Velocity")
					.addNumber(positionValue->acceleration, "Acceleration");
					return 1;
					}
				default: return 0;
			}
		}
	
		int getActualValue(lua_State* L){
			auto animatable = lua_Animatable.checkDerivedArgument(L, 1);
			return pushAnimationValue(L, animatable->getActualValue());
		}
	
		int getAnimationValue(lua_State* L){
			auto animatable = lua_Animatable.checkDerivedArgument(L, 1);
			return pushAnimationValue(L, animatable->getAnimationValue());
		}
		
	};




	//———————————— Animatable Boolean ———————————

	namespace Lua_AnimatableBoolean{
		//create, enable, disable, modify parameter constraints
	};


	//———————————— Animatable State ————————————

	namespace Lua_AnimatableState{

		int getStates(lua_State* L){
			auto animatableState = lua_AnimatableState.checkArgument(L, 1);
			auto& states = animatableState->getStates();
			LuaTable statesTable(L);
			statesTable.begin(0, states.size());
			for(auto& state : states){
				AnimatableStateStruct* stateStructPointer = &state;
				statesTable.addCustom(&state, lua_AnimatableStateStruct.push, state.saveName);
			}
			return 1;
		}
	
		int state_toString(lua_State* L){
			auto animatableStateStruct = lua_AnimatableStateStruct.checkArgument(L, 1);
			lua_pushstring(L, animatableStateStruct->displayName);
			return 1;
		}

		int state_toInteger(lua_State* L){
			auto animatableStateStruct = lua_AnimatableStateStruct.checkArgument(L, 1);
			lua_pushinteger(L, animatableStateStruct->integerEquivalent);
			return 1;
		}
	
	};



	//—————————— Animatable Position ——————————


	namespace Lua_AnimatablePosition{
		//create, enable, disable, modify parameter constraints
	};
	

	//————————————————— LIBRARY ——————————————————
	
	void openlib(lua_State* L){
		
		lua_AnimatableType.declare(L);
		
		lua_Machine.addMethod("getName", Lua_Machine::getName);
		lua_Machine.addMethod("hasAnimatable", Lua_Machine::hasAnimatable);
		lua_Machine.addMethod("getAnimatable", Lua_Machine::getAnimatable);
		lua_Machine.addMethod("getAnimatables", Lua_Machine::getAnimatables);
		lua_Machine.setToStringMethod(Lua_Machine::toString);
		lua_Machine.declare(L);
		
		lua_Animatable.addMethod("getName", Lua_Animatable::getName);
		lua_Animatable.addMethod("getType", Lua_Animatable::getType);
		lua_Animatable.addMethod("getActualValue", Lua_Animatable::getActualValue);
		lua_Animatable.addMethod("getAnimationValue", Lua_Animatable::getAnimationValue);
		lua_Animatable.declare(L);
		
		lua_AnimatableBoolean.inherit(lua_Animatable);
		lua_AnimatableBoolean.declare(L);
		
		lua_AnimatableState.addMethod("getStates", Lua_AnimatableState::getStates);
		lua_AnimatableState.inherit(lua_Animatable);
		lua_AnimatableState.declare(L);
		lua_AnimatableStateStruct.addMethod("toInteger", Lua_AnimatableState::state_toInteger);
		lua_AnimatableStateStruct.addMethod("toString", Lua_AnimatableState::state_toString);
		lua_AnimatableStateStruct.declare(L);
		
		lua_AnimatablePosition.inherit(lua_Animatable);
		lua_AnimatablePosition.declare(L);
		 
		LuaTable environnementLibrary(L);
		environnementLibrary.begin()
		.addFunction(Lua_Environnement::getMachineCount, "getMachineCount")
		.addFunction(Lua_Environnement::hasMachine, "hasMachine")
		.addFunction(Lua_Environnement::getMachine, "getMachine")
		.addFunction(Lua_Environnement::getMachines, "getMachines")
		.setGlobal("Environnement");
		
	}

};
