#pragma once

#include "AnimationTypes.h"
#include "Legato/Editor/Component.h"
#include "Legato/Editor/ListComponent.h"

namespace AnimationSystem{

class Animatable;
class CompositeAnimation;



class Animation : public Legato::Component{
	
	DECLARE_PROTOTYPE_INTERFACE_METHODS(Animation)
	
public:
	
	virtual void onConstruction() override{
		Component::onConstruction();
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		Component::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		bool success = Component::onSerialization();
		return success;
	}
	virtual bool onDeserialization() override{
		bool success = Component::onDeserialization();
		return success;
	}
	
	std::shared_ptr<Animatable> getAnimatable(){ return animatable; }
	
	virtual AnimationType getType() = 0;
	
	virtual bool canStartPlayback() = 0;
	virtual void startPlayback() = 0;
	virtual void stopPlayback() = 0;
	
	virtual bool isCompositeAnimation() { return false; }
	bool isTopLevelAnimation(){ return parentComposite == nullptr; }
	void setParentComposite(std::shared_ptr<CompositeAnimation> parent){ parentComposite = parent; }
	std::shared_ptr<CompositeAnimation> getParentComposite(){ return parentComposite; }
	const std::vector<std::shared_ptr<Animation>>& getChildAnimations(){ return childAnimations; }
	
protected:
	
	friend class Animatable;
	
	//Composite Structure
	std::shared_ptr<CompositeAnimation> parentComposite = nullptr;
	
	std::vector<std::shared_ptr<Animation>> childAnimations = {};
	
	//Animatable
	std::shared_ptr<Animatable> animatable = nullptr;
	
};



class CompositeAnimation : public Animation{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(CompositeAnimation)
	
public:
	
	virtual void onConstruction() override{
		Animation::onConstruction();
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		Animation::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		bool success = Animation::onSerialization();
		return success;
	}
	virtual bool onDeserialization() override{
		bool success = Animation::onSerialization();
		return success;
	}
	
	void setType(AnimationType animationType){ type = animationType; }
	virtual AnimationType getType() override { return type; }
	
	virtual bool isCompositeAnimation() override { return true; }
	
	void addChildAnimation(std::shared_ptr<Animation> animation){
		childAnimations.push_back(animation);
		animation->setParentComposite(std::static_pointer_cast<CompositeAnimation>(shared_from_this()));
	}
	
	void removeChildAnimation(std::shared_ptr<Animation> animation){
		for(int i = (int)childAnimations.size() - 1; i >= 0; i--){
			if(childAnimations[i] == animation){
				animation->setParentComposite(nullptr);
				childAnimations.erase(childAnimations.begin() + i);
				break;
			}
		}
	}
	
	virtual bool canStartPlayback() override { return false; }
	virtual void startPlayback() override {}
	virtual void stopPlayback() override {}
	
private:
	
	AnimationType type;
	
};









//————————— LEAF ANIMATIONS





class TargetAnimation : public Animation{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(TargetAnimation)
	
public:
	
	virtual void onConstruction() override{
		Animation::onConstruction();
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		Animation::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		bool success = Animation::onSerialization();
		return success;
	}
	virtual bool onDeserialization() override{
		bool success = Animation::onDeserialization();
		return success;
	}
	
	virtual AnimationType getType() override { return AnimationType::TARGET; }
	
	virtual bool canStartPlayback() override { return false; }
	virtual void startPlayback() override {}
	virtual void stopPlayback() override {}
	
	//-curve interpolation type parameter
	//-curve target parameter
	//-constraints to build curve on playback start:
	//	-constraint type: velocity, time, none
	//	-curve ramps, in and out
	
};

class SequenceAnimation : public Animation{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(SequenceAnimation)
	
public:
	
	virtual void onConstruction() override{
		Animation::onConstruction();
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		Animation::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		bool success = Animation::onSerialization();
		return success;
	}
	virtual bool onDeserialization() override{
		bool success = Animation::onDeserialization();
		return success;
	}
	
	virtual AnimationType getType() override { return AnimationType::SEQUENCE; }
	
	virtual bool canStartPlayback() override { return false; }
	virtual void startPlayback() override {}
	virtual void stopPlayback() override {}
	
	//-curve interpolation type parameter
	//-curve start point (with time, velocity and ramps)
	//-curve target parameter (with time velocity and ramps)
	//-curve duration parameter
	
};

class StopAnimation : public Animation{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(StopAnimation)
	
public:
	
	virtual void onConstruction() override{
		Animation::onConstruction();
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		Animation::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		bool success = Animation::onSerialization();
		return success;
	}
	virtual bool onDeserialization() override{
		bool success = Animation::onDeserialization();
		return success;
	}
	
	virtual AnimationType getType() override { return AnimationType::STOP; }
	
	virtual bool canStartPlayback() override { return false; }
	virtual void startPlayback() override {}
	virtual void stopPlayback() override {}
	
	//-curve interpolation type parameter
	//-constraints to build curve on playback start:
	//	-stopping ramp
	
};



}
