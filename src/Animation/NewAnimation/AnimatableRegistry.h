#pragma once

namespace AnimationSystem{

class AnimatableOwner;

class AnimatableRegistry{
public:
	
	AnimatableRegistry(){}
	
	void registerAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner){
		animatableOwners.push_back(animatableOwner);
	}
	
	void unregisterAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner){
		for(int i = (int)animatableOwners.size() - 1; i >= 0; i--){
			if(animatableOwners[i] == animatableOwner){
				animatableOwners.erase(animatableOwners.begin() + i);
				break;
			}
		}
	}
	
	std::vector<std::shared_ptr<AnimatableOwner>>& getAnimatableOwners(){ return animatableOwners; }
	
private:
	std::vector<std::shared_ptr<AnimatableOwner>> animatableOwners = {};
	
};

};
