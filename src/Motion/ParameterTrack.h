#pragma once

class AnimatableParameter;
class CurvePoint;

#include "Motion/Curve/Curve.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/MotionTypes.h"

class ParameterTrack{
public:

	ParameterTrack(const ParameterTrack& original);

	ParameterTrack(std::shared_ptr<AnimatableParameter>& param);

	void initialize();

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
