#pragma once

#include "AnimationTypes.h"
#include "Legato/Editor/Component.h"
#include "Legato/Editor/ListComponent.h"

namespace AnimationSystem{

class Animatable;
class CompositeAnimation;



class Animation : public Legato::Component{
	
	//DECLARE_PROTOTYPE_INTERFACE_METHODS(Animation)
	
public:
	
	virtual void onConstruction() override{
		//Component::onConstruction();
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
		//Component::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		//bool success = Component::onSerialization();
		bool success = true;
		switch(getType()){
			case AnimationType::TARGET:
				success &= serializeAttribute("Type", "Target");
				break;
			case AnimationType::SEQUENCE:
				success &= serializeAttribute("Type", "Sequence");
				break;
			case AnimationType::STOP:
				success &= serializeAttribute("Type", "Stop");
				break;
		}
		if(isCompositeAnimation()) success &= serializeAttribute("IsComposite", true);
		return success;
	}
	virtual bool onDeserialization() override{
		//bool success = Component::onDeserialization();
		bool success = true;
		return success;
	}
	
	static std::shared_ptr<Animation> createInstanceFromTypeString(bool b_isComposite, std::string typeString);
	
	std::shared_ptr<Animatable> getAnimatable(){ return animatable; }
	
	virtual AnimationType getType() = 0;
	
	virtual bool canStartPlayback() = 0;
	virtual void startPlayback() = 0;
	virtual void stopPlayback() = 0;
	
	virtual bool isCompositeAnimation() { return false; }
	bool isTopLevelAnimation(){ return parentComposite == nullptr; }
	void setParentComposite(std::shared_ptr<CompositeAnimation> parent){ parentComposite = parent; }
	std::shared_ptr<CompositeAnimation> getParentComposite(){ return parentComposite; }
	
protected:
	
	friend class Animatable;
	
	//Composite Structure
	std::shared_ptr<CompositeAnimation> parentComposite = nullptr;
	
	//Animatable
	std::shared_ptr<Animatable> animatable = nullptr;
	
};



class CompositeAnimation : public Animation{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(CompositeAnimation)
	
public:
	
	virtual void onConstruction() override{
		Animation::onConstruction();
		childAnimations = Legato::ListComponent<Animation>::createInstance();
		childAnimations->setSaveString("ChildAnimations");
		childAnimations->setEntrySaveString("Animation");
		childAnimations->setEntryConstructor([](Serializable& abstract) -> std::shared_ptr<Animation> {
			bool b_isComposite;
			abstract.deserializeAttribute("IsComposite", b_isComposite);
			std::string typeString;
			if(abstract.deserializeAttribute("Type", typeString)){
				return Animation::createInstanceFromTypeString(b_isComposite, typeString);
			}
			else return nullptr;
		});
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
		Animation::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		bool success = Animation::onSerialization();
		success &= childAnimations->serializeIntoParent(this);
		return success;
	}
	virtual bool onDeserialization() override{
		bool success = Animation::onSerialization();
		success &= childAnimations->deserializeFromParent(this);
		return success;
	}
	
	void setType(AnimationType animationType){ type = animationType; }
	virtual AnimationType getType() override { return type; }
	
	virtual bool isCompositeAnimation() override { return true; }
	
	void addChildAnimation(std::shared_ptr<Animation> animation){
		childAnimations->addEntry(animation);
		animation->setParentComposite(downcasted_shared_from_this<CompositeAnimation>());
	}
	
	void removeChildAnimation(std::shared_ptr<Animation> animation){
		childAnimations->removeEntry(animation);
		animation->setParentComposite(nullptr);
	}
	
	virtual bool canStartPlayback() override { return false; }
	virtual void startPlayback() override {}
	virtual void stopPlayback() override {}
	
	const std::vector<std::shared_ptr<Animation>>& getChildAnimations(){ return childAnimations->getEntries(); }
	
private:
	
	AnimationType type;
	
	//Composite Structure
	std::shared_ptr<Legato::ListComponent<Animation>> childAnimations = nullptr;
	
};









//————————— LEAF ANIMATIONS





class TargetAnimation : public Animation{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(TargetAnimation)
	
public:
	
	virtual void onConstruction() override{
		Animation::onConstruction();
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
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
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
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
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
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
