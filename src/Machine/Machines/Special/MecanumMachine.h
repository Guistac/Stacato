#pragma once

#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"
#include "Project/Editor/Parameter.h"


#include <span>

template <typename T, std::size_t Size>
class RingBuffer {
public:
	RingBuffer() : head(0), full(false) {}

	void push(T value) {
		buffer[head] = value;
		head = (head + 1) % Size;
		if (!full && head == 0) {
			full = true;
		}
	}

	void data(std::vector<T>& out) const {
		  out.clear();
		  if (!full && head == 0) return; // No data yet

		  out.reserve(Size); // Preallocate to avoid multiple reallocations

		  if (full) {
			  // Copy in two parts: from head to end, then from start to head
			  out.insert(out.end(), buffer.begin() + head, buffer.end());
			  out.insert(out.end(), buffer.begin(), buffer.begin() + head);
		  } else {
			  // Copy only valid elements
			  out.insert(out.end(), buffer.begin(), buffer.begin() + head);
		  }
	  }
	
	void clear() {
			head = 0;
			full = false;
		}

private:
	std::vector<T> buffer = std::vector<T>(Size);
	std::size_t head;
	bool full;
};


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
		
		double wheelCircumference = 0.0;
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
	std::shared_ptr<bool> brakeOverrideFeedback = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> enablePin = std::make_shared<NodePin>(enableControl, NodePin::Direction::NODE_INPUT, "Enable Control", "EnableControl");
	std::shared_ptr<NodePin> speedModePin = std::make_shared<NodePin>(speedModeControl, NodePin::Direction::NODE_INPUT, "Speed Mode Control", "SpeedModeControl");
	std::shared_ptr<NodePin> moveModePin = std::make_shared<NodePin>(moveModeControl, NodePin::Direction::NODE_INPUT, "Move Mode Control", "MoveModeControl");
	std::shared_ptr<NodePin> brakeFeedbackPin = std::make_shared<NodePin>(brakeFeedback, NodePin::Direction::NODE_INPUT, "Brake Feedback", "BrakeFeedback");
	std::shared_ptr<NodePin> brakeOverrideFeedbackPin = std::make_shared<NodePin>(brakeOverrideFeedback, NodePin::Direction::NODE_INPUT, "Brake Override Feedback", "BrakeOverrideFeedback");
	
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
	
	NumberParam<double> linearAccelerationLimit = NumberParameter<double>::make(100.0, "Linear Acceleration", "LinearAcceleration", "%.1fmm/s\xc2\xb2");
	NumberParam<double> angularAccelerationLimit = NumberParameter<double>::make(100.0, "Angular Acceleration", "AngularAcceleration", "%.1f°/s\xc2\xb2");
	NumberParam<double> linearVelocityLimit = NumberParameter<double>::make(100.0, "Linear Velocity Limit [High Speed]", "LinearVelocityLimit_HighSpeed", "%.1fmm/s");
	NumberParam<double> angularVelocityLimit = NumberParameter<double>::make(100.0, "Angular Velocity Limit [High Speed]", "AngularVelocityLimit_HighSpeed", "%.1f°/s");
	
	NumberParam<double> headingCorrectFactor = NumberParameter<double>::make(100.0, "Heading Correction Factor", "HeadingCorrectionFactor", "%.2f%%");
	
	float lowSpeed_userAdjust = 50.0;
	float globalAcceleration_userAdjust = 100.0;
	
	std::vector<std::shared_ptr<Parameter>> userParameters = {
		linearAccelerationLimit,
		angularAccelerationLimit,
		linearVelocityLimit,
		linearVelocityLimit
	};
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;
	
	virtual void onAddToNodeGraph() override { widget->addToDictionnary(); }
	virtual void onRemoveFromNodeGraph() override { widget->removeFromDictionnary(); }
	virtual bool hasSetupGui() override { return false; }
	virtual void setupGui() override;
	
	class MecanumWidget : public Widget{
	public:
		MecanumWidget(std::shared_ptr<MecanumMachine> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<MecanumMachine> machine;
		virtual void gui() override;
		virtual std::string getName() override {
			return machine->getName();
		}
	};
	std::shared_ptr<MecanumWidget> widget;
	
	void drawWheelWidget();
	
	
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
	bool b_localControlMoveMode = false;
	float localControl_X = 0.0;
	float localControl_Y = 0.0;
	float localControl_R = 0.0;
	
	Motion::Profile xProfile;
	Motion::Profile yProfile;
	Motion::Profile rProfile;
	
	bool b_brakesOpened = false;
	bool b_brakeOverride = false;
	bool b_highSpeedMode = false;
	bool b_absoluteMoveMode = false;
	double translationVelocityLimitCurrent = 0.0;
	double rotationVelocityLimitCurrent = 0.0;
	double translationAccelerationCurrent = 0.0;
	double rotationAccelerationCurrent = 0.0;
	double estimatedHeading_degrees = 0.0;
	glm::vec2 estimatedPosition_absolute = glm::vec2(0.0, 0.0);
	
	//visualisation values
	glm::vec2 translationVelocity_relative = glm::vec2(0.0, 0.0);
	glm::vec2 translationVelocity_absolute = glm::vec2(0.0, 0.0);
	double angularVelocity = 0.0;
	RingBuffer<ImVec2, 200> positionHistory;
	uint64_t lastPointTime = 0;
	std::mutex positionHistoryMutex;
};
