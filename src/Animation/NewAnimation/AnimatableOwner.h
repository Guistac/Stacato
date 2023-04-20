#pragma once

#include "Legato/Editor/NamedObject.h"
#include "Legato/Editor/SharedObject.h"
#include "Legato/Editor/Serializable.h"
#include "Legato/Editor/ListComponent.h"

#include "Animatable.h"

namespace AnimationSystem{

	class AnimatableOwner : public virtual Legato::NamedObject, public virtual Legato::Serializable, public Legato::SharedObject<AnimatableOwner>{
	public:
		
		virtual bool onSerialization() override;
		virtual bool onDeserialization() override;
		
		void addAnimatable(std::shared_ptr<Animatable> animatable);
		void removeAnimatable(std::shared_ptr<Animatable> animatable);
		
		std::vector<std::shared_ptr<Animatable>>& getAnimatables(){ return animatables->getEntries(); }
		
	private:
		
		friend class AnimatableRegistry;
		std::shared_ptr<Legato::ListComponent<Animatable>> animatables = Legato::ListComponent<Animatable>::createInstance();
		
	};

};
