#pragma once

#include "Animatable.h"

class AnimatablePosition : public AnimationSystem::Animatable{
public:
	
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
			AnimationSystem::TargetAnimationConstraintType::TIME,
			AnimationSystem::TargetAnimationConstraintType::VELOCITY
		};
		return supportedTargetAnimationCosntraintTypes;
	}
	
};
