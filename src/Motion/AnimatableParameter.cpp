#include <pch.h>

#include "AnimatableParameter.h"

std::vector<InterpolationType::Type> AnimatableParameter::getCompatibleInterpolationTypes() {
	std::vector<InterpolationType::Type> output;
	switch (dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
		case ParameterDataType::INTEGER_PARAMETER:
		case ParameterDataType::STATE_PARAMETER:
			output.push_back(InterpolationType::Type::STEP);
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::VECTOR_3D_PARAMETER:
			output.push_back(InterpolationType::Type::STEP);
			output.push_back(InterpolationType::Type::LINEAR);
			output.push_back(InterpolationType::Type::TRAPEZOIDAL);
			output.push_back(InterpolationType::Type::BEZIER);
			break;
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			output.push_back(InterpolationType::Type::TRAPEZOIDAL);
			break;
	}
	return output;
}