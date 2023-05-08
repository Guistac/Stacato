#pragma once

#include "Animatable.h"

namespace AnimationSystem{

class PositionAnimatable : public AnimationSystem::LeafAnimatable{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(PositionAnimatable)
	
public:
	
	static std::shared_ptr<PositionAnimatable> createInstance(std::string name, std::string saveString){
		auto newAnimatable = createInstance();
		newAnimatable->setName(name);
		newAnimatable->setSaveString(saveString);
		return newAnimatable;
	}
	
	virtual void onConstruction() override{
		LeafAnimatable::onConstruction();
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
		LeafAnimatable::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		bool success = LeafAnimatable::onSerialization();
		return success;
	}
	virtual bool onDeserialization() override{
		bool success = LeafAnimatable::onDeserialization();
		return success;
	}
	
	virtual AnimatableType getType() override {
		return AnimatableType::POSITION;
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
			AnimationSystem::TargetAnimationConstraintType::TIME,
			AnimationSystem::TargetAnimationConstraintType::VELOCITY
		};
		return supportedTargetAnimationConstraintTypes;
	}
	
	virtual void updateAnimationValue() override {}
	
	virtual int getCurveCount() override { return 1; }
	virtual std::vector<std::string>& getCurveNames() override {
		static std::vector<std::string> curveNames = {};
		return curveNames;
	}
	
};

}
