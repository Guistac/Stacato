#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Gui/Environnement/Dashboard/Widget.h"

class Brake : public Node{
public:
	
	DEFINE_NODE(Brake, "Brake", "Brake", Node::Type::PROCESSOR, "Safety")
	
	
	std::shared_ptr<bool> brakeOpenStatus_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeClosedStatus_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeOpenControl_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeClosedControl_Signal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpio_Pin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT,
																 "Gpio Device", "GpioDevice",
																 NodePin::Flags::AcceptMultipleInputs);
	
	std::shared_ptr<NodePin> brakeOpenStatus_Pin = std::make_shared<NodePin>(brakeOpenStatus_Signal, NodePin::Direction::NODE_INPUT, "Brake Open Status", "BrakeOpenStatus", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> brakeClosedStatus_Pin = std::make_shared<NodePin>(brakeClosedStatus_Signal, NodePin::Direction::NODE_INPUT, "Brake Closed Status", "BrakeClosedStatus",
																			   NodePin::Flags::DisableDataField);
	
	std::shared_ptr<NodePin> brakeOpenControl_Pin = std::make_shared<NodePin>(brakeOpenControl_Signal, NodePin::Direction::NODE_OUTPUT,
																			  "Brake Open Control", "BrakeOpenControl");
	std::shared_ptr<NodePin> brakeClosedControl_Pin = std::make_shared<NodePin>(brakeClosedControl_Signal, NodePin::Direction::NODE_OUTPUT,
																				"Brake Closed Control", "BrakeClosedControl");
	
	enum class State{
		CLOSED,
		NOT_CLOSED,
		OPEN,
		OFFLINE,
		ERROR
	};
	enum class Target{
		CLOSED,
		STOPPED,
		OPEN
	};
	
	State state = State::OFFLINE;
	Target target = Target::STOPPED;
	
	virtual void inputProcess() override;
	
	virtual void nodeSpecificGui() override;
	
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); };
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); };
		
	void widgetGui(bool b_withTitle);
	
private:
	
	class ControlWidget : public Widget{
	public:
		
		ControlWidget(std::shared_ptr<Brake> brake_) : Widget("Brake"), brake(brake_){}
		std::shared_ptr<Brake> brake;
		virtual void gui() override;
		virtual std::string getName() override {
			return brake->getName();
		}
	};
	std::shared_ptr<ControlWidget> controlWidget;
	
};
