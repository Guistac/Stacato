#pragma once

#include <ofxLua.h>

typedef int (*LuaCFunction)(lua_State*);

struct LuaMethod{
	const char* name;
	LuaCFunction function;
	const char* objectTypeString;
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
	LuaTable& addCustom(T customData, void (*pushCustomFunction)(lua_State*, T), const char* key = nullptr){
		setKey(key);
		pushCustomFunction(L, customData);
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



class LuaStaticStruct{
	
};


#define LuaSharedPointer(ObjectType)																\
namespace LuaSharedPointer_##ObjectType{															\
																									\
	inline std::vector<LuaMethod>& getMethods(){													\
		static std::vector<LuaMethod> methods;														\
		return methods;																				\
	}																								\
	inline void addMethod(const char* methodName, LuaCFunction luaCFunction){						\
		getMethods().push_back(LuaMethod{.name = methodName,										\
										 .function = luaCFunction,									\
										 .objectTypeString = #ObjectType});							\
	}																								\
																									\
	inline void inheritMethods(std::vector<LuaMethod>& inheritedMethods){							\
		for(auto& inheritedMethod : inheritedMethods){												\
			getMethods().push_back(inheritedMethod);												\
		}																							\
	}																								\
																									\
	inline LuaCFunction& getToStringMethod(){														\
		static LuaCFunction toStringMethod;															\
		return toStringMethod;																		\
	}																								\
	inline void setToStringMethod(LuaCFunction luaCFunction){ getToStringMethod() = luaCFunction; }	\
																									\
	inline void push(lua_State* L, std::shared_ptr<ObjectType> object){								\
		void* userData = lua_newuserdata(L, sizeof(std::shared_ptr<ObjectType>));					\
		new(userData) std::shared_ptr<ObjectType>(object); /*new placement operator*/				\
		luaL_getmetatable(L, #ObjectType);															\
		lua_setmetatable(L, -2);																	\
		return 1;																					\
	}																								\
																									\
	inline std::shared_ptr<ObjectType> checkArgument(lua_State* L, int argumentIndex){				\
		void* userData = luaL_checkudata(L, argumentIndex, #ObjectType);							\
		auto pointerToSharedPointer = static_cast<std::shared_ptr<ObjectType>*>(userData);			\
		return *pointerToSharedPointer;																\
	}																								\
																									\
	inline int destroy(lua_State* L){																\
		void* userData = luaL_checkudata(L, 1, #ObjectType);										\
		auto pointerToSharedPointer = static_cast<std::shared_ptr<ObjectType>*>(userData);			\
		pointerToSharedPointer->reset();															\
		return 0;																					\
	}																								\
																									\
	inline void declare(lua_State* L){																\
		luaL_newmetatable(L, #ObjectType); /*new MetaTable for UserData*/							\
																									\
		lua_newtable(L); /*index table for UserData (holds member functions)*/						\
																									\
																									\
		std::vector<const char*> baseClassStrings;													\
		for(auto& method : getMethods()){															\
			bool b_alreadyHasBaseClass = false;														\
			for(auto baseClassString : baseClassStrings){											\
				if(strcmp(baseClassString, method.objectTypeString) == 0){							\
					b_alreadyHasBaseClass = true;													\
					break;																			\
				}																					\
			}																						\
			if(!b_alreadyHasBaseClass) baseClassStrings.push_back(method.objectTypeString);			\
		}																							\
		LuaTable baseClassesTable(L);																\
		baseClassesTable.begin(baseClassStrings.size());											\
		for(auto baseClassString : baseClassStrings){												\
			baseClassesTable.addString(baseClassString);											\
			Logger::critical("ADDED BASE CLASS {}", baseClassString);								\
		}																							\
		lua_setfield(L, -2, "BaseClasses");															\
																									\
																									\
		for(auto& method : getMethods()){															\
			lua_pushcfunction(L, method.function);													\
			lua_setfield(L, -2, method.name);														\
		}																							\
		lua_setfield(L, -2, "__index");	/*set index table key*/										\
																									\
		/*set destroy function for garbage collector metamethod*/									\
		/*to decrement the shared pointer user count when lua garbage collects the user data */		\
		lua_pushcfunction(L, destroy);																\
		lua_setfield(L, -2, "__gc");																\
																									\
																									\
																									\
																									\
																									\
																									\
		if(LuaCFunction toStringMethod = getToStringMethod()){ /*set tostring metmethod*/			\
			lua_pushcfunction(L, toStringMethod);													\
			lua_setfield(L, -2, "__tostring");														\
		}																							\
																									\
		lua_pop(L, 1); /*pop the UserData MetaTable off the stack	*/								\
	}																								\
};																									\




#define LuaPointer(ObjectType)																\
namespace LuaPointer_##ObjectType{															\
																							\
	inline void push(lua_State* L, ObjectType* pointer){									\
		void* userData = lua_newuserdata(L, sizeof(ObjectType*));							\
		auto pointerToPointer = static_cast<ObjectType**>(userData);						\
		*pointerToPointer = pointer;														\
		luaL_getmetatable(L, #ObjectType);													\
		lua_setmetatable(L, -2);															\
	}																						\
																							\
	inline ObjectType* checkArgument(lua_State* L, int argumentIndex){						\
		void* userData = luaL_checkudata(L, argumentIndex, #ObjectType);					\
		auto pointerToPointer = static_cast<ObjectType**>(userData);						\
		return *pointerToPointer;															\
	}																						\
																							\
	inline std::vector<LuaMethod>& getMethods(){											\
		static std::vector<LuaMethod> methods;												\
		return methods;																		\
	}																						\
	inline void addMethod(const char* methodName, LuaCFunction luaCFunction){				\
		getMethods().push_back(LuaMethod{.name = methodName, .function = luaCFunction});	\
	}																						\
																							\
	inline int equals(lua_State* L){														\
		ObjectType* pointer1 = checkArgument(L, 1);											\
		ObjectType* pointer2 = checkArgument(L, 2);											\
		lua_pushboolean(L, pointer1 == pointer2);											\
		return 1;																			\
	}																						\
																							\
	inline void declare(lua_State* L){														\
		luaL_newmetatable(L, #ObjectType);													\
																							\
		lua_newtable(L); /*index table for UserData Metatable*/								\
		for(auto& method : getMethods()){													\
			lua_pushstring(L, method.name);													\
			lua_pushcfunction(L, method.function);											\
			lua_settable(L, -3);															\
		}																					\
		lua_setfield(L, -2, "__index"); /*set index table key*/								\
																							\
		lua_pushcfunction(L, equals);														\
		lua_setfield(L, -2, "__eq"); /*set equality metathod key*/							\
																							\
		lua_pop(L, 1); /*pop metatable*/													\
	}																						\
																							\
}																							\



#define LuaEnumerator(EnumeratorType)															\
namespace LuaEnumerator_##EnumeratorType{														\
																								\
	inline void push(lua_State* L, EnumeratorType enumerator){									\
		auto typeStructure = Enumerator::getTypeStructure(enumerator);							\
		void* userData = lua_newuserdata(L, sizeof(Enumerator::TypeStruct<EnumeratorType>*));	\
		new(userData) Enumerator::TypeStruct<EnumeratorType>*(typeStructure);					\
		luaL_getmetatable(L, #EnumeratorType);													\
		lua_setmetatable(L, -2);																\
	}																							\
																								\
	inline EnumeratorType checkArgument(lua_State* L, int argumentIndex){						\
		void* userData = luaL_checkudata(L, argumentIndex, #EnumeratorType);					\
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
		luaL_newmetatable(L, #EnumeratorType); /*new MetaTable for ObjectType*/					\
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
		lua_setglobal(L, #EnumeratorType);														\
	}																							\
																								\
};																								\
