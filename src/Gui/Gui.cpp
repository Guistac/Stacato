#include "Gui.h"
#include "Fieldbus/EthernetIpTest.h"

#include <imgui.h>
#include <imgui_internal.h>

void gui() {
	
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_Always);

	ImGui::Begin("EtherNET/IP Fieldbus Test Program", nullptr, ImGuiWindowFlags_NoResize);

	if (ImGui::Button("Discover EIP Devices")) {
		EthernetIPFieldbus::discoverDevices();
	}
	ImGui::SameLine();
	if (ImGui::Button("Discover & Add All")) {
		EthernetIPFieldbus::discoverDevices();
		for (EipDevice& device : EthernetIPFieldbus::discoveredDevices) {
			EthernetIPFieldbus::addDevice(device);
		}
	}

	for (EipDevice& device : EthernetIPFieldbus::discoveredDevices) {
		if (ImGui::TreeNode(device.displayName)) {

			ImGui::Text(device.host);
			ImGui::Text("Product Code: %i", device.productCode);
			ImGui::Text("Serial Number: %i", device.serialNumber);
			ImGui::Text("Version: %i.%i", device.revisionMajor, device.revisionMinor);
			ImGui::Text("Vendor ID: %i", device.vendorId);
			ImGui::Text("Status: %d", device.status);
			if (ImGui::Button("Add Device")) {
				EthernetIPFieldbus::addDevice(device);
			}

			ImGui::TreePop();
		}
	}

	std::shared_ptr<EipServoDrive> removedDrive = nullptr;

	if (ImGui::BeginTabBar("ServoDrives")) {
		for (std::shared_ptr<EipServoDrive> drive : EthernetIPFieldbus::servoDrives) {

			if (ImGui::BeginTabItem(drive->identity.displayName)) {

				if (drive->b_connected) {
					if (ImGui::Button("Disconnect")) drive->disconnect();
				}
				else {
					if (ImGui::Button("Connect")) drive->connect();
				}

				if (!drive->b_connected) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
				}

				if (ImGui::Button("Enable Power Stage")) drive->enablePowerStage = true;
				ImGui::SameLine();
				if (ImGui::Button("Disable Power Stage")) drive->disablePowerStage = true;
				if (ImGui::Button("Fault Reset")) drive->performFaultReset = true;
				ImGui::SameLine();
				if (ImGui::Button("Quick Stop")) drive->performQuickStop = true;
				if (drive->state == EipServoDrive::State::QUICK_STOP_ACTIVE || drive->quickstopActive) {
					ImGui::SameLine();
					ImGui::Text("QuickStop Active, Perform Fault Reset");
				}
				if (ImGui::Button("Halt")) drive->enableHalt = true;
				ImGui::SameLine();
				if (ImGui::Button("Clear Halt")) drive->clearHalt = true;
				if (drive->halted) {
					ImGui::SameLine();
					ImGui::Text("Halt Active");
				}
				if (ImGui::Button("Reboot")) drive->reboot();

				int maxVelocity = 7000;
				ImGui::SliderInt("##Velocity", &drive->requestedVelocity, -maxVelocity, maxVelocity, "%d rpm Command");
				ImVec2 velSliderSize = ImGui::GetItemRectSize();
				ImGui::SameLine();
				if (ImGui::Button("Stop Movement")) drive->requestedVelocity = 0;
				float velFraction = (float)(drive->_v_act + maxVelocity) / (float)(2 * maxVelocity);
				static char velocityChar[32];
				sprintf(velocityChar, "%i rpm", drive->_v_act);
				ImGui::ProgressBar(velFraction, velSliderSize, velocityChar);

				static int ramps[2];
				ramps[0] = drive->Ramp_v_acc;
				ramps[1] = drive->Ramp_v_dec;
				ImGui::SliderInt2("Speed Ramps (acc/dec)", ramps, 0, 10000, "%d rpm/s");
				drive->Ramp_v_acc = ramps[0];
				drive->Ramp_v_dec = ramps[1];

				ImGui::Text("Position: %i", drive->_p_act);
				ImGui::Text("Current: %i", drive->_i_act);
				
				ImGui::Separator();

				ImGui::Text("State: %s", drive->stateChar);
				ImGui::Text("Power Stage: %d", drive->b_powerStageEnabled);
				ImGui::Text("Error: %d", drive->error);
				ImGui::SameLine();
				ImGui::Text("Code: %X", drive->lastErrorCode);
				ImGui::Text("Warning: %d", drive->warning);
				ImGui::SameLine();
				ImGui::Text("Code: %X", drive->lastWarningCode);
				ImGui::Text("Halted: %d", drive->halted);
				ImGui::Text("Homing Valid: %d", drive->homingValid);
				ImGui::Text("QuickStop Active: %d", drive->quickstopActive);
				ImGui::Text("OperatingMode Terminated: %d", drive->operatingModeTerminated);
				ImGui::Text("OperatingMode Terminated with error: %d", drive->operatingModeTerminatedWithError);
				ImGui::Text("OperatingMode Specific Info: %d", drive->operatingModeSpecificInfo);

				ImGui::Separator();

				ImGui::Text("Operating Mode: %s", drive->modeChar);
				ImGui::Text("Data Error: %d", drive->dataError);
				ImGui::Text("Mode Error: %d", drive->modeError);
				ImGui::Text("Mode Toggle : %d", drive->modeToggle);
				ImGui::Text("Cap1 : %i", drive->cap1);
				ImGui::Text("Cap2 : %i", drive->cap2);
				
				ImGui::Separator();

				ImGui::Text("Positive Limit : %d", drive->positiveLimit);
				ImGui::Text("Negative Limit : %d", drive->negativeLimit);
				ImGui::Text("Motor Standstill: %d", drive->motorStandstill);
				ImGui::Text("Motor Negative Movement: %d", drive->motorNegativeMovement);
				ImGui::Text("Motor Positive Movement: %d", drive->motorPositiveMovement);
				ImGui::Text("DS402intLim: %d", drive->DS402intLim);
				ImGui::Text("DPL_intLim: %d", drive->DPL_intLim);
				ImGui::Text("Profile Generator Standstill: %d", drive->profileGeneratorStandstill);
				ImGui::Text("Profile Generator Decelerates: %d", drive->profileGeneratorDecelerates);
				ImGui::Text("Profile Generator Accelerates: %d", drive->profileGeneratorAccelerates);
				ImGui::Text("Profile Generator Constant Velocity: %d", drive->profileGeneratorConstantVelocity);

				if (!drive->b_connected) {
					ImGui::PopItemFlag();
					ImGui::PopStyleColor();
				}

				if (ImGui::Button("Remove Device")) removedDrive = drive;
				ImGui::EndTabItem();
			}

		}
		ImGui::EndTabBar();
	}

	if(removedDrive != nullptr) EthernetIPFieldbus::removeDevice(removedDrive);

	ImGui::End();

}
