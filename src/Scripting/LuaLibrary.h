#pragma once

#include <ofxLua.h>

typedef int (*LuaCFunction)(lua_State*);

struct LuaFunction{
	std::string name;
	LuaCFunction function;
};

class LuaTable{
public:
	
	LuaTable(lua_State* LuaState) : L(LuaState){}
	
	LuaTable& begin(int arrayEntries = 0, int nonArrayEntries = 0){
		if(arrayEntries == 0 && nonArrayEntries == 0) lua_newtable(L);
		else lua_createtable(L, arrayEntries, nonArrayEntries);
		return *this;
	}
	
	template<typename T>
	LuaTable& addShared(T sharedPointer, void (*pushSharedFunction)(lua_State*, T), const char* key = nullptr){
		setKey(key);
		pushSharedFunction(L, sharedPointer);
		lua_settable(L, -3);
		return *this;
	}
	
	LuaTable& addBoolean(bool value, const char* key = nullptr){
		setKey(key);
		lua_pushboolean(L, value);
		lua_settable(L, -3);
		return *this;
	}

	LuaTable& addInteger(lua_Integer value, const char* key = nullptr){
		setKey(key);
		lua_pushinteger(L, value);
		lua_settable(L, -3);
		return *this;
	}
	
	LuaTable& addNumber(lua_Number value, const char* key = nullptr){
		setKey(key);
		lua_pushnumber(L, value);
		lua_settable(L, -3);
		return *this;
	}
	
	LuaTable& addString(const char* value, const char* key = nullptr){
		setKey(key);
		lua_pushstring(L, value);
		lua_settable(L, -3);
		return *this;
	}
	
	LuaTable& addFunction(LuaCFunction value, const char* key = nullptr){
		setKey(key);
		lua_pushcfunction(L, value);
		lua_settable(L, -3);
		return *this;
	}
	
	void setGlobal(const char* tableName){
		lua_setglobal(L, tableName);
	}
	
private:
	lua_State* L;
	int arrayIndex = 1;
	
	void setKey(const char* key){
		if(key == nullptr) {
			lua_pushinteger(L, arrayIndex);
			arrayIndex++;
		}else lua_pushstring(L, key);
	}
};


#define LuaShared(ObjectType, TypeName)																\
namespace LuaShared_##ObjectType{																	\
																									\
	inline const char* getTypeString(){ return TypeName; }											\
																									\
	inline std::vector<LuaFunction>& getMethods(){													\
		static std::vector<LuaFunction> methods;													\
		return methods;																				\
	}																								\
	inline void addMethod(std::string methodName, LuaCFunction luaFunction){						\
		getMethods().push_back(LuaFunction{.name = methodName, .function = luaFunction});			\
	}																								\
																									\
	inline LuaCFunction& getToStringMethod(){														\
		static LuaCFunction toStringMethod;															\
		return toStringMethod;																		\
	}																								\
	inline void setToStringMethod(LuaCFunction luaFunction){ getToStringMethod() = luaFunction; }	\
																									\
	inline void push(lua_State* L, std::shared_ptr<ObjectType> object){								\
		void* userData = lua_newuserdata(L, sizeof(std::shared_ptr<ObjectType>));					\
		new(userData) std::shared_ptr<ObjectType>(object); /*new placement operator*/				\
		luaL_getmetatable(L, getTypeString());														\
		lua_setmetatable(L, -2);																	\
		return 1;																					\
	}																								\
																									\
	inline std::shared_ptr<ObjectType> checkArgument(lua_State* L, int argumentIndex){				\
		void* userData = luaL_checkudata(L, argumentIndex, getTypeString());						\
		auto pointerToSharedPointer = static_cast<std::shared_ptr<ObjectType>*>(userData);			\
		return *pointerToSharedPointer;																\
	}																								\
																									\
	inline int destroy(lua_State* L){																\
		void* userData = luaL_checkudata(L, 1, getTypeString());									\
		auto pointerToSharedPointer = static_cast<std::shared_ptr<ObjectType>*>(userData);			\
		pointerToSharedPointer->reset();															\
		return 0;																					\
	}																								\
																									\
	inline void declare(lua_State* L){																\
		luaL_newmetatable(L, getTypeString()); /*new MetaTable for UserData*/						\
																									\
		lua_newtable(L); /*index table for UserData (holds member functions)*/						\
		for(auto& method : getMethods()){															\
			lua_pushcfunction(L, method.function);													\
			lua_setfield(L, -2, method.name.c_str());												\
		}																							\
		lua_setfield(L, -2, "__index");	/*set index table key*/										\
																									\
		/*set destroy function for garbage collector metamethod*/									\
		/*to decrement the shared pointer user count when lua garbage collects the user data */		\
		lua_pushcfunction(L, destroy);																\
		lua_setfield(L, -2, "__gc");																\
																									\
		if(LuaCFunction toStringMethod = getToStringMethod()){ /*set tostring metmethod*/			\
			lua_pushcfunction(L, toStringMethod);													\
			lua_setfield(L, -2, "__tostring");														\
		}																							\
																									\
		lua_pop(L, 1); /*pop the UserData MetaTable off the stack	*/								\
	}																								\
};																									\




#define LuaEnumerator(EnumeratorType, TypeName)													\
namespace LuaEnumerator_##EnumeratorType{														\
																								\
	inline void push(lua_State* L, EnumeratorType enumerator){									\
		auto typeStructure = Enumerator::getTypeStructure(enumerator);							\
		void* userData = lua_newuserdata(L, sizeof(Enumerator::TypeStruct<EnumeratorType>*));	\
		new(userData) Enumerator::TypeStruct<EnumeratorType>*(typeStructure);					\
		luaL_getmetatable(L, TypeName);															\
		lua_setmetatable(L, -2);																\
	}																							\
																								\
	inline EnumeratorType checkArgument(lua_State* L, int argumentIndex){						\
		void* userData = luaL_checkudata(L, argumentIndex, TypeName);							\
		auto typeStruct = static_cast<Enumerator::TypeStruct<EnumeratorType>*>(userData);		\
		return typeStruct->enumerator;															\
	}																							\
																								\
	inline int getString(lua_State* L){															\
		EnumeratorType enumerator = checkArgument(L, 1);										\
		lua_pushstring(L, Enumerator::getSaveString(enumerator));								\
		return 1;																				\
	}																							\
																								\
	inline int getInteger(lua_State* L){														\
		EnumeratorType enumerator = checkArgument(L, 1);										\
		lua_pushinteger(L, (int)enumerator);													\
		return 1;																				\
	}																							\
																								\
	inline int equals(lua_State* L){															\
		lua_pushboolean(L, checkArgument(L, 1) == checkArgument(L, 2));							\
		return 1;																				\
	}																							\
																								\
	inline void declare(lua_State* L){															\
		luaL_newmetatable(L, TypeName); /*new MetaTable for ObjectType*/						\
		lua_newtable(L); /*index table for UserData Metatable*/									\
		lua_pushcfunction(L, getString);														\
		lua_setfield(L, -2, "getString");														\
		lua_pushcfunction(L, getInteger);														\
		lua_setfield(L, -2, "getInteger");														\
		lua_setfield(L, -2, "__index"); /*set index table key*/									\
		lua_pushcfunction(L, equals);															\
		lua_setfield(L, -2, "__eq");															\
		lua_pop(L, 1); /*pop the UserData MetaTable off the stack*/								\
		/*create a table containing all enumerators*/											\
		auto& types = Enumerator::getTypes<EnumeratorType>(); 									\
		lua_createtable(L, types.size(), 1);													\
		for(auto& type : types){																\
			lua_pushstring(L, type.saveString); /*key*/											\
			push(L, type.enumerator); /*value*/													\
			lua_settable(L, -3);																\
		}																						\
		lua_setglobal(L, TypeName);																\
	}																							\
																								\
};																								\
