#include <pch.h>

#include "AnimationValue.h"
#include "Machine/Machine.h"
#include "Animation/Animation.h"

std::shared_ptr<Animation> Animatable::makeAnimation(ManoeuvreType manoeuvreType){
	auto track = Animation::create(shared_from_this(), manoeuvreType);
	//subscribe track to parameter changes
	track->subscribeToMachineParameter();
	return track;
}

int Animatable::getCurveCount(){
	switch(getType()){
		case AnimatableType::BOOLEAN:
		case AnimatableType::INTEGER:
		case AnimatableType::STATE:
		case AnimatableType::REAL:
		case AnimatableType::POSITION:
		case AnimatableType::VELOCITY:
			return 1;
		case AnimatableType::VECTOR_2D:
		case AnimatableType::POSITION_2D:
		case AnimatableType::VELOCITY_2D:
			return 2;
		case AnimatableType::VECTOR_3D:
		case AnimatableType::POSITION_3D:
		case AnimatableType::VELOCITY_3D:
			return 3;
		case AnimatableType::COMPOSITE:
			return 0;
	}
}

std::vector<Motion::Interpolation::Type>& Animatable::getCompatibleInterpolationTypes(){
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
	
	switch (getType()) {
		case AnimatableType::BOOLEAN:
		case AnimatableType::INTEGER:
		case AnimatableType::STATE:
			return stepOnly;
		case AnimatableType::REAL:
		case AnimatableType::VECTOR_2D:
		case AnimatableType::VECTOR_3D:
			return allInterpolationTypes;
		case AnimatableType::POSITION:
		case AnimatableType::POSITION_2D:
		case AnimatableType::POSITION_3D:
			return forPosition;
		case AnimatableType::VELOCITY:
		case AnimatableType::VELOCITY_2D:
		case AnimatableType::VELOCITY_3D:
			return forVelocity;
		case AnimatableType::COMPOSITE:
			return none;
	}
}


void AnimatableNumber::setUnit(Unit u){
	unit = u;
	for(auto animation : getAnimations()) animation->setUnit(u);
}

std::shared_ptr<AnimationValue> Animatable::getActualValue(){
	return getMachine()->getActualAnimatableValue(shared_from_this());
}

std::shared_ptr<Parameter> Animatable::makeParameter(){
	switch(getType()){
		case AnimatableType::BOOLEAN: 		return std::make_shared<BooleanParameter>(false, "DefaultName", "DefaultSaveString");
		case AnimatableType::INTEGER: 		return NumberParameter<int>::make(0, "DefaultName");
		case AnimatableType::STATE:			//return std::make_shared<StateParameter>(&castToState()->getStates().front(), &castToState()->getStates(), "DefaultName", "DefaultSaveString");
		case AnimatableType::POSITION:
		case AnimatableType::VELOCITY:
		case AnimatableType::REAL:			return NumberParameter<double>::make(0.0, "DefaultName");
		case AnimatableType::POSITION_2D:
		case AnimatableType::VELOCITY_2D:
		case AnimatableType::VECTOR_2D:		return std::make_shared<VectorParameter<glm::vec2>>(glm::vec2(0.0), "DefaultName", "DefaultSaveString");
		case AnimatableType::POSITION_3D:
		case AnimatableType::VELOCITY_3D:
		case AnimatableType::VECTOR_3D:		return std::make_shared<VectorParameter<glm::vec3>>(glm::vec3(0.0), "DefaultName", "DefaultSaveString");
		case AnimatableType::COMPOSITE:		return nullptr;
	}
}


void Animatable::setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value){
	switch(getType()){
		case AnimatableType::BOOLEAN: 		std::dynamic_pointer_cast<BooleanParameter>(parameter)->overwrite(value->toBoolean()->value); break;
		case AnimatableType::INTEGER: 		std::dynamic_pointer_cast<NumberParameter<int>>(parameter)->overwrite(value->toInteger()->value); break;
		case AnimatableType::STATE: 		std::dynamic_pointer_cast<StateParameter>(parameter)->overwrite(value->toState()->value); break;
		case AnimatableType::POSITION:		std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toPosition()->position); break;
		case AnimatableType::VELOCITY:		std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toVelocity()->velocity); break;
		case AnimatableType::REAL:			std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toReal()->value); break;
		case AnimatableType::POSITION_2D:	std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->overwrite(value->to2dPosition()->position); break;
		case AnimatableType::VELOCITY_2D:	std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->overwrite(value->to2dVelocity()->velocity); break;
		case AnimatableType::VECTOR_2D:		std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->overwrite(value->to2dVector()->value); break;
		case AnimatableType::POSITION_3D:	std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->overwrite(value->to3dPosition()->position); break;
		case AnimatableType::VELOCITY_3D:	std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->overwrite(value->to3dVelocity()->velocity); break;
		case AnimatableType::VECTOR_3D:		std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->overwrite(value->to3dVector()->value); break;
		case AnimatableType::COMPOSITE:		return nullptr;
	}
}

void Animatable::copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to){
	switch(getType()){
		case AnimatableType::BOOLEAN: 	std::dynamic_pointer_cast<BooleanParameter>(to)->overwrite(std::dynamic_pointer_cast<BooleanParameter>(from)->value); break;
		case AnimatableType::INTEGER: 	std::dynamic_pointer_cast<NumberParameter<int>>(to)->overwrite(std::dynamic_pointer_cast<NumberParameter<int>>(from)->value); break;
		case AnimatableType::STATE: 		std::dynamic_pointer_cast<StateParameter>(to)->overwrite(std::dynamic_pointer_cast<StateParameter>(from)->value); break;
		case AnimatableType::POSITION:
		case AnimatableType::VELOCITY:
		case AnimatableType::REAL:		std::dynamic_pointer_cast<NumberParameter<double>>(to)->overwrite(std::dynamic_pointer_cast<NumberParameter<double>>(from)->value); break;
		case AnimatableType::POSITION_2D:
		case AnimatableType::VELOCITY_2D:
		case AnimatableType::VECTOR_2D:	std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(to)->overwrite(std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(from)->value); break;
		case AnimatableType::POSITION_3D:
		case AnimatableType::VELOCITY_3D:
		case AnimatableType::VECTOR_3D:	std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(to)->overwrite(std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(from)->value); break;
		case AnimatableType::COMPOSITE:		return nullptr;
	}
}

std::shared_ptr<AnimationValue> Animatable::parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter){
	
	std::shared_ptr<AnimationValue> output;
	
	switch(getType()){
		case AnimatableType::BOOLEAN:{
			auto output = AnimationValue::makeBoolean();
			output->value = std::dynamic_pointer_cast<BooleanParameter>(parameter)->value;
			return output;
		}
		case AnimatableType::INTEGER:{
			auto output = AnimationValue::makeInteger();
			output->value = std::dynamic_pointer_cast<NumberParameter<int>>(parameter)->value;
			return output;
		}
		case AnimatableType::STATE:{
			auto output = AnimationValue::makeState();
			output->value = std::dynamic_pointer_cast<StateParameter>(parameter)->value;
			return output;
		}
		case AnimatableType::POSITION:{
			auto output = AnimationValue::makePosition();
			output->position = std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->value;
			return output;
		}
		case AnimatableType::VELOCITY:{
			auto output = AnimationValue::makeVelocity();
			output->velocity = std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->value;
			return output;
		}
		case AnimatableType::REAL:{
			auto output = AnimationValue::makeReal();
			output->value = std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->value;
			return output;
		}
		case AnimatableType::POSITION_2D:{
			auto output = AnimationValue::make2dPosition();
			output->position = std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->value;
			return output;
		}
		case AnimatableType::VELOCITY_2D:{
			auto output = AnimationValue::make2dVelocity();
			output->velocity = std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->value;
			return output;
		}
		case AnimatableType::VECTOR_2D:{
			auto output = AnimationValue::make2dVector();
			output->value = std::dynamic_pointer_cast<VectorParameter<glm::vec2>>(parameter)->value;
			return output;
		}
		case AnimatableType::POSITION_3D:{
			auto output = AnimationValue::make3dPosition();
			output->position = std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->value;
			return output;
		}
		case AnimatableType::VELOCITY_3D:{
			auto output = AnimationValue::make3dVelocity();
			output->velocity = std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->value;
			return output;
		}
		case AnimatableType::VECTOR_3D:{
			auto output = AnimationValue::make3dVector();
			output->value = std::dynamic_pointer_cast<VectorParameter<glm::vec3>>(parameter)->value;
			return output;
		}
		case AnimatableType::COMPOSITE:		return nullptr;
	}
}


bool Animatable::isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2){
	switch(getType()){
		case AnimatableType::BOOLEAN: 	return value1->toBoolean()->value 		== value2->toBoolean()->value;
		case AnimatableType::INTEGER: 	return value1->toInteger()->value 		== value2->toInteger()->value;
		case AnimatableType::STATE: 		return value1->toState()->value 		== value2->toState()->value;
		case AnimatableType::POSITION:	return value1->toPosition()->position 	== value2->toPosition()->position;
		case AnimatableType::VELOCITY:	return value1->toVelocity()->velocity 	== value2->toVelocity()->velocity;
		case AnimatableType::REAL:		return value1->toReal()->value 			== value2->toReal()->value;
		case AnimatableType::POSITION_2D:	return value1->to2dPosition()->position == value2->to2dPosition()->position;
		case AnimatableType::VELOCITY_2D:	return value1->to2dVelocity()->velocity == value2->to2dVelocity()->velocity;
		case AnimatableType::VECTOR_2D:	return value1->to2dVector()->value 		== value2->to2dVector()->value;
		case AnimatableType::POSITION_3D:	return value1->to3dPosition()->position == value2->to3dPosition()->position;
		case AnimatableType::VELOCITY_3D:	return value1->to3dVelocity()->velocity == value2->to3dVelocity()->velocity;
		case AnimatableType::VECTOR_3D:	return value1->to3dVector()->value 		== value2->to3dVector()->value;
		case AnimatableType::COMPOSITE:		return false;
	}
}

std::shared_ptr<AnimationValue> Animatable::getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds){
	switch(getType()){
			
		case AnimatableType::BOOLEAN: {
			auto output = AnimationValue::makeBoolean();
			output->value = animation->getCurves().front().getPointAtTime(time_seconds).position >= 1.0;
			return output;
		}
		case AnimatableType::INTEGER:{
			auto output = AnimationValue::makeInteger();
			output->value = std::round(animation->getCurves().front().getPointAtTime(time_seconds).position);
			return output;
		}
		case AnimatableType::STATE:{
			auto output = AnimationValue::makeState();
			output->values = &toState()->getStates();
			int integer = std::round(animation->getCurves().front().getPointAtTime(time_seconds).position);
			for(int i = 0; i < output->values->size(); i++){
				if(output->values->at(i).integerEquivalent == i){
					output->value = &output->values->at(i);
					break;
				}
			}
			return output;
		}
		case AnimatableType::POSITION:{
			auto output = AnimationValue::makePosition();
			Motion::Point point = animation->getCurves().front().getPointAtTime(time_seconds);
			output->position = point.position;
			output->velocity = point.velocity;
			output->acceleration = point.acceleration;
			return output;
		}
		case AnimatableType::VELOCITY:{
			auto output = AnimationValue::makeVelocity();
			Motion::Point point = animation->getCurves().front().getPointAtTime(time_seconds);
			output->velocity = point.position;
			output->acceleration = point.velocity;
			return output;
		}
		case AnimatableType::REAL:{
			auto output = AnimationValue::makeReal();
			output->value = animation->getCurves().front().getPointAtTime(time_seconds).position;
			return output;
		}
		case AnimatableType::POSITION_2D:{
			auto output = AnimationValue::make2dPosition();
			Motion::Point pointX = animation->getCurves().at(0).getPointAtTime(time_seconds);
			Motion::Point pointY = animation->getCurves().at(1).getPointAtTime(time_seconds);
			output->position.x = pointX.position;
			output->position.y = pointY.position;
			output->velocity.x = pointX.velocity;
			output->velocity.y = pointY.velocity;
			output->acceleration.x = pointX.acceleration;
			output->acceleration.y = pointY.acceleration;
			return output;
		}
		case AnimatableType::VELOCITY_2D:{
			auto output = AnimationValue::make2dVelocity();
			Motion::Point pointX = animation->getCurves().at(0).getPointAtTime(time_seconds);
			Motion::Point pointY = animation->getCurves().at(1).getPointAtTime(time_seconds);
			output->velocity.x = pointX.position;
			output->velocity.y = pointY.position;
			output->acceleration.x = pointX.velocity;
			output->acceleration.y = pointY.velocity;
			return output;
		}
		case AnimatableType::VECTOR_2D:{
			auto output = AnimationValue::make2dVector();
			Motion::Point pointX = animation->getCurves().at(0).getPointAtTime(time_seconds);
			Motion::Point pointY = animation->getCurves().at(1).getPointAtTime(time_seconds);
			output->value.x = pointX.position;
			output->value.y = pointY.position;
			return output;
		}
		case AnimatableType::POSITION_3D:{
			auto output = AnimationValue::make3dPosition();
			Motion::Point pointX = animation->getCurves().at(0).getPointAtTime(time_seconds);
			Motion::Point pointY = animation->getCurves().at(1).getPointAtTime(time_seconds);
			Motion::Point pointZ = animation->getCurves().at(2).getPointAtTime(time_seconds);
			output->position.x = pointX.position;
			output->position.y = pointY.position;
			output->position.z = pointZ.position;
			output->velocity.x = pointX.velocity;
			output->velocity.y = pointY.velocity;
			output->velocity.z = pointZ.velocity;
			output->acceleration.x = pointX.acceleration;
			output->acceleration.y = pointY.acceleration;
			output->acceleration.z = pointZ.acceleration;
			return output;
		}
		case AnimatableType::VELOCITY_3D:{
			auto output = AnimationValue::make3dVelocity();
			Motion::Point pointX = animation->getCurves().at(0).getPointAtTime(time_seconds);
			Motion::Point pointY = animation->getCurves().at(1).getPointAtTime(time_seconds);
			Motion::Point pointZ = animation->getCurves().at(2).getPointAtTime(time_seconds);
			output->velocity.x = pointX.position;
			output->velocity.y = pointY.position;
			output->velocity.z = pointZ.position;
			output->acceleration.x = pointX.velocity;
			output->acceleration.y = pointY.velocity;
			output->acceleration.z = pointZ.velocity;
			return output;
		}
		case AnimatableType::VECTOR_3D:{
			auto output = AnimationValue::make3dVector();
			Motion::Point pointX = animation->getCurves().at(0).getPointAtTime(time_seconds);
			Motion::Point pointY = animation->getCurves().at(1).getPointAtTime(time_seconds);
			Motion::Point pointZ = animation->getCurves().at(2).getPointAtTime(time_seconds);
			output->value.x = pointX.position;
			output->value.y = pointY.position;
			output->value.z = pointZ.position;
			return output;
		}
		case AnimatableType::COMPOSITE: return nullptr;
	}
}




std::vector<double> Animatable::getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value){
	switch(getType()){
		case AnimatableType::BOOLEAN:		return {value->toBoolean()->value ? 1.0 : 0.0};
		case AnimatableType::INTEGER: 	return {(double)value->toInteger()->value};
		case AnimatableType::STATE: 		return {(double)value->toState()->value->integerEquivalent};
		case AnimatableType::POSITION:	return {value->toPosition()->position};
		case AnimatableType::VELOCITY:	return {value->toVelocity()->velocity};
		case AnimatableType::REAL:		return {value->toReal()->value};
		case AnimatableType::POSITION_2D:	{ glm::dvec2& pos = value->to2dPosition()->position; return {pos.x, pos.y}; }
		case AnimatableType::VELOCITY_2D:	{ glm::dvec2& vel = value->to2dVelocity()->velocity; return {vel.x, vel.y}; }
		case AnimatableType::VECTOR_2D:	{ glm::dvec2& vec = value->to2dVector()->value; return {vec.x, vec.y}; }
		case AnimatableType::POSITION_3D:	{ glm::dvec3& pos = value->to3dPosition()->position; return {pos.x, pos.y, pos.z}; }
		case AnimatableType::VELOCITY_3D:	{ glm::dvec3& vel = value->to3dVelocity()->velocity; return {vel.x, vel.y, vel.z}; }
		case AnimatableType::VECTOR_3D:	{ glm::dvec3& vec = value->to3dVector()->value; return {vec.x, vec.y, vec.z}; }
		case AnimatableType::COMPOSITE:		return {};
	}
}

std::shared_ptr<AnimationValue> Animatable::getAnimationValue(){
	return currentAnimation->getValueAtPlaybackTime();
}

void Animatable::stopAnimationPlayback(){
	if(hasAnimation()){
		auto animation = currentAnimation;
		currentAnimation = nullptr;
		animation->stop();
	}
}
