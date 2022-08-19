#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Gui/Environnement/Dashboard/Widget.h"

class SafetySignal : public Node {
public:
	
	DEFINE_NODE(SafetySignal, "Safety Signal", "SafetySignal", Node::Type::PROCESSOR, "Safety")
	
	std::shared_ptr<bool> safetyValidSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> resetSafetyFaultSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Gpio Device", "GpioDevice");
	std::shared_ptr<NodePin> safetyValidPin = std::make_shared<NodePin>(safetyValidSignal, NodePin::Direction::NODE_INPUT, "Safety Valid", "SafetyValid");
	std::shared_ptr<NodePin> resetSafetyFaultPin = std::make_shared<NodePin>(resetSafetyFaultSignal, NodePin::Direction::NODE_OUTPUT, "Reset Safety Fault", "ResetSafetyFault");
	
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
		
		ControlWidget(std::shared_ptr<SafetySignal> safetySignal_, std::string name) : Widget(name, "Safety"), safetySignal(safetySignal_){}
		std::shared_ptr<SafetySignal> safetySignal;
		virtual void gui() override;
		virtual bool hasFixedContentSize() override { return true; }
		virtual glm::vec2 getFixedContentSize() override;
	};
	std::shared_ptr<ControlWidget> controlWidget;
};
