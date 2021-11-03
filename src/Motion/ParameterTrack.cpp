#include <pch.h>

#include "ParameterTrack.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Curve/Curve.h"


ParameterTrack::ParameterTrack(std::shared_ptr<AnimatableParameter>& param) : parameter(param) {
	initialize();
}

//COPY CONSTRUCTOR
ParameterTrack::ParameterTrack(const ParameterTrack& original) {
	parameter = original.parameter;
	initialize();
	for (int i = 0; i < original.startPoints.size(); i++) {
		curves[i] = std::make_shared<Motion::Curve>(*original.curves[i]);
		startPoints[i] = std::make_shared<Motion::ControlPoint>(*original.startPoints[i]);
		endPoints[i] = std::make_shared<Motion::ControlPoint>(*original.endPoints[i]);
		//sequenceInterpolations[i] = std::make_shared<Motion::Interpolation>(*original.sequenceInterpolations[i]);
	}
	sequenceType = original.sequenceType;
	interpolationType = original.interpolationType;
	originIsPreviousTarget = original.originIsPreviousTarget;
	origin = original.origin;
	target = original.target;
	velocityConstraint = original.velocityConstraint;
	timeConstraint = original.timeConstraint;
	timeOffset = original.timeOffset;
	rampIn = original.rampIn;
	rampOut = original.rampOut;
	rampsAreEqual = original.rampsAreEqual;
}

void ParameterTrack::initialize() {

	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
		case ParameterDataType::INTEGER_PARAMETER:
		case ParameterDataType::STATE_PARAMETER:
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			curves.push_back(std::make_shared<Motion::Curve>());
			startPoints.push_back(std::make_shared<Motion::ControlPoint>());
			endPoints.push_back(std::make_shared<Motion::ControlPoint>());
			//sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			for (int i = 0; i < 2; i++) {
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::ControlPoint>());
				endPoints.push_back(std::make_shared<Motion::ControlPoint>());
				//sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
			}
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			for (int i = 0; i < 3; i++) {
				curves.push_back(std::make_shared<Motion::Curve>());
				startPoints.push_back(std::make_shared<Motion::ControlPoint>());
				endPoints.push_back(std::make_shared<Motion::ControlPoint>());
				//sequenceInterpolations.push_back(std::make_shared<Motion::Interpolation>());
			}
			break;
	}


	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
		case ParameterDataType::INTEGER_PARAMETER:
		case ParameterDataType::STATE_PARAMETER:
			interpolationType = InterpolationType::Type::STEP;
			sequenceType = SequenceType::Type::STEP_MOVE;
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::VECTOR_3D_PARAMETER:
			interpolationType = InterpolationType::Type::LINEAR;
			sequenceType = SequenceType::Type::TIMED_MOVE;
			break;
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			interpolationType = InterpolationType::Type::TRAPEZOIDAL;
			sequenceType = SequenceType::Type::TIMED_MOVE;
			break;
	}

	switch (getCurveCount()) {
		case 1:
			sprintf(startPoints.back()->name, "Start Point");
			sprintf(endPoints.back()->name, "End Point");
			break;
		case 2:
			sprintf(startPoints[0]->name, "X Start Point");
			sprintf(endPoints[0]->name, "X End Point");
			sprintf(startPoints[1]->name, "Y Start Point");
			sprintf(endPoints[1]->name, "Y End Point");
			break;
		case 3:
			sprintf(startPoints[0]->name, "X Start Point");
			sprintf(endPoints[0]->name, "X End Point");
			sprintf(startPoints[1]->name, "Y Start Point");
			sprintf(endPoints[1]->name, "Y End Point");
			sprintf(startPoints[2]->name, "Z Start Point");
			sprintf(endPoints[2]->name, "Z End Point");
			break;
	}


	origin.type = parameter->dataType;
	target.type = parameter->dataType;
	if (parameter->dataType == ParameterDataType::STATE_PARAMETER) {
		origin.stateValues = parameter->stateParameterValues;
		target.stateValues = parameter->stateParameterValues;
		origin.stateValue = &(*parameter->stateParameterValues)[0];
		target.stateValue = &(*parameter->stateParameterValues)[0];
	}
}

void ParameterTrack::setInterpolationType(InterpolationType::Type t) {
	interpolationType = t;
	bool sequenceTypeCompatible = false;
	for (auto st : getCompatibleSequenceTypes()) {
		if (st == sequenceType) {
			sequenceTypeCompatible = true;
			break;
		}
	}
	for (auto& curve : curves) curve->interpolationType = t;
	if (!sequenceTypeCompatible) {
		setSequenceType(getCompatibleSequenceTypes().front());
	}
	else {
		updateCurves();
	}
}

void ParameterTrack::setSequenceType(SequenceType::Type t) {
	sequenceType = t;
	switch (sequenceType) {
		case SequenceType::Type::STEP_MOVE:
		case SequenceType::Type::VELOCITY_MOVE:
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


std::vector<SequenceType::Type> ParameterTrack::getCompatibleSequenceTypes() {
	std::vector<SequenceType::Type> output;
	switch (interpolationType) {
	case InterpolationType::Type::STEP:
		output.push_back(SequenceType::Type::STEP_MOVE);
		output.push_back(SequenceType::Type::ANIMATED_MOVE);
		break;
	case InterpolationType::Type::LINEAR:
		output.push_back(SequenceType::Type::TIMED_MOVE);
		output.push_back(SequenceType::Type::ANIMATED_MOVE);
		break;
	case InterpolationType::Type::BEZIER:
		output.push_back(SequenceType::Type::TIMED_MOVE);
		output.push_back(SequenceType::Type::ANIMATED_MOVE);
		break;
	case InterpolationType::Type::TRAPEZOIDAL:
		output.push_back(SequenceType::Type::TIMED_MOVE);
		output.push_back(SequenceType::Type::VELOCITY_MOVE);
		output.push_back(SequenceType::Type::ANIMATED_MOVE);
		break;
	}
	return output;
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
			break;
	}

	//assign time and acceleration values for in and out points
	switch (sequenceType) {
		case SequenceType::Type::STEP_MOVE:
			for (int i = 0; i < getCurveCount(); i++) {
				startPoints[i]->time = 0.0;
				endPoints[i]->time = timeOffset;
				startPoints[i]->acceleration = 0.0;
				endPoints[i]->acceleration = 0.0;
			}
			break;
		case SequenceType::Type::TIMED_MOVE:
		case SequenceType::Type::VELOCITY_MOVE:
			for (int i = 0; i < getCurveCount(); i++) {
				startPoints[i]->time = timeOffset;
				endPoints[i]->time = timeOffset + timeConstraint;
				startPoints[i]->acceleration = rampIn;
				endPoints[i]->acceleration = rampOut;
			}
			break;
	}

	for (int i = 0; i < getCurveCount(); i++) {
		curves[i]->removeAllPoints();
		//curves[i]->interpolationType = interpolationType;
		//curves[i]->interpolations.push_back(sequenceInterpolations[i]);
		curves[i]->addPoint(startPoints[i]);
		curves[i]->addPoint(endPoints[i]);
		curves[i]->interpolationType = interpolationType;
		curves[i]->refresh();
	}


}


