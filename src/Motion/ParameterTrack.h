#pragma once

class AnimatableParameter;
class CurvePoint;

#include "Motion/Curve/Curve.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/MotionTypes.h"

class ParameterTrack{
public:

	ParameterTrack(std::shared_ptr<AnimatableParameter>& param) : parameter(param) {
		switch (parameter->dataType) {
			case ParameterDataType::BOOLEAN_PARAMETER:
			case ParameterDataType::INTEGER_PARAMETER:
			case ParameterDataType::STATE_PARAMETER:
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::Point>());
				endPoints.push_back(std::make_shared<Motion::Point>());
				sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
				interpolationType = InterpolationType::Type::STEP;
				sequenceType = SequenceType::Type::STEP_MOVE;
				break;
			case ParameterDataType::REAL_PARAMETER:
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::Point>());
				endPoints.push_back(std::make_shared<Motion::Point>());
				sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
			case ParameterDataType::VECTOR_2D_PARAMETER:
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::Point>());
				endPoints.push_back(std::make_shared<Motion::Point>());
				sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
			case ParameterDataType::VECTOR_3D_PARAMETER:
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::Point>());
				endPoints.push_back(std::make_shared<Motion::Point>());
				sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
				interpolationType = InterpolationType::Type::LINEAR;
				sequenceType = SequenceType::Type::TIMED_MOVE;
				break;
			case ParameterDataType::KINEMATIC_POSITION_CURVE:
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::Point>());
				endPoints.push_back(std::make_shared<Motion::Point>());
				sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
			case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::Point>());
				endPoints.push_back(std::make_shared<Motion::Point>());
				sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
			case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::Point>());
				endPoints.push_back(std::make_shared<Motion::Point>());
				sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
				interpolationType = InterpolationType::Type::TRAPEZOIDAL;
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

	SequenceType::Type sequenceType;
	InterpolationType::Type interpolationType;
	void setInterpolationType(InterpolationType::Type t);
	void setSequenceType(SequenceType::Type t);
	std::vector<SequenceType::Type> getCompatibleSequenceTypes();

	std::vector<std::shared_ptr<Motion::Curve>> curves;
	void updateCurves();

	//points for simple movement sequences (not manual animated moves)
	std::vector<std::shared_ptr<Motion::Point>> startPoints;
	std::vector<std::shared_ptr<Motion::Point>> endPoints;
	std::vector<std::shared_ptr<Motion::Interpolation>> sequenceInterpolations;


	bool originIsPreviousTarget = false;
	AnimatableParameterValue origin;
	AnimatableParameterValue target;
	double velocityConstraint = 0.0;
	double timeConstraint = 0.0;
	double timeOffset = 0.0;
	double rampIn = 0.0;
	double rampOut = 0.0;
	bool rampsAreEqual = false;

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
};
