#pragma once

#include "AnimationTypes.h"
#include "Legato/Editor/Component.h"
#include "Legato/Editor/ListComponent.h"

namespace AnimationSystem{

	class Animatable;
	class CompositeAnimation;
	class AnimatableRegistry;


	class Animation : public Legato::Component{
		
	public:
		
		static std::shared_ptr<Animation> createInstanceFromAbstractSerializable(Serializable& abstract);
		void setAnimatableRegistry(std::shared_ptr<AnimatableRegistry> registry){ animatableRegistry = registry; }
		
		virtual void onConstruction() override{}
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{}
		virtual bool onSerialization() override;
		virtual bool onDeserialization() override;
		
		std::shared_ptr<Animatable> getAnimatable(){ return animatable; }
		
		virtual AnimationType getType() = 0;
		
		virtual bool canStartPlayback() = 0;
		virtual void startPlayback() = 0;
		virtual void stopPlayback() = 0;
		
		virtual bool isCompositeAnimation() { return false; }
		bool isTopLevelAnimation(){ return parentComposite == nullptr; }
		void setParentComposite(std::shared_ptr<CompositeAnimation> parent){ parentComposite = parent; }
		std::shared_ptr<CompositeAnimation> getParentComposite(){ return parentComposite; }
		
		int getAnimatableUniqueID(){ return animatableUniqueID; }
		void setAnimatable(std::shared_ptr<Animatable> animatable_){
			animatable = animatable_;
			onSetAnimatable();
		}
		virtual void onSetAnimatable() = 0;
		
		virtual void parameterGui() = 0;
		
	protected:
		
		friend class Animatable;
		
		//Composite Structure
		std::shared_ptr<CompositeAnimation> parentComposite = nullptr;
		
		//Animatable
		std::shared_ptr<Animatable> animatable = nullptr;
		int animatableUniqueID = -1;
		
		std::shared_ptr<AnimatableRegistry> animatableRegistry = nullptr;
		
	};



	class CompositeAnimation : public Animation{
		
		DECLARE_PROTOTYPE_IMPLENTATION_METHODS(CompositeAnimation)
		
	public:
		
		virtual void onConstruction() override;
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
			Animation::onCopyFrom(source);
		}
		virtual bool onSerialization() override;
		virtual bool onDeserialization() override;
		
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
		
		virtual void parameterGui() override {}
		
		virtual void onSetAnimatable() override {}
		
	private:
		
		AnimationType type;
		
		//Composite Structure
		std::shared_ptr<Legato::ListComponent<Animation>> childAnimations = nullptr;
		
	};


}
