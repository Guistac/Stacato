#include <pch.h>

#include "Fieldbus/EtherCatFieldbus.h"

#include <imgui.h>
#include <implot.h>

void etherCatMetrics() {

	//=====METRICS=====

	if (ImGui::BeginChild("MetricsChild")) {

        double processTime = EtherCatFieldbus::getMetrics().fieldbusTime_seconds;
        int processHours = std::floor(processTime / 3600.0);
        int processMinutes = std::floor((processTime - processHours * 3600.0) / 60.0);
        double processSeconds = processTime - (processHours * 3600.0) - (processMinutes * 60.0);
        ImGui::Text("Process Time: %ih%im%.1fs", processHours, processMinutes, processSeconds);
        ImGui::SameLine();
        ImGui::Text("Cycles: %i (%ims/cycle)", (int)EtherCatFieldbus::getMetrics().frameCount, EtherCatFieldbus::processInterval_milliseconds);
        ImGui::SameLine();
        ImGui::Text("Dropped Frames: %i (%.3f%%)", (int)EtherCatFieldbus::getMetrics().droppedFrameCount, 100.0 * double(EtherCatFieldbus::getMetrics().droppedFrameCount) / double(EtherCatFieldbus::getMetrics().frameCount));
    
		EtherCatMetrics& metrics = EtherCatFieldbus::getMetrics();
		float plotHeight = ImGui::GetTextLineHeight() * 20.0;

		static bool lockXAxis = true;
		static bool lockYAxis = true;
		ImGui::Checkbox("Lock X Axis", &lockXAxis);
		ImGui::SameLine();
		ImGui::Checkbox("Lock Y Axis", &lockYAxis);
		ImGui::SameLine();
		static float historyLength_seconds = 10.0;
		ImGui::SliderFloat("##History Length", &historyLength_seconds, 1.0, EtherCatFieldbus::getMetrics().scrollingBufferLength_seconds, "%g seconds");

		ImPlotFlags plotFlags = ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus;
		//if (lockXAxis && lockYAxis) plotFlags |= ImPlotFlags_NoChild;

		ScrollingBuffer& dcTimeErrors = EtherCatFieldbus::getMetrics().dcTimeErrors;
		ScrollingBuffer& averageDcTimeErrors = EtherCatFieldbus::getMetrics().averageDcTimeErrors;
		float maxPositiveDrift = EtherCatFieldbus::processInterval_milliseconds / 2.0;
		float maxNegativeDrift = -maxPositiveDrift;
		float positiveThreshold = EtherCatFieldbus::clockStableThreshold_milliseconds;
		float negativeThreshold = -positiveThreshold;
		
		if (lockXAxis) ImPlot::SetNextAxisLimits(ImAxis_X1, (double)dcTimeErrors.newest().x - (double)historyLength_seconds, dcTimeErrors.newest().x, ImGuiCond_Always);
		if (lockYAxis) ImPlot::SetNextAxisLimits(ImAxis_Y1, -1.0, 1.0, ImGuiCond_Always);

		if (ImPlot::BeginPlot("Clock Drift", NULL, NULL, ImVec2(-1, plotHeight), plotFlags)) {
			ImPlot::SetupAxisFormat(ImAxis_Y1, "%gms");
			ImPlot::SetupAxisFormat(ImAxis_X1, "%gs");
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
			ImPlot::PlotLine("Clock Drift", &dcTimeErrors.front().x, &dcTimeErrors.front().y, dcTimeErrors.size(), ImPlotLineFlags_None, dcTimeErrors.offset(), dcTimeErrors.stride());
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0);
			ImPlot::PlotLine("Average", &averageDcTimeErrors.front().x, &averageDcTimeErrors.front().y, averageDcTimeErrors.size(), ImPlotLineFlags_None, averageDcTimeErrors.offset(), averageDcTimeErrors.stride());
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), 2.0);
			ImPlot::PlotInfLines("Limits", &maxPositiveDrift, 1, ImPlotInfLinesFlags_Horizontal);
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), 2.0);
			ImPlot::PlotInfLines("Limits", &maxNegativeDrift, 1, ImPlotInfLinesFlags_Horizontal);
			ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 0.5f), 2.0);
			ImPlot::PlotInfLines("Threshold", &positiveThreshold, 1, ImPlotInfLinesFlags_Horizontal);
			ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 0.5f), 2.0);
			ImPlot::PlotInfLines("Threshold", &negativeThreshold, 1, ImPlotInfLinesFlags_Horizontal);
			ImPlot::EndPlot();
		}

		ScrollingBuffer& sendDelays = EtherCatFieldbus::getMetrics().sendDelays;
		ScrollingBuffer& timeoutDelays = EtherCatFieldbus::getMetrics().timeoutDelays;
		ScrollingBuffer& timeouts = EtherCatFieldbus::getMetrics().timeouts;
		ScrollingBuffer& receiveDelays = EtherCatFieldbus::getMetrics().receiveDelays;
		ScrollingBuffer& processDelays = EtherCatFieldbus::getMetrics().processDelays;
		ScrollingBuffer& cycleLengths = EtherCatFieldbus::getMetrics().cycleLengths;
		
		if (lockXAxis) ImPlot::SetNextAxisLimits(ImAxis_X1, (double)dcTimeErrors.newest().x - (double)historyLength_seconds, dcTimeErrors.newest().x, ImGuiCond_Always);
		if (lockYAxis) ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0, EtherCatFieldbus::processInterval_milliseconds * 1.1, ImGuiCond_Always);
		if (ImPlot::BeginPlot("Cycle Timing", NULL, NULL, ImVec2(-1, plotHeight), plotFlags)) {
			ImPlot::SetupAxisFormat(ImAxis_Y1, "%gms");
			ImPlot::SetupAxisFormat(ImAxis_X1, "%gs");
			ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 1.0f);
			if (!cycleLengths.empty()) {
				ImPlot::SetNextFillStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
				ImPlot::PlotShaded("Cycle", &cycleLengths.front().x, &cycleLengths.front().y, cycleLengths.size(), 0.0L, ImPlotShadedFlags_None, cycleLengths.offset(), cycleLengths.stride());
				ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
				ImPlot::PlotLine("Cycle", &cycleLengths.front().x, &cycleLengths.front().y, cycleLengths.size(), ImPlotLineFlags_None, cycleLengths.offset(), cycleLengths.stride());
			}
			if (!timeoutDelays.empty()) {
				ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 1.0f, 0.5f), 2.0);
				ImPlot::PlotLine("Timeout", &timeoutDelays.front().x, &timeoutDelays.front().y, timeoutDelays.size(), ImPlotLineFlags_None, timeoutDelays.offset(), timeoutDelays.stride());
			}
			if (!processDelays.empty()) {
				ImPlot::SetNextFillStyle(ImVec4(1.0f, 1.0f, 0.0f, 0.5f));
				ImPlot::PlotShaded("Process", &processDelays.front().x, &processDelays.front().y, processDelays.size(), 0.0L, ImPlotShadedFlags_None, processDelays.offset(), processDelays.stride());
				ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 1.0);
				ImPlot::PlotLine("Process", &processDelays.front().x, &processDelays.front().y, processDelays.size(), ImPlotLineFlags_None, processDelays.offset(), processDelays.stride());
			}
			if (!receiveDelays.empty()) {
				ImPlot::SetNextFillStyle(ImVec4(0.0f, 0.0f, 1.0f, 0.5f));
				ImPlot::PlotShaded("Receive", &receiveDelays.front().x, &receiveDelays.front().y, receiveDelays.size(), 0.0L, ImPlotShadedFlags_None, receiveDelays.offset(), receiveDelays.stride());
				ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), 1.0);
				ImPlot::PlotLine("Receive", &receiveDelays.front().x, &receiveDelays.front().y, receiveDelays.size(), ImPlotLineFlags_None, receiveDelays.offset(), receiveDelays.stride());
			}
			if (!sendDelays.empty()) {
				ImPlot::SetNextFillStyle(ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
				ImPlot::PlotShaded("Send", &sendDelays.front().x, &sendDelays.front().y, sendDelays.size(), 0.0L, ImPlotShadedFlags_None, sendDelays.offset(), sendDelays.stride());
				ImPlot::SetNextLineStyle(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 1.0);
				ImPlot::PlotLine("Send", &sendDelays.front().x, &sendDelays.front().y, sendDelays.size(), ImPlotLineFlags_None, sendDelays.offset(), sendDelays.stride());
			}
			if (!timeouts.empty()) {
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 4.0, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.0);
				ImPlot::PlotScatter("##Timeouts", &timeouts.front().x, &timeouts.front().y, timeouts.size(), ImPlotScatterFlags_None, timeouts.offset(), timeouts.stride());
			}
			ImPlot::PopStyleVar();
			ImPlot::EndPlot();
		}

		ImPlot::SetNextAxesLimits(0, 1, 0, 1, ImGuiCond_Always);
		if (ImPlot::BeginPlot("##WorkingCounter2", NULL, NULL, ImVec2(plotHeight, plotHeight), plotFlags)) {
			int totalCount = metrics.frameReturnTypeCounters[0] + metrics.frameReturnTypeCounters[1];
			float percentages[2];
			if (totalCount > 0) {
				percentages[0] = 100.0 * (float)metrics.frameReturnTypeCounters[0] / (float)totalCount;
				percentages[1] = 100.0 * (float)metrics.frameReturnTypeCounters[1] / (float)totalCount;
			}
			else {
				percentages[0] = 0.0;
				percentages[1] = 0.0;
			}
			ImPlot::PlotPieChart(metrics.frameReturnTypeChars, percentages, 2, 0.5, 0.5, 0.4, "%.2f%%");
			ImPlot::EndPlot();
		}

		ImGui::SameLine();

		ScrollingBuffer& workingCounters = EtherCatFieldbus::getMetrics().workingCounters;
		if (lockXAxis) ImPlot::SetNextAxisLimits(ImAxis_X1, (double)workingCounters.newest().x - (double)historyLength_seconds, workingCounters.newest().x, ImGuiCond_Always);
		ImPlot::SetNextAxisLimits(ImAxis_Y1, -6.0, 10.0, ImGuiCond_Always);

		if (ImPlot::BeginPlot("Working Counter", NULL, NULL, ImVec2(-1, plotHeight), plotFlags)) {
			ImPlot::SetupAxisFormat(ImAxis_X1, "%gs");
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0);
			ImPlot::PlotStairs("##WorkingCounter", &workingCounters.front().x, &workingCounters.front().y, workingCounters.size(), ImPlotStairsFlags_None, workingCounters.offset(), workingCounters.stride());
			ImPlot::EndPlot();
		}

		ImGui::EndChild();
	}
}
