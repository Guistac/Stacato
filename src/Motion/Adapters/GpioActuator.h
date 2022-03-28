#pragma once

#include "Environnement/Node.h"
#include "Motion/MotionTypes.h"
#include "Motion/SubDevice.h"
#include "Motion/Profile.h"

class GpioActuator : public Node{
public:
	
	DEFINE_NODE(GpioActuator, "Gpio Actuator", "GpioActuator", Node::Type::PROCESSOR, "Adapters")
	
	//output data
	std::shared_ptr<bool> enableSignal = std::make_shared<bool>(false);
	std::shared_ptr<double> controlSignal = std::make_shared<double>(0.0);
	std::shared_ptr<ActuatorDevice> actuator = std::make_shared<ActuatorDevice>("Actuator", Units::AngularDistance::Revolution);
	
	std::shared_ptr<NodePin> enablePin = std::make_shared<NodePin>(enableSignal, NodePin::Direction::NODE_OUTPUT, "Enable");
	std::shared_ptr<NodePin> controlSignalPin = std::make_shared<NodePin>(controlSignal, NodePin::Direction::NODE_OUTPUT, "Control Signal");
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(actuator, NodePin::Direction::NODE_OUTPUT, "Actuator");
	
	//input data
	std::shared_ptr<bool> readySignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> emergencyStopSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "GPIO Device");
	std::shared_ptr<NodePin> readyPin = std::make_shared<NodePin>(readySignal, NodePin::Direction::NODE_INPUT, "Ready");
	std::shared_ptr<NodePin> emergencyStopPin = std::make_shared<NodePin>(emergencyStopSignal, NodePin::Direction::NODE_INPUT, "Emergency Stop");
	
	//connection checking
	bool isGpioDeviceConnected(){ return gpioDevicePin->isConnected(); }
	std::shared_ptr<GpioDevice> getGpioDevice(){ return gpioDevicePin->getConnectedPin()->getSharedPointer<GpioDevice>(); }
	
	bool isActuatorPinConnected(){ return actuatorPin->isConnected(); }
	
	bool areAllPinsConnected();
	
	//processing
	virtual void process();
	virtual void updatePin(std::shared_ptr<NodePin> pin);
	
	//control signal
	double getControlSignalLowLimit();
	double getControlSignalHighLimit();
	double getControlSignalZero();
	double controlSignalToActuatorVelocity(double signal);
	double actuatorVelocityToControlSignal(double velocity);
	double getControlSignalLimitedVelocity();
	
	//parameters
	double controlSignalRange = 10.0;
	bool b_controlSignalIsCenteredOnZero = true;
	double controlSignalUnitsPerActuatorVelocityUnit = 1.0;
	bool b_invertControlSignal = false;
	double positionFeedbackUnitsPerActuatorUnit = 0.0;
	double manualAcceleration;
	void sanitizeParameters();
	
	//control
	Motion::Profile motionProfile;
	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	void setVelocityTarget(double velocityTarget);
	void fastStop();
	double manualVelocityTarget = 0.0;
	void controlLoop();
	
	void enable();
	void disable();
	void onEnable();
	void onDisable();
	
	//gui stuff
	virtual void nodeSpecificGui();
	void controlGui();
	void settingsGui();
	float manualVelocityDisplay = 0.0;
	
	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);
};
