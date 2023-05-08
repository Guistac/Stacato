#pragma once

#include "Legato/Editor/Parameters/OptionParameter.h"

namespace AnimationSystem{

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

   enum class AnimatableStatus{
	   OFFLINE,
	   NOT_READY,
	   READY
   };

   enum class TargetAnimationConstraintType{
	   NONE,
	   TIME,
	   VELOCITY
   };

	enum AnimationType{
		TARGET = 1,
		SEQUENCE = 2,
		STOP = 3
	};

	Legato::Option* getAnimationType(AnimationType type);
	Legato::Option* getAnimationType(std::string saveString);

};
