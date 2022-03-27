#pragma once

namespace newUnits{

	enum class Type{
		LINEAR_DISTANCE,
		ANGULAR_DISTANCE,
		TIME,
		FREQUENCY,
		VOLTAGE,
		CURRENT,
		TEMPERATURE,
		MASS,
		NONE
	};

	enum class Enumerator{
	//Linear Distance
		DEGREE,
		RADIAN,
		REVOLUTION,
	//Angular Distance
		METER,
		CENTIMETER,
		MILLIMETER,
	//Time
		NANOSECOND,
		MICROSECOND,
		MILLISECOND,
		SECOND,
		MINUTE,
		HOUR,
		DAY,
		WEEK,
	//Frequency
		HERTZ,
		KILOHERTZ,
		MEGAHERTZ,
		GIGAHERTZ,
	//Voltage
		MILLIVOLT,
		VOLT,
		KILOVOLT,
	//Current
		AMPERE,
		MILLIAMPERE,
	//Temperature
		KELVIN,
		CELSIUS,
		FAHRENHEIT,
	//Mass
		GRAM,
		KILOGRAM,
		TON
	};

	struct UnitStructure{
	   newUnits::Enumerator enumerator;
	   newUnits::Type unitType;
	   const char* singular;
	   const char* plural;
	   const char* abbreviated;
	   const char* saveString;
	   double baseMultiplier;
	   double baseOffset;
	};

	const UnitStructure* const getUnit(newUnits::Enumerator unitEnumerator);
	const std::vector<const UnitStructure>& getUnits(newUnits::Type unitType);
	newUnits::Type getUnitType(newUnits::Enumerator unitEnumerator);

	bool isValidSaveString(const char* saveString);
	bool isValidSaveString(const char* saveString, newUnits::Type unitType);
	const UnitStructure* const getUnitFromSaveString(const char* saveString);

	double convert(double input, newUnits::Enumerator inputUnit, newUnits::Enumerator outputUnit);
	void convert(const std::vector<double>& input, std::vector<double>& output, newUnits::Enumerator inputUnit, newUnits::Enumerator outputUnit);

}


