#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Utilities/ScrollingBuffer.h"

#include <imgui.h>
#include <implot.h>
#include <tinyxml2.h>

class PlotterNode : public Node {
public:

	DEFINE_NODE(PlotterNode, "Plotter", "Plotter", Node::Type::PROCESSOR, "Utility")

	int bufferSize = 512;
	float displayLengthSeconds = 5.0;
	ScrollingBuffer data;
	bool wasConnected = false;

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "input", NodePin::Flags::ForceDataField | NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess() override;
	
	virtual void nodeSpecificGui() override {
		if (ImGui::BeginTabItem("Plot")) {
			if (ImGui::InputInt("Buffer Size", &bufferSize, 0, 0)) data.setMaxSize(bufferSize);
			ImGui::InputFloat("Display Length Second", &displayLengthSeconds, 0.1, 1.0, "%.1fs");
			ImPlot::SetNextAxisToFit(ImAxis_Y1);
			ImPlot::SetNextAxisLimits(ImAxis_X1, (double)data.newest().x - (double)displayLengthSeconds, data.newest().x, ImGuiCond_Always);
			ImPlotFlags plotFlags = ImPlotFlags_CanvasOnly | ImPlotFlags_NoTitle;
			if (ImPlot::BeginPlot("Plot", nullptr, nullptr, ImVec2(-1, ImGui::GetTextLineHeight() * 15.0), plotFlags)) {
				ImPlot::PlotLine("Data", &data.front().x, &data.front().y, data.size(), data.offset(), data.stride());
				ImPlot::EndPlot();
			}
			ImGui::EndTabItem();
		}
	}

	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		Logger::trace("Loading Plotter Node Specific Attributes for node {}", getName());
		XMLElement* plotterXML = xml->FirstChildElement("Plotter");
		if (!plotterXML) return Logger::warn("Coulnd't load Plotter attribute of node {}", getName());
		if (plotterXML->QueryIntAttribute("BufferSize", &bufferSize) != XML_SUCCESS) return Logger::warn("Couldn't load BufferSize Attribute");
		if (plotterXML->QueryFloatAttribute("DisplayLengthSeconds", &displayLengthSeconds) != XML_SUCCESS) return Logger::warn("Couldn't load DisplayLengthSeconds Attribute");
		Logger::debug("Successfully Loaded Plotter Node Attributes");
		return true;
	}

	virtual bool save(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		XMLElement* plotterXML = xml->InsertNewChildElement("Plotter");
		plotterXML->SetAttribute("BufferSize", bufferSize);
		plotterXML->SetAttribute("DisplayLengthSeconds", displayLengthSeconds);
		return true;
	}
};

void PlotterNode::initialize(){
	input->assignData(inputPinValue);
	addNodePin(input);
	data.setMaxSize(bufferSize);
}

void PlotterNode::inputProcess() {
	if (input->isConnected()) {
		input->copyConnectedPinValue();
		if (!wasConnected) {
			wasConnected = true;
			data.clear();
		}
		glm::vec2 newPoint;
		newPoint.x = Timing::getProgramTime_seconds();
		newPoint.y = *inputPinValue;
		data.addPoint(newPoint);
	}
	else {
		if (wasConnected) {
			data.clear();
			wasConnected = false;
		}
	}
}
