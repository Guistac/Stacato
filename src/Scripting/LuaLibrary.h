#pragma once

#include <ofxLua.h>

typedef int (*LuaCFunction)(lua_State*);



struct LuaFunction{
	std::string name;
	LuaCFunction function;
};



class LuaLibrary{
public:
		
	std::vector<LuaFunction> functions;
	
	void addFunction(std::string functionName, LuaCFunction function){
		functions.push_back(LuaFunction{.name = functionName, .function = function});
	}
	
	void declare(lua_State* L, std::string libraryName){
		//declare library table
		lua_newtable(L);
		//declare library functions
		for(auto& function : functions){
			lua_pushcfunction(L, function.function);
			lua_setfield(L, -2, function.name.c_str());
		}
		//set library name
		lua_setglobal(L, libraryName.c_str());
	}
	
};


template<typename T>
class LuaObject{
public:
	
	LuaObject(std::string typeName_) : typeName(typeName_){}
	std::string typeName;
	
	std::vector<LuaFunction> methods;
	
	void addMethod(std::string methodName, LuaCFunction luaFunction){
		methods.push_back(LuaFunction{.name = methodName, .function = luaFunction});
	}
	
	LuaCFunction toString = nullptr;
	void setToStringMethod(LuaCFunction luaFunction){ toString = luaFunction; }
	
	void push(lua_State* L, T* objectPointer){
		T** userData = (T**)lua_newuserdata(L, sizeof(T*));
		*userData = objectPointer;
		luaL_getmetatable(L, typeName.c_str());
		lua_setmetatable(L, -2);
	}
	
	T* checkArgument(lua_State* L, int argumentIndex){
		T** userData = (T**)luaL_checkudata(L, argumentIndex, typeName.c_str());
		return *userData;
	}
	
	void declare(lua_State* L){
		//new MetaTable for ObjectType
		luaL_newmetatable(L, typeName.c_str());
		
		//index table for ObjectType Metatable (holds member functions as indeces)
		lua_newtable(L);
		//declare all object member functions as entries in the index table of the object metatable
		for(auto& method : methods){
			lua_pushcfunction(L, method.function);
			lua_setfield(L, -2, method.name.c_str());
		}
		//set index table key
		lua_setfield(L, -2, "__index");
		
		//if a tostring method was declared, set tostring function key
		if(toString){
			lua_pushcfunction(L, toString);
			lua_setfield(L, -2, "__tostring");
		}
		
		//pop the ObjectType MetaTable off the stack
		lua_pop(L, 1);
	}
	
	
};



/*
template<typename T>
class LuaEnumerator{
public:
	
	LuaEnumerator(std::string typeName_) : typeName(typeName_){}
	std::string typeName;
	
	
	void push(lua_State* L, T enumerator){
		Enumerator::TypeStruct<T>** userData = (Enumerator::TypeStruct<T>**)lua_newuserdata(L, sizeof(Enumerator::TypeStruct<T>*));
		*userData = Enumerator::getTypeStructure(enumerator);
		luaL_getmetatable(L, typeName.c_str());
		lua_setmetatable(L, -2);
	}
	
	T checkArgument(lua_State* L, int argumentIndex){
		Enumerator::TypeStruct<T>** userData = (Enumerator::TypeStruct<T>**)luaL_checkudata(L, argumentIndex, typeName.c_str());
		auto typeStruct = *userData;
		return typeStruct->enumerator;
	}
	
	static int getString(lua_State* L){
		T enumerator = checkArgument(L, 1);
		lua_pushstring(L, Enumerator::getSaveString(T));
		return 1;
	}
	
	static int getInteger(lua_State* L){
		T enumerator = checkArgument(L, 1);
		lua_pushinteger(L, (int)enumerator);
		return 1;
	}

	void declare(lua_State* L){
		
		//new MetaTable for ObjectType
		luaL_newmetatable(L, typeName.c_str());
		
		
		
		//pop the ObjectType MetaTable off the stack
		lua_pop(L, 1);
		
		auto& types = Enumerator::getTypes<T>();
		lua_createtable(L, types.size(), 1);
		for(auto& type : types){
			lua_pushstring(L, type.saveString); //key
			push(L, type.enumerator); //value
			lua_settable(L, -3);
		}
		lua_setglobal(L, typeName.c_str());
	}
	
};
 */



#define LuaEnumerator(EnumeratorType, TypeName)																							\
namespace LuaEnumerator_##EnumeratorType{																								\
																																		\
	inline void push(lua_State* L, EnumeratorType enumerator){																			\
		auto userData = (Enumerator::TypeStruct<EnumeratorType>**)lua_newuserdata(L, sizeof(Enumerator::TypeStruct<EnumeratorType>*));	\
		*userData = Enumerator::getTypeStructure(enumerator);																			\
		luaL_getmetatable(L, TypeName);																									\
		lua_setmetatable(L, -2);																										\
	}																																	\
																																		\
	inline EnumeratorType checkArgument(lua_State* L, int argumentIndex){																\
		auto userData = (Enumerator::TypeStruct<EnumeratorType>**)luaL_checkudata(L, argumentIndex, TypeName);							\
		auto typeStruct = *userData;																									\
		return typeStruct->enumerator;																									\
	}																																	\
																																		\
	inline int getString(lua_State* L){																									\
		EnumeratorType enumerator = checkArgument(L, 1);																				\
		lua_pushstring(L, Enumerator::getSaveString(enumerator));																		\
		return 1;																														\
	}																																	\
																																		\
	inline int getInteger(lua_State* L){																								\
		EnumeratorType enumerator = checkArgument(L, 1);																				\
		lua_pushinteger(L, (int)enumerator);																							\
		return 1;																														\
	}																																	\
																																		\
	inline int equals(lua_State* L){																									\
		lua_pushboolean(L, checkArgument(L, 1) == checkArgument(L, 2));																	\
		return 1;																														\
	}																																	\
																																		\
	inline void declare(lua_State* L){																									\
		luaL_newmetatable(L, TypeName); /*new MetaTable for ObjectType*/																\
		lua_newtable(L); /*index table for ObjectType Metatable (holds member functions as indeces)*/									\
		lua_pushcfunction(L, getString);																								\
		lua_setfield(L, -2, "getString");																								\
		lua_pushcfunction(L, getInteger);																								\
		lua_setfield(L, -2, "getInteger");																								\
		lua_setfield(L, -2, "__index"); /*set index table key*/																			\
		lua_pushcfunction(L, equals);																									\
		lua_setfield(L, -2, "__eq");																									\
		lua_pop(L, 1); /*pop the ObjectType MetaTable off the stack*/																	\
		auto& types = Enumerator::getTypes<EnumeratorType>(); /*create a table containing all enumerators*/								\
		lua_createtable(L, types.size(), 1);																							\
		for(auto& type : types){																										\
			lua_pushstring(L, type.saveString); /*key*/																					\
			push(L, type.enumerator); /*value*/																							\
			lua_settable(L, -3);																										\
		}																																\
		lua_setglobal(L, TypeName);																										\
	}																																	\
																																		\
};																																		\
