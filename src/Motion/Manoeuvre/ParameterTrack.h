#pragma once

class CurvePoint;

#include "Motion/AnimatableParameter.h"

#include <tinyxml2.h>

struct SequenceType {
	enum class Type {
		NO_MOVE,
		TIMED_MOVE,
		ANIMATED_MOVE
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};

std::vector<SequenceType>& getSequenceTypes();
SequenceType* getSequenceType(SequenceType::Type t);
SequenceType* getSequenceType(const char* saveName);

class ParameterTrack{
public:

	ParameterTrack() {}
	ParameterTrack(const ParameterTrack& original);
	ParameterTrack(std::shared_ptr<AnimatableParameter>& param);

	void initialize();

	std::shared_ptr<AnimatableParameter> parameter;

	bool b_priming = false;
	void prime();
	void cancelPriming();
	bool isPrimed();
	float getPrimingProgress();

	double playbackStartTime_seconds;
	void getParameterValueAtPlaybackTime(AnimatableParameterValue& output);

	SequenceType::Type sequenceType;
	InterpolationType::Type interpolationType;
	void setInterpolationType(InterpolationType::Type t);
	void setSequenceType(SequenceType::Type t);

	std::vector<std::shared_ptr<Motion::Curve>> curves;
	void updateCurves();
	void updateParametersAfterCurveEdit();
	int getCurveCount();
	double getLength_seconds();

	//points for simple movement sequences (not manual animated moves)
	std::vector<std::shared_ptr<Motion::ControlPoint>> startPoints;
	std::vector<std::shared_ptr<Motion::ControlPoint>> endPoints;

	//parameters for timed sequences
	bool originIsPreviousTarget = false;
	bool targetIsNextOrigin = false;
	AnimatableParameterValue origin;
	AnimatableParameterValue target;
	double movementTime = 1.0;
	double timeOffset = 0.0;
	double rampIn = 0.1;
	double rampOut = 0.1;
	bool rampsAreEqual = true;

	//gui stuff
	bool sequenceTypeSelectorGui();
	bool interpolationTypeSelectorGui();
	bool originIsPreviousTargetCheckboxGui();
	bool targetIsNextOriginCheckboxGui();
	bool originInputGui();
	bool targetInputGui();
	bool timeInputGui();
	bool timeOffsetInputGui();
	bool rampIntInputGui();
	bool rampOutInputGui();
	bool equalRampsCheckboxGui();

	void drawCurves(double startTime, double endTime);
	bool drawControlPoints();

	bool save(tinyxml2::XMLElement* trackXML);
	bool load(tinyxml2::XMLElement* trackXML);
};
