#pragma once

namespace AnimationSystem{

class Animatable;
class Animation;


class LeafAnimation{
public:
	
	bool isTopLevelAnimation(){ return parentCompositeAnimation == nullptr; }
	
private:
	
	std::shared_ptr<Animatable> animatable = nullptr;
	std::shared_ptr<Animation> parentCompositeAnimation = nullptr;
	
};



class Animation : public LeafAnimation{
public:
	
	virtual bool canStartPlayback() = 0;
	virtual void startPlayback() = 0;
	virtual void stopPlayback() = 0;
	
private:
	
	std::vector<std::shared_ptr<Animation>> childAnimations = {};
	
};



}
