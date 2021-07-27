#include "Gui.h"
#include "Fieldbus/EthernetIpTest.h"

#include <imgui.h>
#include <imgui_internal.h>

void gui() {
	
	ImGui::Begin("Discovery");

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
			if (ImGui::Button("Add Device")) {
				EthernetIPFieldbus::addDevice(device);
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();

	ImGui::Begin("Devices");

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
				if (ImGui::Button("Halt")) drive->enableHalt = true;
				ImGui::SameLine();
				if (ImGui::Button("Clear Halt")) drive->clearHalt = true;

				ImGui::SliderInt("Velocity", &drive->requestedVelocity, -7000, 7000, "%drpm");

				ImGui::Text("Position: %i", drive->_p_act);
				ImGui::Text("Velocity: %i", drive->_v_act);
				ImGui::Text("Current: %i", drive->_i_act);
				
				ImGui::Separator();

				ImGui::Text("State: %s", drive->stateChar);
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
