#pragma once

#include "Motion/MotionTypes.h"

struct AnimatableParameterState{
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};

/*
struct AnimatableParameterValue {
	union {
		MotionPoint motion;
		MotionPoint2D motion2D;
		MotionPoint3D motion3D;
		bool boolean;
		int integer;
		double real;
		glm::dvec2 vector2;
		glm::dvec3 vector3;
		AnimatableParameterState* state;
	};
};
*/





struct BooleanParameterValue;
struct IntegerParameterValue;
struct RealParameterValue;
struct StateParameterValue;
struct Vector2DParameterValue;
struct Vector3DParameterValue;
struct PositionParameterValue;
struct Position2DParameterValue;
struct Position3DParameterValue;
struct VelocityParameterValue;
struct Velocity2DParameterValue;
struct Velocity3DParameterValue;

struct AnimatableParameterValue : public std::enable_shared_from_this<AnimatableParameterValue>{
	virtual MachineParameterType getType() = 0;
	
	std::shared_ptr<BooleanParameterValue> toBoolean(){ return std::dynamic_pointer_cast<BooleanParameterValue>(shared_from_this()); }
	std::shared_ptr<IntegerParameterValue> toInteger(){ return std::dynamic_pointer_cast<IntegerParameterValue>(shared_from_this()); }
	std::shared_ptr<RealParameterValue> toReal(){ return std::dynamic_pointer_cast<RealParameterValue>(shared_from_this()); }
	std::shared_ptr<StateParameterValue> toState(){ return std::dynamic_pointer_cast<StateParameterValue>(shared_from_this()); }
	std::shared_ptr<Vector2DParameterValue> to2dVector(){ return std::dynamic_pointer_cast<Vector2DParameterValue>(shared_from_this()); }
	std::shared_ptr<Vector3DParameterValue> to3dVector(){ return std::dynamic_pointer_cast<Vector3DParameterValue>(shared_from_this()); }
	std::shared_ptr<PositionParameterValue> toPosition(){ return std::dynamic_pointer_cast<PositionParameterValue>(shared_from_this()); }
	std::shared_ptr<Position2DParameterValue> to2dPosition(){ return std::dynamic_pointer_cast<Position2DParameterValue>(shared_from_this()); }
	std::shared_ptr<Position3DParameterValue> to3dPosition(){ return std::dynamic_pointer_cast<Position3DParameterValue>(shared_from_this()); }
	std::shared_ptr<VelocityParameterValue> toVelocity(){ return std::dynamic_pointer_cast<VelocityParameterValue>(shared_from_this()); }
	std::shared_ptr<Velocity2DParameterValue> to2dVelocity(){ return std::dynamic_pointer_cast<Velocity2DParameterValue>(shared_from_this()); }
	std::shared_ptr<Velocity3DParameterValue> to3dVelocity(){ return std::dynamic_pointer_cast<Velocity3DParameterValue>(shared_from_this()); }
	
	static std::shared_ptr<BooleanParameterValue> makeBoolean(){ return std::make_shared<BooleanParameterValue>(); }
	static std::shared_ptr<IntegerParameterValue> makeInteger(){ return std::make_shared<IntegerParameterValue>(); }
	static std::shared_ptr<RealParameterValue> makeReal(){ return std::make_shared<RealParameterValue>(); }
	static std::shared_ptr<StateParameterValue> makeState(){ return std::make_shared<StateParameterValue>(); }
	static std::shared_ptr<Vector2DParameterValue> make2dVector(){ return std::make_shared<Vector2DParameterValue>(); }
	static std::shared_ptr<Vector3DParameterValue> make3dVector(){ return std::make_shared<Vector3DParameterValue>(); }
	static std::shared_ptr<PositionParameterValue> makePosition(){ return std::make_shared<PositionParameterValue>(); }
	static std::shared_ptr<Position2DParameterValue> make2dPosition(){ return std::make_shared<Position2DParameterValue>(); }
	static std::shared_ptr<Position3DParameterValue> make3dPosition(){ return std::make_shared<Position3DParameterValue>(); }
	static std::shared_ptr<VelocityParameterValue> makeVelocity(){ return std::make_shared<VelocityParameterValue>(); }
	static std::shared_ptr<Velocity2DParameterValue> make2dVelocity(){ return std::make_shared<Velocity2DParameterValue>(); }
	static std::shared_ptr<Velocity3DParameterValue> make3dVelocity(){ return std::make_shared<Velocity3DParameterValue>(); }
};

struct BooleanParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::BOOLEAN; }
	bool value;
};

struct IntegerParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::INTEGER; }
	int value;
};

struct RealParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::REAL; }
	double value;
};

struct StateParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::STATE; }
	AnimatableParameterState* value;
	std::vector<AnimatableParameterState>* values;
};

struct Vector2DParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::VECTOR_2D; }
	glm::dvec2 value;
};

struct Vector3DParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::VECTOR_3D; }
	glm::dvec3 value;
};

struct PositionParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::POSITION; }
	double position;
	double velocity;
	double acceleration;
};

struct Position2DParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::POSITION_2D; }
	glm::dvec2 position;
	glm::dvec2 velocity;
	glm::dvec2 acceleration;
};

struct Position3DParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::POSITION_3D; }
	glm::dvec3 position;
	glm::dvec3 velocity;
	glm::dvec3 acceleration;
};

struct VelocityParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::VELOCITY; }
	double position;
	double velocity;
	double acceleration;
};

struct Velocity2DParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::VELOCITY_2D; }
	glm::dvec2 position;
	glm::dvec2 velocity;
	glm::dvec2 acceleration;
};

struct Velocity3DParameterValue : public AnimatableParameterValue{
	virtual MachineParameterType getType(){ return MachineParameterType::VELOCITY_3D; }
	glm::dvec3 position;
	glm::dvec3 velocity;
	glm::dvec3 acceleration;
};

