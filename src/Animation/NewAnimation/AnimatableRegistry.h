#pragma once

namespace AnimationSystem{

	class AnimatableOwner;
	class Animatable;

	class AnimatableRegistry{
	public:
		
		AnimatableRegistry(){}
		
		void registerAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner);
		void unregisterAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner);
		
		std::vector<std::shared_ptr<AnimatableOwner>>& getAnimatableOwners(){ return animatableOwners; }
		
		std::shared_ptr<Animatable> getAnimatable(int uniqueID);
		
	private:
		
		void registerAnimatable(std::shared_ptr<Animatable> animatable);
		void unregisterAnimatable(std::shared_ptr<Animatable> animatable);
		
		std::vector<std::shared_ptr<AnimatableOwner>> animatableOwners = {};
		
		int uniqueIDCounter = 1;
		
	};

};
