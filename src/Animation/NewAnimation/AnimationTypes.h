#pragma once

#include "Legato/Editor/Parameters/OptionParameter.h"

namespace AnimationSystem{

	enum class AnimatableStatus{
		OFFLINE,
		NOT_READY,
		READY
	};


	enum AnimatableType{
		COMPOSITE = 0,
		BOOLEAN = 1,
		INTEGER = 2,
		REAL = 3,
		STATE = 4,
		POSITION = 7,
		VELOCITY = 8
	};
	Legato::Option* getAnimatableType(AnimatableType type);
	Legato::Option* getAnimatableType(std::string saveString);


	enum AnimationType{
		TARGET = 1,
		SEQUENCE = 2,
		STOP = 3
	};
	Legato::Option* getAnimationType(AnimationType type);
	Legato::Option* getAnimationType(std::string saveString);


	enum TargetAnimationConstraintType{
		TARGET_CONSTRAINT_NONE = 0,
		TARGET_CONSTRAINT_TIME = 1,
		TARGET_CONSTRAINT_VELOCITY = 2
	};

	enum StopAnimationConstraintType{
		STOP_CONSTRAINT_TIME = 0,
		STOP_CONSTRAINT_RAMP = 1
	};

};
