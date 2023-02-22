#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Motion/Interfaces.h"
#include "Project/Editor/Parameter.h"

class AxisNode : public Node {
public:
	
	DEFINE_NODE(AxisNode, "Axis", "Axis", Node::Type::AXIS, "")
	
	std::shared_ptr<AxisInterface> axis;
	
	std::shared_ptr<NodePin> actuatorPin;
	std::shared_ptr<NodePin> feedbackPin;
	std::shared_ptr<NodePin> gpioPin;
	std::shared_ptr<NodePin> lowerLimitSignalPin;
	std::shared_ptr<NodePin> upperLimitSignalPin;
	std::shared_ptr<NodePin> referenceSignalPin;
	std::shared_ptr<NodePin> surveillanceResetSignalPin;
	
	std::shared_ptr<NodePin> axisPin;
	std::shared_ptr<NodePin> surveillanceValidSignalPin;
	
	std::shared_ptr<bool> lowerLimitSignal;
	std::shared_ptr<bool> upperLimitSignal;
	std::shared_ptr<bool> referenceSignal;
	std::shared_ptr<bool> surveillanceResetSignal;
	std::shared_ptr<bool> surveillanceValidSignal;
	
	virtual bool prepareProcess() override;
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	virtual bool needsOutputProcess() override;
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;
	
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool save(tinyxml2::XMLElement* xml) override;
	
	virtual void nodeSpecificGui() override;
	
	void positionControlSettingsGui();
	void velocityControlSettingsGui();
	void forceControlSettingsGui();
	
private:
	
	std::vector<std::shared_ptr<ActuatorInterface>> connectedActuatorInterfaces;
	std::vector<std::shared_ptr<MotionFeedbackInterface>> connectedFeedbackModules;
	std::vector<std::shared_ptr<GpioInterface>> connectedGpioInterfaces;
	std::vector<std::shared_ptr<MotionFeedbackInterface>> allConnectedFeedbackModules;
	
	static OptionParameter::Option controlModePosition;
	static OptionParameter::Option controlModeVelocity;
	static OptionParameter::Option controlModeForce;
	static std::vector<OptionParameter::Option*> controlModeParameterOptions;
	OptionParam controlModeParameter;
	
	std::shared_ptr<MotionFeedbackInterface> positionFeedbackModule;
	std::shared_ptr<MotionFeedbackInterface> velocityFeedbackModule;
	std::shared_ptr<MotionFeedbackInterface> forceFeedbackModule;
	double positionFeedbackUnitsPerAxisUnits;
	double velocityFeedbackUnitsPerAxisUnits;
	double forceFeedbackUnitsPerAxisUnits;
	void updateConnectedModules();
	
	class ActuatorControlUnit{
	public:
		ActuatorControlUnit(std::shared_ptr<ActuatorInterface> actuator_) : actuator(actuator_) {}
		ActuatorInterface::ControlMode controlModeSelection = ActuatorInterface::ControlMode::VELOCITY;
		double actuatorUnitsPerAxisUnits = 1.0;
		double actuatorPositionOffset = 0.0;
		std::shared_ptr<ActuatorInterface> actuator;
	};
	
	std::vector<std::shared_ptr<ActuatorControlUnit>> actuatorControlUnits;
	
	BoolParam enableSurveillanceParameter = BooleanParameter::make(false, "Enable Surveillance", "EnableSurveillance");
	std::shared_ptr<MotionFeedbackInterface> surveillanceFeedbackModule;
	double surveillanceFeedbackUnitsPerAxisUnits;
	
	//each connected actuator module has:
	//	-control mode selection option
	//	-if the control mode is position or velocity, we need a ratio to convert between actuator and axis units
	//	-if the control mode is force, we need to have parametrize the force command given to each actuator
	
};


