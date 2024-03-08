#pragma once

#include "Submodules/EL722xActuator.h"

class EL7222_0010 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(EL7222_0010, "EL7222-0010", "EL7222-0010", "Beckhoff", "Servo Drives", 0x2, 0x50227569)

	class EL7222Gpio : public GpioInterface{
	public:
		EL7222Gpio(std::shared_ptr<EtherCatDevice> parentDevice) : etherCatDevice(parentDevice){}
		std::shared_ptr<EtherCatDevice> etherCatDevice;
		virtual std::string getName() override { return std::string(etherCatDevice->getName()) + " GPIO"; }
		virtual std::string getStatusString() override { return "No Status String Implemented..."; }
	};
	
	std::shared_ptr<EL722x_Actuator> actuator1;
	std::shared_ptr<EL722x_Actuator> actuator2;
	std::shared_ptr<EL7222Gpio> gpio;
	std::shared_ptr<bool> digitalInput1A_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput2A_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput1B_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput2B_Value = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "GPIO");
	std::shared_ptr<NodePin> digitalInput1A_pin = std::make_shared<NodePin>(digitalInput1A_Value, NodePin::Direction::NODE_OUTPUT, "DI1A");
	std::shared_ptr<NodePin> digitalInput2A_pin = std::make_shared<NodePin>(digitalInput2A_Value, NodePin::Direction::NODE_OUTPUT, "DI2A");
	std::shared_ptr<NodePin> digitalInput1B_pin = std::make_shared<NodePin>(digitalInput1B_Value, NodePin::Direction::NODE_OUTPUT, "DI1B");
	std::shared_ptr<NodePin> digitalInput2B_pin = std::make_shared<NodePin>(digitalInput2B_Value, NodePin::Direction::NODE_OUTPUT, "DI2B");
	
	void downloadDiagnostics();
	
};
