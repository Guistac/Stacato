#pragma once

namespace AnimationSystem{


enum class AnimatableType{
	   BOOLEAN,
	   INTEGER,
	   REAL,
	   STATE,
	   VECTOR_2D,
	   VECTOR_3D,
	   POSITION,
	   POSITION_2D,
	   POSITION_3D,
	   VELOCITY,
	   VELOCITY_2D,
	   VELOCITY_3D,
	   COMPOSITE
   };

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

	enum class AnimationType{
		TARGET,
		SEQUENCE,
		STOP
	};

};
