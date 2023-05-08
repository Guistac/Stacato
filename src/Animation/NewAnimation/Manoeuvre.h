#pragma once

#include "Legato/Editor/Component.h"
#include "Legato/Editor/ListComponent.h"
#include "Animation.h"

namespace AnimationSystem{

	class Animation;
	class Animatable;
	class AnimatableRegistry;

	class Manoeuvre : public Legato::Component{
		
		DECLARE_PROTOTYPE_IMPLENTATION_METHODS(Manoeuvre)
		
	public:
		
		void setAnimatableRegistry(std::shared_ptr<AnimatableRegistry> registry){ animatableRegistry = registry; }
		
		virtual void onConstruction() override;
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override;
		virtual bool onSerialization() override;
		virtual bool onDeserialization() override;
		
		void editorGui();
		
		void addAnimation(std::shared_ptr<Animation> animation);
		void removeAnimation(std::shared_ptr<Animation> animation);
		
	private:
		
		std::shared_ptr<AnimatableRegistry> animatableRegistry = nullptr;
		std::shared_ptr<Legato::ListComponent<Animation>> animations = nullptr;
		
	};

};
