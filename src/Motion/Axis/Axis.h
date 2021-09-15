#pragma once

#include "NodeGraph/ioNode.h"

enum PositionReferenceType {
	LOW_LIMIT,
	HIGH_LIMIT,
	LOW_AND_HIGH_LIMIT,
	POSITION_REFERENCE,
	NO_LIMIT,
	POSITION_REFERENCE_COUNT
};

struct PositionReference {
	PositionReferenceType type;
	const char displayName[64];
	const char saveName[64];
};

enum PositionFeedbackType {
	ABSOLUTE_FEEDBACK,
	INCREMENTAL_FEEDBACK,
	NO_FEEDBACK,
	FEEDBACK_COUNT
};

struct PositionFeedback {
	PositionFeedbackType type;
	const char displayName[64];
	const char saveName[64];
};

enum HomingDirectionType {
	NEGATIVE,
	POSITIVE,
	DONT_CARE
};

struct HomingDirection {
	HomingDirectionType type;
	const char displayName[64];
	const char saveName[64];
};

enum ControlMode {
	VELOCITY_CONTROL,
	TARGET_CONTROL,
	FOLLOW_CURVE,
	HOMING
};

class Axis : public ioNode {
public:

	DEFINE_AXIS_NODE("Axis", Axis);

	std::shared_ptr<ioData> deviceLink = std::make_shared<ioData>(DataType::DEVICE_LINK, DataDirection::NODE_INPUT, "Actuators", ioDataFlags_AcceptMultipleInputs | ioDataFlags_NoDataField);
	std::shared_ptr<ioData> positionFeedback = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "Actual Position");
	std::shared_ptr<ioData> positionLimits = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Limits", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> maxMotorVelocity = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "Max Motor Velocity", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> maxMotorAcceleration = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "Max Motor Acceleration", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> motorLoad = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "Motor Load", ioDataFlags_AcceptMultipleInputs);

	std::shared_ptr<ioData> positionCommand = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> positionError = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position Error", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> velocityCommand = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> accelerationCommand = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Acceleration", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> resetPositionFeedback = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Reset Position Feedback", ioDataFlags_DisableDataField);

	double velocityLimit_degreesPerSecond = 0.0;
	double accelerationLimit_degreesPerSecondSquared = 0.0;

	double defaultMovementVelocity_degreesPerSecond = 10.0;
	double defaultMovementAcceleration_degreesPerSecondSquared = 5.0;

	double homingVelocity_degreesPerSecond = 0.0;

	double maxPositiveDeviationFromReference_degrees = 0.0;
	double maxNegativeDeviationFromReference_degrees = 0.0;

	PositionReferenceType positionReferenceType = NO_LIMIT;
	PositionFeedbackType positionFeedbackType = ABSOLUTE_FEEDBACK;
	HomingDirectionType homingDirectionType = DONT_CARE;

	ControlMode controlMode = VELOCITY_CONTROL;
	double velocityControlTarget_degreesPerSecond = 0.0;

	double lastUpdateTime_seconds = 0.0;

	double currentVelocity_degreesPerSecond = 0.0;
	double currentPosition_degrees = 0.0;


	virtual void assignIoData() {
		addIoData(deviceLink);
		addIoData(positionFeedback);
		addIoData(positionLimits);
		addIoData(maxMotorVelocity);
		addIoData(maxMotorAcceleration);
		addIoData(motorLoad);

		addIoData(positionCommand);
		addIoData(positionError);
		addIoData(velocityCommand);
		addIoData(accelerationCommand);
		addIoData(resetPositionFeedback);
	}

	virtual void process();

	virtual void nodeSpecificGui();

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);

};

























class StateMachineAxis : public ioNode {
public:

	DEFINE_AXIS_NODE("State Machine Axis", StateMachineAxis);

	std::shared_ptr<ioData> deviceLink = std::make_shared<ioData>(DataType::DEVICE_LINK, DataDirection::NODE_INPUT, "Actuators", ioDataFlags_AcceptMultipleInputs | ioDataFlags_NoDataField);
	std::shared_ptr<ioData> state0ref = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 0 Feedback");
	std::shared_ptr<ioData> state1ref = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 1 Feedback");
	std::shared_ptr<ioData> state2ref = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 2 Feedback");

	std::shared_ptr<ioData> state0Command = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> state1Command = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> state2Command = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", ioDataFlags_DisableDataField);

	virtual void assignIoData() {
		addIoData(deviceLink);
		addIoData(state0ref);
		addIoData(state1ref);
		addIoData(state2ref);

		addIoData(state0Command);
		addIoData(state1Command);
		addIoData(state2Command);
	}

	virtual void process() {}

	virtual void nodeSpecificGui() {}
	virtual bool load(tinyxml2::XMLElement* xml) { return true; }
	virtual bool save(tinyxml2::XMLElement* xml) { return true; }

};

