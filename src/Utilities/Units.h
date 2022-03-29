#pragma once

namespace Units{

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

	struct UnitStructure{
		Units::Type unitType;
		const char* singular;
		const char* plural;
		const char* abbreviated;
		const char* saveString;
		double baseMultiplier;
		double baseOffset;
	};

}

typedef const Units::UnitStructure* Unit;

#define UNIT_TYPE_FUNCTIONS(Type) \
	const std::vector<const UnitStructure* const>& get();\
	bool isValidSaveString(const char* saveString);\


namespace Units{

	namespace None{
	UNIT_TYPE_FUNCTIONS(Type::NONE);
		extern const UnitStructure* const None;
	}

	namespace LinearDistance{
	UNIT_TYPE_FUNCTIONS(Type::LINEAR_DISTANCE);
		extern const UnitStructure* const Millimeter;
		extern const UnitStructure* const Centimeter;
		extern const UnitStructure* const Meter;
	};

	namespace AngularDistance{
	UNIT_TYPE_FUNCTIONS(Type::ANGULAR_DISTANCE);
		extern const UnitStructure* const Degree;
		extern const UnitStructure* const Radian;
		extern const UnitStructure* const Revolution;
	}

	namespace Time{
	UNIT_TYPE_FUNCTIONS(Type::TIME);
		extern const UnitStructure* const Nanosecond;
		extern const UnitStructure* const Microsecond;
		extern const UnitStructure* const Millisecond;
		extern const UnitStructure* const Second;
		extern const UnitStructure* const Minute;
		extern const UnitStructure* const Hour;
		extern const UnitStructure* const Day;
		extern const UnitStructure* const Week;
	}

	namespace Frequency{
	UNIT_TYPE_FUNCTIONS(Type::FREQUENCY);
		extern const UnitStructure* const Hertz;
		extern const UnitStructure* const Kilohertz;
		extern const UnitStructure* const Megahertz;
		extern const UnitStructure* const Gigahertz;
	}

	namespace Voltage{
	UNIT_TYPE_FUNCTIONS(Type::VOLTAGE);
		extern const UnitStructure* const Millivolt;
		extern const UnitStructure* const Volt;
		extern const UnitStructure* const Kilovolt;
	}

	namespace Current{
	UNIT_TYPE_FUNCTIONS(Type::CURRENT);
		extern const UnitStructure* const Milliampere;
		extern const UnitStructure* const Ampere;
	}

	namespace Temperature{
	UNIT_TYPE_FUNCTIONS(Type::TEMPERATURE);
		extern const UnitStructure* const Kelvin;
		extern const UnitStructure* const Celsius;
		extern const UnitStructure* const Fahrenheit;
	}

	namespace Mass{
	UNIT_TYPE_FUNCTIONS(Type::MASS);
		extern const UnitStructure* const Gram;
		extern const UnitStructure* const Kilogram;
		extern const UnitStructure* const Ton;
	}

	bool isValidSaveString(const char* saveString);

	const UnitStructure* const fromSaveString(const char* saveString);

	double convert(double input, const UnitStructure* inputUnit, const UnitStructure* outputUnit);
	void convert(const std::vector<double>& input, std::vector<double>& output, const UnitStructure* inputUnit, const UnitStructure* outputUnit);

}
