#include <pch.h>

#include "AnimationTypes.h"

namespace AnimationSystem{

Legato::Option option_CompositeAnimatable = Legato::Option(AnimatableType::COMPOSITE, 	"Composite",	"Composite");
Legato::Option option_BooleanAnimatable = 	Legato::Option(AnimatableType::BOOLEAN, 	"Boolean", 		"Boolean");
Legato::Option option_IntegerAnimatable = 	Legato::Option(AnimatableType::INTEGER, 	"Integer", 		"Integer");
Legato::Option option_RealAnimatable = 		Legato::Option(AnimatableType::REAL, 		"Real", 		"Real");
Legato::Option option_StateAnimatable = 	Legato::Option(AnimatableType::STATE, 		"State", 		"State");
Legato::Option option_PositionAnimatable = 	Legato::Option(AnimatableType::POSITION, 	"Position", 	"Position");
Legato::Option option_VelocityAnimatable = 	Legato::Option(AnimatableType::VELOCITY, 	"Velocity", 	"Velocity");

std::vector<Legato::Option*> animatableTypeOptions = {
	&option_CompositeAnimatable,
	&option_BooleanAnimatable,
	&option_IntegerAnimatable,
	&option_RealAnimatable,
	&option_StateAnimatable,
	&option_PositionAnimatable,
	&option_VelocityAnimatable
};

Legato::Option* getAnimatableType(AnimatableType type){
	for(auto option : animatableTypeOptions){
		if(option->getInt() == type) return option;
	}
	return nullptr;
}

Legato::Option* getAnimatableType(std::string saveString){
	for(auto option : animatableTypeOptions){
		if(option->getSaveString() == saveString) return option;
	}
	return nullptr;
}

};
