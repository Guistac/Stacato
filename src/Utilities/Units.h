#pragma once

namespace Units{

	enum class Type{
		LINEAR_DISTANCE,
		ANGULAR_DISTANCE,
		TIME,
		FREQUENCY,
		VOLTAGE,
		CURRENT,
		RESISTANCE,
		POWER,
		TEMPERATURE,
		MASS,
		FORCE,
		DATA,
		FRACTION,
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

typedef Units::UnitStructure* Unit;

#define UNIT_TYPE_FUNCTIONS(Type) \
	std::vector<UnitStructure*> get();\
	bool isValidSaveString(const char* saveString);\


namespace Units{

	namespace None{
	UNIT_TYPE_FUNCTIONS(Type::NONE);
		extern UnitStructure* None;
	}

	namespace Fraction{
	UNIT_TYPE_FUNCTIONS(Type::Fraction)
		extern UnitStructure* Percent;
		extern UnitStructure* Permille;
	};

	namespace LinearDistance{
	UNIT_TYPE_FUNCTIONS(Type::LINEAR_DISTANCE);
		extern UnitStructure* Millimeter;
		extern UnitStructure* Centimeter;
		extern UnitStructure* Meter;
	};

	namespace AngularDistance{
	UNIT_TYPE_FUNCTIONS(Type::ANGULAR_DISTANCE);
		extern UnitStructure* Degree;
		extern UnitStructure* Radian;
		extern UnitStructure* Revolution;
	}

	namespace Time{
	UNIT_TYPE_FUNCTIONS(Type::TIME);
		extern UnitStructure* Nanosecond;
		extern UnitStructure* Microsecond;
		extern UnitStructure* Millisecond;
		extern UnitStructure* Second;
		extern UnitStructure* Minute;
		extern UnitStructure* Hour;
		extern UnitStructure* Day;
		extern UnitStructure* Week;
	}

	namespace Frequency{
	UNIT_TYPE_FUNCTIONS(Type::FREQUENCY);
		extern UnitStructure* Hertz;
		extern UnitStructure* Kilohertz;
		extern UnitStructure* Megahertz;
		extern UnitStructure* Gigahertz;
	}

	namespace Voltage{
	UNIT_TYPE_FUNCTIONS(Type::VOLTAGE);
		extern UnitStructure* Millivolt;
		extern UnitStructure* Volt;
		extern UnitStructure* Kilovolt;
	}

	namespace Current{
	UNIT_TYPE_FUNCTIONS(Type::CURRENT);
		extern UnitStructure* Milliampere;
		extern UnitStructure* Ampere;
	}

	namespace Resistance{
	UNIT_TYPE_FUNCTIONS(Type::RESISTANCE);
		extern UnitStructure* Ohm;
		extern UnitStructure* KiloOhm;
	}


	namespace Power{
	UNIT_TYPE_FUNCTIONS(Type::POWER);
		extern UnitStructure* Watt;
		extern UnitStructure* KiloWatt;
	};

	namespace Temperature{
	UNIT_TYPE_FUNCTIONS(Type::TEMPERATURE);
		extern UnitStructure* Kelvin;
		extern UnitStructure* Celsius;
		extern UnitStructure* Fahrenheit;
	}

	namespace Mass{
	UNIT_TYPE_FUNCTIONS(Type::MASS);
		extern UnitStructure* Gram;
		extern UnitStructure* Kilogram;
		extern UnitStructure* Ton;
	}

	namespace Force{
	UNIT_TYPE_FUNCTIONS(Type::FORCE);
		extern UnitStructure* Newton;
		extern UnitStructure* KiloNewton;
	}

	namespace Data{
	UNIT_TYPE_FUNCTIONS(Type::Data);
		extern UnitStructure* Bit;
		extern UnitStructure* Byte;
		extern UnitStructure* KiloBit;
		extern UnitStructure* KiloByte;
		extern UnitStructure* MegaBit;
		extern UnitStructure* MegaByte;
		extern UnitStructure* GigaBit;
		extern UnitStructure* GigaByte;
		extern UnitStructure* TeraBit;
		extern UnitStructure* TeraByte;
	}

	bool isValidSaveString(const char* saveString);

	UnitStructure* fromSaveString(const char* saveString);

	double convert(double input, UnitStructure* inputUnit, UnitStructure* outputUnit);
	void convert(std::vector<double>& input, std::vector<double>& output, UnitStructure* inputUnit, UnitStructure* outputUnit);

}
