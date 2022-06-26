#include <pch.h>
#include "EnvironnementLibrary.h"

#include "LuaLibrary.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

#include "Animation/Animatables/AnimatableBoolean.h"
#include "Animation/Animatables/AnimatableState.h"
#include "Animation/Animatables/AnimatablePosition.h"

#include <iostream>


//luaL_argcheck(L, index >= 1 && index <= getThingCount(), -1, "getThing() array index out of range");


namespace Scripting::EnvironnementLibrary{

	//————————————————— TYPES ——————————————————

	LuaEnumerator(AnimatableType);
	LuaSharedPointer(Machine);
	LuaSharedPointer(Animatable);
	LuaSharedPointer(AnimationValue);
	
	LuaSharedPointer(AnimatableBoolean);

	LuaSharedPointer(AnimatableState);
	LuaPointer(AnimatableStateStruct);

	LuaSharedPointer(AnimatablePosition);

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
					LuaSharedPointer_Machine::push(L, machine);
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
				machinesTable.addCustom(machine, LuaSharedPointer_Machine::push);
			}
			return 1;
		}

	}



	//————————————————— MACHINE ——————————————————

	namespace Lua_Machine{

		int getName(lua_State* L){
			auto machine = LuaSharedPointer_Machine::checkArgument(L, 1);
			lua_pushstring(L, machine->getName());
			return 1;
		}

		int hasAnimatable(lua_State* L){
			auto machine = LuaSharedPointer_Machine::checkArgument(L, 1);
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
			auto machine = LuaSharedPointer_Machine::checkArgument(L, 1);
			const char* animatableName = luaL_checkstring(L, 2);
			for(auto& animatable : machine->animatables){
				if(strcmp(animatable->getName(), animatableName) == 0){
					switch(animatable->getType()){
						case AnimatableType::BOOLEAN:
							LuaSharedPointer_AnimatableBoolean::push(L, animatable->toBoolean()); return 1;
						case AnimatableType::STATE:
							LuaSharedPointer_AnimatableState::push(L, animatable->toState()); return 1;
						case AnimatableType::POSITION:
							LuaSharedPointer_AnimatablePosition::push(L, animatable->toPosition()); return 1;
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
			auto machine = LuaSharedPointer_Machine::checkArgument(L, 1);
			auto& animatables = machine->animatables;
			lua_createtable(L, animatables.size(), 1);
			for(int i = 0; i < animatables.size(); i++){
				lua_pushinteger(L, i);
				LuaSharedPointer_Animatable::push(L, animatables[i]);
				lua_settable(L, -3);
			}
			return 1;
		}
	
		int toString(lua_State* L){
			auto machine = LuaSharedPointer_Machine::checkArgument(L, 1);
			lua_pushstring(L, machine->getName());
			return 1;
		}

	}



	//————————————————— ANIMATABLE ——————————————————

	namespace Lua_Animatable{

	
	
		std::shared_ptr<Animatable> checkBaseClass(lua_State* L, int index, const char* baseClassName){
			//get the index table of the metatable of the object at the index
			if(!luaL_getmetafield(L, index, "__index")) luaL_error(L, "Object has no __index metamethod");
			//get the field "BaseClasses" which should contain a table with a string for each base class of the object
			if(!lua_getfield(L, -1, "BaseClasses")) luaL_error(L, "Object has no BaseClasses Field");
			if(lua_type(L, -1) != LUA_TTABLE) luaL_error(L, "BaseClasses is not a table");
			size_t baseClassCount = lua_rawlen(L, -1);
			//iterate through all the fields of the table containing strings of baseclass names
			for(int i = 1; i <= baseClassCount; i++){
				//push the value of table index i
				lua_rawgeti(L, -1, i);
				if(lua_type(L, -1) == LUA_TSTRING){
					const char* className = luaL_checkstring(L, -1);
					if(strcmp(className, baseClassName) == 0) {
						lua_pop(L, 3);
						void* userData = lua_touserdata(L, index);
						return *static_cast<std::shared_ptr<Animatable>*>(userData);
					}
				}else luaL_error(L, "BaseClasses field is not a string");
				//pop the value, so the table is on top again
				lua_pop(L, 1);
			}
			lua_pop(L, 2);
			luaL_error(L, "object is not has no base class of time %s", baseClassName);
			return nullptr;
		}
	
		
	
		int getName(lua_State* L){
			auto animatable = checkBaseClass(L, 1, "Animatable");
			lua_pushstring(L, animatable->getName());
			return 1;
		}

		int getType(lua_State* L){
			auto animatable = checkBaseClass(L, 1, "Animatable");
			AnimatableType type = animatable->getType();
			LuaEnumerator_AnimatableType::push(L, type);
			return 1;
		}
	
		int getActualValue(lua_State* L){
			auto animatable = checkBaseClass(L, 1, "Animatable");
			auto value = animatable->getActualValue();
			LuaSharedPointer_AnimationValue::push(L, value);
			return 1;
		}
	
		int getAnimationValue(lua_State* L){
			auto animatable = checkBaseClass(L, 1, "Animatable");
			auto value = animatable->getAnimationValue();
			LuaSharedPointer_AnimationValue::push(L, value);
			return 1;
		}
		
	};




	//———————————— Animatable Boolean ———————————

	namespace Lua_AnimatableBoolean{
	
	};


	//———————————— Animatable State ————————————

	namespace Lua_AnimatableState{

		int getStates(lua_State* L){
			auto animatableState = LuaSharedPointer_AnimatableState::checkArgument(L, 1);
			auto& states = animatableState->getStates();
			LuaTable statesTable(L);
			statesTable.begin(0, states.size());
			for(auto& state : states){
				AnimatableStateStruct* stateStructPointer = &state;
				statesTable.addCustom(&state, LuaPointer_AnimatableStateStruct::push, state.saveName);
			}
			return 1;
		}
	
	};

	namespace Lua_AnimatableStateStruct{

		int toInteger(lua_State* L){
			auto animatableStateStruct = LuaPointer_AnimatableStateStruct::checkArgument(L, 1);
			lua_pushinteger(L, animatableStateStruct->integerEquivalent);
			return 1;
		}
	
	}



	//—————————— Animatable Position ——————————


	namespace Lua_AnimatablePosition{
	
	};





	//———————————— Animatation Value —————————————

	namespace Lua_AnimationValue{
	
		int getType(lua_State* L){
			auto animationValue = LuaSharedPointer_AnimationValue::checkArgument(L, 1);
			AnimatableType type = animationValue->getType();
			LuaEnumerator_AnimatableType::push(L, type);
			return 1;
		}
	
		int toBoolean(lua_State* L){
			auto animationValue = LuaSharedPointer_AnimationValue::checkArgument(L, 1);
			luaL_argcheck(L, animationValue->getType() == AnimatableType::BOOLEAN, 1, "Animation Value type is not Boolean");
			lua_pushboolean(L, animationValue->toBoolean()->value);
			return 1;
		}
	
		int toState(lua_State* L){
			auto animationValue = LuaSharedPointer_AnimationValue::checkArgument(L, 1);
			luaL_argcheck(L, animationValue->getType() == AnimatableType::STATE, 1, "Animation Value type is not State");
			auto stateValue = animationValue->toState();
			return 1;
		}
	
		int toPosition(lua_State* L){
			auto animationValue = LuaSharedPointer_AnimationValue::checkArgument(L, 1);
			luaL_argcheck(L, animationValue->getType() == AnimatableType::POSITION, 1, "Animation Value type is not Position");
			auto positionValue = animationValue->toPosition();
			LuaTable positionValueTable(L);
			positionValueTable.begin(0, 3)
			.addNumber(positionValue->position, "Position")
			.addNumber(positionValue->velocity, "Velocity")
			.addNumber(positionValue->acceleration, "Acceleration");
			return 1;
		}
	
	};
	

	//————————————————— LIBRARY ——————————————————
	
	void openlib(lua_State* L){
		
		LuaEnumerator_AnimatableType::declare(L);
		
		LuaSharedPointer_Machine::addMethod("getName", Lua_Machine::getName);
		LuaSharedPointer_Machine::addMethod("hasAnimatable", Lua_Machine::hasAnimatable);
		LuaSharedPointer_Machine::addMethod("getAnimatable", Lua_Machine::getAnimatable);
		LuaSharedPointer_Machine::addMethod("getAnimatables", Lua_Machine::getAnimatables);
		LuaSharedPointer_Machine::setToStringMethod(Lua_Machine::toString);
		LuaSharedPointer_Machine::declare(L);
		
		LuaSharedPointer_Animatable::addMethod("getName", Lua_Animatable::getName);
		LuaSharedPointer_Animatable::addMethod("getType", Lua_Animatable::getType);
		LuaSharedPointer_Animatable::addMethod("getActualValue", Lua_Animatable::getActualValue);
		LuaSharedPointer_Animatable::addMethod("getAnimationValue", Lua_Animatable::getAnimationValue);
		LuaSharedPointer_Animatable::declare(L);
		
		LuaSharedPointer_AnimatableBoolean::inheritMethods(LuaSharedPointer_Animatable::getMethods());
		LuaSharedPointer_AnimatableBoolean::declare(L);
		
		LuaSharedPointer_AnimatableState::addMethod("getStates", Lua_AnimatableState::getStates);
		LuaSharedPointer_AnimatableState::inheritMethods(LuaSharedPointer_Animatable::getMethods());
		LuaSharedPointer_AnimatableState::declare(L);
		
		LuaPointer_AnimatableStateStruct::addMethod("toInteger", Lua_AnimatableStateStruct::toInteger);
		LuaPointer_AnimatableStateStruct::declare(L);
		
		LuaSharedPointer_AnimatablePosition::inheritMethods(LuaSharedPointer_Animatable::getMethods());
		LuaSharedPointer_AnimatablePosition::declare(L);
		
		LuaSharedPointer_AnimationValue::addMethod("getType", Lua_AnimationValue::getType);
		LuaSharedPointer_AnimationValue::addMethod("toPosition", Lua_AnimationValue::toPosition);
		LuaSharedPointer_AnimationValue::declare(L);
		 
		LuaTable environnementLibrary(L);
		environnementLibrary.begin()
		.addFunction(Lua_Environnement::getMachineCount, "getMachineCount")
		.addFunction(Lua_Environnement::hasMachine, "hasMachine")
		.addFunction(Lua_Environnement::getMachine, "getMachine")
		.addFunction(Lua_Environnement::getMachines, "getMachines")
		.setGlobal("Environnement");
		
	}

};









