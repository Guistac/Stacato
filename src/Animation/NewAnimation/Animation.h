#pragma once

namespace AnimationSystem{

class Animatable;
class CompositeAnimation;

class Animation{
public:
	
	std::shared_ptr<Animatable> getAnimatable(){ return animatable; }
	
	virtual bool canStartPlayback() = 0;
	virtual void startPlayback() = 0;
	virtual void stopPlayback() = 0;
	
	virtual bool isCompositeAnimation() { return false; }
	bool isTopLevelAnimation(){ return parentComposite == nullptr; }
	std::shared_ptr<CompositeAnimatable> getParentComposite(){ return parentComposite; }
	const std::vector<std::shared_ptr<BaseAnimatable>>& getChildAnimations(){ return childAnimations; }
	
protected:
	
	//Composite Structure
	std::shared_ptr<CompositeAnimation> parentComposite = nullptr;
	std::vector<std::shared_ptr<Animation>> childAnimations;
	
	//Animatable
	std::shared_ptr<Animatable> animatable = nullptr;
	
};



class CompositeAnimation : public BaseAnimation{
public:
	
	virtual bool isCompositeAnimation() override { return true; }
	
	void addChildAnimation(std::shared_ptr<BaseAnimation> animation){
		childAnimations.push_back(animation);
		animation->parentComposite = shared_from_this();
	}
	
	void removeChildAnimation(std::shared_ptr<BaseAnimation> animation){
		for(int i = childAnimations.size() - 1; i >= 0; i--){
			if(childAnimations[i] == animation){
				animation->parentComposite = nullptr;
				childAnimations.erase(childAnimations.begin() + i);
				break;
			}
		}
	}
	
};


class TargetAnimation : public BaseAnimation{
	
	//-curve interpolation type parameter
	//-curve target parameter
	//-constraints to build curve on playback start:
	//	-constraint type: velocity, time, none
	//	-curve ramps, in and out
	
};

class SequenceAnimation : public BaseAnimation{
	
	//-curve interpolation type parameter
	//-curve start point (with time, velocity and ramps)
	//-curve target parameter (with time velocity and ramps)
	//-curve duration parameter
	
};

class StopAnimation : public BaseAnimation{
	
	//-curve interpolation type parameter
	//-constraints to build curve on playback start:
	//	-stopping ramp
	
};



}
