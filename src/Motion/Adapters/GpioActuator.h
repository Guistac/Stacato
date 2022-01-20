#pragma once

#include "NodeGraph/Node.h"
#include "Motion/MotionTypes.h"
#include "Motion/SubDevice.h"
#include "Motion/Profile/Profile.h"

class GpioActuator : public Node{
public:
	
	DEFINE_NODE(GpioActuator, "Gpio Actuator", "GpioActuator", Node::Type::PROCESSOR, "Adapters")
	
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
	std::shared_ptr<bool> emergencyStopSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "GPIO Device");
	std::shared_ptr<NodePin> readyPin = std::make_shared<NodePin>(readySignal, NodePin::Direction::NODE_INPUT, "Ready");
	std::shared_ptr<NodePin> brakePin = std::make_shared<NodePin>(brakeSignal, NodePin::Direction::NODE_INPUT, "Brake");
	std::shared_ptr<NodePin> emergencyStopPin = std::make_shared<NodePin>(emergencyStopSignal, NodePin::Direction::NODE_INPUT, "Emergency Stop");
	
	bool isGpioDeviceConnected(){ return gpioDevicePin->isConnected(); }
	std::shared_ptr<GpioDevice> getGpioDevice(){ return gpioDevicePin->getConnectedPin()->getSharedPointer<GpioDevice>(); }
	
	bool areAllPinsConnected();
	
	virtual void process();
	virtual void updatePin(std::shared_ptr<NodePin> pin);

	double getControlSignalLowLimit();
	double getControlSignalHighLimit();
	double getControlSignalZero();
	double controlSignalToActuatorVelocity(double signal);
	double actuatorVelocityToControlSignal(double velocity);
	double getControlSignalLimitVelocity();
	
	double controlSignalRange = 10.0;
	bool b_controlSignalIsCenteredOnZero = true;
	double controlSignalUnitsPerActuatorVelocityUnit = 1.0;
	bool b_invertControlSignal = false;
	double positionFeedbackUnitsPerActuatorUnit = 0.0;
	
	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	Motion::Profile motionProfile;
	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	
	virtual void nodeSpecificGui();
	void controlGui();
	void settingsGui();
	
	
	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);
};
