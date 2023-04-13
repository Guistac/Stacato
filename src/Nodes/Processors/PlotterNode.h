#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Utilities/ScrollingBuffer.h"

#include <imgui.h>
#include <implot.h>
#include <tinyxml2.h>

class PlotterNode : public Node {
public:

	DEFINE_NODE(PlotterNode, Node::Type::PROCESSOR, "Utility")

	int bufferSize = 512;
	float displayLengthSeconds = 5.0;
	ScrollingBuffer data;
	bool wasConnected = false;

	std::shared_ptr<NodePin> input;

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	
	virtual void nodeSpecificGui() override {
		if (ImGui::BeginTabItem("Plot")) {
			if (ImGui::InputInt("Buffer Size", &bufferSize, 0, 0)) data.setMaxSize(bufferSize);
			ImGui::InputFloat("Display Length Second", &displayLengthSeconds, 0.1, 1.0, "%.1fs");
			ImPlot::FitNextPlotAxes(false, true);
			ImPlot::SetNextPlotLimitsX((double)data.newest().x - (double)displayLengthSeconds, data.newest().x, ImGuiCond_Always);
			ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_CanvasOnly | ImPlotFlags_NoChild | ImPlotFlags_Query | ImPlotFlags_NoTitle;
			if (ImPlot::BeginPlot("Plot", nullptr, nullptr, ImVec2(-1, ImGui::GetTextLineHeight() * 15.0), plotFlags)) {
				ImPlot::PlotLine("Data", &data.front().x, &data.front().y, data.size(), data.offset(), data.stride());
				ImPlot::EndPlot();
			}
			ImGui::EndTabItem();
		}
	}

	virtual bool onSerialization() override {
		bool success = true;
		success &= Node::onSerialization();
		success &= serializeAttribute("BufferSize", bufferSize);
		success &= serializeAttribute("DisplayLEngthSeconds", displayLengthSeconds);
		return success;
	}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= Node::onDeserialization();
		success &= deserializeAttribute("BufferSize", bufferSize);
		success &= deserializeAttribute("DisplayLEngthSeconds", displayLengthSeconds);
		return success;
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Plotter");
		
		input = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "input", "input", NodePin::Flags::ForceDataField | NodePin::Flags::DisableDataField);
		
		input->assignData(inputPinValue);
		addNodePin(input);
		data.setMaxSize(bufferSize);
	};
	
	void inputProcess() override {
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
	
};
