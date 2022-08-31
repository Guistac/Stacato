#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Gui/Environnement/Dashboard/Widget.h"

class SafetySignal : public Node {
public:
	
	DEFINE_NODE(SafetySignal, "Safety Signal", "SafetySignal", Node::Type::PROCESSOR, "Safety")
	
	std::shared_ptr<bool> safetyStateValidSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> safetyLineValidSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<bool> resetSafetySignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> stateLedSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Gpio Device", "GpioDevice", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> safetyLineValidPin = std::make_shared<NodePin>(safetyLineValidSignal, NodePin::Direction::NODE_INPUT, "Safety Line Valid", "SafetyLineValid");
	std::shared_ptr<NodePin> safetyStateValidPin = std::make_shared<NodePin>(safetyStateValidSignal, NodePin::Direction::NODE_INPUT, "Safety State Valid", "SafetyStateValid");
	
	std::shared_ptr<NodePin> resetSafetyFaultPin = std::make_shared<NodePin>(resetSafetySignal, NodePin::Direction::NODE_OUTPUT, "Reset Safety Fault", "ResetSafetyFault");
	std::shared_ptr<NodePin> stateLedPin = std::make_shared<NodePin>(stateLedSignal, NodePin::Direction::NODE_OUTPUT, "State LED Signal", "StateLEDSignal");
	
	enum class State{
		OFFLINE,
		EMERGENCY_STOP,
		UNCLEARED_SAFETY_FAULT,
		CLEAR
	};
	State safetyState = State::OFFLINE;
	
	virtual void inputProcess() override;
	
	virtual void nodeSpecificGui() override;
	
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); };
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); };
		
	void widgetGui();
	
public:
	
	void resetSafety(){ b_shouldResetSafety = true; }
	
private:
	
	bool b_shouldResetSafety = false;
	bool b_isResettingFault = false;
	long long resetPulseStartTime = 0;
	
	std::shared_ptr<NumberParameter<double>> faultResetPulseTime = NumberParameter<double>::make(0.0,
																								 "Fault Reset Pulse Time",
																								 "FaultResetPulseTime",
																								 "%.2f",
																								 Units::Time::Second,
																								 false);
	
	std::shared_ptr<NumberParameter<double>> unclearedFaultLedBlinkFrequency = NumberParameter<double>::make(0.0,
                                                                                                        "Uncleared Fault LED Blink Frequency",
                                                                                                        "UnclearedFaultLedBlinkFrequency",
                                                                                                        "%.2f",
                                                                                                        Units::Frequency::Hertz,
                                                                                                        false);
	
	class ControlWidget : public Widget{
	public:
		
		ControlWidget(std::shared_ptr<SafetySignal> safetySignal_) : Widget("Safety"), safetySignal(safetySignal_){}
		std::shared_ptr<SafetySignal> safetySignal;
		virtual void gui() override;
		virtual std::string getName() override {
			return safetySignal->getName();
		}
	};
	std::shared_ptr<ControlWidget> controlWidget;
};
