#include <pch.h>

#include "AnimationValue.h"

#include "Animatables/AnimatableState.h"
#include "Animatables/AnimatablePosition.h"


std::shared_ptr<AnimatableStateValue> AnimationValue::toState(){ return std::static_pointer_cast<AnimatableStateValue>(shared_from_this()); }
std::shared_ptr<AnimatablePositionValue> AnimationValue::toPosition(){ return std::static_pointer_cast<AnimatablePositionValue>(shared_from_this()); }

std::shared_ptr<AnimatableStateValue> AnimationValue::makeState(){ return std::make_shared<AnimatableStateValue>(); }
std::shared_ptr<AnimatablePositionValue> AnimationValue::makePosition(){ return std::make_shared<AnimatablePositionValue>(); }
