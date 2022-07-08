#pragma once

#include "Motion/MotionTypes.h"

struct AnimatableStateValue;
struct AnimatablePositionValue;
struct AnimatableBooleanValue;
 
struct AnimationValue : public std::enable_shared_from_this<AnimationValue>{
	virtual AnimatableType getType() = 0;
	
	std::shared_ptr<AnimatableBooleanValue> toBoolean();
	std::shared_ptr<AnimatableStateValue> toState();
	std::shared_ptr<AnimatablePositionValue> toPosition();

	static std::shared_ptr<AnimatableBooleanValue> makeBoolean();
	static std::shared_ptr<AnimatableStateValue> makeState();
	static std::shared_ptr<AnimatablePositionValue> makePosition();
};

