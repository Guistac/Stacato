#include <pch.h>
#include "EnvironnementLibrary.h"

#include <iostream>

#include <ofxLua.h>

 namespace CoolContainer{

	 class CoolThing{
	 public:
		 CoolThing(const char* n){
			 strcpy(name, n);
			 nameLength = strlen(name);
		 }
		 char name[128];
		 int nameLength;
		 void doAction(){ std::cout << "Called from C: I am " << name << std::endl; }
		 int getNameLength() { return nameLength; }
	 };

 
	 std::vector<CoolThing> coolThings = {
		 CoolThing("firstThing"),
		 CoolThing("anotherThing"),
		 CoolThing("weirdThing"),
		 CoolThing("lastThing")
	 };
	 
	 int getThingCount(){
		 return coolThings.size();
	 }
	 
	 CoolThing* getThing(int index){
		 return &coolThings[index];
	 }
	 
	 CoolThing* getThingByName(const char* n){
		 for(int i = 0; i < getThingCount(); i++) if(strcmp(n, coolThings[i].name) == 0) return &coolThings[i];
		 return nullptr;
	 }

 
 
	 namespace Lua{

		 int getCount(lua_State* L){
			 lua_pushinteger(L, CoolContainer::getThingCount());
			 return 1;
		 }

		 int get(lua_State* L){
			 int index = luaL_checkinteger(L, -1);
			 luaL_argcheck(L, index >= 1 && index <= getThingCount(), -1, "getThing() array index out of range");
			 
			 CoolThing** userData = (CoolThing**)lua_newuserdata(L, sizeof(CoolThing*));
			 *userData = getThing(index-1);
			 luaL_getmetatable(L, "CoolThing.Type");
			 lua_setmetatable(L, -2);
			 
			 return 1;
		 }
	 
		 int getByName(lua_State* L){
			 const char* name = luaL_checkstring(L, -1);
			 CoolThing* thing = getThingByName(name);
			 if(thing != nullptr) {
				 CoolThing** userData = (CoolThing**)lua_newuserdata(L, sizeof(CoolThing*));
				 *userData = thing;
				 luaL_getmetatable(L, "CoolThing.Type");
				 lua_setmetatable(L, -2);
			 }else{
				 lua_pushnil(L);
			 }
			 return 1;
		 }

		 int thing_getName(lua_State* L){
			 CoolThing** userData = (CoolThing**)luaL_checkudata(L, 1, "CoolThing.Type");
			 CoolThing* thing = *userData;
			 luaL_argcheck(L, thing != nullptr, -1, "GetThingName() thing is nullptr");
			 lua_pushstring(L, thing->name);
			 return 1;
		 }

		 int thing_doAction(lua_State* L){
			 CoolThing** userData = (CoolThing**)luaL_checkudata(L, 1, "CoolThing.Type");
			 CoolThing* thing = *userData;
			 luaL_argcheck(L, thing != nullptr, -1, "doThingAction() thing is nullptr");
			 thing->doAction();
			 return 0;
		 }
	 
		 int thing_getNameLength(lua_State* L){
			 CoolThing** userData = (CoolThing**)luaL_checkudata(L, 1, "CoolThing.Type");
			 CoolThing* thing = *userData;
			 luaL_argcheck(L, thing != nullptr, -1, "doThingAction() thing is nullptr");
			 lua_pushinteger(L, thing->getNameLength());
			 return 1;
		 }

		 int thing_toString(lua_State* L){
			 CoolThing** userData = (CoolThing**)luaL_checkudata(L, 1, "CoolThing.Type");
			 CoolThing* thing = *userData;
			 luaL_argcheck(L, thing!= nullptr, -1, "toString() thing is nullptr");
			 lua_pushfstring(L, "Thing with name %s of length %d", thing->name, thing->getNameLength());
			 return 1;
		 }

		 int openlib(lua_State* L){
			 
			 //metatable for CoolThing
			 luaL_newmetatable(L, "CoolThing.Type");
			 
			 //index table for CoolThing.Type Metatable
			 lua_newtable(L);
			 
			 lua_pushcfunction(L, thing_getName);
			 lua_setfield(L, -2, "getName");
			 lua_pushcfunction(L, thing_doAction);
			 lua_setfield(L, -2, "doAction");
			 lua_pushcfunction(L, thing_getNameLength);
			 lua_setfield(L, -2, "getNameLength");
			 
			 //set index table key
			 lua_setfield(L, -2, "__index");
		 
			 //set tostring function key
			 lua_pushcfunction(L, thing_toString);
			 lua_setfield(L, -2, "__tostring");
			 
			 //pop the CoolThing metatable off the stack
			 lua_pop(L, 1);
			 
			 //declare library table
			 lua_newtable(L);
			 
			 //declare library functions
			 lua_pushcfunction(L, getCount);
			 lua_setfield(L, -2, "getCount");
			 lua_pushcfunction(L, get);
			 lua_setfield(L, -2, "get");
			 lua_pushcfunction(L, getByName);
			 lua_setfield(L, -2, "getByName");
			 
			 //set library name
			 lua_setglobal(L, "Things");
			 
			 return 1;
		 }
	 }


 };
 
