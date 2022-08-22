#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Gui/Environnement/Dashboard/Widget.h"

class SafetySignal : public Node {
public:
	
	DEFINE_NODE(SafetySignal, "Safety Signal", "SafetySignal", Node::Type::PROCESSOR, "Safety")
	
	std::shared_ptr<bool> safetyValidSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> safetyLineStateSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<bool> resetSafetyFaultSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> stateLedSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Gpio Device", "GpioDevice", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> safetyLineStatePin = std::make_shared<NodePin>(safetyLineStateSignal, NodePin::Direction::NODE_INPUT, "Safety Line State", "SafetyLineState");
	std::shared_ptr<NodePin> safetyValidPin = std::make_shared<NodePin>(safetyValidSignal, NodePin::Direction::NODE_INPUT, "Safety Valid", "SafetyValid");
	
	std::shared_ptr<NodePin> resetSafetyFaultPin = std::make_shared<NodePin>(resetSafetyFaultSignal, NodePin::Direction::NODE_OUTPUT, "Reset Safety Fault", "ResetSafetyFault");
	std::shared_ptr<NodePin> stateLedPin = std::make_shared<NodePin>(stateLedSignal, NodePin::Direction::NODE_OUTPUT, "State LED Signal", "StateLEDSignal");
	
	enum class State{
		OFFLINE,
		EMERGENCY_STOP,
		UNCLEARED_SAFETY_FAULT,
		CLEAR
	};
	State state = State::OFFLINE;
	
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	
	virtual void nodeSpecificGui() override;
	
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); };
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); };
		
	void widgetGui();
	
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
