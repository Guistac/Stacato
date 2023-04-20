#pragma once

#include "AnimationTypes.h"
#include "Legato/Editor/Component.h"

namespace AnimationSystem{

	class Animation;
	class AnimationValue;
	class CompositeAnimatable;
	class AnimatableOwner;

	class Animatable : public Legato::Component{
		
	public:
		
		virtual void onConstruction() override{
			Component::onConstruction();
		}
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
			Component::onCopyFrom(source);
		}
		virtual bool onSerialization() override{
			bool success = Component::onSerialization();
			success &= serializeAttribute("UniqueID", uniqueID);
			return success;
		}
		virtual bool onDeserialization() override{
			bool success = Component::onDeserialization();
			success &= deserializeAttribute("UniqueID", uniqueID);
			return success;
		}
		
		AnimatableStatus status = AnimatableStatus::OFFLINE;
		
		virtual AnimatableType getType() = 0;
		virtual std::vector<AnimationType>& getSupportedAnimationTypes() = 0;
		virtual std::vector<TargetAnimationConstraintType>& getSupportTargetAnimationConstraintTypes() = 0;
		
		void addAnimation(std::shared_ptr<Animation> animation){ animations.push_back(animation); }
		void removeAnimation(std::shared_ptr<Animation> animation) {
			for(int i = (int)animations.size() - 1; i >= 0; i--){
				if(animations[i] == animation){
					animations.erase(animations.begin() + i);
					break;
				}
			}
		}
		void deleteAllAnimations();
		
		virtual bool isCompositeAnimatable() { return false; }
		bool isTopLevelAnimatable(){ return parentComposite == nullptr; }
		void setParentComposite(std::shared_ptr<CompositeAnimatable> parentAnimatable){ parentComposite = parentAnimatable; }
		std::shared_ptr<CompositeAnimatable> getParentComposite(){ return parentComposite; }
		const std::vector<std::shared_ptr<Animatable>>& getChildAnimatables(){ return childAnimatables; }
		
		std::shared_ptr<Animation> makeAnimation(AnimationType type);
		
		std::shared_ptr<AnimatableOwner> getOwner(){ return owner; }
		
	protected:
		
		//Composite Structure
		std::shared_ptr<CompositeAnimatable> parentComposite = nullptr;
		std::vector<std::shared_ptr<Animatable>> childAnimatables;
		
		//Animations
		std::vector<std::shared_ptr<Animation>> animations = {};
		std::shared_ptr<Animation> playingAnimation = nullptr;
		
		friend class AnimatableOwner;
		friend class AnimatableRegistry;
		
		//Owner
		std::shared_ptr<AnimatableOwner> owner = nullptr;
		int uniqueID = -1;
		
	};



	class CompositeAnimatable : public Animatable{
	
		DECLARE_PROTOTYPE_IMPLENTATION_METHODS(CompositeAnimatable)
		
	public:
		
		virtual void onConstruction() override{
			Animatable::onConstruction();
		}
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
			Animatable::onCopyFrom(source);
		}
		virtual bool onSerialization() override{
			bool success = Animatable::onSerialization();
			return success;
		}
		virtual bool onDeserialization() override{
			bool success = Animatable::onDeserialization();
			return success;
		}
		
		virtual AnimatableType getType() override { return AnimatableType::COMPOSITE; }
		virtual bool isCompositeAnimatable() override { return true; }
		
		void addChildAnimatable(std::shared_ptr<Animatable> animatable){
			childAnimatables.push_back(animatable);
			animatable->setParentComposite(downcasted_shared_from_this<CompositeAnimatable>());
		}
		
		void removeChildAnimatable(std::shared_ptr<Animatable> animatable){
			for(int i = (int)childAnimatables.size() - 1; i >= 0; i--){
				if(childAnimatables[i] == animatable){
					animatable->setParentComposite(nullptr);
					childAnimatables.erase(childAnimatables.begin() + i);
					break;
				}
			}
		}
		
		void setSupportedAnimationTypes(std::vector<AnimationType> supportedTypes){
			supportedAnimationTypes = supportedTypes;
		}
		
		virtual std::vector<AnimationType>& getSupportedAnimationTypes() override {
			return supportedAnimationTypes;
		}
		
		virtual std::vector<TargetAnimationConstraintType>& getSupportTargetAnimationConstraintTypes() override {
			static std::vector<TargetAnimationConstraintType> output;
			return output;
		}
		
		
	private:
		
		std::vector<AnimationType> supportedAnimationTypes = {};
		
	};


	class LeafAnimatable : public Animatable{
		
	public:
		
		virtual void onConstruction() override{
			Animatable::onConstruction();
		}
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
			Animatable::onCopyFrom(source);
		}
		virtual bool onSerialization() override{
			bool success = Animatable::onSerialization();
			return success;
		}
		virtual bool onDeserialization() override{
			bool success = Animatable::onDeserialization();
			return success;
		}
		
		std::shared_ptr<AnimationValue> getAnimationValue(){ return animationValue; }
		virtual void updateAnimationValue() = 0;
		
		virtual int getCurveCount() = 0;
		virtual std::vector<std::string>& getCurveNames() = 0;
		
		/*
		virtual std::vector<InterpolationType>& getCompatibleInterpolationTypes() = 0;
		virtual std::shared_ptr<Parameter> makeParameter() = 0;
		virtual void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) = 0;
		virtual void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) = 0;
		virtual std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) = 0;
		virtual bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) = 0;
		virtual std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) = 0;
		virtual std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) = 0;
		*/
		 
	private:
		
		//each animatable has to initialize this on construction
		std::shared_ptr<AnimationValue> animationValue;
	
	};


	//leaf animatables can be controlled individually and have their own state
	//composite animatables hold child animatables, can be controlled and have their own state
	//child animatables cannot be controlled and do not have their own state

};
