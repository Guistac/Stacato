#include <pch.h>

#include "ParameterTrack.h"

#include "Motion/AnimatableParameter.h"
#include "Motion/Curve/Curve.h"
#include "Motion/Machine/Machine.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Project/Environnement.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Project/Plot.h"


ParameterTrack::ParameterTrack(std::shared_ptr<AnimatableParameter>& param, std::shared_ptr<Manoeuvre> m) : parameter(param), parentManoeuvre(m) {
	initialize();
	if (parameter->dataType == ParameterDataType::PARAMETER_GROUP) {
		for (auto& childParameter : parameter->childParameters) {
			std::shared_ptr<ParameterTrack> childParameterTrack = std::make_shared<ParameterTrack>(childParameter, m);
			childParameterTracks.push_back(childParameterTrack);
		}
	}
}

//COPY CONSTRUCTOR
ParameterTrack::ParameterTrack(const ParameterTrack& original) {
	parameter = original.parameter;
	initialize();
	for (int i = 0; i < original.startPoints.size(); i++) {
		curves[i] = std::make_shared<Motion::Curve>(*original.curves[i]);
		startPoints[i] = std::make_shared<Motion::ControlPoint>(*original.startPoints[i]);
		endPoints[i] = std::make_shared<Motion::ControlPoint>(*original.endPoints[i]);
	}
	sequenceType = original.sequenceType;
	interpolationType = original.interpolationType;
	originIsPreviousTarget = original.originIsPreviousTarget;
	origin = original.origin;
	target = original.target;
	movementTime = original.movementTime;
	timeOffset = original.timeOffset;
	rampIn = original.rampIn;
	rampOut = original.rampOut;
	rampsAreEqual = original.rampsAreEqual;
	childParameterTracks.clear();
	for (auto& childParameterTrack : original.childParameterTracks) {
		std::shared_ptr<ParameterTrack> childParameterTrackCopy = std::make_shared<ParameterTrack>(*childParameterTrack);
		childParameterTracks.push_back(childParameterTrackCopy);
	}
}

int ParameterTrack::getCurveCount() {
	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
		case ParameterDataType::INTEGER_PARAMETER:
		case ParameterDataType::STATE_PARAMETER:
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			return 1;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			return 2;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			return 3;
		case ParameterDataType::PARAMETER_GROUP:
		default:
			return 0;
	}
}

void ParameterTrack::initialize() {

	for (int i = 0; i < getCurveCount(); i++) {
		curves.push_back(std::make_shared<Motion::Curve>());
		startPoints.push_back(std::make_shared<Motion::ControlPoint>());
		endPoints.push_back(std::make_shared<Motion::ControlPoint>());
	}
	nextChainedCurvePoints.resize(getCurveCount());
	previousChainedCurvePoints.resize(getCurveCount());

	origin.type = parameter->dataType;
	target.type = parameter->dataType;
	origin.shortUnitString = parameter->shortUnitString;
	target.shortUnitString = parameter->shortUnitString;
	if (parameter->dataType == ParameterDataType::STATE_PARAMETER) {
		origin.stateValues = parameter->stateParameterValues;
		target.stateValues = parameter->stateParameterValues;
	}

	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			origin.boolValue = false;
			target.boolValue = true;
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			origin.integerValue = 0;
			target.integerValue = 1;
			break;
		case ParameterDataType::STATE_PARAMETER:
			origin.stateValue = &origin.stateValues->front();
			target.stateValue = &origin.stateValues->at(1);
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			origin.realValue = 0.0;
			target.realValue = 1.0;
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			origin.vector2value = glm::vec2(0.0);
			target.vector2value = glm::vec2(1.0);
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			origin.vector3value = glm::vec3(0.0);
			target.vector3value = glm::vec3(1.0);
			break;
	}


	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
		case ParameterDataType::INTEGER_PARAMETER:
		case ParameterDataType::STATE_PARAMETER:
			interpolationType = Motion::InterpolationType::STEP;
			sequenceType = SequenceType::Type::TIMED_MOVE;
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::VECTOR_3D_PARAMETER:
			interpolationType = Motion::InterpolationType::LINEAR;
			sequenceType = SequenceType::Type::TIMED_MOVE;
			break;
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			interpolationType = Motion::InterpolationType::TRAPEZOIDAL;
			sequenceType = SequenceType::Type::TIMED_MOVE;
			break;
		default:
			break;
	}

	switch (getCurveCount()) {
		case 1:
			sprintf(startPoints.back()->name, "Start Point");
			sprintf(endPoints.back()->name, "End Point");
			sprintf(curves.back()->name, "%s %s %s", parameter->machine->getName(), parameter->name, "");
			break;
		case 2:
			sprintf(startPoints[0]->name, "X Start Point");
			sprintf(endPoints[0]->name, "X End Point");
			sprintf(curves.back()->name, "%s %s %s", parameter->machine->getName(), parameter->name, "X");
			sprintf(startPoints[1]->name, "Y Start Point");
			sprintf(endPoints[1]->name, "Y End Point");
			sprintf(curves.back()->name, "%s %s %s", parameter->machine->getName(), parameter->name, "Y");
			break;
		case 3:
			sprintf(startPoints[0]->name, "X Start Point");
			sprintf(endPoints[0]->name, "X End Point");
			sprintf(curves.back()->name, "%s %s %s", parameter->machine->getName(), parameter->name, "X");
			sprintf(startPoints[1]->name, "Y Start Point");
			sprintf(endPoints[1]->name, "Y End Point");
			sprintf(curves.back()->name, "%s %s %s", parameter->machine->getName(), parameter->name, "Y");
			sprintf(startPoints[2]->name, "Z Start Point");
			sprintf(endPoints[2]->name, "Z End Point");
			sprintf(curves.back()->name, "%s %s %s", parameter->machine->getName(), parameter->name, "Z");
			break;
	}
}


//================================== EDITING ==================================


void ParameterTrack::setInterpolationType(Motion::InterpolationType t) {
	interpolationType = t;
	for (auto& curve : curves) curve->interpolationType = t;
}

void ParameterTrack::setSequenceType(SequenceType::Type t) {
	sequenceType = t;
	for (auto& childParameterTrack : childParameterTracks) {
		childParameterTrack->setSequenceType(t);
	}
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE: {
			//reduce to two curve points
			for (int i = 0; i < curves.size(); i++) {
				curves[i]->removeAllPoints();
				curves[i]->addPoint(startPoints[i]);
				curves[i]->addPoint(endPoints[i]);
			}
		}break;
		case SequenceType::Type::ANIMATED_MOVE:
			//keep the points, but editing will allow more points in the graph
			break;
		case SequenceType::Type::CONSTANT:
			break;
	}
}


void ParameterTrack::refreshAfterParameterEdit() {
	if (sequenceType == SequenceType::Type::CONSTANT) origin = target;

	if (nextChainedSlaveTrack) {
		nextChainedSlaveTrack->origin = target;
		nextChainedSlaveTrack->refreshAfterChainedDependenciesRefresh();
	}
	if (previousChainedSlaveTrack) {
		previousChainedSlaveTrack->target = origin;
		previousChainedSlaveTrack->refreshAfterChainedDependenciesRefresh();
	}
	
	refreshAfterChainedDependenciesRefresh();
}

void ParameterTrack::refreshAfterChainedDependenciesRefresh() {

	if (rampsAreEqual) {
		rampOut = rampIn;
	}

	//set start & stop point position values of in and out points
	//convert from non real parameter datatypes
	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			startPoints.front()->position = origin.boolValue ? 1.0 : 0.0;
			endPoints.front()->position = target.boolValue ? 1.0 : 0.0;
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			startPoints.front()->position = origin.integerValue;
			endPoints.front()->position = target.integerValue;
			break;
		case ParameterDataType::STATE_PARAMETER:
			startPoints.front()->position = origin.stateValue->integerEquivalent;
			endPoints.front()->position = target.stateValue->integerEquivalent;
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			startPoints[2]->position = origin.vector3value.z;
			endPoints[2]->position = origin.vector3value.z;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			startPoints[1]->position = origin.vector2value.y;
			endPoints[1]->position = origin.vector2value.y;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			startPoints[0]->position = origin.realValue;
			endPoints[0]->position = target.realValue;
			startPoints[0]->acceleration = rampIn;
			startPoints[0]->rampIn = rampIn;
			startPoints[0]->rampOut = rampIn;
			endPoints[0]->acceleration = rampOut;
			endPoints[0]->rampIn = rampOut;
			endPoints[0]->rampOut = rampOut;
			break;
	}

	//all manoeuvres always start and stop with zero velocity
	for (int i = 0; i < getCurveCount(); i++) {
		startPoints[i]->velocity = 0.0;
		endPoints[i]->velocity = 0.0;
	}

	if (parameter->dataType == ParameterDataType::PARAMETER_GROUP) {
		b_valid = true; //TODO; implement validation for group tracks in a proper manner
	}

	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE: {

			//assign time and acceleration values for in and out points
			for (int i = 0; i < getCurveCount(); i++) {
				startPoints[i]->time = timeOffset;
				endPoints[i]->time = timeOffset + movementTime;
			}

			//construct curve from start and end points
			for (int i = 0; i < getCurveCount(); i++) {
				curves[i]->removeAllPoints();
				curves[i]->addPoint(startPoints[i]);
				curves[i]->addPoint(endPoints[i]);
				curves[i]->interpolationType = interpolationType;
				curves[i]->refresh();
			}

			//validate curve and validate parameter track
			b_valid = true;
			if (!parameter->machine->validateParameterTrack(shared_from_this())) b_valid = false;
			if (isNextCrossChained()) b_valid = false;
			if (isPreviousCrossChained()) b_valid = false;
			if (isNextChainingMasterMissing()) b_valid = false;
			if (isPreviousChainingMasterMissing()) b_valid = false;

		}break;

		case SequenceType::Type::ANIMATED_MOVE:
			b_valid = false;
			break;
		case SequenceType::Type::CONSTANT: {
			origin = target;
			for (int i = 0; i < getCurveCount(); i++) {
				curves[i]->removeAllPoints();
				curves[i]->addPoint(endPoints[i]);
				curves[i]->interpolationType = Motion::InterpolationType::LINEAR;
				curves[i]->refresh();
			}
			b_valid = true;
			if (!parameter->machine->validateParameterTrack(shared_from_this())) b_valid = false;
		}break;
	}
	
	//the current track just updated its curve display points
	//we use this opportunity to update the chained display points of chained curves
	if (previousChainedSlaveTrack) previousChainedSlaveTrack->refreshNextChainedCurvePoints();
	if (previousChainedMasterTrack) previousChainedMasterTrack->refreshNextChainedCurvePoints();
	if (nextChainedSlaveTrack) nextChainedSlaveTrack->refreshPreviousChainedCurvePoints();
	if (nextChainedMasterTrack) nextChainedMasterTrack->refreshPreviousChainedCurvePoints();

	//the current manoeuvre length might have changed
	//so we also update the chained curve points of the current manoeuvre
	for (auto& track : parentManoeuvre->tracks) {
		track->refreshNextChainedCurvePoints();
	}

	parentManoeuvre->refresh();
}

void ParameterTrack::refreshAfterCurveEdit() {
	//copy the settings from the start and endpoints to the track settings

	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			origin.boolValue = startPoints.front()->position > 0.5 ? true : false;
			target.boolValue = endPoints.front()->position > 0.5 ? true : false;
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			origin.integerValue = std::round(startPoints.front()->position);
			target.integerValue = std::round(endPoints.front()->position);
			break;
		case ParameterDataType::STATE_PARAMETER: {
			int originInteger = std::round(startPoints.front()->position);
			clampValue(originInteger, 0, origin.stateValues->size() - 1)
			origin.stateValue = &origin.stateValues->at(originInteger);
			int targetInteger = std::round(endPoints.front()->position);
			clampValue(targetInteger, 0, target.stateValues->size() - 1)
			target.stateValue = &target.stateValues->at(targetInteger);
			}break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			origin.vector3value.x = startPoints[0]->position;
			origin.vector3value.y = startPoints[1]->position;
			origin.vector3value.z = startPoints[2]->position;
			target.vector3value.x = endPoints[0]->position;
			target.vector3value.y = endPoints[1]->position;
			target.vector3value.z = endPoints[2]->position;
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			origin.vector2value.x = startPoints[0]->position;
			origin.vector2value.y = startPoints[1]->position;
			target.vector2value.x = endPoints[0]->position;
			target.vector2value.y = endPoints[1]->position;
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			origin.realValue = startPoints[0]->position;
			target.realValue = endPoints[0]->position;
			break;
		case ParameterDataType::PARAMETER_GROUP:
			break;
	}

	bool offsetChanged = false;
	//detect if the startpoint was moved (offset time changed)
	for (int i = 0; i < getCurveCount(); i++) {
		if (startPoints[i]->time != timeOffset) {
			timeOffset = startPoints[i]->time;
			if (timeOffset < 0) timeOffset = 0;
			offsetChanged = true;
			break;
		}
	}

	switch (interpolationType) {
		case Motion::InterpolationType::STEP:
			timeOffset = 0.0;
			break;
		default:
			break;
	}

	if (!offsetChanged) {
		//detect if the endpoint was moved (movement time constraint changed)
		for (int i = 0; i < getCurveCount(); i++) {
			double thisCurveTimeConstraint = endPoints[i]->time - startPoints[i]->time;
			if (thisCurveTimeConstraint != movementTime) {
				movementTime = thisCurveTimeConstraint;
				if (movementTime < 0) movementTime = 0;
				break;
			}
		}
	}

	refreshAfterParameterEdit();
}


bool ParameterTrack::isNextChainingMasterMissing() {
	return targetIsNextOrigin && nextChainedMasterTrack == nullptr;
}

bool ParameterTrack::isPreviousChainingMasterMissing() {
	return originIsPreviousTarget && previousChainedMasterTrack == nullptr;
}

bool ParameterTrack::isPreviousCrossChained() {
	return originIsPreviousTarget && previousChainedMasterTrack && previousChainedMasterTrack->targetIsNextOrigin;
}

bool ParameterTrack::isNextCrossChained() {
	return targetIsNextOrigin && nextChainedMasterTrack && nextChainedMasterTrack->originIsPreviousTarget;
}

void ParameterTrack::refreshPreviousChainedCurvePoints() {
	if (previousChainedMasterTrack || previousChainedSlaveTrack) {
		std::shared_ptr<ParameterTrack> previousChainedTrack;
		if (previousChainedMasterTrack) previousChainedTrack = previousChainedMasterTrack;
		else previousChainedTrack = previousChainedSlaveTrack;
		double currentManoeuvreLength = parentManoeuvre->getLength_seconds();
		for (int i = 0; i < getCurveCount(); i++) {
			previousChainedCurvePoints[i].clear();
			if (previousChainedTrack->curves[i]->interpolations.empty()) continue;
			int pointCount = 2;
			for (auto& interpolation : previousChainedTrack->curves[i]->interpolations) {
				pointCount += interpolation->displayPoints.size();
			}
			previousChainedCurvePoints[i].reserve(pointCount);
			double previousTrackStartPosition = previousChainedTrack->curves[i]->points.front()->position;
			double previousTrackLength = previousChainedTrack->parentManoeuvre->getLength_seconds();
			previousChainedCurvePoints[i].push_back(Motion::CurvePoint(-previousTrackLength, previousTrackStartPosition, 0.0, 0.0));
			for (auto& interpolation : previousChainedTrack->curves[i]->interpolations) {
				for (auto& point : interpolation->displayPoints) {
					previousChainedCurvePoints[i].push_back(Motion::CurvePoint(point.time - previousTrackLength, point.position, point.acceleration, point.velocity));
				}
			}
			double previousTrackEndPosition = previousChainedTrack->curves[i]->interpolations.back()->displayPoints.back().position;
			previousChainedCurvePoints[i].push_back(Motion::CurvePoint(0.0, previousTrackEndPosition, 0.0, 0.0));
		}
	}
}

void ParameterTrack::refreshNextChainedCurvePoints() {
	if (nextChainedSlaveTrack || nextChainedMasterTrack) {
		std::shared_ptr<ParameterTrack> nextChainedTrack;
		if (nextChainedSlaveTrack) nextChainedTrack = nextChainedSlaveTrack;
		else nextChainedTrack = nextChainedMasterTrack;
		double currentManoeuvreLength = parentManoeuvre->getLength_seconds();
		for (int i = 0; i < getCurveCount(); i++) {
			nextChainedCurvePoints[i].clear();
			if (nextChainedTrack->curves[i]->interpolations.empty()) continue;
			int pointCount = 2;
			for (auto& interpolation : nextChainedTrack->curves[i]->interpolations) {
				pointCount += interpolation->displayPoints.size();
			}
			nextChainedCurvePoints[i].reserve(pointCount);
			double nextTrackStartPosition = nextChainedTrack->curves[i]->points.front()->position;
			nextChainedCurvePoints[i].push_back(Motion::CurvePoint(currentManoeuvreLength, nextTrackStartPosition, 0.0, 0.0));
			for (auto& interpolation : nextChainedTrack->curves[i]->interpolations) {
				for (auto& point : interpolation->displayPoints) {
					nextChainedCurvePoints[i].push_back(Motion::CurvePoint(currentManoeuvreLength + point.time, point.position, point.acceleration, point.velocity));
				}
			}
			double nextTrackLength = nextChainedTrack->parentManoeuvre->getLength_seconds();
			double nextTrackEndPosition = nextChainedTrack->curves[i]->interpolations.back()->displayPoints.back().position;
			nextChainedCurvePoints[i].push_back(Motion::CurvePoint(currentManoeuvreLength + nextTrackLength, nextTrackEndPosition, 0.0, 0.0));
		}
	}
}


//========================================== PLAYBACK ====================================================



void ParameterTrack::rapidToStart() {
	if (parameter->machine->isEnabled()) {
		parameter->machine->rapidParameterToValue(parameter, origin);
	}
}

void ParameterTrack::rapidToEnd() {
	if (parameter->machine->isEnabled()) {
		parameter->machine->rapidParameterToValue(parameter, target);
	}
}

void ParameterTrack::rapidToPlaybackPosition() {
	if (parameter->machine->isEnabled()) {
		AnimatableParameterValue parameterValueAtPlaybackPosition;
		getParameterValueAtPlaybackTime(parameterValueAtPlaybackPosition);
		parameter->machine->rapidParameterToValue(parameter, parameterValueAtPlaybackPosition);
	}
}

void ParameterTrack::cancelRapid() {
	parameter->machine->cancelParameterRapid(parameter);
}


float ParameterTrack::getRapidProgress() {
	return parameter->machine->getParameterRapidProgress(parameter);
}


bool ParameterTrack::isPrimedToStart() {
	return parameter->machine->isParameterReadyToStartPlaybackFromValue(parameter, origin);
}

bool ParameterTrack::isPrimedToEnd() {
	return parameter->machine->isParameterReadyToStartPlaybackFromValue(parameter, target);
}

bool ParameterTrack::isPrimedToPlaybackPosition() {
	AnimatableParameterValue parameterValueAtPlaybackPosition;
	getParameterValueAtPlaybackTime(parameterValueAtPlaybackPosition);
	return parameter->machine->isParameterReadyToStartPlaybackFromValue(parameter, parameterValueAtPlaybackPosition);
}


double ParameterTrack::getLength_seconds() {
	double longestCurve = 0.0;
	for (auto& curve : curves) {
		double time;
		if (curve->points.size() <= 1) time = 0;
		else if (curve->getEnd()->time > longestCurve) longestCurve = curve->getEnd()->time;
	}
	return longestCurve;
}


AnimatableParameterValue& ParameterTrack::getOrigin() {
	return origin;
}

AnimatableParameterValue& ParameterTrack::getTarget() {
	return target;
}

void ParameterTrack::getParameterValueAtPlaybackTime(AnimatableParameterValue& output) {
	output.type = parameter->dataType;
	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			output.boolValue = curves[0]->getPointAtTime(playbackPosition_seconds).position > 0.5;
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			output.integerValue = std::round(curves[0]->getPointAtTime(playbackPosition_seconds).position);
			break;
		case ParameterDataType::STATE_PARAMETER:
			for (auto& stateValue : *parameter->stateParameterValues) {
				if (std::round(curves[0]->getPointAtTime(playbackPosition_seconds).position) == stateValue.integerEquivalent) {
					output.stateValue = &stateValue;
					return;
				}
			}
			output.stateValue = &parameter->stateParameterValues->at(0);
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			output.vector3value.x = curves[0]->getPointAtTime(playbackPosition_seconds).position;
			output.vector3value.y = curves[1]->getPointAtTime(playbackPosition_seconds).position;
			output.vector3value.z = curves[2]->getPointAtTime(playbackPosition_seconds).position;
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			output.vector2value.x = curves[0]->getPointAtTime(playbackPosition_seconds).position;
			output.vector2value.y = curves[1]->getPointAtTime(playbackPosition_seconds).position;
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			output.realValue = curves[0]->getPointAtTime(playbackPosition_seconds).position;
			break;
		case ParameterDataType::PARAMETER_GROUP:
			break;
	}
}



void ParameterTrack::captureCurrentValueToOrigin(){
	parameter->machine->getActualParameterValue(parameter, origin);
}

void ParameterTrack::captureCurrentValueToTarget(){
	parameter->machine->getActualParameterValue(parameter, target);
}





std::vector<SequenceType> sequenceTypes = {
	{SequenceType::Type::CONSTANT, "Constant", "Constant"},
	{SequenceType::Type::TIMED_MOVE, "Timed", "TimedMove"},
	{SequenceType::Type::ANIMATED_MOVE, "Animated", "AnimatedMove"}
};
std::vector<SequenceType>& getSequenceTypes() {
	return sequenceTypes;
}
SequenceType* getSequenceType(SequenceType::Type t) {
	for (auto& sequenceType : sequenceTypes) {
		if (sequenceType.type == t) return &sequenceType;
	}
	return nullptr;
}
SequenceType* getSequenceType(const char* saveName) {
	for (auto& sequenceType : sequenceTypes) {
		if (strcmp(sequenceType.saveName, saveName) == 0) return &sequenceType;
	}
	return nullptr;
}
