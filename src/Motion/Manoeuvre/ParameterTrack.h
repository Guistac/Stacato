#pragma once

class CurvePoint;
class Manoeuvre;

#include "Motion/AnimatableParameter.h"

#include <tinyxml2.h>

struct SequenceType {
	enum class Type {
		CONSTANT,
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

class ParameterTrack : public std::enable_shared_from_this<ParameterTrack> {
public:

	ParameterTrack(std::shared_ptr<Manoeuvre> m) : parentManoeuvre(m) {}
	ParameterTrack(const ParameterTrack& original);
	ParameterTrack(std::shared_ptr<AnimatableParameter>& param, std::shared_ptr<Manoeuvre> m);

	void initialize();

	std::shared_ptr<Manoeuvre> parentManoeuvre = nullptr;
	std::shared_ptr<AnimatableParameter> parameter;

	bool b_priming = false;
	void rapidToStart();
	void rapidToEnd();
	void rapidToPlaybackPosition();
	void cancelRapid();
	float getRapidProgress();

	bool b_valid = false;

	//AnimatableParameterValue primingTarget;
	bool isPrimedToStart(); //is axis at the same location as the playback position
	bool isPrimedToEnd(); //is axis at the same location as the playback position
	bool isPrimedToPlaybackPosition();

	double playbackPosition_seconds;
	void getParameterValueAtPlaybackTime(AnimatableParameterValue& output);

	SequenceType::Type sequenceType;
	InterpolationType::Type interpolationType;
	void setInterpolationType(InterpolationType::Type t);
	void setSequenceType(SequenceType::Type t);

	std::vector<std::shared_ptr<Motion::Curve>> curves;
	void refreshAfterParameterEdit();
	void refreshAfterChainedDependenciesRefresh();
	void refreshAfterCurveEdit();
	int getCurveCount();
	double getLength_seconds();

	//points for simple movement sequences (not manual animated moves)
	std::vector<std::shared_ptr<Motion::ControlPoint>> startPoints;
	std::vector<std::shared_ptr<Motion::ControlPoint>> endPoints;

	//parameters for timed sequences
	bool originIsPreviousTarget = false;
	bool targetIsNextOrigin = false;
	std::shared_ptr<ParameterTrack> previousChainedMasterTrack = nullptr;
	std::shared_ptr<ParameterTrack> nextChainedMasterTrack = nullptr;
	std::shared_ptr<ParameterTrack> previousChainedSlaveTrack = nullptr;
	std::shared_ptr<ParameterTrack> nextChainedSlaveTrack = nullptr;
	bool isNextChainingMasterMissing();
	bool isPreviousChainingMasterMissing();
	bool isPreviousCrossChained();
	bool isNextCrossChained();

	void refreshPreviousChainedCurvePoints();
	void refreshNextChainedCurvePoints();
	std::vector<std::vector<Motion::CurvePoint>> nextChainedCurvePoints;
	std::vector<std::vector<Motion::CurvePoint>> previousChainedCurvePoints;

	AnimatableParameterValue origin;
	AnimatableParameterValue target;
	AnimatableParameterValue& getOrigin();
	AnimatableParameterValue& getTarget();

	double movementTime = 1.0;
	double timeOffset = 0.0;
	double rampIn = 0.1;
	double rampOut = 0.1;
	bool rampsAreEqual = true;

	std::shared_ptr<ParameterTrack> parentParameterTrack = nullptr;
	std::vector<std::shared_ptr<ParameterTrack>> childParameterTracks;
	bool hasChildParameterTracks() { return !childParameterTracks.empty(); }
	bool hasParentParameterTrack() { return parentParameterTrack != nullptr; }

	//gui stuff
	bool sequenceTypeSelectorGui();
	bool interpolationTypeSelectorGui();
	bool chainPreviousGui(float width);
	bool chainNextGui(float width);
	bool originInputGui(float width);
	bool targetInputGui(float width);
	bool timeInputGui();
	bool timeOffsetInputGui();
	bool rampInputGui(float width);

	void drawCurves(double startTime, double endTime);
	bool drawControlPoints();
	void drawChainedCurves();

	bool save(tinyxml2::XMLElement* trackXML);
	bool load(tinyxml2::XMLElement* trackXML);
};
