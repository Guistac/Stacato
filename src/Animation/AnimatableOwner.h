#pragma once

class Animatable;
class AnimationValue;

class AnimatableOwner{
public:
	
	virtual bool isAnimatableReadyToMove(std::shared_ptr<Animatable> animatable) = 0;
	virtual bool isAnimatableReadyToStartPlaybackFromValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue>) = 0;
	
	virtual void onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable) = 0;
	virtual void onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable) = 0;
	virtual void onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable) = 0;
	
	virtual std::shared_ptr<AnimationValue> getActualAnimatableValue(std::shared_ptr<Animatable> animatable) = 0;
	
	virtual void fillAnimationDefaults(std::shared_ptr<Animation> animation) = 0;
	virtual bool validateAnimation(std::shared_ptr<Animation> animation) = 0;
	virtual bool generateTargetAnimation(std::shared_ptr<Animation> animation) = 0;
	
};
