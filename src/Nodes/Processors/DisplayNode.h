#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Gui/Environnement/Dashboard/Widget.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

class DisplayNode : public Node {
public:

	DEFINE_NODE(DisplayNode, "Display", "Display", Node::Type::PROCESSOR, "Utility")

	std::shared_ptr<NodePin> displayInput = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "value: ", NodePin::Flags::DisableDataField | NodePin::Flags::ForceDataField);
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess() override;
	
	virtual void onAddToNodeGraph() override { widget->addToDictionnary(); }
	virtual void onRemoveFromNodeGraph() override { widget->removeFromDictionnary(); }
	
	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml) override {
		xml->QueryIntAttribute("WidgetUniqueID", &widget->uniqueID);
		return true;
	}
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute("WidgetUniqueID", widget->uniqueID);
		return true;
	}
	
	class DisplayWidget : public Widget{
	public:
		DisplayWidget(std::shared_ptr<DisplayNode> node_) : Widget("Nodes"), node(node_){}
		std::shared_ptr<DisplayNode> node;
		virtual void gui() override {
			ImGui::PushFont(Fonts::sansBold20);
			ImVec2 fieldSize = ImVec2(ImGui::GetTextLineHeight()*4.0, ImGui::GetTextLineHeight()*1.2);
			backgroundText(getName().c_str(), fieldSize, Colors::veryDarkGray, Colors::gray);
			char val[32];
			snprintf(val, 32, "%.3f", *node->inputPinValue);
			backgroundText(val, fieldSize, Colors::black);
			ImGui::PopFont();
		}
		virtual std::string getName() override { return node->getName(); }
	};
	std::shared_ptr<DisplayWidget> widget;
	
};

void DisplayNode::initialize(){
	displayInput->assignData(inputPinValue);
	addNodePin(displayInput);
	auto thisDisplayNode = std::static_pointer_cast<DisplayNode>(shared_from_this());
	widget = std::make_shared<DisplayWidget>(thisDisplayNode);
}

void DisplayNode::inputProcess() {
	if (displayInput->isConnected()) displayInput->copyConnectedPinValue();
}

