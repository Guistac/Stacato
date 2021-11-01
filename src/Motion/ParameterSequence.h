#pragma once

class AnimatableParameter;
class CurvePoint;

#include "Motion/Curve/Curve.h"
#include "Motion/Machine/AnimatableParameter.h"
#include "Motion/MotionTypes.h"

class ParameterSequence{
public:

	ParameterSequence(std::shared_ptr<AnimatableParameter>& param) : parameter(param) {
		curve = std::make_shared<Motion::Curve>();
		switch (parameter->dataType) {
			case ParameterDataType::BOOLEAN_PARAMETER:
			case ParameterDataType::INTEGER_PARAMETER:
			case ParameterDataType::STATE_PARAMETER:
				curve->interpolationType = InterpolationType::Type::STEP;
				sequenceType = SequenceType::Type::STEP_MOVE;
				break;
			case ParameterDataType::REAL_PARAMETER:
			case ParameterDataType::VECTOR_2D_PARAMETER:
			case ParameterDataType::VECTOR_3D_PARAMETER:
				curve->interpolationType = InterpolationType::Type::LINEAR;
				sequenceType = SequenceType::Type::TIMED_MOVE;
				break;
			case ParameterDataType::KINEMATIC_POSITION_CURVE:
			case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
				curve->interpolationType = InterpolationType::Type::TRAPEZOIDAL;
				sequenceType = SequenceType::Type::TIMED_MOVE;
				break;
		}
		origin.type = parameter->dataType;
		target.type = parameter->dataType;
		if(parameter->dataType == ParameterDataType::STATE_PARAMETER){
			origin.stateValues = parameter->stateParameterValues;
			target.stateValues = parameter->stateParameterValues;
			origin.stateValue = &(*parameter->stateParameterValues)[0];
			target.stateValue = &(*parameter->stateParameterValues)[0];
		}
	}

	std::shared_ptr<AnimatableParameter> parameter;
	std::shared_ptr<Motion::Curve> curve;

	SequenceType::Type sequenceType;
	bool originIsPreviousTarget = false;
	bool rampsAreEqual = false;

	AnimatableParameterValue origin;
	AnimatableParameterValue target;
	double velocityConstraint = 0.0;
	double timeConstraint = 0.0;
	double timeOffset = 0.0;
	double rampIn = 0.0;
	double rampOut = 0.0;


	void sequenceTypeSelectorGui();
	void interpolationTypeSelectorGui();
	void chainPreviousTargetCheckboxGui();
	void originInputGui();
	void targetInputGui();
	void constraintInputGui();
	void timeOffsetInputGui();
	void rampIntInputGui();
	void rampOutInputGui();
	void equalRampsCheckboxGui();

	void setInterpolationType(InterpolationType::Type t);
	void setSequenceType(SequenceType::Type t);
};
