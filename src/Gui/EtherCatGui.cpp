#include "EtherCatGui.h"

#include "Fieldbus/EtherCAT/EtherCatFieldbus.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h";

bool etherCatGui() {

	ImGui::Begin("EtherCAT Fieldbus Test Program", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	if (ImGui::Button("Scan Network")) EtherCatFieldbus::scanNetwork();
	ImGui::SameLine();
	if (!EtherCatFieldbus::b_networkScanned) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
	}
	if (ImGui::Button("Configure Slaves")) EtherCatFieldbus::configureSlaves();
	if (!EtherCatFieldbus::b_networkScanned) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}
	ImGui::SameLine();
	if (!EtherCatFieldbus::b_ioMapConfigured) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
	}

	if (ImGui::Button("Start Cyclic Exchange")) EtherCatFieldbus::startCyclicExchange();
	ImGui::SameLine();
	if (ImGui::Button("Stop Cyclic Exchange")) EtherCatFieldbus::stopCyclicExchange();
	if (!EtherCatFieldbus::b_ioMapConfigured) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}

	ImGui::Separator();
	
	if (ImGui::Button("Fault Reset All")) for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) drive.performFaultReset = true;
	ImGui::SameLine();
	if (ImGui::Button("Enable All")) for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) { drive.enableOperation = true; drive.positionCommand = drive.position; drive.counter = 0; }
	ImGui::SameLine();
	if(ImGui::Button("Disable All")) for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) drive.disableOperation = true;

	ImGui::Separator();

	

	if (EtherCatFieldbus::b_networkScanned) {
		for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) {
		
			if (ImGui::TreeNode(drive.identity.displayName)) {
				ImGui::Text("Address: %i", drive.identity.address);
				if (!drive.identity.b_configured) ImGui::Text("Not Configured");
				else {
					ImGui::Text("Input Bytes: %i (%i bits)", drive.identity.slave_ptr->Ibytes, drive.identity.slave_ptr->Ibits);
					ImGui::Text("Output Bytes: %i (%i bits)", drive.identity.slave_ptr->Obytes, drive.identity.slave_ptr->Obits);
				}

				ec_slavet& slave = *drive.identity.slave_ptr;
				slave.DCactive;
				slave.DCcycle;
				slave.DCnext;
				slave.DCprevious;
				slave.DCrtA;
				slave.DCrtB;
				slave.DCrtC;
				slave.DCrtD;
				slave.DCshift;
				slave.topology;
				slave.state;
				slave.ALstatuscode;
				slave.hasdc;
				slave.pdelay;
				slave.activeports;
				slave.consumedports;
				slave.Ebuscurrent;
				slave.Dtype;
				slave.parent;

				ImGui::Text("state: %i   ALstatuscode: %i", slave.state, slave.ALstatuscode);
				ImGui::Text("ptype: %i  topology: %i  activeports: %i  consumedports: %i", slave.ptype, slave.topology, slave.activeports, slave.consumedports);

				ImGui::Text("hasDC: %i  DCactive: %i", slave.hasdc, slave.DCactive);
				ImGui::Text("pdelay: %i  DCnext: %i  DCprevious: %i  DCcycle: %i  DCshift: %i", slave.pdelay, slave.DCnext, slave.DCprevious, slave.DCcycle, slave.DCshift);
				ImGui::Text("parent: %i  parentport: %i  entryport: %i  ebuscurrent: %i", slave.parent, slave.parentport, slave.entryport, slave.Ebuscurrent);
				ImGui::Text("DCrtA: %i  DcrtB: %i  DCrtC: %i  DCrtD: %i", slave.DCrtA, slave.DCrtB, slave.DCrtC, slave.DCrtD);

				ImGui::Separator();

				if (ImGui::Button("Enable Voltage")) drive.enableVoltage = true;
				ImGui::SameLine();
				if (ImGui::Button("Disable Voltage")) drive.disableVoltage = true;

				if (ImGui::Button("Switch On")) drive.switchOn = true;
				ImGui::SameLine();
				if (ImGui::Button("Shut Down")) drive.shutdown = true;

				if (ImGui::Button("Enable Operation")) drive.enableOperation = true;
				ImGui::SameLine();
				if (ImGui::Button("Disable Operation")) drive.disableOperation = true;

				if (ImGui::Button("Fault Reset")) drive.performFaultReset = true;
				ImGui::SameLine();
				if (ImGui::Button("Quick Stop")) drive.performQuickStop = true;

				const char* stateChar;
				switch (drive.state) {
					case ECatServoDrive::State::Fault:					stateChar = "Fault"; break;
					case ECatServoDrive::State::FaultReactionActive:	stateChar = "Fault Reaction Active"; break;
					case ECatServoDrive::State::NotReadyToSwitchOn:		stateChar = "Not Ready To Switch On"; break;
					case ECatServoDrive::State::OperationEnabled:		stateChar = "Operation Enabled"; break;
					case ECatServoDrive::State::QuickStopActive:		stateChar = "Quick Stop Active"; break;
					case ECatServoDrive::State::ReadyToSwitchOn:		stateChar = "Ready To Switch On"; break;
					case ECatServoDrive::State::SwitchedOn:				stateChar = "Switched On"; break;
					case ECatServoDrive::State::SwitchOnDisabled:		stateChar = "Switch On Disabled"; break;
				}
				ImGui::Text("State: %s", stateChar);
				if (drive.state == ECatServoDrive::State::Fault) {
					ImGui::SameLine();
					ImGui::Text("(%X)", drive.lastErrorCode);
				}

				ImGui::Separator();

				ImGui::Text("Position: %i", drive.position);
				ImGui::Text("Velocity: %i", drive.velocity);
				ImGui::Text("Torque: %i", drive.torque);

				ImGui::Text("Digital Inputs: %i %i %i %i %i %i",
					drive.DI0,
					drive.DI1,
					drive.DI2,
					drive.DI3,
					drive.DI4,
					drive.DI5
				);

				ImGui::Separator();

				ImGui::Text("Current Operating Mode: %s (%i)", drive.modeChar, drive.mode);

				if (ImGui::BeginCombo("Mode Switch", ECatServoDrive::modelist[drive.modeCommand].c_str(), ImGuiComboFlags_HeightLargest)) {
					for (const auto& [key, value] : ECatServoDrive::modelist) {
						if (key == 0) continue;
						if (ImGui::Selectable(value.c_str(), false)) drive.modeCommand = key;
						
					}
					ImGui::EndCombo();
				}

				ImPlot::SetNextPlotLimitsX(drive.positions.newest().x - 1000.0, drive.positions.newest().x, ImGuiCond_Always);
				ImPlot::SetNextPlotFormatY("%g ticks");
				ImPlot::FitNextPlotAxes(false, true);

				if (ImPlot::BeginPlot("positions", NULL, NULL, ImVec2(-1, 600))) {
					ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
					ImPlot::PlotLine("Position", &drive.positions.front().x, &drive.positions.front().y, drive.positions.size(), drive.positions.offset(), drive.positions.stride());
					ImPlot::EndPlot();
				}

				ImGui::Checkbox("Invert", &drive.b_inverted);
				
				int position = drive.position;
				int velocity = drive.velocity;
				int torque = drive.torque;

				float velocityFraction = (((double)velocity / 7000.0) + 1.0) / 2.0;
				ImGui::ProgressBar(velocityFraction, ImVec2(0, 0), "velocity");

				/*
				ImGui::InputInt("Position Command", &positionCommand, 1000, 10000);
				ImGui::SameLine();
				if (ImGui::Button("Zero")) positionCommand = 0;

				ImGui::SliderInt("##Velocity", &velocityCommand, -7000, 7000, "%d rpm");
				drive.positionCommand = positionCommand;
				ImGui::SameLine();
				if(ImGui::Button("Stop")) velocityCommand = 0;

				ImGui::SliderInt("##Torque", &torqueCommand, -100, 100, "%d t");
				ImGui::SameLine();
				if (ImGui::Button("Cut")) torqueCommand = 0;
				*/

				ImGui::Checkbox("DQ0", &drive.DQ0);
				ImGui::SameLine();
				ImGui::Checkbox("DQ1", &drive.DQ1);
				ImGui::SameLine();
				ImGui::Checkbox("DQ2", &drive.DQ2);

				if (ImGui::Button("<<--")) {
					drive.jog = true;
					drive.direction = false;
					drive.fast = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("<--")) {
					drive.jog = true;
					drive.direction = false;
					drive.fast = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("0")) {
					drive.stop = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("-->")) {
					drive.jog = true;
					drive.direction = true;
					drive.fast = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("-->>")) {
					drive.jog = true;
					drive.direction = true;
					drive.fast = true;
				}

				ImGui::Separator();

				ImGui::Text("motorVoltagePresent : %i", drive.motorVoltagePresent);
				ImGui::Text("class0error : %i", drive.class0error);
				ImGui::Text("halted : %i", drive.halted);
				ImGui::Text("fieldbusControlActive : %i", drive.fieldbusControlActive);
				ImGui::Text("targetReached : %i", drive.targetReached);
				ImGui::Text("internalLimitActive : %i", drive.internalLimitActive);
				ImGui::Text("operatingModeSpecificFlag : %i", drive.operatingModeSpecificFlag);
				ImGui::Text("stoppedByError : %i", drive.stoppedByError);
				ImGui::Text("operatingModeFinished : %i", drive.operatingModeFinished);
				ImGui::Text("validPositionReference : %i", drive.validPositionReference);
				ImGui::TreePop();
			}
			
		}
	}

	ImGui::Separator();

	//=====METRICS=====

	ImGui::Text("Process Time: %.1fs  cycles: %i (%.1fms/cycle)",
		EtherCatFieldbus::metrics.processTime_seconds,
		EtherCatFieldbus::metrics.cycleCounter,
		EtherCatFieldbus::processInterval_milliseconds);

	static bool lockXAxis = true;
	static bool lockYAxis = true;
	ImGui::Checkbox("Lock X Axis", &lockXAxis);
	ImGui::SameLine();
	ImGui::Checkbox("Lock Y Axis", &lockYAxis);
	ImGui::SameLine();
	static float historyLength_seconds = 10.0;
	ImGui::SliderFloat("##History Length", &historyLength_seconds, 1.0, EtherCatFieldbus::metrics.scrollingBufferLength_seconds, "%g seconds");

	ScrollingBuffer& dcTimeErrors = EtherCatFieldbus::metrics.dcTimeErrors;
	ScrollingBuffer& averageDcTimeErrors = EtherCatFieldbus::metrics.averageDcTimeErrors;
	float maxPositiveDrift = EtherCatFieldbus::processInterval_milliseconds / 2.0;
	float maxNegativeDrift = -maxPositiveDrift;
	if (lockXAxis) ImPlot::SetNextPlotLimitsX((double)dcTimeErrors.newest().x - (double)historyLength_seconds, dcTimeErrors.newest().x, ImGuiCond_Always);
	if (lockYAxis) ImPlot::SetNextPlotLimitsY(-1.0, 1.0, ImGuiCond_Always);
	ImPlot::SetNextPlotFormatY("%gms");
	ImPlot::SetNextPlotFormatX("%gs");
	if (ImPlot::BeginPlot("Clock Drift", NULL, NULL, ImVec2(-1, 600), ImPlotFlags_AntiAliased)) {
		ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
		ImPlot::PlotLine("Clock Drift", &dcTimeErrors.front().x, &dcTimeErrors.front().y, dcTimeErrors.size(), dcTimeErrors.offset(), dcTimeErrors.stride());
		ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0);
		ImPlot::PlotLine("Average", &averageDcTimeErrors.front().x, &averageDcTimeErrors.front().y, averageDcTimeErrors.size(), averageDcTimeErrors.offset(), averageDcTimeErrors.stride());
		ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), 2.0);
		ImPlot::PlotHLines("Limits", &maxPositiveDrift, 1);
		ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), 2.0);
		ImPlot::PlotHLines("Limits", &maxNegativeDrift, 1);
		ImPlot::EndPlot();
	}

	ScrollingBuffer& sendDelays =		EtherCatFieldbus::metrics.sendDelays;
	ScrollingBuffer& timeoutDelays =	EtherCatFieldbus::metrics.timeoutDelays;
	ScrollingBuffer& timeouts =			EtherCatFieldbus::metrics.timeouts;
	ScrollingBuffer& receiveDelays =	EtherCatFieldbus::metrics.receiveDelays;
	ScrollingBuffer& processDelays =	EtherCatFieldbus::metrics.processDelays;
	ScrollingBuffer& cycleLengths =		EtherCatFieldbus::metrics.cycleLengths;

	if (lockXAxis) ImPlot::SetNextPlotLimitsX((double)dcTimeErrors.newest().x - (double)historyLength_seconds, dcTimeErrors.newest().x, ImGuiCond_Always);
	if (lockYAxis) ImPlot::SetNextPlotLimitsY(0.0, EtherCatFieldbus::processInterval_milliseconds * 1.1, ImGuiCond_Always);
	ImPlot::SetNextPlotFormatY("%gms");
	ImPlot::SetNextPlotFormatX("%gs");
	float receiveTimeoutLine = EtherCatFieldbus::processDataTimeout_milliseconds;
	if (ImPlot::BeginPlot("Cycle Timing", NULL, NULL, ImVec2(-1, 600), ImPlotFlags_AntiAliased)) {
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
	if (ImPlot::BeginPlot("##WorkingCounter2", NULL, NULL, ImVec2(600, 600), ImPlotFlags_AntiAliased | ImPlotFlags_Equal | ImPlotFlags_NoMousePos, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations)) {
		ECatMetrics& metrics = EtherCatFieldbus::metrics;
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
	if (ImPlot::BeginPlot("Working Counter", NULL, NULL, ImVec2(-1, 600), ImPlotFlags_AntiAliased)) {
		ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0);
		ImPlot::PlotStairs("##WorkingCounter", &workingCounters.front().x, &workingCounters.front().y, workingCounters.size(), workingCounters.offset(), workingCounters.stride());
		ECatMetrics& metrics = EtherCatFieldbus::metrics;
		ImPlot::EndPlot();
	}

	ImGui::Separator();
	bool exitFieldbus = false;
	if (ImGui::Button("Return to Fieldbus Selection")) exitFieldbus = true;
	
	ImGui::End();

	return !exitFieldbus;
}