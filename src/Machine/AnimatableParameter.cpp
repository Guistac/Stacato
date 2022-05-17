#include <pch.h>

#include "AnimatableParameter.h"

std::vector<Motion::Interpolation::Type>& AnimatableNumericalParameter::getCompatibleInterpolationTypes(){
	static std::vector<Motion::Interpolation::Type> stepOnly = {
		Motion::Interpolation::Type::STEP
	};
	
	static std::vector<Motion::Interpolation::Type> allInterpolationTypes = {
		Motion::Interpolation::Type::STEP,
		Motion::Interpolation::Type::LINEAR,
		Motion::Interpolation::Type::TRAPEZOIDAL,
		//Motion::Interpolation::Type::BEZIER
	};
	
	static std::vector<Motion::Interpolation::Type> forPosition = {
		Motion::Interpolation::Type::TRAPEZOIDAL,
		//Motion::Interpolation::Type::BEZIER
	};
	
	static std::vector<Motion::Interpolation::Type> forVelocity = {
		Motion::Interpolation::Type::LINEAR,
		Motion::Interpolation::Type::TRAPEZOIDAL,
		//Motion::Interpolation::Type::BEZIER
	};
	
	static std::vector<Motion::Interpolation::Type> none = {};
	
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
