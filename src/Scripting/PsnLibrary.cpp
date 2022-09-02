#include <pch.h>
#include "PsnLibrary.h"
#include "LuaLibraryHelper.h"

#include "Networking/Psn/PsnServer.h"



namespace Scripting::PsnLibrary{

	//————————————————— TYPES ——————————————————

	LuaSharedPointer<PsnServer, "PsnServer"> lua_PsnServer;
	LuaSharedPointer<PsnTracker, "PsnTracker"> lua_PsnTracker;


	int luaPsnServer_createNewTracker(lua_State* L){
		const char* name = luaL_checkstring(L, -1);
		lua_getglobal(L, "PsnServer");
		auto psnServer = lua_PsnServer.checkArgument(L, -1);
		if(psnServer == nullptr) lua_pushnil(L);
		else {
			auto tracker = psnServer->createNewTracker(name);
			lua_PsnTracker.push(L, tracker);
		}
		return 1;
	}


	int luaTracker_setPosition(lua_State* L){
		auto tracker = lua_PsnTracker.checkArgument(L, 1);
		double x = luaL_checknumber(L, 2);
		double y = luaL_checknumber(L, 3);
		double z = luaL_checknumber(L, 4);
		tracker->setPosition(glm::vec3(x,y,z));
		return 0;
	}

	int luaTracker_setVelocity(lua_State* L){
		auto tracker = lua_PsnTracker.checkArgument(L, 1);
		double x = luaL_checknumber(L, 2);
		double y = luaL_checknumber(L, 3);
		double z = luaL_checknumber(L, 4);
		tracker->setVelocity(glm::vec3(x,y,z));
		return 0;
	}

	int luaTracker_setAcceleration(lua_State* L){
		auto tracker = lua_PsnTracker.checkArgument(L, 1);
		double x = luaL_checknumber(L, 2);
		double y = luaL_checknumber(L, 3);
		double z = luaL_checknumber(L, 4);
		tracker->setAcceleration(glm::vec3(x,y,z));
		return 0;
	}

	int luaTracker_setTarget(lua_State* L){
		auto tracker = lua_PsnTracker.checkArgument(L, 1);
		double x = luaL_checknumber(L, 2);
		double y = luaL_checknumber(L, 3);
		double z = luaL_checknumber(L, 4);
		tracker->setTarget(glm::vec3(x,y,z));
		return 0;
	}

	int luaTracker_setOrigin(lua_State* L){
		auto tracker = lua_PsnTracker.checkArgument(L, 1);
		double x = luaL_checknumber(L, 2);
		double y = luaL_checknumber(L, 3);
		double z = luaL_checknumber(L, 4);
		tracker->setOrigin(glm::vec3(x,y,z));
		return 0;
	}

	int luaTracker_setStatus(lua_State* L){
		auto tracker = lua_PsnTracker.checkArgument(L, 1);
		double status = luaL_checknumber(L, 2);
		tracker->setStatus(status);
		return 0;
	}



	


	//————————————————— LIBRARY ——————————————————
	
	void openlib(lua_State* L, std::shared_ptr<PsnServer> server){
		
		lua_PsnServer.addMethod("createNewTracker", luaPsnServer_createNewTracker);
		lua_PsnServer.declare(L);
		
		lua_PsnTracker.addMethod("setPosition", luaTracker_setPosition);
		lua_PsnTracker.addMethod("setVelocity", luaTracker_setVelocity);
		lua_PsnTracker.addMethod("setAcceleration", luaTracker_setAcceleration);
		lua_PsnTracker.addMethod("setTarget", luaTracker_setTarget);
		lua_PsnTracker.addMethod("setOrigin", luaTracker_setOrigin);
		lua_PsnTracker.addMethod("setStatus", luaTracker_setStatus);
		lua_PsnTracker.declare(L);
		
		lua_PsnServer.push(L, server);
		lua_setglobal(L, "PsnServer");
		
		
		
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
