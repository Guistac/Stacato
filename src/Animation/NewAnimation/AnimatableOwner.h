#pragma once

#include "Legato/Editor/NamedObject.h"

namespace AnimationSystem{

	class Animatable;

	class AnimatableOwner : public virtual Legato::NamedObject{
	public:
		
		void addAnimatable(std::shared_ptr<Animatable> animatable);
		void removeAnimatable(std::shared_ptr<Animatable> animatable);
		
		std::vector<std::shared_ptr<Animatable>>& getAnimatables(){ return animatables; }
		
	private:
		
		std::vector<std::shared_ptr<Animatable>> animatables = {};
		
	};

};
