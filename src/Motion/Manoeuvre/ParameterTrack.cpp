#include <pch.h>

#include "ParameterTrack.h"

#include "Motion/AnimatableParameter.h"
#include "Motion/Curve/Curve.h"
#include "Motion/Machine/Machine.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Project/Environnement.h"


ParameterTrack::ParameterTrack(std::shared_ptr<AnimatableParameter>& param) : parameter(param) {
	initialize();
	updateCurves();
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
	updateCurves();
}

int ParameterTrack::getCurveCount() {
	switch (parameter->dataType) {
		case ParameterDataType::Type::BOOLEAN_PARAMETER:
		case ParameterDataType::Type::INTEGER_PARAMETER:
		case ParameterDataType::Type::STATE_PARAMETER:
		case ParameterDataType::Type::REAL_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_POSITION_CURVE:
			return 1;
		case ParameterDataType::Type::VECTOR_2D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_2D_POSITION_CURVE:
			return 2;
		case ParameterDataType::Type::VECTOR_3D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_3D_POSITION_CURVE:
			return 3;
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

	origin.type = parameter->dataType;
	target.type = parameter->dataType;
	if (parameter->dataType == ParameterDataType::Type::STATE_PARAMETER) {
		origin.stateValues = parameter->stateParameterValues;
		target.stateValues = parameter->stateParameterValues;
	}

	switch (parameter->dataType) {
		case ParameterDataType::Type::BOOLEAN_PARAMETER:
			origin.boolValue = false;
			target.boolValue = true;
			break;
		case ParameterDataType::Type::INTEGER_PARAMETER:
			origin.integerValue = 0;
			target.integerValue = 1;
			break;
		case ParameterDataType::Type::STATE_PARAMETER:
			origin.stateValue = &origin.stateValues->front();
			target.stateValue = &origin.stateValues->at(1);
			break;
		case ParameterDataType::Type::REAL_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_POSITION_CURVE:
			origin.realValue = 0.0;
			target.realValue = 1.0;
			break;
		case ParameterDataType::Type::VECTOR_2D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_2D_POSITION_CURVE:
			origin.vector2value = glm::vec2(0.0);
			target.vector2value = glm::vec2(1.0);
			break;
		case ParameterDataType::Type::VECTOR_3D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_3D_POSITION_CURVE:
			origin.vector3value = glm::vec3(0.0);
			target.vector3value = glm::vec3(1.0);
			break;
	}


	switch (parameter->dataType) {
		case ParameterDataType::Type::BOOLEAN_PARAMETER:
		case ParameterDataType::Type::INTEGER_PARAMETER:
		case ParameterDataType::Type::STATE_PARAMETER:
			interpolationType = InterpolationType::Type::STEP;
			sequenceType = SequenceType::Type::TIMED_MOVE;
			break;
		case ParameterDataType::Type::REAL_PARAMETER:
		case ParameterDataType::Type::VECTOR_2D_PARAMETER:
		case ParameterDataType::Type::VECTOR_3D_PARAMETER:
			interpolationType = InterpolationType::Type::LINEAR;
			sequenceType = SequenceType::Type::TIMED_MOVE;
			break;
		case ParameterDataType::Type::KINEMATIC_POSITION_CURVE:
		case ParameterDataType::Type::KINEMATIC_2D_POSITION_CURVE:
		case ParameterDataType::Type::KINEMATIC_3D_POSITION_CURVE:
			interpolationType = InterpolationType::Type::TRAPEZOIDAL;
			sequenceType = SequenceType::Type::TIMED_MOVE;
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

void ParameterTrack::setInterpolationType(InterpolationType::Type t) {
	interpolationType = t;
	for (auto& curve : curves) curve->interpolationType = t;
	updateCurves();
}

void ParameterTrack::setSequenceType(SequenceType::Type t) {
	sequenceType = t;
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
	}
	updateCurves();
}


void ParameterTrack::updateCurves() {

	//all manoeuvres start and stop with zero velocity
	for (int i = 0; i < getCurveCount(); i++) {
		startPoints[i]->velocity = 0.0;
		endPoints[i]->velocity = 0.0;
	}

	//set start & stop point position values of in and out points
	//convert from non real parameter datatypes
	switch (parameter->dataType) {
		case ParameterDataType::Type::BOOLEAN_PARAMETER:
			startPoints.front()->position = origin.boolValue ? 1.0 : 0.0;
			endPoints.front()->position = target.boolValue ? 1.0 : 0.0;
			break;
		case ParameterDataType::Type::INTEGER_PARAMETER:
			startPoints.front()->position = origin.integerValue;
			endPoints.front()->position = target.integerValue;
			break;
		case ParameterDataType::Type::STATE_PARAMETER:
			startPoints.front()->position = origin.stateValue->integerEquivalent;
			endPoints.front()->position = target.stateValue->integerEquivalent;
			break;
		case ParameterDataType::Type::VECTOR_3D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_3D_POSITION_CURVE:
			startPoints[2]->position = origin.vector3value.z;
			endPoints[2]->position = origin.vector3value.z;
		case ParameterDataType::Type::VECTOR_2D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_2D_POSITION_CURVE:
			startPoints[1]->position = origin.vector2value.y;
			endPoints[1]->position = origin.vector2value.y;
		case ParameterDataType::Type::REAL_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_POSITION_CURVE:
			startPoints[0]->position = origin.realValue;
			endPoints[0]->position = target.realValue;
			break;
	}

	//assign time and acceleration values for in and out points
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			for (int i = 0; i < getCurveCount(); i++) {
				startPoints[i]->time = timeOffset;
				endPoints[i]->time = timeOffset + movementTime;
				startPoints[i]->acceleration = rampIn;
				endPoints[i]->acceleration = rampOut;
			}
			break;
	}

	for (int i = 0; i < getCurveCount(); i++) {
		curves[i]->removeAllPoints();
		curves[i]->addPoint(startPoints[i]);
		curves[i]->addPoint(endPoints[i]);
		curves[i]->interpolationType = interpolationType;
		curves[i]->refresh();
	}
}


void ParameterTrack::updateParametersAfterCurveEdit() {
	//copy the settings from the start and endpoints to the track settings

	switch (parameter->dataType) {
		case ParameterDataType::Type::BOOLEAN_PARAMETER:
			origin.boolValue = startPoints.front()->position > 0.5 ? true : false;
			target.boolValue = endPoints.front()->position > 0.5 ? true : false;
			break;
		case ParameterDataType::Type::INTEGER_PARAMETER:
			origin.integerValue = std::round(startPoints.front()->position);
			target.integerValue = std::round(endPoints.front()->position);
			break;
		case ParameterDataType::Type::STATE_PARAMETER: {
			int originInteger = std::round(startPoints.front()->position);
			clamp(originInteger, 0, origin.stateValues->size() - 1)
			origin.stateValue = &origin.stateValues->at(originInteger);
			int targetInteger = std::round(endPoints.front()->position);
			clamp(targetInteger, 0, target.stateValues->size() - 1)
			target.stateValue = &target.stateValues->at(targetInteger);
			}break;
		case ParameterDataType::Type::VECTOR_3D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_3D_POSITION_CURVE:
			origin.vector3value.x = startPoints[0]->position;
			origin.vector3value.y = startPoints[1]->position;
			origin.vector3value.z = startPoints[2]->position;
			target.vector3value.x = endPoints[0]->position;
			target.vector3value.y = endPoints[1]->position;
			target.vector3value.z = endPoints[2]->position;
			break;
		case ParameterDataType::Type::VECTOR_2D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_2D_POSITION_CURVE:
			origin.vector2value.x = startPoints[0]->position;
			origin.vector2value.y = startPoints[1]->position;
			target.vector2value.x = endPoints[0]->position;
			target.vector2value.y = endPoints[1]->position;
			break;
		case ParameterDataType::Type::REAL_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_POSITION_CURVE:
			origin.realValue = startPoints[0]->position;
			target.realValue = endPoints[0]->position;
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
		case InterpolationType::Type::STEP:
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

	updateCurves();

	//double rampIn = 0.1;
	//double rampOut = 0.1;
	//bool rampsAreEqual = true;


}


void ParameterTrack::prime() {
	if (parameter->machine->isEnabled()) {
		parameter->machine->rapidParameterToValue(parameter, origin);
	}
}

void ParameterTrack::cancelPriming() {
	parameter->machine->cancelParameterRapid(parameter);
}

bool ParameterTrack::isPrimed() {
	return parameter->machine->isParameterAtValue(parameter, origin);
}

float ParameterTrack::getPrimingProgress() {
	return parameter->machine->getParameterRapidProgress(parameter);
}

double ParameterTrack::getLength_seconds() {
	double longestCurve = 0.0;
	for (auto& curve : curves) {
		if (curve->getEnd()->time > longestCurve) {
			longestCurve = curve->getEnd()->time;
		}
	}
	return longestCurve;
}


void ParameterTrack::getParameterValueAtPlaybackTime(AnimatableParameterValue& output) {
	double playbackTime = EtherCatFieldbus::getCycleProgramTime_seconds() - playbackStartTime_seconds;
	switch (parameter->dataType) {
		case ParameterDataType::Type::BOOLEAN_PARAMETER:
			output.boolValue = curves[0]->getPointAtTime(playbackTime).position > 0.5;
			break;
		case ParameterDataType::Type::INTEGER_PARAMETER:
			output.integerValue = std::round(curves[0]->getPointAtTime(playbackTime).position);
			break;
		case ParameterDataType::Type::STATE_PARAMETER:
			for (auto& stateValue : *parameter->stateParameterValues) {
				if (std::round(curves[0]->getPointAtTime(playbackTime).position) == stateValue.integerEquivalent) {
					output.stateValue = &stateValue;
					return;
				}
			}
			output.stateValue = &parameter->stateParameterValues->at(0);
			break;
		case ParameterDataType::Type::VECTOR_3D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_3D_POSITION_CURVE:
			output.vector3value.x = curves[0]->getPointAtTime(playbackTime).position;
			output.vector3value.y = curves[1]->getPointAtTime(playbackTime).position;
			output.vector3value.z = curves[2]->getPointAtTime(playbackTime).position;
			break;
		case ParameterDataType::Type::VECTOR_2D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_2D_POSITION_CURVE:
			output.vector2value.x = curves[0]->getPointAtTime(playbackTime).position;
			output.vector2value.y = curves[1]->getPointAtTime(playbackTime).position;
			break;
		case ParameterDataType::Type::REAL_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_POSITION_CURVE:
			output.realValue = curves[0]->getPointAtTime(playbackTime).position;
			break;
	}
}




std::vector<SequenceType> sequenceTypes = {
	{SequenceType::Type::NO_MOVE, "No Move", "NoMove"},
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