#pragma once

#include "NodeGraph/ioNode.h"

class LinearAxis : public ioNode {
public:

	DEFINE_AXIS_NODE("Linear Axis", LinearAxis);

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

	virtual void process() {}

	virtual void nodeSpecificGui() {}
	virtual bool load(tinyxml2::XMLElement* xml) { return false; }
	virtual bool save(tinyxml2::XMLElement* xml) { return false; }

};

class RotatingAxis : public ioNode {
public:

	DEFINE_AXIS_NODE("Rotating Axis", RotatingAxis);

	std::shared_ptr<ioData> deviceLink = std::make_shared<ioData>(DataType::DEVICE_LINK, DataDirection::NODE_INPUT, "Actuators", ioDataFlags_AcceptMultipleInputs | ioDataFlags_NoDataField);
	std::shared_ptr<ioData> positionFeedback = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "Actual Position");
	std::shared_ptr<ioData> positionReference = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Reference");

	std::shared_ptr<ioData> velocityCommand = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> positionCommand = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> positionError = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position Error", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> resetPositionFeedback = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Reset Position Feedback", ioDataFlags_DisableDataField);

	virtual void assignIoData() {
		addIoData(deviceLink);
		addIoData(positionFeedback);
		addIoData(positionReference);

		addIoData(velocityCommand);
		addIoData(positionCommand);
		addIoData(positionError);
		addIoData(resetPositionFeedback);
	}

	virtual void process() {}

	virtual void nodeSpecificGui() {}
	virtual bool load(tinyxml2::XMLElement* xml) { return false; }
	virtual bool save(tinyxml2::XMLElement* xml) { return false; }

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
	virtual bool load(tinyxml2::XMLElement* xml) { return false; }
	virtual bool save(tinyxml2::XMLElement* xml) { return false; }

};

