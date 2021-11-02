#include <pch.h>

#include "ParameterTrack.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Curve/Curve.h"

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

void ParameterTrack::updateCurves() {

	/*

	//all manoeuvres start and stop with zero velocity
	startPoint->velocity = 0.0;
	endPoint->velocity = 0.0;

	//set start & stop point position values
	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			startPoint->position = origin.boolValue ? 1.0 : 0.0;
			endPoint->position = target.boolValue ? 1.0 : 0.0;
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			startPoint->position = origin.integerValue;
			endPoint->position = target.integerValue;
			break;
		case ParameterDataType::STATE_PARAMETER:
			startPoint->position = origin.stateValue->integerEquivalent;
			endPoint->position = target.stateValue->integerEquivalent;
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			startPointZ->position = origin.vector3value.z;
			endPointZ->position = origin.vector3value.z;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			startPointY->position = origin.vector2value.y;
			endPointY->position = origin.vector2value.y;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			startPoint->position = origin.integerValue;
			endPoint->position = target.integerValue;
			break;
	}

	//initialize start/stop point times and acceleration values
	switch (sequenceType) {
		case SequenceType::Type::STEP_MOVE:
			switch (parameter->dataType) {
				case ParameterDataType::VECTOR_3D_PARAMETER:
				case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
					startPointZ->time = 0.0;
					endPointZ->time = timeOffset;
					startPointZ->acceleration = 0.0;
					endPointZ->acceleration = 0.0;
				case ParameterDataType::VECTOR_2D_PARAMETER:
				case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
					startPointY->time = 0.0;
					endPointY->time = timeOffset;
					startPointY->acceleration = 0.0;
					endPointY->acceleration = 0.0;
				case ParameterDataType::REAL_PARAMETER:
				case ParameterDataType::KINEMATIC_POSITION_CURVE:
					startPoint->time = 0.0;
					endPoint->time = timeOffset;
					startPoint->acceleration = 0.0;
					endPoint->acceleration = 0.0;
					break;
			}
			break;
		case SequenceType::Type::TIMED_MOVE:
			switch (parameter->dataType) {
				case ParameterDataType::VECTOR_3D_PARAMETER:
				case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
					startPointZ->time = timeOffset;
					endPointZ->time = timeOffset + timeConstraint;
					startPointZ->acceleration = rampIn;
					endPointZ->acceleration = rampOut;
				case ParameterDataType::VECTOR_2D_PARAMETER:
				case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
					startPointY->time = timeOffset;
					endPointY->time = timeOffset + timeConstraint;
					startPointY->acceleration = rampIn;
					endPointY->acceleration = rampOut;
				case ParameterDataType::REAL_PARAMETER:
				case ParameterDataType::KINEMATIC_POSITION_CURVE:
					startPoint->time = timeOffset;
					endPoint->time = timeOffset + timeConstraint;
					startPoint->acceleration = rampIn;
					endPoint->acceleration = rampOut;
					break;
			}
			break;
		case SequenceType::Type::VELOCITY_MOVE:
			switch (parameter->dataType) {
				case ParameterDataType::VECTOR_3D_PARAMETER:
				case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
					startPointZ->time = timeOffset;
					endPointZ->time = timeOffset;
					startPointZ->acceleration = rampIn;
					endPointZ->acceleration = rampOut;
				case ParameterDataType::VECTOR_2D_PARAMETER:
				case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
					startPointY->time = timeOffset;
					endPointY->time = timeOffset;
					startPointY->acceleration = rampIn;
					endPointY->acceleration = rampOut;
				case ParameterDataType::REAL_PARAMETER:
				case ParameterDataType::KINEMATIC_POSITION_CURVE:
					startPoint->time = timeOffset;
					endPoint->time = timeOffset;
					startPoint->acceleration = rampIn;
					endPoint->acceleration = rampOut;
					break;
			}
			break;
		case SequenceType::Type::ANIMATED_MOVE:
			break;
	}

	//we remove all previous points from the curve
	curve->removeAllPoints();
	curveY->removeAllPoints();
	curveZ->removeAllPoints();

	//all point values have been set
	//we need to add the points to the interpolation and the curves
	switch (parameter->dataType) {
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			curveZ->addPoint(startPointZ);
			curveZ->addPoint(endPointZ);
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			curveY->addPoint(startPointY);
			curveY->addPoint(endPointY);
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
		default:
			curve->addPoint(startPoint);
			curve->addPoint(endPoint);
			break;
	}

	curve->interpolationType = interpolationType;
	curveY->interpolationType = interpolationType;
	curveZ->interpolationType = interpolationType;
	sequenceInterpolation->inPoint = startPoint;
	sequenceInterpolation->outPoint = endPoint;
	sequenceInterpolationY->inPoint = startPointY;
	sequenceInterpolationY->outPoint = endPointY;
	sequenceInterpolationZ->inPoint = startPointZ;
	sequenceInterpolationZ->outPoint = endPointZ;
	sequenceInterpolation->type = interpolationType;
	sequenceInterpolationY->type = interpolationType;
	sequenceInterpolationZ->type = interpolationType;

	switch (parameter->dataType) {
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			curveZ->refresh();
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			curveY->refresh();
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
		default:
			curve->refresh();
			break;
	}
	*/

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