#pragma once

namespace AnimationSystem{

	class Animation;
	class AnimationValue;
	class CompositeAnimatable;




	class Animatable{
	public:
		
		enum class Type {
			BOOLEAN,
			INTEGER,
			REAL,
			STATE,
			VECTOR_2D,
			VECTOR_3D,
			POSITION,
			POSITION_2D,
			POSITION_3D,
			VELOCITY,
			VELOCITY_2D,
			VELOCITY_3D,
			COMPOSITE
		};

		enum class State{
			OFFLINE,
			NOT_READY,
			READY
		}state = State::OFFLINE;
		
		virtual Type getType() = 0;
		
		bool isComposite() { return getType() == AnimatableType::COMPOSITE; }
		bool isTopLevelAnimatable(){ return parentComposite == nullptr; }
		std::shared_ptr<CompositeAnimatable> getParentComposite(){ return parentComposite; }
		
	protected:
		std::shared_ptr<CompositeAnimatable> parentComposite = nullptr;
		
	};






	class LeafAnimatable : public Animatable{
	public:
		
		std::shared_ptr<AnimationValue> getAnimationValue(){ return animationValue; }
		virtual void updateAnimationValue() = 0;
		
		
		virtual int getCurveCount() = 0;
		virtual std::vector<std::string>& getCurveNames() = 0;
		
		virtual std::vector<InterpolationType>& getCompatibleInterpolationTypes() = 0;
		virtual std::shared_ptr<Parameter> makeParameter() = 0;
		virtual void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) = 0;
		virtual void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) = 0;
		virtual std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) = 0;
		virtual bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) = 0;
		virtual std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) = 0;
		virtual std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) = 0;
		
	private:
		
		//each animatable has to initialize this on construction
		std::shared_ptr<AnimationValue> animationValue;
		
		//Animations
		std::vector<std::shared_ptr<Animation>> animations = {};
		std::shared_ptr<Animation> playingAnimation = nullptr;
		
	};






	class CompositeAnimatable : public Animatable{
	public:
		
		virtual Type getType() override { return Type::COMPOSITE; }
		
		void addChildAnimatable(std::shared_ptr<AbstractAnimatable> animatable){
			childAnimatables.push_back(animatable);
			animatable->parentComposite = animatable;
		}
		
		void removeChildAnimatable(std::shared_ptr<AbstractAnimatable> animatable){
			for(int i = childAnimatables.size() - 1; i >= 0; i--){
				if(childAnimatables[i] == animatable){
					animatable->parentComposite = nullptr;
					childAnimatables.erase(childAnimatables.begin() + i);
					break;
				}
			}
		}
		
		const std::vector<std::shared_ptr<AbstractAnimatable>>& getChildAnimatables(){ return childAnimatables; }
		
	private:
		
		std::vector<std::shared_ptr<AbstractAnimatable>> childAnimatables;
		
	};


	//simple animatables have their own state and can be controlled individually
	//composite animatables hold child animatables and can have animations
	//child animatables cannot be individually controlled and do not have their own state

};
