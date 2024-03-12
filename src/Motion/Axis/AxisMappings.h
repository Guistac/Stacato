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
	virtual std::shared_ptr<NodePin> getPin() { return feedbackPin; }
	
	NumberParam<double> deviceUnitsPerAxisUnits = NumberParameter<double>::make(1.0, "Feedback units per axis unit", "UnitConversion");
	
	bool save(tinyxml2::XMLElement* parent);
	bool load(tinyxml2::XMLElement* parent);
	
	virtual std::string getName(){
		if(!isFeedbackConnected()) return feedbackPin->displayString;
		else return getFeedbackInterface()->getName();
	}
	
	virtual bool isReady(){ return getFeedbackInterface()->isReady(); }
	virtual bool isEnabled(){ return getFeedbackInterface()->isReady(); }
	
};



class ActuatorMapping : public FeedbackMapping{
public:
	
	ActuatorMapping(std::shared_ptr<AxisNode> axis);
	
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Actuator");
	
	bool isActuatorConnected(){ return actuatorPin->isConnected(); }
	std::shared_ptr<ActuatorInterface> getActuatorInterface(){ return actuatorPin->getConnectedPin()->getSharedPointer<ActuatorInterface>(); }
	
	virtual bool isFeedbackConnected() override { return actuatorPin->isConnected(); }
	virtual std::shared_ptr<MotionFeedbackInterface> getFeedbackInterface() override { return actuatorPin->getConnectedPin()->getSharedPointer<MotionFeedbackInterface>(); }
	virtual std::shared_ptr<NodePin> getPin() override { return actuatorPin; }
	
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
	
	void enable(){ if(isActuatorConnected()) getActuatorInterface()->enable(); }
	void disable(){ if(isActuatorConnected()) getActuatorInterface()->disable(); }
	virtual bool isReady() override {
		if(isActuatorConnected()) return getActuatorInterface()->isReady();
		return false;
	}
	virtual bool isEnabled() override {
		if(isActuatorConnected()) return getActuatorInterface()->isEnabled();
		return false;
	}
	
	virtual std::string getName() override {
		if(!isActuatorConnected()) return actuatorPin->displayString;
		else return getActuatorInterface()->getName();
	}
	
};


class SafetyRule{
public:
	SafetyRule(std::shared_ptr<AxisNode> axis) : axisNode(axis){}
	std::shared_ptr<AxisNode> axisNode;
	virtual bool isRespected() = 0;
	virtual void gui() = 0;
	virtual bool save(tinyxml2::XMLElement* parent) = 0;
	virtual bool load(tinyxml2::XMLElement* parent) = 0;
	virtual std::string getSaveString() = 0;
	virtual std::string getDisplayString() = 0;
	
	bool b_enabled = false;
	
	static std::shared_ptr<SafetyRule> loadFromSaveString(std::string saveString, std::shared_ptr<AxisNode> axis);
	virtual std::shared_ptr<SafetyRule> createInstance(std::shared_ptr<AxisNode> axis) = 0;
	virtual void onAxisInterfaceChange() = 0;
};


class FeedbackToFeedbackVelocityComparison : public SafetyRule{
public:
	FeedbackToFeedbackVelocityComparison(std::shared_ptr<AxisNode> axis) : SafetyRule(axis){}
	virtual std::string getSaveString() override { return "FeedbackToFeedbackVelocityComparison"; }
	virtual std::string getDisplayString() override { return "Feedback to Feedback Velocity Comparison"; }
	virtual std::shared_ptr<SafetyRule> createInstance(std::shared_ptr<AxisNode> axis) override { return std::make_shared<FeedbackToFeedbackVelocityComparison>(axis); }
	
	std::shared_ptr<FeedbackMapping> mapping1 = nullptr;
	std::shared_ptr<FeedbackMapping> mapping2 = nullptr;
	double maxVelocityDeviation = 0.0;
	
	virtual bool isRespected() override;
	
	virtual bool save(tinyxml2::XMLElement* parent) override;
	virtual bool load(tinyxml2::XMLElement* parent) override;
	
	virtual void gui() override;
	virtual void onAxisInterfaceChange() override;
};


class FeedbackToFeedbackPositionComparison : public SafetyRule{
public:
	FeedbackToFeedbackPositionComparison(std::shared_ptr<AxisNode> axis) : SafetyRule(axis){}
	virtual std::string getSaveString() override { return "FeedbackToFeedbackPositionComparison"; }
	virtual std::string getDisplayString() override { return "Position to Position Velocity Comparison"; }
	virtual std::shared_ptr<SafetyRule> createInstance(std::shared_ptr<AxisNode> axis) override { return std::make_shared<FeedbackToFeedbackPositionComparison>(axis); }
	
	std::shared_ptr<FeedbackMapping> mapping1 = nullptr;
	std::shared_ptr<FeedbackMapping> mapping2 = nullptr;
	double maxPositionDeviation = 0.0;
	
	virtual bool isRespected() override;
	
	virtual bool save(tinyxml2::XMLElement* parent) override;
	virtual bool load(tinyxml2::XMLElement* parent) override;
	
	virtual void gui() override;
	virtual void onAxisInterfaceChange() override;
};
