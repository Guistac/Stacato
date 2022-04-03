#pragma once

/*
USAGE:

Step 1 : Declare Scoped Enumerator :

enum class enumeratedValues{
	TestValue1,
	TestValue2,
	TestValue3,
	TestValue4
};
 

Step 2 : Declare Macro containing strings for each enumeration value: {EnumeratorValue, DisplayString, SaveString}


#define EnumeratorStrings 	{TestValue::TestValue1, "V1", "SV1"},\
							{TestValue::TestValue2, "V2", "SV2"},\
							{TestValue::TestValue3, "V3", "SV3"},\
							{TestValue::TestValue4, "V4", "SV4"}\


Step 3 : Enable Enumerator Usage by using the following macro, specify Enumerator Type Name and the previously declared string macro

DEFINE_ENUMERATOR(TestValue, EnumeratorStrings)
*/
 


//======================================= SCOPED TEMPLATED METHOD ENUMERATORS =======================================

namespace Enumerator{

	template<typename E>
	struct TypeStruct{
		E enumerator;
		char displayString[256];
		char saveString[256];
	};
		
	template<typename E>
	inline std::vector<TypeStruct<E>>& getTypes();
		
	namespace{
		template<typename E>
		TypeStruct<E>* getTypeStructure(E enumerator){
			for(auto& typeStruct : getTypes<E>()) if(enumerator == typeStruct.enumerator) return &typeStruct;
			return nullptr;
		}

		template<typename E>
		TypeStruct<E>* getTypeStructureFromSaveString(const char* saveString){
			for(auto& typeStruct : getTypes<E>()) if(strcmp(saveString, typeStruct.saveString) == 0) return &typeStruct;
			return nullptr;
		}
	}

	template<typename E>
	const char* getSaveString(E enumerator){
		TypeStruct<E>* typeStructure = getTypeStructure<E>(enumerator);
		if(typeStructure) return typeStructure->saveString;
		return nullptr;
	}

	template<typename E>
	const char* getDisplayString(E enumerator){
		TypeStruct<E>* typeStructure = getTypeStructure<E>(enumerator);
		if(typeStructure) return typeStructure->displayString;
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
	

#define DEFINE_ENUMERATOR(TypeName, typeStructList)\
namespace Enumerator{\
	template<>\
	inline std::vector<TypeStruct<TypeName>>& getTypes<TypeName>(){\
		static std::vector<TypeStruct<TypeName>> typeStructs = {typeStructList};\
		return typeStructs;\
	}\
}\
