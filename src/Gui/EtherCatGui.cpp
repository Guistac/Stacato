#include "EtherCatGui.h"

#include "Fieldbus/EtherCAT/EtherCatFieldbus.h"

#include "imgui.h"
#include "imgui_internal.h"

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

	if (EtherCatFieldbus::b_networkScanned) {
		for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) {
			if (ImGui::TreeNode(drive.identity.displayName)) {
				ImGui::Text("Address: %i", drive.identity.address);
				if (!drive.identity.b_configured) ImGui::Text("Not Configured");
				else {
					ImGui::Text("Input Bytes: %i (%i bits)", drive.identity.slave_ptr->Ibytes, drive.identity.slave_ptr->Ibits);
					ImGui::Text("Output Bytes: %i (%i bits)", drive.identity.slave_ptr->Obytes, drive.identity.slave_ptr->Obits);
				}

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
				ImGui::Text("Position Error: %i", drive.positionError);
				ImGui::Text("Torque: %i", drive.torque);

				ImGui::Text("Digital Inputs: %i %i %i %i %i",
					drive.digitalInputs & 0x1,
					drive.digitalInputs & 0x2,
					drive.digitalInputs & 0x4,
					drive.digitalInputs & 0x8,
					drive.digitalInputs & 0x10
				);

				ImGui::Separator();

				ImGui::Text("Current Operating Mode: %s (%i)", drive.modeChar, drive.mode);

				if (ImGui::BeginCombo("Mode Switch", "Select Mode")) {
					for (const auto& [key, value] : ECatServoDrive::modelist) {
						if (ImGui::Selectable(value.c_str(), false)) drive.modeCommand = key;
						
					}
					ImGui::EndCombo();
				}

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
				if (ImGui::Button("Stop")) {
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

	return !exitFieldbus;
}