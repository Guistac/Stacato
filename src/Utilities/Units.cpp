#include "Units.h"

#define UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type) \
	const std::vector<const UnitStructure* const>& get(){\
		for(const UnitTypeStructure& type : getAllUnits()){\
			if(type.unitType == Type) return type.units;\
		}\
	}\
	bool isValidSaveString(const char* saveString){\
		for(auto& unitType : getAllUnits()){\
			if(unitType.unitType != Type) continue;\
			for(auto& unit : unitType.units){\
				if(strcmp(saveString, unit->saveString) == 0) return true;\
			}\
		}\
		return false;\
	}\

namespace Units{

	struct UnitTypeStructure{
		Units::Type unitType;
		const std::vector<const UnitStructure* const> units;
	};

	std::vector<UnitTypeStructure>& getAllUnits();

	namespace None{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::NONE);
		extern const UnitStructure* const None = new UnitStructure{
			.unitType = Units::Type::NONE,
			.singular = "",
			.plural = "",
			.abbreviated = "",
			.saveString = "NoUnit",
			.baseMultiplier = 0.0,
			.baseOffset = 0.0
		};
	}

	namespace Fraction{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::FRACTION)
		extern const UnitStructure* const Percent = new UnitStructure{
			.unitType = Units::Type::FRACTION,
			.singular = "Percent",
			.plural = "Percent",
			.abbreviated = "%",
			.saveString = "Percent",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		extern const UnitStructure* const Permille = new UnitStructure{
			.unitType = Units::Type::FRACTION,
			.singular = "Permille",
			.plural = "Permille",
			.abbreviated = "‰",
			.saveString = "Permille",
			.baseMultiplier = 10.0,
			.baseOffset = 0.0
		};
	}

	namespace LinearDistance{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::LINEAR_DISTANCE);
		const UnitStructure* const Millimeter = new UnitStructure{
			.unitType = Units::Type::LINEAR_DISTANCE,
			.singular = "Millimeter",
			.plural = "Millimeters",
			.abbreviated = "mm",
			.saveString = "Millimeter",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Centimeter = new UnitStructure{
			.unitType = Units::Type::LINEAR_DISTANCE,
			.singular = "Centimeter",
			.plural = "Centimeter",
			.abbreviated = "cm",
			.saveString = "Centimeter",
			.baseMultiplier = 10.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Meter = new UnitStructure{
			.unitType = Units::Type::LINEAR_DISTANCE,
			.singular = "Meter",
			.plural = "Meters",
			.abbreviated = "m",
			.saveString = "Meter",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
	};

	namespace AngularDistance{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::ANGULAR_DISTANCE);
		const UnitStructure* const Degree = new UnitStructure{
			.unitType = Units::Type::ANGULAR_DISTANCE,
			.singular = "Degree",
			.plural = "Degrees",
			.abbreviated = "\xC2\xB0",
			.saveString = "Degree",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Radian = new UnitStructure{
			.unitType = Units::Type::ANGULAR_DISTANCE,
			.singular = "Radian",
			.plural = "Radians",
			.abbreviated = "rad",
			.saveString = "Radian",
			.baseMultiplier = 57.2958,
			.baseOffset = 0.0
		};
		const UnitStructure* const Revolution = new UnitStructure{
			.unitType = Units::Type::ANGULAR_DISTANCE,
			.singular = "Revolution",
			.plural = "Revolutions",
			.abbreviated = "rev",
			.saveString = "Revolution",
			.baseMultiplier = 360.0,
			.baseOffset = 0.0
		};
	}

	namespace Time{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::TIME);
		const UnitStructure* const Nanosecond = new UnitStructure{
			.unitType = Units::Type::TIME,
			.singular = "Nanosecond",
			.plural = "Nanoseconds",
			.abbreviated = "ns",
			.saveString = "Nanosecond",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Microsecond = new UnitStructure{
			.unitType = Units::Type::TIME,
			.singular = "Microsecond",
			.plural = "Microseconds",
			.abbreviated = "µs",
			.saveString = "Microsecond",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Millisecond = new UnitStructure{
			.unitType = Units::Type::TIME,
			.singular = "Millisecond",
			.plural = "Milliseconds",
			.abbreviated = "ms",
			.saveString = "Millisecond",
			.baseMultiplier = 1000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Second = new UnitStructure{
			.unitType = Units::Type::TIME,
			.singular = "Second",
			.plural = "Second",
			.abbreviated = "s",
			.saveString = "Second",
			.baseMultiplier = 1000000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Minute = new UnitStructure{
			.unitType = Units::Type::TIME,
			.singular = "Minute",
			.plural = "Minutes",
			.abbreviated = "min",
			.saveString = "Minute",
			.baseMultiplier = 60000000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Hour = new UnitStructure{
			.unitType = Units::Type::TIME,
			.singular = "Hour",
			.plural = "Hours",
			.abbreviated = "h",
			.saveString = "Hour",
			.baseMultiplier = 3600000000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Day = new UnitStructure{
			.unitType = Units::Type::TIME,
			.singular = "Day",
			.plural = "Days",
			.abbreviated = "d",
			.saveString = "Day",
			.baseMultiplier = 86400000000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Week = new UnitStructure{
			.unitType = Units::Type::TIME,
			.singular = "Week",
			.plural = "Weeks",
			.abbreviated = "w",
			.saveString = "Week",
			.baseMultiplier = 604800000000000.0,
			.baseOffset = 0.0
		};
	}

	namespace Frequency{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::FREQUENCY);
		const UnitStructure* const Hertz = new UnitStructure{
			.unitType = Units::Type::FREQUENCY,
			.singular = "Hertz",
			.plural = "Hertz",
			.abbreviated = "Hz",
			.saveString = "Hertz",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Kilohertz = new UnitStructure{
			.unitType = Units::Type::FREQUENCY,
			.singular = "Kilohertz",
			.plural = "Kilohertz",
			.abbreviated = "KHz",
			.saveString = "Kilohertz",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Megahertz = new UnitStructure{
			.unitType = Units::Type::FREQUENCY,
			.singular = "Megahertz",
			.plural = "Megahertz",
			.abbreviated = "MHz",
			.saveString = "Megahertz",
			.baseMultiplier = 1000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Gigahertz = new UnitStructure{
			.unitType = Units::Type::FREQUENCY,
			.singular = "Gigahertz",
			.plural = "Gigahertz",
			.abbreviated = "GHz",
			.saveString = "Gigahertz",
			.baseMultiplier = 1000000000.0,
			.baseOffset = 0.0
		};
	}

	namespace Voltage{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::VOLTAGE);
		const UnitStructure* const Millivolt = new UnitStructure{
			.unitType = Units::Type::VOLTAGE,
			.singular = "Millivolt",
			.plural = "Millivolts",
			.abbreviated = "mV",
			.saveString = "Millivolt",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Volt = new UnitStructure{
			.unitType = Units::Type::VOLTAGE,
			.singular = "Volt",
			.plural = "Volts",
			.abbreviated = "V",
			.saveString = "Volt",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Kilovolt = new UnitStructure{
			.unitType = Units::Type::VOLTAGE,
			.singular = "Kilovolt",
			.plural = "Kilovolts",
			.abbreviated = "KV",
			.saveString = "Kilovolt",
			.baseMultiplier = 1000000.0,
			.baseOffset = 0.0
		};
	}

	namespace Current{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::CURRENT);
		const UnitStructure* const Milliampere = new UnitStructure{
			.unitType = Units::Type::CURRENT,
			.singular = "Milliampere",
			.plural = "Milliamperes",
			.abbreviated = "mA",
			.saveString = "Milliampere",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Ampere = new UnitStructure{
			.unitType = Units::Type::CURRENT,
			.singular = "Ampere",
			.plural = "Amperes",
			.abbreviated = "A",
			.saveString = "Amperes",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
	}

	namespace Resistance{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::RESISTANCE);
		const UnitStructure* const Ohm = new UnitStructure{
			.unitType = Units::Type::RESISTANCE,
			.singular = "Ohm",
			.plural = "Ohms",
			.abbreviated = "Ohm",
			.saveString = "Ohm",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const KiloOhm = new UnitStructure{
			.unitType = Units::Type::RESISTANCE,
			.singular = "KiloOhm",
			.plural = "KiloOhms",
			.abbreviated = "A", //03A9 or 03C9
			.saveString = "KiloOhms",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
	}

	namespace Power{
	UNIT_TYPE_FUNCTIONS(Type::POWER);
		const UnitStructure* const Watt = new UnitStructure{
			.unitType = Units::Type::POWER,
			.singular = "Watt",
			.plural = "Watts",
			.abbreviated = "W",
			.saveString = "Watt",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const KiloWatt = new UnitStructure{
			.unitType = Units::Type::POWER,
			.singular = "KiloWatt",
			.plural = "KiloWatts",
			.abbreviated = "KW",
			.saveString = "KiloWatt",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
	};

	namespace Temperature{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::TEMPERATURE);
		const UnitStructure* const Kelvin = new UnitStructure{
			.unitType = Units::Type::TEMPERATURE,
			.singular = "Kelvin",
			.plural = "Kelvin",
			.abbreviated = "K",
			.saveString = "Kelvin",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Celsius = new UnitStructure{
			.unitType = Units::Type::TEMPERATURE,
			.singular = "Degree Celsius",
			.plural = "Degrees Celsius",
			.abbreviated = "°C",
			.saveString = "Celsius",
			.baseMultiplier = 1.0,
			.baseOffset = 273.15
		};
		const UnitStructure* const Fahrenheit = new UnitStructure{
			.unitType = Units::Type::TEMPERATURE,
			.singular = "Degree Farenheit",
			.plural = "Degrees Farenheit",
			.abbreviated = "°F",
			.saveString = "Farenheit",
			.baseMultiplier = 5.0 / 9.0,
			.baseOffset = 273.15 - 32.0 * 5.0 / 9.0
		};
	}

	namespace Mass{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::MASS);
		const UnitStructure* const Gram = new UnitStructure{
			.unitType = Units::Type::MASS,
			.singular = "Gram",
			.plural = "Grams",
			.abbreviated = "g",
			.saveString = "Gram",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Kilogram = new UnitStructure{
			.unitType = Units::Type::MASS,
			.singular = "Kilogram",
			.plural = "Kilograms",
			.abbreviated = "Kg",
			.saveString = "Kilogram",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Ton = new UnitStructure{
			.unitType = Units::Type::MASS,
			.singular = "Ton",
			.plural = "Tons",
			.abbreviated = "T",
			.saveString = "Ton",
			.baseMultiplier = 1000000.0,
			.baseOffset = 0.0
		};
	}

	namespace Data{
	UNIT_TYPE_FUNCTIONS_IMPLEMENTATION(Type::DATA);
		const UnitStructure* const Bit = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "Bit",
			.plural = "Bits",
			.abbreviated = "b",
			.saveString = "Bit",
			.baseMultiplier = 1.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const Byte = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "Byte",
			.plural = "Bytes",
			.abbreviated = "B",
			.saveString = "Byte",
			.baseMultiplier = 8.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const KiloBit = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "KiloBit",
			.plural = "KiloBits",
			.abbreviated = "Kb",
			.saveString = "KiloBit",
			.baseMultiplier = 1000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const KiloByte = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "KiloByte",
			.plural = "KiloBytes",
			.abbreviated = "KB",
			.saveString = "KiloByte",
			.baseMultiplier = 8000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const MegaBit = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "MegaBit",
			.plural = "MegaBits",
			.abbreviated = "Mb",
			.saveString = "MegaBit",
			.baseMultiplier = 1000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const MegaByte = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "MegaByte",
			.plural = "MegaBytes",
			.abbreviated = "MB",
			.saveString = "MegaByte",
			.baseMultiplier = 8000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const GigaBit = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "GigaBit",
			.plural = "GigaBits",
			.abbreviated = "Gb",
			.saveString = "GigaBit",
			.baseMultiplier = 1000000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const GigaByte = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "GigaByte",
			.plural = "GigaBytes",
			.abbreviated = "GB",
			.saveString = "GigaByte",
			.baseMultiplier = 8000000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const TeraBit = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "TeraBit",
			.plural = "TeraBits",
			.abbreviated = "Tb",
			.saveString = "TeraBit",
			.baseMultiplier = 1000000000000.0,
			.baseOffset = 0.0
		};
		const UnitStructure* const TeraByte = new UnitStructure{
			.unitType = Units::Type::DATA,
			.singular = "TeraByte",
			.plural = "TeraBytes",
			.abbreviated = "TB",
			.saveString = "TeraByte",
			.baseMultiplier = 8000000000000.0,
			.baseOffset = 0.0
		};
	};



	std::vector<UnitTypeStructure>& getAllUnits(){
		static std::vector<UnitTypeStructure> allUnits = {
			UnitTypeStructure{
				.unitType = Units::Type::NONE,
				.units = {
					None::None
				}
			},
			UnitTypeStructure{
				.unitType = Units::Type::LINEAR_DISTANCE,
				.units = {
					LinearDistance::Millimeter,
					LinearDistance::Centimeter,
					LinearDistance::Meter
				}
			},
			UnitTypeStructure{
				.unitType = Units::Type::ANGULAR_DISTANCE,
				.units = {
					AngularDistance::Degree,
					AngularDistance::Radian,
					AngularDistance::Revolution
				}
			},
			UnitTypeStructure{
				.unitType = Units::Type::TIME,
				.units = {
					Time::Nanosecond,
					Time::Microsecond,
					Time::Millisecond,
					Time::Second,
					Time::Minute,
					Time::Hour,
					Time::Day,
					Time::Week
				}
			},
			UnitTypeStructure{
				.unitType = Units::Type::FREQUENCY,
				.units = {
					Frequency::Hertz,
					Frequency::Kilohertz,
					Frequency::Megahertz,
					Frequency::Gigahertz
				}
			},
			UnitTypeStructure{
				.unitType = Units::Type::VOLTAGE,
				.units = {
					Voltage::Millivolt,
					Voltage::Volt,
					Voltage::Kilovolt
				}
			},
			UnitTypeStructure{
				.unitType = Units::Type::CURRENT,
				.units = {
					Current::Milliampere,
					Current::Ampere
				}
			},
			UnitTypeStructure{
				.unitType = Units::Type::TEMPERATURE,
				.units = {
					Temperature::Kelvin,
					Temperature::Celsius,
					Temperature::Fahrenheit
				}
			},
			UnitTypeStructure{
				.unitType = Units::Type::MASS,
				.units = {
					Mass::Gram,
					Mass::Kilogram,
					Mass::Ton
				}
			}
		};
		return allUnits;
	}






	bool isValidSaveString(const char* saveString){
		for(auto& unitType : getAllUnits()){
			for(auto& unit : unitType.units){
				if(strcmp(saveString, unit->saveString) == 0) return true;
			}
		}
		return false;
	}

	const UnitStructure* const fromSaveString(const char* saveString){
		for(auto& unitType : getAllUnits()){
			for(auto& unit : unitType.units){
				if(strcmp(saveString, unit->saveString) == 0) return unit;
			}
		}
		return nullptr;
	}

	double convert(double input, const UnitStructure* inputUnit, const UnitStructure* outputUnit){
		double baseToInputMultiplier = inputUnit->baseMultiplier;
		double baseToInputOffset = inputUnit->baseOffset;
		double baseToOutputMultiplier = outputUnit->baseMultiplier;
		double baseToOutputOffset = outputUnit->baseOffset;
		double multiplier = baseToInputMultiplier / baseToOutputMultiplier;
		double offset = (baseToInputOffset - baseToOutputOffset) / baseToOutputMultiplier;
		return input * multiplier + offset;
	}


	void convert(const std::vector<double>& input, std::vector<double>& output, const UnitStructure* inputUnit, const UnitStructure* outputUnit){
		double baseToInputMultiplier = inputUnit->baseMultiplier;
		double baseToInputOffset = inputUnit->baseOffset;
		double baseToOutputMultiplier = outputUnit->baseMultiplier;
		double baseToOutputOffset = outputUnit->baseOffset;
		double multiplier = baseToInputMultiplier / baseToOutputMultiplier;
		double offset = (baseToInputOffset - baseToOutputOffset) / baseToOutputMultiplier;
		output.resize(input.size());
		for(int i = 0; i < input.size(); i++) output[i] = input[i] * multiplier + offset;
	}


}


