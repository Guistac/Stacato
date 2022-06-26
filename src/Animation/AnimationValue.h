#pragma once

#include "Motion/MotionTypes.h"

struct AnimatableStateValue;
struct AnimatablePositionValue;
struct AnimatableBooleanValue;

/*
struct IntegerAnimationValue;
struct RealAnimationValue;
struct Vector2DAnimationValue;
struct Vector3DAnimationValue;
struct Position2DAnimationValue;
struct Position3DAnimationValue;
struct VelocityAnimationValue;
struct Velocity2DAnimationValue;
struct Velocity3DAnimationValue;
*/
 
struct AnimationValue : public std::enable_shared_from_this<AnimationValue>{
	virtual AnimatableType getType() = 0;
	
	std::shared_ptr<AnimatableBooleanValue> toBoolean();
	std::shared_ptr<AnimatableStateValue> toState();
	std::shared_ptr<AnimatablePositionValue> toPosition();
	
	/*
	std::shared_ptr<IntegerAnimationValue> toInteger(){ return std::dynamic_pointer_cast<IntegerAnimationValue>(shared_from_this()); }
	std::shared_ptr<RealAnimationValue> toReal(){ return std::dynamic_pointer_cast<RealAnimationValue>(shared_from_this()); }
	std::shared_ptr<Vector2DAnimationValue> to2dVector(){ return std::dynamic_pointer_cast<Vector2DAnimationValue>(shared_from_this()); }
	std::shared_ptr<Vector3DAnimationValue> to3dVector(){ return std::dynamic_pointer_cast<Vector3DAnimationValue>(shared_from_this()); }
	std::shared_ptr<Position2DAnimationValue> to2dPosition(){ return std::dynamic_pointer_cast<Position2DAnimationValue>(shared_from_this()); }
	std::shared_ptr<Position3DAnimationValue> to3dPosition(){ return std::dynamic_pointer_cast<Position3DAnimationValue>(shared_from_this()); }
	std::shared_ptr<VelocityAnimationValue> toVelocity(){ return std::dynamic_pointer_cast<VelocityAnimationValue>(shared_from_this()); }
	std::shared_ptr<Velocity2DAnimationValue> to2dVelocity(){ return std::dynamic_pointer_cast<Velocity2DAnimationValue>(shared_from_this()); }
	std::shared_ptr<Velocity3DAnimationValue> to3dVelocity(){ return std::dynamic_pointer_cast<Velocity3DAnimationValue>(shared_from_this()); }
	*/
	
	
	
	static std::shared_ptr<AnimatableBooleanValue> makeBoolean();
	static std::shared_ptr<AnimatableStateValue> makeState();
	static std::shared_ptr<AnimatablePositionValue> makePosition();
	
	/*
	static std::shared_ptr<IntegerAnimationValue> makeInteger(){ return std::make_shared<IntegerAnimationValue>(); }
	static std::shared_ptr<RealAnimationValue> makeReal(){ return std::make_shared<RealAnimationValue>(); }
	static std::shared_ptr<Vector2DAnimationValue> make2dVector(){ return std::make_shared<Vector2DAnimationValue>(); }
	static std::shared_ptr<Vector3DAnimationValue> make3dVector(){ return std::make_shared<Vector3DAnimationValue>(); }
	static std::shared_ptr<Position2DAnimationValue> make2dPosition(){ return std::make_shared<Position2DAnimationValue>(); }
	static std::shared_ptr<Position3DAnimationValue> make3dPosition(){ return std::make_shared<Position3DAnimationValue>(); }
	static std::shared_ptr<VelocityAnimationValue> makeVelocity(){ return std::make_shared<VelocityAnimationValue>(); }
	static std::shared_ptr<Velocity2DAnimationValue> make2dVelocity(){ return std::make_shared<Velocity2DAnimationValue>(); }
	static std::shared_ptr<Velocity3DAnimationValue> make3dVelocity(){ return std::make_shared<Velocity3DAnimationValue>(); }
	*/
};


/*
struct IntegerAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::INTEGER; }
	int value;
};

struct RealAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::REAL; }
	double value;
};

struct Vector2DAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::VECTOR_2D; }
	glm::dvec2 value;
};

struct Vector3DAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::VECTOR_3D; }
	glm::dvec3 value;
};

struct Position2DAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::POSITION_2D; }
	glm::dvec2 position;
	glm::dvec2 velocity;
	glm::dvec2 acceleration;
};

struct Position3DAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::POSITION_3D; }
	glm::dvec3 position;
	glm::dvec3 velocity;
	glm::dvec3 acceleration;
};

struct VelocityAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::VELOCITY; }
	double velocity;
	double acceleration;
};

struct Velocity2DAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::VELOCITY_2D; }
	glm::dvec2 velocity;
	glm::dvec2 acceleration;
};

struct Velocity3DAnimationValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::VELOCITY_3D; }
	glm::dvec3 velocity;
	glm::dvec3 acceleration;
};
*/

