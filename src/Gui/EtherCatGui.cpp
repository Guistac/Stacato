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
	if (ImGui::Button("Enable All")) for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) { drive.enableOperation = true; drive.positionCommand = drive.position; drive.counter = 0; }
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

				ImPlot::SetNextPlotLimitsX(drive.positions.back().x - 1000.0, drive.positions.back().x, ImGuiCond_Always);
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

	bool exitFieldbus = false;
	if (ImGui::Button("Return to Fieldbus Selection")) exitFieldbus = true;

	ImGui::End();


	
	/*
	ImGui::Begin("EtherCAT Timing");
	ScrollingBuffer& clockDrift = EtherCatFieldbus::clockDrift;
	ScrollingBuffer& averageClockDrift = EtherCatFieldbus::averageClockDrift;
	static bool lockAxes = true;
	ImGui::Checkbox("Lock Axes", &lockAxes);
	if (lockAxes) {
		ImPlot::SetNextPlotLimitsX(clockDrift.back().x - 1000.0, clockDrift.back().x, ImGuiCond_Always);
		ImPlot::FitNextPlotAxes(false, true);
	}
	ImPlot::SetNextPlotFormatY("%g ms");
	if (ImPlot::BeginPlot("drift", NULL, NULL, ImVec2(-1, 600))) {
		ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
		ImPlot::PlotLine("clock drift", &clockDrift.front().x, &clockDrift.front().y, clockDrift.size(), clockDrift.offset(), clockDrift.stride());
		//ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 8.0);
		//ImPlot::PlotLine("clock drift", &averageClockDrift.front().x, &averageClockDrift.front().y, averageClockDrift.size(), averageClockDrift.offset(), averageClockDrift.stride());
		ImPlot::EndPlot();
	}
	ScrollingBuffer& intervals = EtherCatFieldbus::timingHistory;
	if (lockAxes) {
		ImPlot::SetNextPlotLimitsX(intervals.back().x - 1000.0, intervals.back().x, ImGuiCond_Always);
		ImPlot::SetNextPlotLimitsY(EtherCatFieldbus::processInterval_microseconds - 500.0, EtherCatFieldbus::processInterval_microseconds + 5000.0, ImGuiCond_Always);
	}
	ImPlot::SetNextPlotFormatY("%g us");
	if (ImPlot::BeginPlot("ec_timing", NULL, NULL, ImVec2(-1, 600))) {
		ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
		ImPlot::PlotLine("SendInterval", &intervals.front().x, &intervals.front().y, intervals.size(), intervals.offset(), intervals.stride());
		ImPlot::EndPlot();
	}
	ScrollingBuffer& workingCounters = EtherCatFieldbus::workingCounterHistory;
	if (lockAxes) {
		ImPlot::SetNextPlotLimitsX(intervals.back().x - 1000.0, intervals.back().x, ImGuiCond_Always);
		ImPlot::SetNextPlotLimitsY(-10.0, 10.0, ImGuiCond_Always);
	}
	ImPlot::SetNextPlotFormatY("%g wc");
	if (ImPlot::BeginPlot("ec_wc", NULL, NULL, ImVec2(-1, 600))) {
		ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0);
		ImPlot::PlotLine("WorkingCounters", &workingCounters.front().x, &workingCounters.front().y, workingCounters.size(), workingCounters.offset(), workingCounters.stride());
		ImPlot::EndPlot();
	}
	
	ImGui::End();
	*/

	//ImGui::SetNextWindowFocus();
	//ImGui::ShowMetricsWindow();
	
	return !exitFieldbus;
}