#pragma once

//===== USAGE:

//----- Step 1 : Declare Scoped Enumerator

/*
enum class enumeratedValues{
	TestValue1,
	TestValue2,
	TestValue3,
	TestValue4
};
 */

//----- Step 2 : Declare Macro which contains strings for each enumeration value: {EnumeratorValue, DisplayString, SaveString}

/*
#define EnumeratorStrings 	{TestValue::TestValue1, "V1", "SV1"},\
							{TestValue::TestValue2, "V2", "SV2"},\
							{TestValue::TestValue3, "V3", "SV3"},\
							{TestValue::TestValue4, "V4", "SV4"}\
*/

//----- Step 3 : Enable Enumerator by using the following macro, specify Enumerator Type Name and the previously declared macro

/*
DEFINE_ENUMERATOR(TestValue, EnumeratorStrings)
*/



namespace Enumerator{

template<typename E>
struct Type{
	E enumerator;
	char displayString[256];
	char saveString[256];
};
	
template<typename E>
std::vector<Type<E>>& getTypes();
	
template<typename E>
Type<E>* getType(E enumerator){
	for(auto& type : getTypes<E>()) if(enumerator == type.enumerator) return &type;
	return nullptr;
}

template<typename E>
const char* getSaveString(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->saveString;
	return nullptr;
}

template<typename E>
const char* getDisplayString(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->displayString;
	return nullptr;
}

template<typename E>
Type<E>* getTypeFromSaveString(const char* saveString){
	for(auto& type : getTypes<E>()) if(strcmp(saveString, type.saveString) == 0) return &type;
	return nullptr;
}

template<typename E>
bool isValidSaveName(const char* saveString){
	return getTypeFromSaveString<E>(saveString) != nullptr;
}

template<typename E>
E getEnumeratorFromSaveString(const char* saveString){
	return getTypeFromSaveString<E>(saveString)->enumerator;
}

};
	

#define DEFINE_ENUMERATOR(TypeName, typeList)\
namespace Enumerator{\
template<>\
inline std::vector<Type<TypeName>>& getTypes<TypeName>(){\
	static std::vector<Type<TypeName>> types = {typeList};\
	return types;\
}\
}\

	


namespace Unit{

template<typename E>
struct Type{
	E enumerator;
	char displayString[256];
	char displayStringPlural[256];
	char abbreviatedString[256];
	char saveString[256];
};
	
template<typename E>
std::vector<Type<E>>& getTypes();
	
template<typename E>
Type<E>* getType(E enumerator){
	for(auto& type : getTypes<E>()) if(enumerator == type.enumerator) return &type;
	return nullptr;
}

template<typename E>
const char* getSaveString(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->saveString;
	return nullptr;
}

template<typename E>
const char* getDisplayString(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->displayString;
	return nullptr;
}

template<typename E>
const char* getDisplayStringPlural(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->displayStringPlural;
	return nullptr;
}

template<typename E>
const char* getAbbreviatedString(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->abbreviatedString;
	return nullptr;
}

template<typename E>
Type<E>* getTypeFromSaveString(const char* saveString){
	for(auto& type : getTypes<E>()) if(strcmp(saveString, type.saveString) == 0) return &type;
	return nullptr;
}

template<typename E>
bool isValidSaveName(const char* saveString){
	return getTypeFromSaveString<E>(saveString) != nullptr;
}

template<typename E>
E getEnumeratorFromSaveString(const char* saveString){
	return getTypeFromSaveString<E>(saveString)->enumerator;
}

}


#define DEFINE_UNIT(TypeName, typeList)\
namespace Unit{\
template<>\
inline std::vector<Type<TypeName>>& getTypes<TypeName>(){\
	static std::vector<Type<TypeName>> types = {typeList};\
	return types;\
}\
}\
