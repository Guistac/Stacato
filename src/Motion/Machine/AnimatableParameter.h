#pragma once

#include "Motion/MotionTypes.h"

class Machine;

struct StateParameterValue {
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};

struct AnimatableParameterValue {
public:

	AnimatableParameterValue() {}
	AnimatableParameterValue(ParameterDataType t) { type = t; }
	AnimatableParameterValue(bool v) { boolValue = v; type = ParameterDataType::BOOLEAN_PARAMETER; }
	AnimatableParameterValue(int v) { integerValue = v; type = ParameterDataType::INTEGER_PARAMETER; }
	AnimatableParameterValue(StateParameterValue* v, std::vector<StateParameterValue>* o) { stateValue = v; stateValues = o; type = ParameterDataType::STATE_PARAMETER; }
	AnimatableParameterValue(double v) { realValue = v; type = ParameterDataType::REAL_PARAMETER; }
	AnimatableParameterValue(glm::vec2 v) { vector2value = v; type = ParameterDataType::VECTOR_2D_PARAMETER; }
	AnimatableParameterValue(glm::vec3 v) { vector3value = v; type = ParameterDataType::VECTOR_2D_PARAMETER; }
	ParameterDataType type;

	void inputFieldGui();

	bool boolValue = false;
	int integerValue = false;
	StateParameterValue* stateValue = nullptr;
	std::vector<StateParameterValue>* stateValues = nullptr;
	double realValue = false;
	glm::vec2 vector2value;
	glm::vec3 vector3value;
};


class AnimatableParameter {
public:

	AnimatableParameter(const char* nm, std::shared_ptr<Machine> mach, ParameterDataType datat) : machine(mach), dataType(datat) {
		strcpy(name, nm);
		lowLimit.type = dataType;
		highLimit.type = dataType;
		velocityLimit.type = dataType;
		accelerationLimit.type = dataType;
	}

	AnimatableParameter(const char* nm, std::shared_ptr<Machine> mach, std::vector<StateParameterValue>* stateValues) : machine(mach), stateParameterValues(stateValues) {
		strcpy(name, nm);
		dataType = ParameterDataType::STATE_PARAMETER;
	}

	char name[128];
	ParameterDataType dataType;
	std::shared_ptr<Machine> machine;

	//general parameter limits
	AnimatableParameterValue lowLimit;
	AnimatableParameterValue highLimit;

	//limits for trapezoidal profiles
	AnimatableParameterValue velocityLimit;
	AnimatableParameterValue accelerationLimit;

	std::vector<InterpolationType::Type> getCompatibleInterpolationTypes();

	std::vector<StateParameterValue>* stateParameterValues;

	//TODO: parameter units
};