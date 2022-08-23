#include <pch.h>
#include "EnvironnementLibrary.h"

#include "LuaLibraryHelper.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

#include "Animation/Animatables/AnimatableState.h"
#include "Animation/Animatables/AnimatablePosition.h"

namespace Scripting::EnvironnementLibrary{

	//————————————————— TYPES ——————————————————

	LuaEnumerator<AnimatableType, "AnimatableType"> lua_AnimatableType;

	LuaSharedPointer<Machine, "Machine"> lua_Machine;
	LuaSharedPointer<Animatable, "Animatable"> lua_Animatable;
	LuaSharedPointer<AnimationConstraint, "AnimationConstraint"> lua_AnimationConstraint;

	LuaSharedPointer<AnimatableState, "AnimatableState"> lua_AnimatableState;
	LuaPointer<AnimatableStateStruct, "AnimatableStateStruct"> lua_AnimatableStateStruct;

	LuaSharedPointer<AnimatablePosition, "AnimatablePosition"> lua_AnimatablePosition;
	LuaSharedPointer<AnimatablePosition_KeepoutConstraint, "AnimatablePositionKeepoutConstraint"> lua_AnimatablePosition_KeepoutConstraint;

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
	
		//indexed table of all machines
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
			for(auto& animatable : machine->getAnimatables()){
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
			for(auto& animatable : machine->getAnimatables()){
				if(strcmp(animatable->getName(), animatableName) == 0){
					switch(animatable->getType()){
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
	
		//indexed table of all animatables
		int getAnimatables(lua_State* L){
			auto machine = lua_Machine.checkArgument(L, 1);
			auto& animatables = machine->getAnimatables();
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
	
		int createHaltConstraint(lua_State* L){
			auto animatable = lua_Animatable.checkDerivedArgument(L, 1);
			const char* constraintName = luaL_checkstring(L, 2);
			auto constraint = std::make_shared<HaltConstraint>(constraintName);
			animatable->addConstraint(constraint);
			lua_AnimationConstraint.push(L, constraint);
			return 1;
		}
	
		int isOnline(lua_State* L){
			auto animatable = lua_Animatable.checkDerivedArgument(L, 1);
			lua_pushboolean(L, animatable->state != Animatable::State::OFFLINE);
			return 1;
		}
	
	};



	//———————————————— Constraint Base ———————————————

	namespace Lua_AnimationConstraint{

		int enable(lua_State* L){
			auto constraint = lua_AnimationConstraint.checkDerivedArgument(L, 1);
			constraint->enable();
			return 0;
		}

		int disable(lua_State* L){
			auto constraint = lua_AnimationConstraint.checkDerivedArgument(L, 1);
			constraint->disable();
			return 0;
		}
	
		int setEnabled(lua_State* L){
			auto constraint = lua_AnimationConstraint.checkDerivedArgument(L, 1);
			bool enabled = lua_toboolean(L, 2);
			if(enabled) constraint->enable();
			else constraint->disable();
		}

		int isEnabled(lua_State* L){
			auto constraint = lua_AnimationConstraint.checkDerivedArgument(L, 1);
			lua_pushboolean(L, constraint->isEnabled());
			return 1;
		}

		int getName(lua_State* L){
			auto constraint = lua_AnimationConstraint.checkDerivedArgument(L, 1);
			lua_pushstring(L, constraint->getName().c_str());
			return 1;
		}

		int isHaltConstraint(lua_State* L){
			auto constraint = lua_AnimationConstraint.checkDerivedArgument(L, 1);
			lua_pushboolean(L, constraint->getType() == AnimationConstraint::Type::HALT);
			return 1;
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
			auto states = animatableState->getStates();
			LuaTable statesTable(L);
			statesTable.begin(0, states->size());
			for(auto& state : *states){
				statesTable.addCustom(state, lua_AnimatableStateStruct.push, state->saveName);
			}
			return 1;
		}
	
	};

	namespace Lua_AnimatableStateStruct{

		int toString(lua_State* L){
			auto animatableStateStruct = lua_AnimatableStateStruct.checkArgument(L, 1);
			lua_pushstring(L, animatableStateStruct->displayName);
			return 1;
		}

		int toInteger(lua_State* L){
			auto animatableStateStruct = lua_AnimatableStateStruct.checkArgument(L, 1);
			lua_pushinteger(L, animatableStateStruct->integerEquivalent);
			return 1;
		}

	};







	//—————————— Animatable Position ——————————


	namespace Lua_AnimatablePosition{
		//create, enable, disable, modify parameter constraints
	
		int createKeepoutConstraint(lua_State* L){
			auto animatablePosition = lua_AnimatablePosition.checkArgument(L, 1);
			const char* constraintName = luaL_checkstring(L, 2);
			double min = luaL_checknumber(L, 3);
			double max = luaL_checknumber(L, 4);
			auto constraint = std::make_shared<AnimatablePosition_KeepoutConstraint>(constraintName, min, max);
			animatablePosition->addConstraint(constraint);
			lua_AnimatablePosition_KeepoutConstraint.push(L, constraint);
			return 1;
		};
	
		int getConstraints(lua_State* L){
			auto animatablePosition = lua_AnimatablePosition.checkDerivedArgument(L, 1);
			auto& constraints = animatablePosition->getConstraints();
			int constraintCount = constraints.size();
			LuaTable constraintsTable(L);
			constraintsTable.begin(constraintCount);
			for(auto& constraint : constraints){
				auto keepoutConstraint = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
				constraintsTable.addCustom(keepoutConstraint, lua_AnimatablePosition_KeepoutConstraint.push);
			}
			return 1;
		}
	
		int getBrakingPosition(lua_State* L){
			auto animatablePosition = lua_AnimatablePosition.checkArgument(L, 1);
			double brakingPosition = animatablePosition->getBrakingPosition();
			lua_pushnumber(L, brakingPosition);
			return 1;
		}
	
	};
	
	namespace Lua_AnimatablePosition_KeepoutConstraint{

		int adjust(lua_State* L){
			auto constraint = lua_AnimatablePosition_KeepoutConstraint.checkArgument(L, 1);
			double min = luaL_checknumber(L, 2);
			double max = luaL_checknumber(L, 3);
			constraint->adjust(min, max);
			return 0;
		}
	
		int getMin(lua_State* L){
			auto constraint = lua_AnimatablePosition_KeepoutConstraint.checkArgument(L, 1);
			lua_pushnumber(L, constraint->keepOutMinPosition);
			return 1;
		}
		
		int getMax(lua_State* L){
			auto constraint = lua_AnimatablePosition_KeepoutConstraint.checkArgument(L, 1);
			lua_pushnumber(L, constraint->keepOutMaxPosition);
			return 1;
		}
	
	};


	int checkRangeOverlap(lua_State* L){
		//get all 4 range arguments
		double range1a = luaL_checknumber(L, 1);
		double range1b = luaL_checknumber(L, 2);
		double range2a = luaL_checknumber(L, 3);
		double range2b = luaL_checknumber(L, 4);
		//make sure the ranges are well formed (min < max)
		double start1 = std::min(range1a, range1b);
		double end1 = std::max(range1a, range1b);
		double start2 = std::min(range2a, range2b);
		double end2 = std::max(range2a, range2b);
		bool overlapping = start1 <= end2 && start2 <= end1;
		lua_pushboolean(L, overlapping);
		return 1;
	}


	//————————————————— LIBRARY ——————————————————
	
	void openlib(lua_State* L, bool includeMotionFunctions){
		
		//——— AnimatableType Enumerator
		lua_AnimatableType.declare(L);
		
		
		//——— Machines
		lua_Machine.addMethod("getName", Lua_Machine::getName);
		lua_Machine.addMethod("hasAnimatable", Lua_Machine::hasAnimatable);
		lua_Machine.addMethod("getAnimatable", Lua_Machine::getAnimatable);
		lua_Machine.addMethod("getAnimatables", Lua_Machine::getAnimatables);
		lua_Machine.setToStringMethod(Lua_Machine::toString);
		lua_Machine.declare(L);
		
		//——— Animatables
		lua_Animatable.addMethod("getName", Lua_Animatable::getName);
		lua_Animatable.addMethod("getType", Lua_Animatable::getType);
		lua_Animatable.addMethod("getActualValue", Lua_Animatable::getActualValue);
		lua_Animatable.addMethod("getAnimationValue", Lua_Animatable::getAnimationValue);
		lua_Animatable.addMethod("createHaltConstraint", Lua_Animatable::createHaltConstraint);
		lua_Animatable.addMethod("isOnline", Lua_Animatable::isOnline);
		lua_Animatable.declare(L);
		
		//——— Constraints 
		lua_AnimationConstraint.addMethod("enable", Lua_AnimationConstraint::enable);
		lua_AnimationConstraint.addMethod("disable", Lua_AnimationConstraint::disable);
		lua_AnimationConstraint.addMethod("setEnabled", Lua_AnimationConstraint::setEnabled);
		lua_AnimationConstraint.addMethod("getName", Lua_AnimationConstraint::getName);
		lua_AnimationConstraint.declare(L);
		
		
		
		//——— Animatable State —————————————————————————————————————————————————————————————————————————
		lua_AnimatableState.inherit(lua_Animatable);
		lua_AnimatableState.addMethod("getStates", Lua_AnimatableState::getStates);
		lua_AnimatableState.declare(L);
		
		//——— Animatable State Struct
		lua_AnimatableStateStruct.addMethod("toInteger", Lua_AnimatableStateStruct::toInteger);
		lua_AnimatableStateStruct.addMethod("toString", Lua_AnimatableStateStruct::toString);
		lua_AnimatableStateStruct.declare(L);
		
		
		//——— Animatable Position ——————————————————————————————————————————————————————————————————————
		lua_AnimatablePosition.inherit(lua_Animatable);
		if(includeMotionFunctions){
			lua_AnimatablePosition.addMethod("createKeepoutConstraint", Lua_AnimatablePosition::createKeepoutConstraint);
		}
		lua_AnimatablePosition.addMethod("getConstraints", Lua_AnimatablePosition::getConstraints);
		lua_AnimatablePosition.addMethod("getBrakingPosition", Lua_AnimatablePosition::getBrakingPosition);
		lua_AnimatablePosition.declare(L);
		
		//——— Animatable Position Keepout Constraint
		lua_AnimatablePosition_KeepoutConstraint.inherit(lua_AnimationConstraint);
		if(includeMotionFunctions){
			lua_AnimatablePosition_KeepoutConstraint.addMethod("adjust", Lua_AnimatablePosition_KeepoutConstraint::adjust);
		}
		lua_AnimatablePosition_KeepoutConstraint.addMethod("getMin", Lua_AnimatablePosition_KeepoutConstraint::getMin);
		lua_AnimatablePosition_KeepoutConstraint.addMethod("getMax", Lua_AnimatablePosition_KeepoutConstraint::getMax);
		lua_AnimatablePosition_KeepoutConstraint.declare(L);
		
		//——— Global Functions ——————————
		
		lua_pushcfunction(L, checkRangeOverlap);
		lua_setglobal(L, "checkRangeOverlap");
		
		//——— Environnement Library ——————————————————————————————————————————————————————————————————————
		
		LuaTable environnementLibrary(L);
		environnementLibrary.begin()
		.addFunction(Lua_Environnement::getMachineCount, "getMachineCount")
		.addFunction(Lua_Environnement::hasMachine, "hasMachine")
		.addFunction(Lua_Environnement::getMachine, "getMachine")
		.addFunction(Lua_Environnement::getMachines, "getMachines")
		.setGlobal("Environnement");
		
	}

};
