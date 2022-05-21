#include <pch.h>

#include "AnimatableParameter.h"

#include "Motion/Manoeuvre/ParameterTrack.h"

std::shared_ptr<ParameterTrack> MachineParameter::createTrack(ManoeuvreType manoeuvreType){
	auto track = ParameterTrack::create(shared_from_this(), manoeuvreType);
	//subscribe track to parameter changes
	track->subscribeToMachineParameter();
	return track;
}

int AnimatableParameter::getCurveCount(){
	switch(getType()){
		case MachineParameterType::BOOLEAN:
		case MachineParameterType::INTEGER:
		case MachineParameterType::STATE:
		case MachineParameterType::REAL:
		case MachineParameterType::POSITION:
		case MachineParameterType::VELOCITY:
			return 1;
		case MachineParameterType::VECTOR_2D:
		case MachineParameterType::POSITION_2D:
		case MachineParameterType::VELOCITY_2D:
			return 2;
		case MachineParameterType::VECTOR_3D:
		case MachineParameterType::POSITION_3D:
		case MachineParameterType::VELOCITY_3D:
			return 3;
		case MachineParameterType::GROUP:
			return 0;
	}
}

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


void AnimatableNumericalParameter::setUnit(Unit u){
	unit = u;
	for(auto track : getTracks()){
		if(track->isAnimated()) track->castToAnimated()->setUnit(unit);
	}
}
