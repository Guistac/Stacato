#pragma once

namespace AnimationSystem{

class AnimatableOwner;

class AnimatableRegistry{
public:
	
	AnimatableRegistry(){}
	
	void registerAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner);
	void unregisterAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner);
	
	std::vector<std::shared_ptr<AnimatableOwner>>& getAnimatableOwners(){ return animatableOwners; }
	
private:
	std::vector<std::shared_ptr<AnimatableOwner>> animatableOwners = {};
	
	int uniqueIDCounter = 1;
	
};

};
