#pragma once

#include "NodeGraph/ioNode.h"
#include "Utilities/ScrollingBuffer.h"

#include <imgui.h>
#include <implot.h>

class PlotterNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Plotter", PlotterNode, "Utilities")

	virtual void assignIoData() {
		addIoData(input);
		data.setMaxSize(bufferSize);
	}

	int bufferSize = 512;
	float displayLengthSeconds = 5.0;
	ScrollingBuffer data;
	bool wasConnected = false;

	std::shared_ptr<ioData> input = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "input", ioDataFlags_ForceDataField | ioDataFlags_DisableDataField);

	virtual void process() {
		if (input->isConnected()) {
			if (!wasConnected) {
				wasConnected = true;
				data.clear();
			}
			glm::vec2 newPoint;
			newPoint.x = Timing::getTime_seconds();
			newPoint.y = input->getLinks().front()->getInputData()->getReal();
			data.addPoint(newPoint);
		}
		else {
			if (wasConnected) {
				data.clear();
				wasConnected = false;
			}
		}
	}

	virtual void nodeSpecificGui() {
		if (ImGui::BeginTabItem("Plot")) {
			
			if (ImGui::InputInt("Buffer Size", &bufferSize, 0, 0)) {
				Logger::warn("buffer size edited");
			}
			ImGui::InputFloat("Display Length Second", &displayLengthSeconds, 0.1, 1.0, "%.1fs");


			ImPlot::FitNextPlotAxes(false, true);
			ImPlot::SetNextPlotLimitsX((double)data.newest().x - (double)displayLengthSeconds, data.newest().x, ImGuiCond_Always);
			
			if (ImPlot::BeginPlot("Plot")) {

				ImPlot::PlotLine("Data", &data.front().x, &data.front().y, data.size(), data.offset(), data.stride());

				ImPlot::EndPlot();
			}
		

			ImGui::EndTabItem();
		}
	}
};