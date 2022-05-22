#include <pch.h>

#include "AnimatableParameter.h"
#include "Machine/Machine.h"
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

std::shared_ptr<AnimatableParameterValue> AnimatableParameter::getActualMachineValue(){
	return getMachine()->getActualParameterValue(shared_from_this()->castToAnimatable());
}

std::shared_ptr<Parameter> AnimatableParameter::getEditableParameter(){
	switch(getType()){
		case MachineParameterType::BOOLEAN: 	return std::make_shared<BooleanParameter>(false, "DefaultName", "DefaultSaveString");
		case MachineParameterType::INTEGER: 	return NumberParameter<int>::make(0, "DefaultName");
		case MachineParameterType::STATE:		return std::make_shared<StateParameter>(&castToState()->getStates().front(), &castToState()->getStates(), "DefaultName", "DefaultSaveString");
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


void AnimatableParameter::setParameterValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimatableParameterValue> value){
	switch(getType()){
		case MachineParameterType::BOOLEAN: 	std::dynamic_pointer_cast<BooleanParameter>(parameter)->overwrite(value->toBoolean()->value); break;
		case MachineParameterType::INTEGER: 	std::dynamic_pointer_cast<NumberParameter<int>>(parameter)->overwrite(value->toInteger()->value); break;
		case MachineParameterType::STATE: 		std::dynamic_pointer_cast<StateParameter>(parameter)->overwrite(value->toState()->value); break;
		case MachineParameterType::POSITION:	std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toPosition()->position); break;
		case MachineParameterType::VELOCITY:	std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toVelocity()->velocity); break;
		case MachineParameterType::REAL:		std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toReal()->value); break;
		case MachineParameterType::POSITION_2D:	std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->overwrite(value->to2dPosition()->position); break;
		case MachineParameterType::VELOCITY_2D:	std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->overwrite(value->to2dVelocity()->velocity); break;
		case MachineParameterType::VECTOR_2D:	std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->overwrite(value->to2dVector()->value); break;
		case MachineParameterType::POSITION_3D:	std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->overwrite(value->to3dPosition()->position); break;
		case MachineParameterType::VELOCITY_3D:	std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->overwrite(value->to3dVelocity()->velocity); break;
		case MachineParameterType::VECTOR_3D:	std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->overwrite(value->to3dVector()->value); break;
		case MachineParameterType::GROUP:		return nullptr;
	}
}

void AnimatableParameter::copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to){
	switch(getType()){
		case MachineParameterType::BOOLEAN: 	std::dynamic_pointer_cast<BooleanParameter>(to)->overwrite(std::dynamic_pointer_cast<BooleanParameter>(from)->value); break;
		case MachineParameterType::INTEGER: 	std::dynamic_pointer_cast<NumberParameter<int>>(to)->overwrite(std::dynamic_pointer_cast<NumberParameter<int>>(from)->value); break;
		case MachineParameterType::STATE: 		std::dynamic_pointer_cast<StateParameter>(to)->overwrite(std::dynamic_pointer_cast<StateParameter>(from)->value); break;
		case MachineParameterType::POSITION:
		case MachineParameterType::VELOCITY:
		case MachineParameterType::REAL:		std::dynamic_pointer_cast<NumberParameter<double>>(to)->overwrite(std::dynamic_pointer_cast<NumberParameter<double>>(from)->value); break;
		case MachineParameterType::POSITION_2D:
		case MachineParameterType::VELOCITY_2D:
		case MachineParameterType::VECTOR_2D:	std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(to)->overwrite(std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(from)->value); break;
		case MachineParameterType::POSITION_3D:
		case MachineParameterType::VELOCITY_3D:
		case MachineParameterType::VECTOR_3D:	std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(to)->overwrite(std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(from)->value); break;
		case MachineParameterType::GROUP:		return nullptr;
	}
}

std::shared_ptr<AnimatableParameterValue> AnimatableParameter::getParameterValue(std::shared_ptr<Parameter> parameter){
	std::shared_ptr<AnimatableParameterValue> output;
	switch(getType()){
		case MachineParameterType::BOOLEAN:
		{
			auto output = AnimatableParameterValue::makeBoolean();
			output->value = std::dynamic_pointer_cast<BooleanParameter>(parameter)->value;
			return output;
		}
		case MachineParameterType::INTEGER:
		{
			auto output = AnimatableParameterValue::makeInteger();
			output->value = std::dynamic_pointer_cast<NumberParameter<int>>(parameter)->value;
			return output;
		}
		case MachineParameterType::STATE:
		{
			auto output = AnimatableParameterValue::makeState();
			output->value = std::dynamic_pointer_cast<StateParameter>(parameter)->value;
			return output;
		}
		case MachineParameterType::POSITION:
		{
			auto output = AnimatableParameterValue::makePosition();
			output->position = std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->value;
			return output;
		}
		case MachineParameterType::VELOCITY:
		{
			auto output = AnimatableParameterValue::makeVelocity();
			output->velocity = std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->value;
			return output;
		}
		case MachineParameterType::REAL:
		{
			auto output = AnimatableParameterValue::makeReal();
			output->value = std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->value;
			return output;
		}
		case MachineParameterType::POSITION_2D:
		{
			auto output = AnimatableParameterValue::make2dPosition();
			output->position = std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->value;
			return output;
		}
		case MachineParameterType::VELOCITY_2D:
		{
			auto output = AnimatableParameterValue::make2dVelocity();
			output->velocity = std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->value;
			return output;
		}
		case MachineParameterType::VECTOR_2D:
		{
			auto output = AnimatableParameterValue::make2dVector();
			output->value = std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->value;
			return output;
		}
		case MachineParameterType::POSITION_3D:
		{
			auto output = AnimatableParameterValue::make3dPosition();
			output->position = std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->value;
			return output;
		}
		case MachineParameterType::VELOCITY_3D:
		{
			auto output = AnimatableParameterValue::make3dVelocity();
			output->velocity = std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->value;
			return output;
		}
		case MachineParameterType::VECTOR_3D:
		{
			auto output = AnimatableParameterValue::make3dVector();
			output->value = std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->value;
			return output;
		}
		case MachineParameterType::GROUP:		return nullptr;
	}
}
