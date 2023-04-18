#pragma once

#include "Animatable.h"

namespace AnimationSystem{

class StateAnimatable : public AnimationSystem::Animatable{
public:
	
	virtual AnimatableType getType() override {
		return AnimatableType::STATE;
	};
	
	virtual std::vector<AnimationSystem::AnimationType>& getSupportedAnimationTypes() override {
		static std::vector<AnimationSystem::AnimationType> supportedAnimationTypes = {
			AnimationSystem::AnimationType::TARGET,
			AnimationSystem::AnimationType::SEQUENCE,
			AnimationSystem::AnimationType::STOP
		};
		return supportedAnimationTypes;
	}
	
	virtual std::vector<TargetAnimationConstraintType>& getSupportTargetAnimationConstraintTypes() override {
		static std::vector<AnimationSystem::TargetAnimationConstraintType> supportedTargetAnimationConstraintTypes = {
			AnimationSystem::TargetAnimationConstraintType::NONE
		};
		return supportedTargetAnimationConstraintTypes;
	}
	
};

}
