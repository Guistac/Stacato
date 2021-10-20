#include "Motion/Machine/Machine.h"

class HoodedLiftStateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(HoodedLiftStateMachine, "Hooded Lift State Machine", "HoodedLiftStateMachine");

	std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "GPIO Device");

	std::shared_ptr<NodePin> hoodShutSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Shut");
	std::shared_ptr<NodePin> hoodOpenSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Open");
	std::shared_ptr<NodePin> platformLoweredSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Platform Lowered");
	std::shared_ptr<NodePin> platformRaisedSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Platform Raised");
	std::shared_ptr<NodePin> emergencyStopSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Emergency Stop Active");
	std::shared_ptr<NodePin> remoteControlEnabledSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Remote Control Enabled");
	std::shared_ptr<NodePin> hoodMotorFuseSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Motor Fuse");
	std::shared_ptr<NodePin> platformMotorFuseSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Platform Motor Fuse");

	std::shared_ptr<NodePin> shutLidCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Shut Lid Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> openLidCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Open Lid Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> lowerPlatformCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Lower Platform Commmand", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> raisePlatformCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Raise Platform Command", NodePinFlags_DisableDataField);

	virtual bool load(tinyxml2::XMLElement* xml) { return true; }
	virtual bool save(tinyxml2::XMLElement* xml) { return true; }
};