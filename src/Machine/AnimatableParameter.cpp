#include <pch.h>

#include "AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Motion/Curve/Curve.h"
#include "Machine/Machine.h"
#include <tinyxml2.h>

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







//========================================================================
//========================= PARAMETER VALUES =============================
//========================================================================


void AnimatableParameterValue::reset() {
	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			boolean = false;
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			integer = 0;
			break;
		case ParameterDataType::STATE_PARAMETER:
			state = &parameter->getStateValues().at(0);
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::POSITION:
			real = 0.0;
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::POSITION_2D:
			vector2 = glm::vec2(0.0, 0.0);
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::POSITION_3D:
			vector2 = glm::vec3(0.0, 0.0, 0.0);
			break;
		case ParameterDataType::PARAMETER_GROUP:
			break;
	}
}

bool AnimatableParameterValue::equals(AnimatableParameterValue& other) {
	if(parameter->dataType != other.parameter->dataType) return false;
	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER: 		return boolean == other.boolean;
		case ParameterDataType::INTEGER_PARAMETER: 		return integer == other.integer;
		case ParameterDataType::STATE_PARAMETER: 		return state == other.state;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::POSITION:				return real == other.real;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::POSITION_2D:			return vector2 == other.vector2;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::POSITION_3D:			return vector3 == other.vector3;
		default:										return false;
	}
}

bool AnimatableParameterValue::save(tinyxml2::XMLElement* parameterValueXML) {
	parameterValueXML->SetAttribute("Type", Enumerator::getSaveString(parameter->dataType));
	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			parameterValueXML->SetAttribute("Boolean", boolean);
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			parameterValueXML->SetAttribute("Integer", integer);
			break;
		case ParameterDataType::STATE_PARAMETER:
			parameterValueXML->SetAttribute("State", state->saveName);
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::POSITION:
			parameterValueXML->SetAttribute("Real", real);
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::POSITION_2D:
			parameterValueXML->SetAttribute("X", vector2.x);
			parameterValueXML->SetAttribute("Y", vector2.y);
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::POSITION_3D:
			parameterValueXML->SetAttribute("X", vector3.x);
			parameterValueXML->SetAttribute("Y", vector3.y);
			parameterValueXML->SetAttribute("Z", vector3.z);
			break;
			case ParameterDataType::PARAMETER_GROUP: break;
	}
	return true;
}



bool AnimatableParameterValue::load(tinyxml2::XMLElement* parameterValueXML) {
	using namespace tinyxml2;
	const char* typeString;
	if (parameterValueXML->QueryStringAttribute("Type", &typeString) != XML_SUCCESS) return Logger::warn("Could not find parameter data type");
	if (!Enumerator::isValidSaveName<ParameterDataType>(typeString)) return Logger::warn("Could not read parameter data type");
	ParameterDataType type = Enumerator::getEnumeratorFromSaveString<ParameterDataType>(typeString);
	switch (type) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			if (parameterValueXML->QueryBoolAttribute("Boolean", &boolean) != XML_SUCCESS) return Logger::warn("Could not read bool value");
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			if (parameterValueXML->QueryIntAttribute("Integer", &integer) != XML_SUCCESS) return Logger::warn("Could not read int value");
			break;
		case ParameterDataType::STATE_PARAMETER: {
			const char* stateValueString;
			if (parameterValueXML->QueryStringAttribute("State", &stateValueString) != XML_SUCCESS) return Logger::warn("Could not read state value");
			for(auto& sv : parameter->getStateValues()){
				if(strcmp(sv.saveName, stateValueString) == 0){
					state = &sv;
					break;
				}
			}
		}break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::POSITION:
			if (parameterValueXML->QueryDoubleAttribute("Real", &real) != XML_SUCCESS) return Logger::warn("Could not read real value");
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::POSITION_2D:
			if (parameterValueXML->QueryFloatAttribute("X", &vector2.x) != XML_SUCCESS) return Logger::warn("Could not read vector2.x value");
			if (parameterValueXML->QueryFloatAttribute("Y", &vector2.y) != XML_SUCCESS) return Logger::warn("Could not read vector2.y value");
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::POSITION_3D:
			if (parameterValueXML->QueryFloatAttribute("X", &vector3.x) != XML_SUCCESS) return Logger::warn("Could not read vector3.x value");
			if (parameterValueXML->QueryFloatAttribute("Y", &vector3.y) != XML_SUCCESS) return Logger::warn("Could not read vector3.y value");
			if (parameterValueXML->QueryFloatAttribute("Z", &vector3.z) != XML_SUCCESS) return Logger::warn("Could not read vector3.z value");
			break;
		case ParameterDataType::PARAMETER_GROUP:
			break;
	}
	return true;
}
