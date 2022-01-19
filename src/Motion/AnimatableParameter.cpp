#include <pch.h>

#include "AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Curve/Curve.h"
#include "Machine/Machine.h"
#include <tinyxml2.h>

//Constructor for Base Parameter Types
AnimatableParameter::AnimatableParameter(const char* nm, ParameterDataType datat, const char* unitShortStr) : dataType(datat) {
	strcpy(name, nm);
	strcpy(shortUnitString, unitShortStr);
}

//Constructor for Parameter with State DataType
AnimatableParameter::AnimatableParameter(const char* nm, std::vector<StateParameterValue>* stateValues) : stateParameterValues(stateValues) {
	strcpy(name, nm);
	strcpy(shortUnitString, "state");
	dataType = ParameterDataType::STATE_PARAMETER;
}

//Constructor for Parameter Group
AnimatableParameter::AnimatableParameter(const char* nm, std::vector<std::shared_ptr<AnimatableParameter>> children) : childParameters(children) {
	strcpy(name, nm);
	strcpy(shortUnitString, "");
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
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
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
	switch (type) {
	case ParameterDataType::BOOLEAN_PARAMETER:
		boolValue = false;
		break;
	case ParameterDataType::INTEGER_PARAMETER:
		integerValue = 0;
		break;
	case ParameterDataType::STATE_PARAMETER:
		stateValue = &stateValues->at(0);
		break;
	case ParameterDataType::REAL_PARAMETER:
	case ParameterDataType::KINEMATIC_POSITION_CURVE:
		realValue = 0.0;
		break;
	case ParameterDataType::VECTOR_2D_PARAMETER:
	case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
		vector2value.x = 0.0;
		vector2value.y = 0.0;
		break;
	case ParameterDataType::VECTOR_3D_PARAMETER:
	case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
		vector3value.x = 0.0;
		vector3value.y = 0.0;
		vector3value.z = 0.0;
		break;
	}
}

bool AnimatableParameterValue::equals(AnimatableParameterValue& other) {
	switch (type) {
	case ParameterDataType::BOOLEAN_PARAMETER:
		return boolValue == other.boolValue;
	case ParameterDataType::INTEGER_PARAMETER:
		return integerValue == other.integerValue;
	case ParameterDataType::STATE_PARAMETER:
		return stateValue == other.stateValue;
	case ParameterDataType::REAL_PARAMETER:
	case ParameterDataType::KINEMATIC_POSITION_CURVE:
		return realValue == other.realValue;
	case ParameterDataType::VECTOR_2D_PARAMETER:
	case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
		return vector2value == other.vector2value;
	case ParameterDataType::VECTOR_3D_PARAMETER:
	case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
		return vector3value == other.vector3value;
	default:
		return false;
	}
}

bool AnimatableParameterValue::save(tinyxml2::XMLElement* parameterValueXML) {
	parameterValueXML->SetAttribute("Type", Enumerator::getSaveString(type));
	switch (type) {
	case ParameterDataType::BOOLEAN_PARAMETER:
		parameterValueXML->SetAttribute("Boolean", boolValue);
		break;
	case ParameterDataType::INTEGER_PARAMETER:
		parameterValueXML->SetAttribute("Integer", integerValue);
		break;
	case ParameterDataType::STATE_PARAMETER:
		parameterValueXML->SetAttribute("State", stateValue->saveName);
		break;
	case ParameterDataType::REAL_PARAMETER:
	case ParameterDataType::KINEMATIC_POSITION_CURVE:
		parameterValueXML->SetAttribute("Real", realValue);
		break;
	case ParameterDataType::VECTOR_2D_PARAMETER:
	case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
		parameterValueXML->SetAttribute("X", vector2value.x);
		parameterValueXML->SetAttribute("Y", vector2value.y);
		break;
	case ParameterDataType::VECTOR_3D_PARAMETER:
	case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
		parameterValueXML->SetAttribute("X", vector3value.x);
		parameterValueXML->SetAttribute("Y", vector3value.y);
		parameterValueXML->SetAttribute("Z", vector3value.z);
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
	type = Enumerator::getEnumeratorFromSaveString<ParameterDataType>(typeString);
	switch (type) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			if (parameterValueXML->QueryBoolAttribute("Boolean", &boolValue) != XML_SUCCESS) return Logger::warn("Could not read bool value");
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			if (parameterValueXML->QueryIntAttribute("Integer", &integerValue) != XML_SUCCESS) return Logger::warn("Could not read int value");
			break;
		case ParameterDataType::STATE_PARAMETER: {
			const char* stateValueString;
			if (parameterValueXML->QueryStringAttribute("State", &stateValueString) != XML_SUCCESS) return Logger::warn("Could not read state value");
			
			for(auto& sv : *stateValues){
				if(strcmp(sv.saveName, stateValueString) == 0){
					stateValue = &sv;
					break;
				}
			}
			
			//decode state type
			
		}break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			if (parameterValueXML->QueryDoubleAttribute("Real", &realValue) != XML_SUCCESS) return Logger::warn("Could not read real value");
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			if (parameterValueXML->QueryFloatAttribute("X", &vector2value.x) != XML_SUCCESS) return Logger::warn("Could not read vector2.x value");
			if (parameterValueXML->QueryFloatAttribute("Y", &vector2value.y) != XML_SUCCESS) return Logger::warn("Could not read vector2.y value");
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			if (parameterValueXML->QueryFloatAttribute("X", &vector3value.x) != XML_SUCCESS) return Logger::warn("Could not read vector3.x value");
			if (parameterValueXML->QueryFloatAttribute("Y", &vector3value.y) != XML_SUCCESS) return Logger::warn("Could not read vector3.y value");
			if (parameterValueXML->QueryFloatAttribute("Z", &vector3value.z) != XML_SUCCESS) return Logger::warn("Could not read vector3.z value");
			break;
		case ParameterDataType::PARAMETER_GROUP:
			break;
	}
	return true;
}
