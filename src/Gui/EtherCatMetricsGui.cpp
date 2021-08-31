#include <pch.h>

#include "Gui.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Fieldbus/EtherCatFieldbus.h"

void etherCatMetrics() {

	//=====METRICS=====

	if (ImGui::BeginChild("MetricsChild")) {

		ImGui::Text("Process Time: %.1fs  cycles: %i (%.1fms/cycle)",
			EtherCatFieldbus::metrics.processTime_seconds,
			(int)EtherCatFieldbus::metrics.cycleCounter,
			EtherCatFieldbus::processInterval_milliseconds);

		EtherCatMetrics& metrics = EtherCatFieldbus::metrics;
		float plotHeight = ImGui::GetTextLineHeight() * 20.0;

		static bool lockXAxis = true;
		static bool lockYAxis = true;
		ImGui::Checkbox("Lock X Axis", &lockXAxis);
		ImGui::SameLine();
		ImGui::Checkbox("Lock Y Axis", &lockYAxis);
		ImGui::SameLine();
		static float historyLength_seconds = 10.0;
		ImGui::SliderFloat("##History Length", &historyLength_seconds, 1.0, EtherCatFieldbus::metrics.scrollingBufferLength_seconds, "%g seconds");

		ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus;
		if (lockXAxis && lockYAxis) plotFlags |= ImPlotFlags_NoChild;

		ScrollingBuffer& dcTimeErrors = EtherCatFieldbus::metrics.dcTimeErrors;
		ScrollingBuffer& averageDcTimeErrors = EtherCatFieldbus::metrics.averageDcTimeErrors;
		float maxPositiveDrift = EtherCatFieldbus::processInterval_milliseconds / 2.0;
		float maxNegativeDrift = -maxPositiveDrift;
		float positiveThreshold = EtherCatFieldbus::clockStableThreshold_milliseconds;
		float negativeThreshold = -positiveThreshold;
		if (lockXAxis) ImPlot::SetNextPlotLimitsX((double)dcTimeErrors.newest().x - (double)historyLength_seconds, dcTimeErrors.newest().x, ImGuiCond_Always);
		if (lockYAxis) ImPlot::SetNextPlotLimitsY(-1.0, 1.0, ImGuiCond_Always);
		ImPlot::SetNextPlotFormatY("%gms");
		ImPlot::SetNextPlotFormatX("%gs");

		if (ImPlot::BeginPlot("Clock Drift", NULL, NULL, ImVec2(-1, plotHeight), plotFlags)) {
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
			ImPlot::PlotLine("Clock Drift", &dcTimeErrors.front().x, &dcTimeErrors.front().y, dcTimeErrors.size(), dcTimeErrors.offset(), dcTimeErrors.stride());
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0);
			ImPlot::PlotLine("Average", &averageDcTimeErrors.front().x, &averageDcTimeErrors.front().y, averageDcTimeErrors.size(), averageDcTimeErrors.offset(), averageDcTimeErrors.stride());
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), 2.0);
			ImPlot::PlotHLines("Limits", &maxPositiveDrift, 1);
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), 2.0);
			ImPlot::PlotHLines("Limits", &maxNegativeDrift, 1);
			ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 0.5f), 2.0);
			ImPlot::PlotHLines("Threshold", &positiveThreshold, 1);
			ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 0.5f), 2.0);
			ImPlot::PlotHLines("Threshold", &negativeThreshold, 1);
			ImPlot::EndPlot();
		}

		ScrollingBuffer& sendDelays = EtherCatFieldbus::metrics.sendDelays;
		ScrollingBuffer& timeoutDelays = EtherCatFieldbus::metrics.timeoutDelays;
		ScrollingBuffer& timeouts = EtherCatFieldbus::metrics.timeouts;
		ScrollingBuffer& receiveDelays = EtherCatFieldbus::metrics.receiveDelays;
		ScrollingBuffer& processDelays = EtherCatFieldbus::metrics.processDelays;
		ScrollingBuffer& cycleLengths = EtherCatFieldbus::metrics.cycleLengths;

		if (lockXAxis) ImPlot::SetNextPlotLimitsX((double)dcTimeErrors.newest().x - (double)historyLength_seconds, dcTimeErrors.newest().x, ImGuiCond_Always);
		if (lockYAxis) ImPlot::SetNextPlotLimitsY(0.0, EtherCatFieldbus::processInterval_milliseconds * 1.1, ImGuiCond_Always);
		ImPlot::SetNextPlotFormatY("%gms");
		ImPlot::SetNextPlotFormatX("%gs");
		if (ImPlot::BeginPlot("Cycle Timing", NULL, NULL, ImVec2(-1, plotHeight), plotFlags)) {
			ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 1.0f);
			if (!cycleLengths.empty()) {
				ImPlot::SetNextFillStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
				ImPlot::PlotShaded("Cycle", &cycleLengths.front().x, &cycleLengths.front().y, cycleLengths.size(), 0.0L, cycleLengths.offset(), cycleLengths.stride());
				ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
				ImPlot::PlotLine("Cycle", &cycleLengths.front().x, &cycleLengths.front().y, cycleLengths.size(), cycleLengths.offset(), cycleLengths.stride());
			}
			if (!timeoutDelays.empty()) {
				ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 1.0f, 0.5f), 2.0);
				ImPlot::PlotLine("Timeout", &timeoutDelays.front().x, &timeoutDelays.front().y, timeoutDelays.size(), timeoutDelays.offset(), timeoutDelays.stride());
			}
			if (!processDelays.empty()) {
				ImPlot::SetNextFillStyle(ImVec4(1.0f, 1.0f, 0.0f, 0.5f));
				ImPlot::PlotShaded("Process", &processDelays.front().x, &processDelays.front().y, processDelays.size(), 0.0L, processDelays.offset(), processDelays.stride());
				ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 1.0);
				ImPlot::PlotLine("Process", &processDelays.front().x, &processDelays.front().y, processDelays.size(), processDelays.offset(), processDelays.stride());
			}
			if (!receiveDelays.empty()) {
				ImPlot::SetNextFillStyle(ImVec4(0.0f, 0.0f, 1.0f, 0.5f));
				ImPlot::PlotShaded("Receive", &receiveDelays.front().x, &receiveDelays.front().y, receiveDelays.size(), 0.0L, receiveDelays.offset(), receiveDelays.stride());
				ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), 1.0);
				ImPlot::PlotLine("Receive", &receiveDelays.front().x, &receiveDelays.front().y, receiveDelays.size(), receiveDelays.offset(), receiveDelays.stride());
			}
			if (!sendDelays.empty()) {
				ImPlot::SetNextFillStyle(ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
				ImPlot::PlotShaded("Send", &sendDelays.front().x, &sendDelays.front().y, sendDelays.size(), 0.0L, sendDelays.offset(), sendDelays.stride());
				ImPlot::SetNextLineStyle(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 1.0);
				ImPlot::PlotLine("Send", &sendDelays.front().x, &sendDelays.front().y, sendDelays.size(), sendDelays.offset(), sendDelays.stride());
			}
			if (!timeouts.empty()) {
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 4.0, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.0);
				ImPlot::PlotScatter("##Timeouts", &timeouts.front().x, &timeouts.front().y, timeouts.size(), timeouts.offset(), timeouts.stride());
			}
			ImPlot::PopStyleVar();
			ImPlot::EndPlot();
		}

		ImPlot::SetNextPlotLimits(0, 1, 0, 1, ImGuiCond_Always);
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
			ImPlot::PlotPieChart(metrics.frameReturnTypeChars, percentages, 2, 0.5, 0.5, 0.4, true, "%.2f%%");
			ImPlot::EndPlot();
		}

		ImGui::SameLine();

		ScrollingBuffer& workingCounters = EtherCatFieldbus::metrics.workingCounters;
		if (lockXAxis) ImPlot::SetNextPlotLimitsX((double)workingCounters.newest().x - (double)historyLength_seconds, workingCounters.newest().x, ImGuiCond_Always);
		ImPlot::SetNextPlotLimitsY(-6.0, 10.0, ImGuiCond_Always);

		ImPlot::SetNextPlotFormatX("%gs");
		if (ImPlot::BeginPlot("Working Counter", NULL, NULL, ImVec2(-1, plotHeight), plotFlags)) {
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0);
			ImPlot::PlotStairs("##WorkingCounter", &workingCounters.front().x, &workingCounters.front().y, workingCounters.size(), workingCounters.offset(), workingCounters.stride());
			ImPlot::EndPlot();
		}

		ImGui::EndChild();
	}
}
