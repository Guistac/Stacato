#pragma once

#include "Motion/MotionTypes.h"

struct AnimatableStateValue;
struct AnimatablePositionValue;
struct AnimatableBooleanValue;
struct AnimatableRealValue;
 
struct AnimationValue : public std::enable_shared_from_this<AnimationValue>{
	virtual AnimatableType getType() = 0;
	
	std::shared_ptr<AnimatableBooleanValue> toBoolean();
	std::shared_ptr<AnimatableStateValue> toState();
	std::shared_ptr<AnimatablePositionValue> toPosition();
	std::shared_ptr<AnimatableRealValue> toReal();

	static std::shared_ptr<AnimatableBooleanValue> makeBoolean();
	static std::shared_ptr<AnimatableStateValue> makeState();
	static std::shared_ptr<AnimatablePositionValue> makePosition();
	static std::shared_ptr<AnimatableRealValue> makeReal();
};

