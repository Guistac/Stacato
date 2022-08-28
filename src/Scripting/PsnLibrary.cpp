#include <pch.h>
#include "PsnLibrary.h"



namespace Scripting::PsnLibrary{

	//————————————————— TYPES ——————————————————

/*
	LuaEnumerator<AnimatableType, "AnimatableType"> lua_AnimatableType;

	LuaSharedPointer<Machine, "Machine"> lua_Machine;
	LuaSharedPointer<Animatable, "Animatable"> lua_Animatable;
	LuaSharedPointer<AnimationConstraint, "AnimationConstraint"> lua_AnimationConstraint;

	LuaSharedPointer<AnimatableState, "AnimatableState"> lua_AnimatableState;
	LuaPointer<AnimatableStateStruct, "AnimatableStateStruct"> lua_AnimatableStateStruct;

	LuaSharedPointer<AnimatablePosition, "AnimatablePosition"> lua_AnimatablePosition;
	LuaSharedPointer<AnimatablePosition_KeepoutConstraint, "AnimatablePositionKeepoutConstraint"> lua_AnimatablePosition_KeepoutConstraint;
*/

	//————————————————— LIBRARY ——————————————————
	
	void openlib(lua_State* L){
		
		/*
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
		*/
	}

};
