#include "UnitT.h"

namespace newUnits{

struct UnitTypeStructure{
	newUnits::Type unitType;
	const std::vector<const UnitStructure> units;
};

const std::vector<UnitTypeStructure> allUnits = {
	UnitTypeStructure{
		.unitType = newUnits::Type::LINEAR_DISTANCE,
		.units = {
			UnitStructure{
				.enumerator = newUnits::Enumerator::MILLIMETER,
				.unitType = newUnits::Type::LINEAR_DISTANCE,
				.singular = "Millimeter",
				.plural = "Millimeters",
				.abbreviated = "mm",
				.saveString = "Millimeter",
				.baseMultiplier = 1.0,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::CENTIMETER,
				.unitType = newUnits::Type::LINEAR_DISTANCE,
				.singular = "Centimeter",
				.plural = "Centimeter",
				.abbreviated = "cm",
				.saveString = "Centimeter",
				.baseMultiplier = 10.0,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::METER,
				.unitType = newUnits::Type::LINEAR_DISTANCE,
				.singular = "Meter",
				.plural = "Meters",
				.abbreviated = "m",
				.saveString = "Meter",
				.baseMultiplier = 1000.0,
				.baseOffset = 0.0
			}
		}
	},
	
	UnitTypeStructure{
		.unitType = newUnits::Type::ANGULAR_DISTANCE,
		.units = {
			UnitStructure{
				.enumerator = newUnits::Enumerator::DEGREE,
				.unitType = newUnits::Type::ANGULAR_DISTANCE,
				.singular = "Degree",
				.plural = "Degrees",
				.abbreviated = "\xC2\xB0",
				.saveString = "Degree",
				.baseMultiplier = 1.0,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::RADIAN,
				.unitType = newUnits::Type::ANGULAR_DISTANCE,
				.singular = "Radian",
				.plural = "Radians",
				.abbreviated = "rad",
				.saveString = "Radian",
				.baseMultiplier = 57.2958,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::REVOLUTION,
				.unitType = newUnits::Type::ANGULAR_DISTANCE,
				.singular = "Revolution",
				.plural = "Revolutions",
				.abbreviated = "rev",
				.saveString = "Revolution",
				.baseMultiplier = 360.0,
				.baseOffset = 0.0
			}
		}
	},
	
	UnitTypeStructure{
		.unitType = newUnits::Type::TIME,
		.units = {
			UnitStructure{
			   .enumerator = newUnits::Enumerator::NANOSECOND,
			   .unitType = newUnits::Type::TIME,
			   .singular = "Nanosecond",
			   .plural = "Nanoseconds",
			   .abbreviated = "ns",
			   .saveString = "Nanosecond",
			   .baseMultiplier = 1.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::MICROSECOND,
			   .unitType = newUnits::Type::TIME,
			   .singular = "Microsecond",
			   .plural = "Microseconds",
			   .abbreviated = "µs",
			   .saveString = "Microsecond",
			   .baseMultiplier = 1000.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::MILLISECOND,
			   .unitType = newUnits::Type::TIME,
			   .singular = "Millisecond",
			   .plural = "Milliseconds",
			   .abbreviated = "ms",
			   .saveString = "Millisecond",
			   .baseMultiplier = 1000000.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::SECOND,
			   .unitType = newUnits::Type::TIME,
			   .singular = "Second",
			   .plural = "Second",
			   .abbreviated = "s",
			   .saveString = "Second",
			   .baseMultiplier = 1000000000.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::MINUTE,
			   .unitType = newUnits::Type::TIME,
			   .singular = "Minute",
			   .plural = "Minutes",
			   .abbreviated = "min",
			   .saveString = "Minute",
			   .baseMultiplier = 60000000000.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::HOUR,
			   .unitType = newUnits::Type::TIME,
			   .singular = "Hour",
			   .plural = "Hours",
			   .abbreviated = "h",
			   .saveString = "Hour",
			   .baseMultiplier = 3600000000000.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::DAY,
			   .unitType = newUnits::Type::TIME,
			   .singular = "Day",
			   .plural = "Days",
			   .abbreviated = "d",
			   .saveString = "Day",
			   .baseMultiplier = 86400000000000.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::WEEK,
			   .unitType = newUnits::Type::TIME,
			   .singular = "Week",
			   .plural = "Weeks",
			   .abbreviated = "w",
			   .saveString = "Week",
			   .baseMultiplier = 604800000000000.0,
			   .baseOffset = 0.0
			}
		}
	},
	
	UnitTypeStructure{
		.unitType = newUnits::Type::FREQUENCY,
		.units = {
			UnitStructure{
			   .enumerator = newUnits::Enumerator::HERTZ,
			   .unitType = newUnits::Type::FREQUENCY,
			   .singular = "Hertz",
			   .plural = "Hertz",
			   .abbreviated = "Hz",
			   .saveString = "Hertz",
			   .baseMultiplier = 1.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::KILOHERTZ,
			   .unitType = newUnits::Type::FREQUENCY,
			   .singular = "Kilohertz",
			   .plural = "Kilohertz",
			   .abbreviated = "KHz",
			   .saveString = "Kilohertz",
			   .baseMultiplier = 1000.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::MEGAHERTZ,
			   .unitType = newUnits::Type::FREQUENCY,
			   .singular = "Megahertz",
			   .plural = "Megahertz",
			   .abbreviated = "MHz",
			   .saveString = "Megahertz",
			   .baseMultiplier = 1000000.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::GIGAHERTZ,
			   .unitType = newUnits::Type::FREQUENCY,
			   .singular = "Gigahertz",
			   .plural = "Gigahertz",
			   .abbreviated = "GHz",
			   .saveString = "Gigahertz",
			   .baseMultiplier = 1000000000.0,
			   .baseOffset = 0.0
			}
		}
	},
	
	UnitTypeStructure{
		.unitType = newUnits::Type::VOLTAGE,
		.units = {
			UnitStructure{
				.enumerator = newUnits::Enumerator::MILLIVOLT,
				.unitType = newUnits::Type::VOLTAGE,
				.singular = "Millivolt",
				.plural = "Millivolts",
				.abbreviated = "mV",
				.saveString = "Millivolt",
				.baseMultiplier = 1.0,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::VOLT,
				.unitType = newUnits::Type::VOLTAGE,
				.singular = "Volt",
				.plural = "Volts",
				.abbreviated = "V",
				.saveString = "Volt",
				.baseMultiplier = 1000.0,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::KILOVOLT,
				.unitType = newUnits::Type::VOLTAGE,
				.singular = "Kilovolt",
				.plural = "Kilovolts",
				.abbreviated = "KV",
				.saveString = "Kilovolt",
				.baseMultiplier = 1000000.0,
				.baseOffset = 0.0
			}
		}
	},
	
	UnitTypeStructure{
		.unitType = newUnits::Type::CURRENT,
		.units = {
			UnitStructure{
				.enumerator = newUnits::Enumerator::MILLIAMPERE,
				.unitType = newUnits::Type::CURRENT,
				.singular = "Milliampere",
				.plural = "Milliamperes",
				.abbreviated = "mA",
				.saveString = "Milliampere",
				.baseMultiplier = 1.0,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::AMPERE,
				.unitType = newUnits::Type::CURRENT,
				.singular = "Ampere",
				.plural = "Amperes",
				.abbreviated = "A",
				.saveString = "Amperes",
				.baseMultiplier = 1000.0,
				.baseOffset = 0.0
			}
		}
	},
	
	UnitTypeStructure{
		.unitType = newUnits::Type::TEMPERATURE,
		.units = {
			UnitStructure{
			   .enumerator = newUnits::Enumerator::KELVIN,
			   .unitType = newUnits::Type::TEMPERATURE,
			   .singular = "Kelvin",
			   .plural = "Kelvin",
			   .abbreviated = "K",
			   .saveString = "Kelvin",
			   .baseMultiplier = 1.0,
			   .baseOffset = 0.0
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::CELSIUS,
			   .unitType = newUnits::Type::TEMPERATURE,
			   .singular = "Degree Celsius",
			   .plural = "Degrees Celsius",
			   .abbreviated = "°C",
			   .saveString = "Celsius",
			   .baseMultiplier = 1.0,
			   .baseOffset = 273.15
			},
			UnitStructure{
			   .enumerator = newUnits::Enumerator::FAHRENHEIT,
			   .unitType = newUnits::Type::TEMPERATURE,
			   .singular = "Degree Farenheit",
			   .plural = "Degrees Farenheit",
			   .abbreviated = "°F",
			   .saveString = "Farenheit",
			   .baseMultiplier = 5.0 / 9.0,
			   .baseOffset = 273.15 - 32.0 * 5.0 / 9.0
			}
		}
	},
	
	UnitTypeStructure{
		.unitType = newUnits::Type::MASS,
		.units = {
			UnitStructure{
				.enumerator = newUnits::Enumerator::GRAM,
				.unitType = newUnits::Type::MASS,
				.singular = "Gram",
				.plural = "Grams",
				.abbreviated = "g",
				.saveString = "Gram",
				.baseMultiplier = 1.0,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::KILOGRAM,
				.unitType = newUnits::Type::MASS,
				.singular = "Kilogram",
				.plural = "Kilograms",
				.abbreviated = "Kg",
				.saveString = "Kilogram",
				.baseMultiplier = 1000.0,
				.baseOffset = 0.0
			},
			UnitStructure{
				.enumerator = newUnits::Enumerator::TON,
				.unitType = newUnits::Type::MASS,
				.singular = "Ton",
				.plural = "Tons",
				.abbreviated = "T",
				.saveString = "Ton",
				.baseMultiplier = 1000000.0,
				.baseOffset = 0.0
			}
		}
	}
	
};




const UnitStructure* const getUnit(newUnits::Enumerator unitEnumerator){
	for(auto& unitType : allUnits){
		for(auto& unit : unitType.units){
			if(unit.enumerator == unitEnumerator) return &unit;
		}
	}
	return nullptr;
}

const std::vector<const UnitStructure>& getUnits(newUnits::Type queriedType){
	for(auto& unitType : allUnits){
		if(unitType.unitType == queriedType) return unitType.units;
	}
}
 
newUnits::Type getUnitType(newUnits::Enumerator unitEnumerator){
	return getUnit(unitEnumerator)->unitType;
}

bool isValidSaveString(const char* saveString){
	for(auto& unitType : allUnits){
		for(auto& unit : unitType.units){
			if(strcmp(saveString, unit.saveString) == 0) return true;
		}
	}
	return false;
}

bool isValidSaveString(const char* saveString, newUnits::Type queriedUnitType){
	for(auto& unitType : allUnits){
		if(unitType.unitType != queriedUnitType) continue;
		for(auto& unit : unitType.units){
			if(strcmp(saveString, unit.saveString) == 0) return true;
		}
	}
	return false;
}

const UnitStructure* const getUnitFromSaveString(const char* saveString){
	for(auto& unitType : allUnits){
		for(auto& unit : unitType.units){
			if(strcmp(saveString, unit.saveString) == 0) return &unit;
		}
	}
	return nullptr;
}

double convert(double input, newUnits::Enumerator inputUnit, newUnits::Enumerator outputUnit){
	const UnitStructure* inUnit = getUnit(inputUnit);
	const UnitStructure* outUnit = getUnit(outputUnit);
	double baseToInputMultiplier = inUnit->baseMultiplier;
	double baseToInputOffset = inUnit->baseOffset;
	double baseToOutputMultiplier = outUnit->baseMultiplier;
	double baseToOutputOffset = outUnit->baseOffset;
	double multiplier = baseToInputMultiplier / baseToOutputMultiplier;
	double offset = (baseToInputOffset - baseToOutputOffset) / baseToOutputMultiplier;
	return input * multiplier + offset;
}


void convert(const std::vector<double>& input, std::vector<double>& output, newUnits::Enumerator inputUnit, newUnits::Enumerator outputUnit){
	
	const UnitStructure* inUnit = getUnit(inputUnit);
	const UnitStructure* outUnit = getUnit(outputUnit);
	double baseToInputMultiplier = inUnit->baseMultiplier;
	double baseToInputOffset = inUnit->baseOffset;
	double baseToOutputMultiplier = outUnit->baseMultiplier;
	double baseToOutputOffset = outUnit->baseOffset;
	double multiplier = baseToInputMultiplier / baseToOutputMultiplier;
	double offset = (baseToInputOffset - baseToOutputOffset) / baseToOutputMultiplier;
	output.resize(input.size());
	for(int i = 0; i < input.size(); i++) output[i] = input[i] * multiplier + offset;
}

}



