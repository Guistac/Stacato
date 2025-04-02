#pragma once

#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"
#include "Project/Editor/Parameter.h"

class MecanumMachine : public Machine{
	
	DEFINE_MACHINE_NODE(MecanumMachine, "Mecanum Machine", "MecanumMachine", "Special")

	class AxisMapping{
	public:
		AxisMapping(std::string name, std::string saveName);
		std::shared_ptr<NodePin> axisPin;
		std::shared_ptr<AxisInterface> axis;
		NumberParam<double> wheelDiameter = NumberParameter<double>::make(100.0, "Wheel Diameter", "WheelDiameter");
		std::shared_ptr<VectorParameter<glm::vec2>> wheelPosition = std::make_shared<VectorParameter<glm::vec2>>(glm::vec2(0.0, 0.0), "Wheel Position", "WheelPosition");
		BoolParam invertAxis = BooleanParameter::make(false, "Invert Axis", "InvertAxis");
		OptionParameter::Option wheelType_A = OptionParameter::Option(0, "Type \\\\\\", "TypeA");
		OptionParameter::Option wheelType_B = OptionParameter::Option(1, "Type ///", "TypeB");
		OptionParam wheelType = OptionParameter::make2(wheelType_A, {&wheelType_A, &wheelType_B}, "Wheel Type", "WheelType");
		glm::vec2 frictionVector = glm::vec2(0.0, 0.0);
		bool save(tinyxml2::XMLElement* xml);
		bool load(tinyxml2::XMLElement* xml);
	};
	std::vector<std::shared_ptr<AxisMapping>> axisMappings;

	

	std::shared_ptr<double> joystickXvalue = std::make_shared<double>(0.0);
	std::shared_ptr<double> joystickYvalue = std::make_shared<double>(0.0);
	std::shared_ptr<double> joystickRvalue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> joystickXpin = std::make_shared<NodePin>(joystickXvalue, NodePin::Direction::NODE_INPUT, "Joystick X", "JoystickX");
	std::shared_ptr<NodePin> joystickYpin = std::make_shared<NodePin>(joystickYvalue, NodePin::Direction::NODE_INPUT, "Joystick Y", "JoystickY");
	std::shared_ptr<NodePin> joystickRpin = std::make_shared<NodePin>(joystickRvalue, NodePin::Direction::NODE_INPUT, "Joystick R", "JoystickR");
	
	std::shared_ptr<bool> enableControl = std::make_shared<bool>(false);
	std::shared_ptr<bool> speedModeControl = std::make_shared<bool>(false);
	std::shared_ptr<bool> moveModeControl = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeFeedback = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> enablePin = std::make_shared<NodePin>(enableControl, NodePin::Direction::NODE_INPUT, "Enable Control", "EnableControl");
	std::shared_ptr<NodePin> speedModePin = std::make_shared<NodePin>(speedModeControl, NodePin::Direction::NODE_INPUT, "Speed Mode Control", "SpeedModeControl");
	std::shared_ptr<NodePin> moveModePin = std::make_shared<NodePin>(moveModeControl, NodePin::Direction::NODE_INPUT, "Move Mode Control", "MoveModeControl");
	std::shared_ptr<NodePin> brakeFeedbackPin = std::make_shared<NodePin>(brakeFeedback, NodePin::Direction::NODE_INPUT, "Brake Feedback", "BrakeFeedback");
	
	std::shared_ptr<bool> enabledFeedback = std::make_shared<bool>(false);
	std::shared_ptr<bool> errorLed = std::make_shared<bool>(false);
	std::shared_ptr<bool> enabledLed = std::make_shared<bool>(false);
	std::shared_ptr<bool> velocityModeFeedback = std::make_shared<bool>(false);
	std::shared_ptr<bool> moveModeFeedback = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeControl = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> enabledFeedbackPin = std::make_shared<NodePin>(enabledFeedback, NodePin::Direction::NODE_OUTPUT, "Enabled Feedback", "EnabledFeedback");
	std::shared_ptr<NodePin> errorLedPin = std::make_shared<NodePin>(errorLed, NodePin::Direction::NODE_OUTPUT, "Error Led", "ErrorLed");
	std::shared_ptr<NodePin> enabledLedPin = std::make_shared<NodePin>(enabledLed, NodePin::Direction::NODE_OUTPUT, "Enabled Led", "EnabledLed");
	std::shared_ptr<NodePin> velocityModeFeedbackPin = std::make_shared<NodePin>(velocityModeFeedback, NodePin::Direction::NODE_OUTPUT, "Velocity Mode Feedback", "VelocityModeFeedback");
	std::shared_ptr<NodePin> moveModeFeedbackPin = std::make_shared<NodePin>(moveModeFeedback, NodePin::Direction::NODE_OUTPUT, "Move Mode Feedback", "MoveModeFeedback");
	std::shared_ptr<NodePin> brakeControlPin = std::make_shared<NodePin>(brakeControl, NodePin::Direction::NODE_OUTPUT, "Brake Control", "BrakeControl");
	
	NumberParam<double> enableTimeout = NumberParameter<double>::make(0.5, "Enable Timeout", "EnableTimeout", "%.1fs");
	NumberParam<double> brakeApplyTime = NumberParameter<double>::make(1.0, "Brake Apply Delay", "BrakeApplyDelay", "%.1fs");
	NumberParam<double> brakeReleaseTimeout = NumberParameter<double>::make(0.2, "Brake Release Timeout", "BrakeReleaseTimeout", "%.1fs");
	
	NumberParam<double> linearAcceleration = NumberParameter<double>::make(100.0, "Linear Acceleration", "LinearAcceleration", "%.1fmm/s\xc2\xb2");
	NumberParam<double> angularAcceleration = NumberParameter<double>::make(100.0, "Angular Acceleration", "AngularAcceleration", "%.1f°/s\xc2\xb2");
	NumberParam<double> linearVelocityLimit_H = NumberParameter<double>::make(100.0, "Linear Velocity Limit [High Speed]", "LinearVelocityLimit_HighSpeed", "%.1fmm/s");
	NumberParam<double> angularVelocityLimit_H = NumberParameter<double>::make(100.0, "Angular Velocity Limit [High Speed]", "AngularVelocityLimit_HighSpeed", "%.1f°/s");
	NumberParam<double> linearVelocityLimit_L = NumberParameter<double>::make(100.0, "Linear Velocity Limit [Low Speed]", "LinearVelocityLimit_LowSpeed", "%.1fmm/s");
	NumberParam<double> angularVelocityLimit_L = NumberParameter<double>::make(100.0, "Angular Velocity Limit [Low Speed]", "AngularVelocityLimit_LowSpeed", "%.1f°/s");
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;


	

	
		
	virtual void onAddToNodeGraph() override {}
	virtual void onRemoveFromNodeGraph() override {}
	virtual bool hasSetupGui() override { return true; }
	virtual void setupGui() override;
	
	/*
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<PositionControlledMachine> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<PositionControlledMachine> machine;
		virtual void gui() override;
		virtual std::string getName() override {
			return machine->getName();
		}
	};
	*/
	
	
	void enableAllAxis();
	void disableAllAxis();
	bool areAllAxisEnabled();
	bool areAllAxisReady();
	bool areAllAxisOnline();
	bool areAllAxisErrorFree();
	bool areAllAxisEstopFree();
	void updateFrictionVectors();
	
	bool b_enableRequest = false;
	bool b_disableRequest = false;
	uint64_t enableRequestTime;
	uint64_t enableTime;
	uint64_t disableTime;
	
	bool b_localControl = false;
	bool b_localControlSpeedMode = false;
	float localControl_X = 0.0;
	float localControl_Y = 0.0;
	float localControl_R = 0.0;
	
	Motion::Profile xProfile;
	Motion::Profile yProfile;
	Motion::Profile rProfile;
	
	bool b_brakesOpened = false;
	bool b_highSpeedMode = false;
	bool b_absoluteMoveMode = false;
	double translationVelocityLimit = 0.0;
	double rotationVelocityLimit = 0.0;
};
