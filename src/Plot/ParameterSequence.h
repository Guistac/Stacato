#pragma once

class AnimatableParameter;
class CurvePoint;

#include "Motion/Curves/Curve.h"
#include "Motion/Machine/AnimatableParameter.h"

struct SequenceType {
	enum class Type {
		SIMPLE_TIMED_MOVE,
		SIMPLE_VELOCITY_MOVE,
		COMPLEX_ANIMATED_MOVE
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};

std::vector<SequenceType>& getSequenceTypes();
SequenceType* getSequenceType(SequenceType::Type t);
SequenceType* getSequenceType(const char* saveName);


class ParameterSequence{
public:

	ParameterSequence(std::shared_ptr<AnimatableParameter>& param, SequenceType::Type t) : parameter(param), type(t) {
		curve = std::make_shared<Motion::Curve>();
		switch (parameter->dataType) {
		case AnimatableParameter::Type::BOOLEAN_PARAMETER:
		case AnimatableParameter::Type::INTEGER_PARAMETER:
			curve->interpolationType = Motion::InterpolationType::Type::NONE;
			break;
		case AnimatableParameter::Type::REAL_PARAMETER:
		case AnimatableParameter::Type::VECTOR_2D_PARAMETER:
		case AnimatableParameter::Type::VECTOR_3D_PARAMETER:
			curve->interpolationType = Motion::InterpolationType::Type::LINEAR;
			break;
		case AnimatableParameter::Type::KINEMATIC_POSITION_CURVE:
		case AnimatableParameter::Type::KINEMATIC_2D_POSITION_CURVE:
		case AnimatableParameter::Type::KINEMATIC_3D_POSITION_CURVE:
			curve->interpolationType = Motion::InterpolationType::Type::KINEMATIC;
			break;
		}
	}

	std::shared_ptr<AnimatableParameter> parameter;
	std::shared_ptr<Motion::Curve> curve;

	SequenceType::Type type;
	double target = 0.0;
	double constraint = 0.0;
	double offsetTime = 0.0;
	double rampIn = 0.0;
	double rampOut = 0.0;
	bool rampEqual = true;
};
