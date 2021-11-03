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
	void updateParametersAfterCurveEdit();
	int getCurveCount();

	//points for simple movement sequences (not manual animated moves)
	std::vector<std::shared_ptr<Motion::ControlPoint>> startPoints;
	std::vector<std::shared_ptr<Motion::ControlPoint>> endPoints;

	bool originIsPreviousTarget = false;
	AnimatableParameterValue origin;
	AnimatableParameterValue target;
	double velocityConstraint = 0.5;
	double timeConstraint = 1.0;
	double timeOffset = 0.0;
	double rampIn = 0.1;
	double rampOut = 0.1;
	bool rampsAreEqual = true;

	bool sequenceTypeSelectorGui();
	bool interpolationTypeSelectorGui();
	bool chainPreviousTargetCheckboxGui();
	bool originInputGui();
	bool targetInputGui();
	bool constraintInputGui();
	bool timeOffsetInputGui();
	bool rampIntInputGui();
	bool rampOutInputGui();
	bool equalRampsCheckboxGui();

	void drawCurves();
	bool drawControlPoints();
};
