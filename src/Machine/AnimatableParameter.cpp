#include <pch.h>

#include "AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Motion/Curve/Curve.h"
#include "Machine/Machine.h"

//Constructor for Base Parameter Types
AnimatableParameter::AnimatableParameter(const char* nm){
	strcpy(name, nm);
	dataType = ParameterDataType::BOOLEAN_PARAMETER;
	unit = Units::None::None;
}

AnimatableParameter::AnimatableParameter(const char* nm, ParameterDataType datat){
	strcpy(name, nm);
	dataType = datat;
	unit = Units::None::None;
}

AnimatableParameter::AnimatableParameter(const char* nm, ParameterDataType datat, Unit u) {
	dataType = datat;
	unit = u;
	strcpy(name, nm);
}

//Constructor for Parameter with State DataType
AnimatableParameter::AnimatableParameter(const char* nm, std::vector<StateParameterValue>* stateValues) : stateParameterValues(stateValues) {
	strcpy(name, nm);
	dataType = ParameterDataType::STATE_PARAMETER;
}

//Constructor for Parameter Group
AnimatableParameter::AnimatableParameter(const char* nm, std::vector<std::shared_ptr<AnimatableParameter>> children) : childParameters(children) {
	strcpy(name, nm);
	dataType = ParameterDataType::PARAMETER_GROUP;
}

std::vector<Motion::InterpolationType> AnimatableParameter::getCompatibleInterpolationTypes() {
	std::vector<Motion::InterpolationType> output;
	switch (dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
		case ParameterDataType::INTEGER_PARAMETER:
		case ParameterDataType::STATE_PARAMETER:
			output.push_back(Motion::InterpolationType::STEP);
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::VECTOR_3D_PARAMETER:
			output.push_back(Motion::InterpolationType::STEP);
			output.push_back(Motion::InterpolationType::LINEAR);
			output.push_back(Motion::InterpolationType::TRAPEZOIDAL);
			output.push_back(Motion::InterpolationType::BEZIER);
			break;
		case ParameterDataType::POSITION:
		case ParameterDataType::POSITION_2D:
		case ParameterDataType::POSITION_3D:
			output.push_back(Motion::InterpolationType::TRAPEZOIDAL);
			break;
		case ParameterDataType::PARAMETER_GROUP:
			break;
	}
	return output;
}

void AnimatableParameter::getActiveTrackParameterValue(AnimatableParameterValue& output) {
	return actualParameterTrack->getParameterValueAtPlaybackTime(output);
}
