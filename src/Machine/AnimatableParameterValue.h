#pragma once

class AnimatableParameter;
class AnimatableParameterState;
struct StateParameterValue;

namespace tinyxml2{ class XMLElement; }

struct MotionPoint{
	double position;
	double velocity;
	double acceleration;
};

struct MotionPoint2D{
	MotionPoint x;
	MotionPoint y;
};

struct MotionPoint3D{
	MotionPoint x;
	MotionPoint y;
	MotionPoint z;
};

struct AnimatableParameterValue {
	union {
		MotionPoint motion;
		MotionPoint2D motion2D;
		MotionPoint3D motion3D;
		bool boolean;
		int integer;
		double real;
		glm::vec2 vector2;
		glm::vec3 vector3;
		AnimatableParameterState* state;
	};
};
