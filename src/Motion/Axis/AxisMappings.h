#pragma once

#include "Project/Editor/Parameter.h"
#include "Motion/Interfaces.h"
#include "Environnement/NodeGraph/NodePin.h"

class AxisNode;

class FeedbackMapping{
public:
	
	FeedbackMapping(std::shared_ptr<AxisNode> axis);
	std::shared_ptr<AxisNode> axisNode;
	
	std::shared_ptr<NodePin> feedbackPin = std::make_shared<NodePin>(NodePin::DataType::MOTIONFEEDBACK_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Feedback");
	
	virtual bool isFeedbackConnected(){ return feedbackPin->isConnected(); }
	virtual std::shared_ptr<MotionFeedbackInterface> getFeedbackInterface(){ return feedbackPin->getConnectedPin()->getSharedPointer<MotionFeedbackInterface>(); }
	
	NumberParam<double> deviceUnitsPerAxisUnits = NumberParameter<double>::make(1.0, "Feedback units per axis unit", "UnitConversion");
	
	bool save(tinyxml2::XMLElement* parent);
	bool load(tinyxml2::XMLElement* parent);
	
	virtual std::string getName(){
		if(!isFeedbackConnected()) return feedbackPin->displayString;
		else return getFeedbackInterface()->getName();
	}
	
	virtual bool isReady(){ return getFeedbackInterface()->isReady(); }
	virtual bool isEnabled(){ return getFeedbackInterface()->isEnabled(); }
	
};



class ActuatorMapping : public FeedbackMapping{
public:
	
	ActuatorMapping(std::shared_ptr<AxisNode> axis);
	
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Actuator");
	
	bool isActuatorConnected(){ return actuatorPin->isConnected(); }
	std::shared_ptr<ActuatorInterface> getActuatorInterface(){ return actuatorPin->getConnectedPin()->getSharedPointer<ActuatorInterface>(); }
	
	virtual bool isFeedbackConnected() override { return actuatorPin->isConnected(); }
	virtual std::shared_ptr<MotionFeedbackInterface> getFeedbackInterface() override { return feedbackPin->getConnectedPin()->getSharedPointer<MotionFeedbackInterface>(); }
	
	enum ActuatorControlMode{
		NO_CONTROL = 0,
		POSITION_CONTROL = 1,
		VELOCITY_CONTROL = 2,
		FORCE_CONTROL = 3
	}controlMode = ActuatorControlMode::NO_CONTROL;
	OptionParameter::Option actuatorMode_None = 	OptionParameter::Option(0, "Disabled", "Disabled");
	OptionParameter::Option actuatorMode_Position = OptionParameter::Option(1, "Position Control", "PositionControl");
	OptionParameter::Option actuatorMode_Velocity = OptionParameter::Option(2, "Velocity Control", "VelocityControl");
	OptionParameter::Option actuatorMode_Force = 	OptionParameter::Option(3, "Force Control", "ForceControl");
	
	OptionParam controlModeParameter = OptionParameter::make2(actuatorMode_Position, {
											&actuatorMode_None,
											&actuatorMode_Position,
											&actuatorMode_Velocity,
											&actuatorMode_Force
										}, "Control Mode", "ControlMode");
	double actuatorPositionOffset = 0.0;
	
	bool save(tinyxml2::XMLElement* parent);
	bool load(tinyxml2::XMLElement* parent);
	
	void enable(){ getActuatorInterface()->enable(); }
	void disable(){ getActuatorInterface()->disable(); }
	virtual bool isReady() override { return getActuatorInterface()->isReady(); }
	virtual bool isEnabled() override { return getActuatorInterface()->isEnabled(); }
	
	virtual std::string getName() override {
		if(!isActuatorConnected()) return actuatorPin->displayString;
		else return getActuatorInterface()->getName();
	}
	
};

