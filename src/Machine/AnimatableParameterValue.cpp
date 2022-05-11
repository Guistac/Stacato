#include "AnimatableParameterValue.h"

#include "AnimatableParameter.h"

#include <tinyxml2.h>


/*
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
	switch (parameter->dataType) {
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
*/
