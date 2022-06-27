#pragma once

#include <ofxLua.h>


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
	
	LuaTable& addFunction(lua_CFunction value, const char* key = nullptr){
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





//trick to pass string literals as template arguments
//copied from: https://ctrpeach.io/posts/cpp20-string-literal-template-parameters/
template<size_t N>
struct StringLiteral {
	constexpr StringLiteral(const char (&str)[N]) {
		std::copy_n(str, N, value);
	}
	char value[N];
};

//struct to hold a c function and its name
struct LuaMethod{
	const char* name;
	lua_CFunction function;
};

//——————————————— SHARED OBJECT POINTER USER DATA ————————————————

//wraps shared pointer functionnality inside lua
//can have methods
//reference count is decremented on lua garbage collection
//can inherit methods from base classes
//supports equality check

template<typename T, StringLiteral typeNameString>
class LuaSharedPointer{
public:
									
	static const char* getTypeName(){
		return typeNameString.value;
	}
	
	static std::vector<LuaMethod>& getMethods(){
		static std::vector<LuaMethod> methods;
		return methods;
	}
	static void addMethod(const char* methodName, lua_CFunction luaCFunction){
		getMethods().push_back(LuaMethod{.name = methodName, .function = luaCFunction});
	}
					
	static std::vector<const char*>& getInheritedTypeNames(){
		static std::vector<const char*> inheritedTypeNames;
		return inheritedTypeNames;
	}
	
	template<typename O, StringLiteral otherTypeNameString>
	static void inherit(LuaSharedPointer<O, otherTypeNameString>& inheritedType){
		auto& methods = getMethods();
		auto& inheritedMethods = inheritedType.getMethods();
		methods.insert(methods.end(), inheritedMethods.begin(), inheritedMethods.end());
		getInheritedTypeNames().push_back(inheritedType.getTypeName());
	}
																									
	static void push(lua_State* L, std::shared_ptr<T> object){
		void* userData = lua_newuserdata(L, sizeof(std::shared_ptr<T>));
		new(userData) std::shared_ptr<T>(object); /*new placement operator*/
		luaL_getmetatable(L, typeNameString.value);
		lua_setmetatable(L, -2);
		return 1;
	}
																									
	static std::shared_ptr<T> checkArgument(lua_State* L, int argumentIndex){
		void* userData = luaL_checkudata(L, argumentIndex, typeNameString.value);
		auto pointerToSharedPointer = static_cast<std::shared_ptr<T>*>(userData);
		return *pointerToSharedPointer;
	}
			
	//checks if the argument on the stack is derived from the local type
	//if it is, cast the argument to the local base class and return it
	static std::shared_ptr<T> checkDerivedArgument(lua_State* L, int index){
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
				if(strcmp(className, typeNameString.value) == 0) {
					lua_pop(L, 3);
					void* userData = lua_touserdata(L, index);
					return *static_cast<std::shared_ptr<T>*>(userData);
				}
			}else luaL_error(L, "BaseClasses field is not a string");
			//pop the value, so the table is on top again
			lua_pop(L, 1);
		}
		lua_pop(L, 2);
		luaL_error(L, "object is not has no base class of time %s", typeNameString.value);
		return nullptr;
	}
	
	static int destroy(lua_State* L){
		void* userData = luaL_checkudata(L, 1, typeNameString.value);
		auto pointerToSharedPointer = static_cast<std::shared_ptr<T>*>(userData);
		pointerToSharedPointer->reset();
		return 0;
	}

	static int equals(lua_State* L){
		auto shared1 = checkArgument(L, 1);
		auto shared2 = checkArgument(L, 2);
		lua_pushboolean(L, shared1 == shared2);
		return 1;
	}
	
	static lua_CFunction& getToStringMethod(){
		static lua_CFunction toStringMethod;
		return toStringMethod;
	}
	static void setToStringMethod(lua_CFunction luaCFunction){
		getToStringMethod() = luaCFunction;
	}
																									
	static void declare(lua_State* L){
		//new MetaTable for UserData
		luaL_newmetatable(L, typeNameString.value);
						
		//index table for UserData (holds member functions)
		lua_newtable(L);
																									
		//if the type inherits from other types
		//add a table field with key "BaseClasses" to the "__index" table
		//this field will contain one string entry for each inherited type name
		//including the current type
		//this "BaseClasses" Field will be checked by the "checkDerivedArgument" method
		//to retrieve a pointer to the base class
		auto& inheritedTypeNames = getInheritedTypeNames();
		if(!inheritedTypeNames.empty()){
			LuaTable inheritedTypesTable(L);
			inheritedTypesTable.begin(inheritedTypeNames.size() + 1);
			for(auto& inheritedTypeName : inheritedTypeNames){
				inheritedTypesTable.addString(inheritedTypeName);
			}
			inheritedTypesTable.addString(getTypeName());
			lua_setfield(L, -2, "BaseClasses");
		}
			
		//add all methods (including inherited ones) to the index table
		for(auto& method : getMethods()){
			lua_pushcfunction(L, method.function);
			lua_setfield(L, -2, method.name);
		}
		
		//set index table key
		lua_setfield(L, -2, "__index");
		
		//set destroy function for garbage collector metamethod
		//to decrement the shared pointer user count when lua garbage collects the user data
		lua_pushcfunction(L, destroy);
		lua_setfield(L, -2, "__gc");
		
		//set equality check metamethod
		lua_pushcfunction(L, equals);
		lua_setfield(L, -2, "__eq");
																
		//set tostring metamethod in case the user added it
		if(lua_CFunction toStringMethod = getToStringMethod()){
			lua_pushcfunction(L, toStringMethod);
			lua_setfield(L, -2, "__tostring");
		}
						
		//pop the UserData MetaTable off the stack
		lua_pop(L, 1);
	}
};



//————————————————— OBJECT POINTER USER DATA ——————————————————

//supports equality check
//can have methods

template<typename T, StringLiteral typeNameString>
class LuaPointer{
public:
	
	static void push(lua_State* L, T* pointer){
		void* userData = lua_newuserdata(L, sizeof(T*));
		auto pointerToPointer = static_cast<T**>(userData);
		*pointerToPointer = pointer;
		luaL_getmetatable(L, typeNameString.value);
		lua_setmetatable(L, -2);
	}
																							
	static T* checkArgument(lua_State* L, int argumentIndex){
		void* userData = luaL_checkudata(L, argumentIndex, typeNameString.value);
		auto pointerToPointer = static_cast<T**>(userData);
		return *pointerToPointer;
	}
																							
	static std::vector<LuaMethod>& getMethods(){
		static std::vector<LuaMethod> methods;
		return methods;
	}
	static void addMethod(const char* methodName, lua_CFunction luaCFunction){
		getMethods().push_back(LuaMethod{.name = methodName, .function = luaCFunction});
	}
																							
	static int equals(lua_State* L){
		T* pointer1 = checkArgument(L, 1);
		T* pointer2 = checkArgument(L, 2);
		lua_pushboolean(L, pointer1 == pointer2);
		return 1;
	}
																							
	static void declare(lua_State* L){
		luaL_newmetatable(L, typeNameString.value);
																							
		lua_newtable(L); /*index table for UserData Metatable*/
		for(auto& method : getMethods()){
			lua_pushstring(L, method.name);
			lua_pushcfunction(L, method.function);
			lua_settable(L, -3);
		}
		lua_setfield(L, -2, "__index"); /*set index table key*/
																							
		lua_pushcfunction(L, equals);
		lua_setfield(L, -2, "__eq"); /*set equality metathod key*/
																							
		lua_pop(L, 1); /*pop metatable*/
	}
																							
};



//——————————————— ENUMERATOR USER DATA ——————————————

//has global table containing all enumerators by Savename Key
//supports equality check
//has getString() method
//has getInteger() method

template<typename T, StringLiteral typeNameString>
class LuaEnumerator{
public:
																								
	static void push(lua_State* L, T enumerator){
		auto typeStructure = Enumerator::getTypeStructure(enumerator);
		void* userData = lua_newuserdata(L, sizeof(Enumerator::TypeStruct<T>*));
		new(userData) Enumerator::TypeStruct<T>*(typeStructure);
		luaL_getmetatable(L, typeNameString.value);
		lua_setmetatable(L, -2);
	}
																								
	static T checkArgument(lua_State* L, int argumentIndex){
		void* userData = luaL_checkudata(L, argumentIndex, typeNameString.value);
		auto typeStruct = static_cast<Enumerator::TypeStruct<T>*>(userData);
		return typeStruct->enumerator;
	}
																								
	static int getString(lua_State* L){
		T enumerator = checkArgument(L, 1);
		lua_pushstring(L, Enumerator::getSaveString(enumerator));
		return 1;
	}
																								
	static int getInteger(lua_State* L){
		T enumerator = checkArgument(L, 1);
		lua_pushinteger(L, (int)enumerator);
		return 1;
	}
																								
	static int equals(lua_State* L){
		lua_pushboolean(L, checkArgument(L, 1) == checkArgument(L, 2));
		return 1;
	}
																								
	static void declare(lua_State* L){
		luaL_newmetatable(L, typeNameString.value); /*new MetaTable for ObjectType*/
		lua_newtable(L); /*index table for UserData Metatable*/
		lua_pushcfunction(L, getString);
		lua_setfield(L, -2, "getString");
		lua_pushcfunction(L, getInteger);
		lua_setfield(L, -2, "getInteger");
		lua_setfield(L, -2, "__index"); /*set index table key*/
		lua_pushcfunction(L, equals);
		lua_setfield(L, -2, "__eq");
		lua_pop(L, 1); /*pop the UserData MetaTable off the stack*/
		/*create a table containing all enumerators*/
		auto& types = Enumerator::getTypes<T>();
		lua_createtable(L, types.size(), 1);
		for(auto& type : types){
			lua_pushstring(L, type.saveString); /*key*/
			push(L, type.enumerator); /*value*/
			lua_settable(L, -3);
		}
		lua_setglobal(L, typeNameString.value);
	}
																								
};





