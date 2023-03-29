#pragma once

namespace AnimationSystem{

class Animatable;

	class AnimatableOwner{
	public:
		
		void addAnimatable(std::shared_ptr<Animatable> animatable);
		void removeAnimatable(std::shared_ptr<Animatable> animatable);
		
		std::vector<std::shared_ptr<Animatable>> animatables = {};
		
	};

};
