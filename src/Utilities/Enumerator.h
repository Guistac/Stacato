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
struct TypeStructure{
	E enumerator;
	char displayString[256];
	char saveString[256];
};
	
template<typename E>
std::vector<TypeStructure<E>>& getTypeStructures();
	
template<typename E>
TypeStructure<E>* getTypeStructure(E enumerator){
	for(auto& typeStructure : getTypeStructures<E>()) if(enumerator == typeStructure.enumerator) return &typeStructure;
	return nullptr;
}

template<typename E>
const char* getSaveName(E enumerator){
	TypeStructure<E>* typeStructure = getTypeStructure<E>(enumerator);
	if(typeStructure) return typeStructure->saveString;
	return nullptr;
}

template<typename E>
const char* getDisplayName(E enumerator){
	TypeStructure<E>* typeStructure = getTypeStructure<E>(enumerator);
	if(typeStructure) return typeStructure->displayString;
	return nullptr;
}

template<typename E>
TypeStructure<E>* getTypeStructureFromSaveString(const char* saveString){
	for(auto& typeStructure : getTypeStructures<E>()) if(strcmp(saveString, typeStructure.saveString) == 0) return &typeStructure;
	return nullptr;
}

template<typename E>
bool isValidSaveName(const char* saveString){
	return getTypeStructureFromSaveString<E>(saveString) != nullptr;
}

template<typename E>
E getEnumeratorFromSaveString(const char* saveString){
	return getTypeStructureFromSaveString<E>(saveString)->enumerator;
}

};
	

#define DEFINE_ENUMERATOR(TypeName, typeStructureList)\
namespace Enumerator{\
template<>\
std::vector<TypeStructure<TypeName>>& getTypeStructures<TypeName>(){\
	static std::vector<TypeStructure<TypeName>> typeStructures = {typeStructureList};\
	return typeStructures;\
}\
}\

	
