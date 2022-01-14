#pragma once

/*====================================================================================================

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

 
 ====================================================================================================*/

 


//=========== TEMPLATED CLASS BASED ENUMERATORS

/*
#define DEFINE_ENUMERATOR(TypeName, typeList)\
template<>\
inline std::vector<Enumerator<TypeName>::Type>& Enumerator<TypeName>::getTypes(){\
	static std::vector<Type> types = {typeList};\
	return types;\
}\

template<typename E>
class Enumerator{

public:
		 
	 struct Type{
		 E enumerator;
		 char displayString[256];
		 char saveString[256];
	 };
		 
	//Function Declaration overridden by DEFINE_ENUMERATOR Macro
	//the inline keyword is absolutely necessary to avoid duplicate symbols
	 static inline std::vector<Type>& getTypes();

	 static const char* getSaveString(E enumerator){
		 Type* type = getType(enumerator);
		 if(type) return type->saveString;
		 return nullptr;
	 }

	 static const char* getDisplayString(E enumerator){
		 Type* type = getType(enumerator);
		 if(type) return type->displayString;
		 return nullptr;
	 }

	 static bool isValidSaveName(const char* saveString){
		 return getTypeFromSaveString(saveString) != nullptr;
	 }

	 static E getEnumeratorFromSaveString(const char* saveString){
		 return getTypeFromSaveString(saveString)->enumerator;
	 }

private:
	
	static Type* getType(E enumerator){
		for(auto& type : getTypes()) if(enumerator == type.enumerator) return &type;
		return nullptr;
	}
	
	static Type* getTypeFromSaveString(const char* saveString){
		for(auto& type : getTypes()) if(strcmp(saveString, type.saveString) == 0) return &type;
		return nullptr;
	}
	
};

 
 





#define DEFINE_UNIT_ENUMERATOR(TypeName, typeList)\
template<>\
inline std::vector<UnitEnumerator<TypeName>::UnitType>& UnitEnumerator<TypeName>::getTypes(){\
	static std::vector<UnitType> types = {typeList};\
	return types;\
}\
template<>\
inline std::vector<Enumerator<TypeName>::Type>& Enumerator<TypeName>::getTypes(){\
	static std::vector<Enumerator<TemperatureUnit>::Type> types;\
	static bool b_initialized = false;\
	if(!b_initialized){\
		b_initialized = true;\
		std::vector<UnitEnumerator<TemperatureUnit>::UnitType> unitTypes = {typeList};\
		for(auto& unitType : unitTypes) types.push_back(unitType);\
	}\
	return types;\
}


template<typename E>
class UnitEnumerator : public Enumerator<E>{
	
public:
	
	struct UnitType : public Enumerator<E>::Type{
		char displayStringPlural[256];
		char displayStringAbbreviated[256];
		bool b_isBase = false;
		double baseMultiple = 0.0;
		double baseOffset = 0.0;
	};
	
	static inline std::vector<UnitType>& getTypes();
	
	static const char* getDisplayStringPlural(E enumerator){
		UnitType* type = getType(enumerator);
		if(type) return type->displayStringPlural;
		return nullptr;
	}
	
	static const char* getDisplayStringAbbreviated(E enumerator){
		UnitType* type = getType(enumerator);
		if(type) return type->displayStringAbbreviated;
		return nullptr;
	}
	
	static bool isBaseUnit(E enumerator){
		UnitType* type = getType(enumerator);
		if(type) return type->b_isBase;
		return false;
	}
	
	static double getBaseUnitMultiple(E enumerator){
		UnitType* type = getType(enumerator);
		if(type) return type->baseMultiple;
		return 0.0;
	}
	
	static double getBaseUnitOffset(E enumerator){
		UnitType* type = getType(enumerator);
		if(type) return type->baseOffset;
		return 0.0;
	}
	
private:
	
	static UnitType* getType(E enumerator){
		for(auto& type : getTypes()) if(enumerator == type.enumerator) return &type;
		return nullptr;
	}
	
};

*/








//================== SCOPED TEMPLATED METHOD ENUMERATORS




namespace Enumerator{

template<typename E>
struct Type{
	E enumerator;
	char displayString[256];
	char saveString[256];
};
	
template<typename E>
inline std::vector<Type<E>>& getTypes();
	
namespace{
	template<typename E>
	Type<E>* getType(E enumerator){
		for(auto& type : getTypes<E>()) if(enumerator == type.enumerator) return &type;
		return nullptr;
	}

	template<typename E>
	Type<E>* getTypeFromSaveString(const char* saveString){
		for(auto& type : getTypes<E>()) if(strcmp(saveString, type.saveString) == 0) return &type;
		return nullptr;
	}
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
	bool b_isBaseUnit = false;
	double baseUnitMultiple = 0.0;
	double baseUnitOffset = 0.0;
};
	
template<typename E>
inline std::vector<Type<E>>& getTypes();
	
namespace{
	template<typename E>
	Type<E>* getType(E enumerator){
		for(auto& type : getTypes<E>()) if(enumerator == type.enumerator) return &type;
		return nullptr;
	}

	template<typename E>
	Type<E>* getTypeFromSaveString(const char* saveString){
		for(auto& type : getTypes<E>()) if(strcmp(saveString, type.saveString) == 0) return &type;
		return nullptr;
	}
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
const char* getSaveString(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->saveString;
	return nullptr;
}

template<typename E>
bool isBaseUnit(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->b_isBaseUnit;
	return false;
}

template<typename E>
double getBaseMultiple(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->baseUnitMultiple;
	return 0.0;
}

template<typename E>
double getBaseOffset(E enumerator){
	Type<E>* type = getType<E>(enumerator);
	if(type) return type->baseUnitOffset;
	return 0.0;
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
namespace Enumerator{\
template<>\
inline std::vector<Type<PositionUnit>>& getTypes<TypeName>(){\
	static std::vector<Type<TypeName>> types;\
	static bool b_initialized = false;\
	if(!b_initialized){\
		b_initialized = true;\
		for(auto& unitType : Unit::getTypes<TypeName>()){\
			types.push_back(Type<TypeName>());\
			auto& pushedType = types.back();\
			pushedType.enumerator = unitType.enumerator;\
			strcpy(pushedType.displayString, unitType.displayString);\
			strcpy(pushedType.saveString, unitType.saveString);\
		}\
	}\
	return types;\
}\
}\


