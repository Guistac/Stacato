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

std::shared_ptr<Parameter> AnimatableParameter::getEditableParameter(){
	switch(getType()){
		case MachineParameterType::BOOLEAN: 	return std::make_shared<BooleanParameter>(false, "DefaultName", "DefaultSaveString");
		case MachineParameterType::INTEGER: 	return NumberParameter<int>::make(0, "DefaultName");
		case MachineParameterType::STATE:		return std::make_shared<StateParameter>(&castToState()->getStates().front(),
																						&castToState()->getStates(),
																						"DefaultName",
																						"DefaultSaveString");
		case MachineParameterType::POSITION:
		case MachineParameterType::VELOCITY:
		case MachineParameterType::REAL:		return NumberParameter<double>::make(0.0, "DefaultName");
		case MachineParameterType::POSITION_2D:
		case MachineParameterType::VELOCITY_2D:
		case MachineParameterType::VECTOR_2D:	return std::make_shared<VectorParameter<glm::vec2>>(glm::vec2(0.0), "DefaultName", "DefaultSaveString");
		case MachineParameterType::POSITION_3D:
		case MachineParameterType::VELOCITY_3D:
		case MachineParameterType::VECTOR_3D:	return std::make_shared<VectorParameter<glm::vec3>>(glm::vec3(0.0), "DefaultName", "DefaultSaveString");
		case MachineParameterType::GROUP:		return nullptr;
	}
}
