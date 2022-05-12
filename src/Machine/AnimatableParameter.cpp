#include <pch.h>

#include "AnimatableParameter.h"

std::vector<Motion::InterpolationType>& AnimatableNumericalParameter::getCompatibleInterpolationTypes(){
	static std::vector<Motion::InterpolationType> stepOnly = {
		Motion::InterpolationType::STEP
	};
	
	static std::vector<Motion::InterpolationType> allInterpolationTypes = {
		Motion::InterpolationType::STEP,
		Motion::InterpolationType::LINEAR,
		Motion::InterpolationType::TRAPEZOIDAL,
		Motion::InterpolationType::BEZIER
	};
	
	static std::vector<Motion::InterpolationType> forPosition = {
		Motion::InterpolationType::TRAPEZOIDAL
	};
	
	static std::vector<Motion::InterpolationType> forVelocity = {
		Motion::InterpolationType::LINEAR,
		Motion::InterpolationType::TRAPEZOIDAL,
		Motion::InterpolationType::BEZIER
	};
	
	static std::vector<Motion::InterpolationType> none = {};
	
	switch (type) {
		case MachineParameterType::BOOLEAN:
		case MachineParameterType::INTEGER:
		case MachineParameterType::STATE:
			return stepOnly;
		case MachineParameterType::REAL:
		case MachineParameterType::VECTOR_2D:
		case MachineParameterType::VECTOR_3D:
			return allInterpolationTypes;
		case MachineParameterType::POSITION:
		case MachineParameterType::POSITION_2D:
		case MachineParameterType::POSITION_3D:
			return forPosition;
		case MachineParameterType::VELOCITY:
		case MachineParameterType::VELOCITY_2D:
		case MachineParameterType::VELOCITY_3D:
			return forVelocity;
		case MachineParameterType::GROUP:
			return none;
	}
}
