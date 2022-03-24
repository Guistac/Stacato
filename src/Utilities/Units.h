#pragma once

//======================================= SCOPED TEMPLATED METHOD UNIT ENUMERATORS =======================================

//here we don't really use polymorphism
//instead we declare the "base" type along with the unit type
//this way the enumerator base type also works for the the same scope enumerator as the unit
//to do this we just need to fill a vector of the base enumerator type using the unit enumerator values
//this wastes a bit of memory and duplicates some code but its the friendliest implementation
//and is the most user friendly

namespace Unit{

	enum class Type{
		DISTANCE,
		TIME,
		FREQUENCY,
		VOLTAGE,
		CURRENT,
		TEMPERATURE,
		MASS,
		NONE,
		UNKNOWN
	};

	enum class Distance{
		DEGREE,
		RADIAN,
		REVOLUTION,
		METER,
		CENTIMETER,
		MILLIMETER
	};

	enum class Time{
		NANOSECOND,
		MICROSECOND,
		MILLISECOND,
		SECOND,
		MINUTE,
		HOUR,
		DAY,
		WEEK
	};

	enum class Frequency{
		HERTZ,
		KILOHERTZ,
		MEGAHERTZ,
		GIGAHERTZ
	};

	enum class Voltage{
		MILLIVOLT,
		VOLT,
		KILOVOLT
	};

	enum class Current{
		AMPERE,
		MILLIAMPERE
	};

	enum class Temperature{
		KELVIN,
		CELSIUS,
		FARENHEIT
	};

	enum class Mass{
		GRAM,
		KILOGRAM,
		TON
	};

	template<typename U>
	struct TypeStruct{
		U enumerator;
		Unit::Type unitType;
		char displayString[256];
		char displayStringPlural[256];
		char abbreviatedString[256];
		char saveString[256];
		bool b_isBaseUnit = false;
		double baseUnitMultiple = 0.0;
		double baseUnitOffset = 0.0;
	};
		
	template<typename U>
	inline std::vector<TypeStruct<U>>& getUnits();
		
	namespace{
		template<typename U>
		TypeStruct<U>* getTypeStruct(U unitEnumerator){
			for(auto& typeStruct : getUnits<U>()) if(unitEnumerator == typeStruct.enumerator) return &typeStruct;
			return nullptr;
		}

		template<typename U>
		TypeStruct<U>* getTypeStructFromSaveString(const char* saveString){
			for(auto& typeStruct : getUnits<U>()) if(strcmp(saveString, typeStruct.saveString) == 0) return &typeStruct;
			return nullptr;
		}
	}

	template<typename U>
	const char* getDisplayString(U unitEnumerator){
		TypeStruct<U>* typeStruct = getTypeStruct<U>(unitEnumerator);
		if(typeStruct) return typeStruct->displayString;
		return nullptr;
	}

	template<typename U>
	const char* getDisplayStringPlural(U unitEnumerator){
		TypeStruct<U>* typeStruct = getTypeStruct<U>(unitEnumerator);
		if(typeStruct) return typeStruct->displayStringPlural;
		return nullptr;
	}

	template<typename U>
	const char* getAbbreviatedString(U unitEnumerator){
		TypeStruct<U>* typeStruct = getTypeStruct<U>(unitEnumerator);
		if(typeStruct) return typeStruct->abbreviatedString;
		return nullptr;
	}

	template<typename U>
	const char* getSaveString(U unitEnumerator){
		TypeStruct<U>* typeStruct = getTypeStruct<U>(unitEnumerator);
		if(typeStruct) return typeStruct->saveString;
		return nullptr;
	}

	template<typename U>
	Unit::Type getUnitType(U unitEnumerator){
		TypeStruct<U>* typeStruct = getTypeStruct<U>(unitEnumerator);
		if(typeStruct) return typeStruct->unitType;
		return Unit::Type::UNKNOWN;
	}

	template<typename U>
	bool isBaseUnit(U unitEnumerator){
		TypeStruct<U>* typeStruct = getTypeStruct<U>(unitEnumerator);
		if(typeStruct) return typeStruct->b_isBaseUnit;
		return false;
	}

	template<typename U>
	double getBaseMultiple(U unitEnumerator){
		TypeStruct<U>* typeStruct = getTypeStruct<U>(unitEnumerator);
		if(typeStruct) return typeStruct->baseUnitMultiple;
		return 0.0;
	}

	template<typename U>
	double getBaseOffset(U unitEnumerator){
		TypeStruct<U>* typeStruct = getTypeStruct<U>(unitEnumerator);
		if(typeStruct) return typeStruct->baseUnitOffset;
		return 0.0;
	}

	template<typename U>
	bool isValidSaveName(const char* saveString){
		return getTypeStructFromSaveString<U>(saveString) != nullptr;
	}

	template<typename U>
	U getEnumeratorFromSaveString(const char* saveString){
		return getTypeStructFromSaveString<U>(saveString)->enumerator;
	}

	template<typename U, typename UU>
	bool isSameType(U unit1, UU unit2){
		return getUnitType(unit1) == getUnitType(unit2);
	}

	template<typename U>
	double convert(double input, U inputUnit, U outputUnit){
		assert(isSameType(inputUnit, outputUnit)); //don't allow converting linear distance to angular distance
		double baseToInputMultiplier = getBaseMultiple(inputUnit);
		double baseToInputOffset = getBaseOffset(inputUnit);
		double baseToOutputMultiplier = getBaseMultiple(outputUnit);
		double baseToOutputOffset = getBaseOffset(outputUnit);
		double multiplier = baseToInputMultiplier / baseToOutputMultiplier;
		double offset = (baseToInputOffset - baseToOutputOffset) / baseToOutputMultiplier;
		return input * multiplier + offset;
	}

	template<typename U1, typename U2>
	double convert(double input, U1 inputUnit, U2 outputUnit){
		assert(false); //cannot convert units of different types
	}

	template<typename U>
	void convert(const std::vector<double>& input, std::vector<double>& output, U inputUnit, U outputUnit){
		assert(isSameType(inputUnit, outputUnit)); //don't allow converting linear distance to angular distance
		double baseToInputMultiplier = getBaseMultiple(inputUnit);
		double baseToInputOffset = getBaseOffset(inputUnit);
		double baseToOutputMultiplier = getBaseMultiple(outputUnit);
		double baseToOutputOffset = getBaseOffset(outputUnit);
		double multiplier = baseToInputMultiplier / baseToOutputMultiplier;
		double offset = (baseToInputOffset - baseToOutputOffset) / baseToOutputMultiplier;
		output.resize(input.size());
		for(int i = 0; i < input.size(); i++) output[i] = input[i] * multiplier + offset;
	}

	template<typename U1, typename U2>
	void convert(const std::vector<double>& input, std::vector<double>& output, U1 inputUnit, U2 outputUnit){
		assert(false); //cannot convert units of different types
	}

}

#define DEFINE_UNIT(TypeName, typeStructList)\
namespace Unit{\
	template<>\
	inline std::vector<TypeStruct<TypeName>>& getUnits<TypeName>(){\
		static std::vector<TypeStruct<TypeName>> typeStructs = {typeStructList};\
		return typeStructs;\
	}\
}\
namespace Enumerator{\
	template<>\
	inline std::vector<TypeStruct<TypeName>>& getTypes<TypeName>(){\
		static std::vector<TypeStruct<TypeName>> typeStructs;\
		static bool b_initialized = false;\
		if(!b_initialized){\
			b_initialized = true;\
			for(auto& unitTypeStruct : Unit::getUnits<TypeName>()){\
				typeStructs.push_back(TypeStruct<TypeName>());\
				auto& pushedTypeStruct = typeStructs.back();\
				pushedTypeStruct.enumerator = unitTypeStruct.enumerator;\
				strcpy(pushedTypeStruct.displayString, unitTypeStruct.displayString);\
				strcpy(pushedTypeStruct.saveString, unitTypeStruct.saveString);\
			}\
		}\
		return typeStructs;\
	}\
}\

//====== UNIT TYPE ENUMERATOR ====================================================================================================================

#define UnitTypeStrings \
	{Unit::Type::DISTANCE, 		"Distance", 	"Distance"},\
	{Unit::Type::TIME, 			"Time", 		"Time"},\
	{Unit::Type::FREQUENCY, 	"Frequency", 	"Frequency"},\
	{Unit::Type::VOLTAGE, 		"Voltage", 		"Voltage"},\
	{Unit::Type::CURRENT, 		"Current", 		"Current"},\
	{Unit::Type::TEMPERATURE, 	"Temperature", 	"Temperature"},\
	{Unit::Type::MASS, 			"Mass", 		"Mass"},\
	{Unit::Type::NONE, 			"None", 		"None"},\
	{Unit::Type::UNKNOWN, 		"Unknown", 		"Unknown"}\
	
DEFINE_ENUMERATOR(Unit::Type, UnitTypeStrings)

//===== DISTANCE UNITS ===========================================================================================================================


static std::vector<Unit::TypeStruct<Unit::Distance>> getDistanceUnitTypeStructs(){
	static std::vector<Unit::TypeStruct<Unit::Distance>> typeStructs = {
		{
			.enumerator = Unit::Distance::MILLIMETER,
			.unitType = Unit::Type::DISTANCE,
			.displayString = "Millimeter",
			.displayStringPlural = "Millimeters",
			.abbreviatedString = "mm",
			.saveString = "Millimeter",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Distance::CENTIMETER,
			.unitType = Unit::Type::DISTANCE,
			.displayString = "Centimeter",
			.displayStringPlural = "Centimeters",
			.abbreviatedString = "cm",
			.saveString = "Centimeter",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 10.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Distance::METER,
			.unitType = Unit::Type::DISTANCE,
			.displayString = "Meter",
			.displayStringPlural = "Meters",
			.abbreviatedString = "m",
			.saveString = "Meter",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 1000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Distance::DEGREE,
			.unitType = Unit::Type::DISTANCE,
			.displayString = "Degree",
			.displayStringPlural = "Degrees",
			.abbreviatedString = "\xC2\xB0",
			.saveString = "Degrees",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 0.0},
		{
			.enumerator = Unit::Distance::RADIAN,
			.unitType = Unit::Type::DISTANCE,
			.displayString = "Radian",
			.displayStringPlural = "Radians",
			.abbreviatedString = "rad",
			.saveString = "Radians",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 57.2958,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Distance::REVOLUTION,
			.unitType = Unit::Type::DISTANCE,
			.displayString = "Revolution",
			.displayStringPlural = "Revolutions",
			.abbreviatedString = "rev",
			.saveString = "Revolutions",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 360.0,
			.baseUnitOffset = 0.0
		}
	};
	return typeStructs;
}


DEFINE_UNIT(Unit::Distance, getDistanceUnitTypeStructs())

//====== DISTANCE UNIT TYPES =================================================================================================================

namespace Unit{

	enum class DistanceType{
		LINEAR,
		ANGULAR
	};

	inline DistanceType getDistanceType(Unit::Distance distanceUnit){
		switch(distanceUnit){
			case Unit::Distance::DEGREE: 		return Unit::DistanceType::ANGULAR;
			case Unit::Distance::RADIAN: 		return Unit::DistanceType::ANGULAR;
			case Unit::Distance::REVOLUTION: 	return Unit::DistanceType::ANGULAR;
			case Unit::Distance::METER: 		return Unit::DistanceType::LINEAR;
			case Unit::Distance::CENTIMETER: 	return Unit::DistanceType::LINEAR;
			case Unit::Distance::MILLIMETER: 	return Unit::DistanceType::LINEAR;
		}
	}

	inline bool isLinearDistance(Unit::Distance t){
		return getDistanceType(t) == Unit::DistanceType::LINEAR;
	}

	inline bool isAngularDistance(Unit::Distance t){
		return getDistanceType(t) == Unit::DistanceType::ANGULAR;
	}

	template<>
	inline bool isSameType(Unit::Distance unit1, Unit::Distance unit2){
		return getDistanceType(unit1) == getDistanceType(unit2);
	}

}

#define DistanceUnitTypeStrings \
	{Unit::DistanceType::LINEAR, "Linear", "Linear"},\
	{Unit::DistanceType::ANGULAR, "Angular", "Angular"}\

DEFINE_ENUMERATOR(Unit::DistanceType, DistanceUnitTypeStrings)


//====== TIME UNITS ==========================================================================================================================


static std::vector<Unit::TypeStruct<Unit::Time>> getTimeUnitTypeStructs(){
	static std::vector<Unit::TypeStruct<Unit::Time>> typeStructs = {
		{
			.enumerator = Unit::Time::NANOSECOND,
			.unitType = Unit::Type::TIME,
			.displayString = "Nanosecond",
			.displayStringPlural = "Nanoseconds",
			.abbreviatedString = "ns",
			.saveString = "Nanosecond",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Time::MICROSECOND,
			.unitType = Unit::Type::TIME,
			.displayString = "Microsecond",
			.displayStringPlural = "Microseconds",
			.abbreviatedString = "µs",
			.saveString = "Microsecond",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Time::MILLISECOND,
			.unitType = Unit::Type::TIME,
			.displayString = "Millisecond",
			.displayStringPlural = "Milliseconds",
			.abbreviatedString = "ms",
			.saveString = "Millisecond",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Time::SECOND,
			.unitType = Unit::Type::TIME,
			.displayString = "Second",
			.displayStringPlural = "Second",
			.abbreviatedString = "s",
			.saveString = "Second",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000000000.0,
			.baseUnitOffset = 0.0},
		{
			.enumerator = Unit::Time::MINUTE,
			.unitType = Unit::Type::TIME,
			.displayString = "Minute",
			.displayStringPlural = "Minutes",
			.abbreviatedString = "min",
			.saveString = "Minute",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 60000000000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Time::HOUR,
			.unitType = Unit::Type::TIME,
			.displayString = "Hour",
			.displayStringPlural = "Hours",
			.abbreviatedString = "h",
			.saveString = "Hour",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 3600000000000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Time::DAY,
			.unitType = Unit::Type::TIME,
			.displayString = "Day",
			.displayStringPlural = "Days",
			.abbreviatedString = "d",
			.saveString = "Day",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 86400000000000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Time::WEEK,
			.unitType = Unit::Type::TIME,
			.displayString = "Week",
			.displayStringPlural = "Weeks",
			.abbreviatedString = "w",
			.saveString = "Week",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 604800000000000.0,
			.baseUnitOffset = 0.0
		}
	};
	return typeStructs;
}

DEFINE_UNIT(Unit::Time, getTimeUnitTypeStructs())



//======== FREQUENCY UNITS ========================================================================================================================


static std::vector<Unit::TypeStruct<Unit::Frequency>> getFrequencyUnitTypeStructs(){
	static std::vector<Unit::TypeStruct<Unit::Frequency>> typeStructs = {
		{
			.enumerator = Unit::Frequency::HERTZ,
			.unitType = Unit::Type::FREQUENCY,
			.displayString = "Hertz",
			.displayStringPlural = "Hertz",
			.abbreviatedString = "Hz",
			.saveString = "Hertz",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Frequency::KILOHERTZ,
			.unitType = Unit::Type::FREQUENCY,
			.displayString = "Kilohertz",
			.displayStringPlural = "Kilohertz",
			.abbreviatedString = "KHz",
			.saveString = "Kilohertz",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Frequency::MEGAHERTZ,
			.unitType = Unit::Type::FREQUENCY,
			.displayString = "Megahertz",
			.displayStringPlural = "Megahertz",
			.abbreviatedString = "MHz",
			.saveString = "Megahertz",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Frequency::GIGAHERTZ,
			.unitType = Unit::Type::FREQUENCY,
			.displayString = "Gigahertz",
			.displayStringPlural = "Gigahertz",
			.abbreviatedString = "GHz",
			.saveString = "Gigahertz",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000000000.0,
			.baseUnitOffset = 0.0
		}
	};
	return typeStructs;
}

DEFINE_UNIT(Unit::Frequency, getFrequencyUnitTypeStructs())


//======== VOLTAGE UNITS ========================================================================================================================

static std::vector<Unit::TypeStruct<Unit::Voltage>> getVoltageUnitTypeStructs(){
	static std::vector<Unit::TypeStruct<Unit::Voltage>> typeStructs = {
		{
			.enumerator = Unit::Voltage::MILLIVOLT,
			.unitType = Unit::Type::VOLTAGE,
			.displayString = "Millivolt",
			.displayStringPlural = "Millivolts",
			.abbreviatedString = "mV",
			.saveString = "Millivolt",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Voltage::VOLT,
			.unitType = Unit::Type::VOLTAGE,
			.displayString = "Volt",
			.displayStringPlural = "Volts",
			.abbreviatedString = "V",
			.saveString = "Volt",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Voltage::KILOVOLT,
			.unitType = Unit::Type::VOLTAGE,
			.displayString = "Kilovolt",
			.displayStringPlural = "Kilovolts",
			.abbreviatedString = "KV",
			.saveString = "Kilovolt",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000000.0,
			.baseUnitOffset = 0.0
		}
	};
	return typeStructs;
}

DEFINE_UNIT(Unit::Voltage, getVoltageUnitTypeStructs())

//======== CURRENT UNITS ========================================================================================================================

static std::vector<Unit::TypeStruct<Unit::Current>> getCurrentUnitTypeStructs(){
	static std::vector<Unit::TypeStruct<Unit::Current>> typeStructs = {
		{
			.enumerator = Unit::Current::MILLIAMPERE,
			.unitType = Unit::Type::CURRENT,
			.displayString = "Milliampere",
			.displayStringPlural = "Milliamperes",
			.abbreviatedString = "mA",
			.saveString = "Milliampere",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Current::AMPERE,
			.unitType = Unit::Type::CURRENT,
			.displayString = "Ampere",
			.displayStringPlural = "Amperes",
			.abbreviatedString = "A",
			.saveString = "Amperes",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000.0,
			.baseUnitOffset = 0.0
		}
	};
	return typeStructs;
}

DEFINE_UNIT(Unit::Current, getCurrentUnitTypeStructs())


//======== TEMPERATURE UNITS ========================================================================================================================

static std::vector<Unit::TypeStruct<Unit::Temperature>> getTemperatureUnitTypeStructs(){
	static std::vector<Unit::TypeStruct<Unit::Temperature>> typeStructs = {
		{
			.enumerator = Unit::Temperature::KELVIN,
			.unitType = Unit::Type::TEMPERATURE,
			.displayString = "Kelvin",
			.displayStringPlural = "Kelvin",
			.abbreviatedString = "K",
			.saveString = "Kelvin",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Temperature::CELSIUS,
			.unitType = Unit::Type::TEMPERATURE,
			.displayString = "Degree Celsius",
			.displayStringPlural = "Degrees Celsius",
			.abbreviatedString = "°C",
			.saveString = "Celsius",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 273.15
		},
		{
			.enumerator = Unit::Temperature::FARENHEIT,
			.unitType = Unit::Type::TEMPERATURE,
			.displayString = "Degree Farenheit",
			.displayStringPlural = "Degrees Farenheit",
			.abbreviatedString = "°F",
			.saveString = "Farenheit",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 5.0 / 9.0,
			.baseUnitOffset = 273.15 - 32.0 * 5.0 / 9.0
		}
	};
	return typeStructs;
}

DEFINE_UNIT(Unit::Temperature, getTemperatureUnitTypeStructs())

//======== MASS UNITS ========================================================================================================================

static std::vector<Unit::TypeStruct<Unit::Mass>> getMassUnitTypeStructs(){
	static std::vector<Unit::TypeStruct<Unit::Mass>> typeStructs = {
		{
			.enumerator = Unit::Mass::GRAM,
			.unitType = Unit::Type::MASS,
			.displayString = "Gram",
			.displayStringPlural = "Grams",
			.abbreviatedString = "g",
			.saveString = "Gram",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 1.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Mass::KILOGRAM,
			.unitType = Unit::Type::MASS,
			.displayString = "Kilogram",
			.displayStringPlural = "Kilograms",
			.abbreviatedString = "Kg",
			.saveString = "Kilogram",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000.0,
			.baseUnitOffset = 0.0
		},
		{
			.enumerator = Unit::Mass::TON,
			.unitType = Unit::Type::MASS,
			.displayString = "Ton",
			.displayStringPlural = "Tons",
			.abbreviatedString = "T",
			.saveString = "Ton",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 1000000.0,
			.baseUnitOffset = 0.0
		}
	};
	return typeStructs;
}

DEFINE_UNIT(Unit::Mass, getMassUnitTypeStructs())
