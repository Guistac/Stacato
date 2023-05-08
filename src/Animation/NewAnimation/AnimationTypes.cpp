#include <pch.h>

#include "AnimationTypes.h"

namespace AnimationSystem{

Legato::Option option_animatableType_CompositeAnimatable = 	Legato::Option(AnimatableType::COMPOSITE, 	"Composite",	"Composite");
Legato::Option option_animatableType_BooleanAnimatable = 	Legato::Option(AnimatableType::BOOLEAN, 	"Boolean", 		"Boolean");
Legato::Option option_animatableType_IntegerAnimatable = 	Legato::Option(AnimatableType::INTEGER, 	"Integer", 		"Integer");
Legato::Option option_animatableType_RealAnimatable = 		Legato::Option(AnimatableType::REAL, 		"Real", 		"Real");
Legato::Option option_animatableType_StateAnimatable = 		Legato::Option(AnimatableType::STATE, 		"State", 		"State");
Legato::Option option_animatableType_PositionAnimatable = 	Legato::Option(AnimatableType::POSITION, 	"Position", 	"Position");
Legato::Option option_animatableType_VelocityAnimatable = 	Legato::Option(AnimatableType::VELOCITY, 	"Velocity", 	"Velocity");

std::vector<Legato::Option*> animatableTypeOptions = {
	&option_animatableType_CompositeAnimatable,
	&option_animatableType_BooleanAnimatable,
	&option_animatableType_IntegerAnimatable,
	&option_animatableType_RealAnimatable,
	&option_animatableType_StateAnimatable,
	&option_animatableType_PositionAnimatable,
	&option_animatableType_VelocityAnimatable
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


Legato::Option option_animationType_Target =	Legato::Option(AnimationType::TARGET,	"Target",	"Target");
Legato::Option option_animationType_Sequence =	Legato::Option(AnimationType::SEQUENCE, "Sequence", "Sequence");
Legato::Option option_animationType_Stop =		Legato::Option(AnimationType::STOP,		"Stop",		"Stop");

std::vector<Legato::Option*> animationTypeOptions = {
	&option_animationType_Target,
	&option_animationType_Sequence,
	&option_animationType_Stop
};

Legato::Option* getAnimationType(AnimationType type){
	for(auto option : animationTypeOptions){
		if(option->getInt() == type) return option;
	}
	return nullptr;
}
Legato::Option* getAnimationType(std::string saveString){
	for(auto option : animationTypeOptions){
		if(option->getSaveString() == saveString) return option;
	}
	return nullptr;
}

};
