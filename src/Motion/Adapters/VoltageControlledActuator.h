#pragma once

#include "NodeGraph/Node.h"
#include "Motion/MotionTypes.h"
#include "Motion/SubDevice.h"

class VoltageControlledActuator : public Node{
public:
	
	DEFINE_NODE(VoltageControlledActuator, "Voltage Controlled Actuator", "VoltageControlledActuator", Node::Type::PROCESSOR, "Adapters")
	
	//output data
	std::shared_ptr<bool> enableSignal = std::make_shared<bool>(false);
	std::shared_ptr<double> controlSignal = std::make_shared<double>(0.0);
	std::shared_ptr<ActuatorDevice> actuator = std::make_shared<ActuatorDevice>("Actuator", PositionUnit::REVOLUTION);
	
	std::shared_ptr<NodePin> enablePin = std::make_shared<NodePin>(enableSignal, NodePin::Direction::NODE_OUTPUT, "Enable");
	std::shared_ptr<NodePin> controlSignalPin = std::make_shared<NodePin>(controlSignal, NodePin::Direction::NODE_OUTPUT, "Control Signal");
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(actuator, NodePin::Direction::NODE_OUTPUT, "Actuator");
	
	//input data
	std::shared_ptr<bool> readySignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "GPIO Device");
	std::shared_ptr<NodePin> readyPin = std::make_shared<NodePin>(readySignal, NodePin::Direction::NODE_INPUT, "Ready");
	std::shared_ptr<NodePin> brakePin = std::make_shared<NodePin>(brakeSignal, NodePin::Direction::NODE_INPUT, "Brake");
	
	virtual void process();
	
};
