#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"

#include "Gui/Utilities/HelpMarker.h"

void etherCatParameters(bool resetNicLists) {

	if (ImGui::BeginChild("EtherCatParameters")) {

		float widgetWidth = ImGui::GetTextLineHeight() * 20.0;
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("EtherCAT Network Hardware");
		ImGui::PopFont();
		
		ImGui::SameLine();
		if(beginHelpMarker("(help)")){
			ImGui::TextWrapped("Selection of the actual physical ethernet port for EtherCAT Communications."
							   "\nFor a redundant ring network topology, a second network interface card can be selected. This will make the system immune to a cable break in the network loop."
							   "\nAuto setup will select the first network interface card to which an EtherCAT device is connnected.");
			endHelpMarker();
		}

		std::shared_ptr<NetworkInterfaceCard> selectedPrimaryNic = EtherCatFieldbus::getActiveNetworkInterfaceCard();
		std::shared_ptr<NetworkInterfaceCard> selectedSecondaryNic = EtherCatFieldbus::getActiveRedundantNetworkInterfaceCard();
		bool b_nicSelected = false;
		
		bool disableNicButtons = EtherCatFieldbus::isRunning();
		
		ImGuiTableFlags tableFlags = ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
		if(ImGui::BeginTable("##nicTable", 2, tableFlags)){
			
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Primary Network Interface Card");
			ImGui::TableNextColumn();
			ImGui::Text("Redundant Network Interface Card (optional)");

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			
			ImGui::BeginDisabled(disableNicButtons);
			
			ImGui::SetNextItemWidth(widgetWidth);
			const char* primaryLabel = selectedPrimaryNic == nullptr ? "None" : selectedPrimaryNic->description;
			if (ImGui::BeginCombo("##NetworkInterfaceCard", primaryLabel)) {
				if (ImGui::Selectable("None", selectedPrimaryNic == nullptr)) {
					selectedPrimaryNic = nullptr;
					selectedSecondaryNic = nullptr;
					b_nicSelected = true;
				}
				for (auto& nic : EtherCatFieldbus::getNetworksInterfaceCards()) {
					bool selected = nic == selectedPrimaryNic;
					if (ImGui::Selectable(nic->description, selected)) {
						selectedPrimaryNic = nic;
						b_nicSelected = true;
					}
				}
				ImGui::EndCombo();
			}
			
			ImGui::TableNextColumn();
			
			bool disableSecondaryNicSelection = selectedPrimaryNic == nullptr;
			ImGui::BeginDisabled(disableSecondaryNicSelection);
			
			ImGui::SetNextItemWidth(widgetWidth);
			const char* secondaryLabel = selectedSecondaryNic == nullptr ? "None" : selectedSecondaryNic->description;
			if (ImGui::BeginCombo("##RedundantNetworkInterfaceCard", secondaryLabel)) {
				if (ImGui::Selectable("None", selectedSecondaryNic == nullptr)) {
					selectedSecondaryNic = nullptr;
					b_nicSelected = true;
				}
				for (auto& nic : EtherCatFieldbus::getNetworksInterfaceCards()) {
					bool disableSelection = nic == selectedPrimaryNic;
					ImGui::BeginDisabled(disableSelection);
					if (ImGui::Selectable(nic->description, selectedSecondaryNic == nic)) {
						selectedSecondaryNic = nic;
						b_nicSelected = true;
					}
					ImGui::EndDisabled();

				}
				ImGui::EndCombo();
			}
			 
			ImGui::EndDisabled();
			ImGui::EndDisabled();
			
			ImGui::EndTable();
		}
		
		
		if(b_nicSelected){
			if (selectedSecondaryNic == nullptr && selectedPrimaryNic == nullptr) EtherCatFieldbus::terminate();
			else if(selectedSecondaryNic == nullptr && selectedPrimaryNic != nullptr) EtherCatFieldbus::init(selectedPrimaryNic);
			else if (selectedPrimaryNic != nullptr) EtherCatFieldbus::init(selectedPrimaryNic, selectedSecondaryNic);
		}
		
		
		static char networkStatusString[256];
		if (EtherCatFieldbus::hasNetworkInterface()) {
			if (!EtherCatFieldbus::hasRedundantInterface())
				sprintf(networkStatusString,
						"Network is Open on Interface '%s'",
						EtherCatFieldbus::getActiveNetworkInterfaceCard()->description);
			else sprintf(networkStatusString,
						 "Network is Open on Interface '%s' with redundancy on '%s'",
						 EtherCatFieldbus::getActiveNetworkInterfaceCard()->description,
						 EtherCatFieldbus::getActiveRedundantNetworkInterfaceCard()->description);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		}
		else {
			sprintf(networkStatusString, "Network is not Open");
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		}
		 
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Button(networkStatusString);
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::BeginDisabled(disableNicButtons);
		if (ImGui::Button("Refresh Device List")) EtherCatFieldbus::updateNetworkInterfaceCardList();
		ImGui::SameLine();
		if(ImGui::Button("Auto Setup")) EtherCatFieldbus::autoInit();
		ImGui::EndDisabled();
		
		static char deviceCountString[128];
		sprintf(deviceCountString, "%i Device%s Detected", (int)EtherCatFieldbus::getDevices().size(), EtherCatFieldbus::getDevices().size() == 1 ? "" : "s");
		glm::vec4 deviceCountButtonColor;
		if(!EtherCatFieldbus::hasDetectedDevices()) deviceCountButtonColor = Colors::blue;
		else deviceCountButtonColor = Colors::green;
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, deviceCountButtonColor);
		ImGui::Button(deviceCountString);
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();
		
		if(ImGui::IsItemHovered()){
			ImGui::BeginTooltip();
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Detected Devices:");
			ImGui::PopFont();
			for(auto& device : EtherCatFieldbus::getDevices()){
				ImGui::Text("%s", device->getName());
			}
			ImGui::EndTooltip();
		}
		
		ImGui::SameLine();
		
		bool disableScanButton = EtherCatFieldbus::isRunning();
		ImGui::BeginDisabled(disableScanButton);
		if(ImGui::Button("Scan for Devices")) EtherCatFieldbus::scanNetwork();
		ImGui::EndDisabled();
		
		ImGui::Separator();

		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("EtherCAT Timing");
		ImGui::PopFont();
		
		ImGui::SameLine();
		
		if(beginHelpMarker("(help)")){
			ImGui::TextWrapped("These parameters become active the next time the fieldbus is started");
			endHelpMarker();
		}
		
		

		bool intervalEdited = false;
		bool frequencyEdited = false;
		float processInterval_milliseconds = EtherCatFieldbus::processInterval_milliseconds;
		float processFrequency_Hertz = 1000.0 / processInterval_milliseconds;
		float processDataTimeoutDelay_milliseconds = EtherCatFieldbus::processDataTimeout_milliseconds;
		float stableClockThreshold_milliseconds = EtherCatFieldbus::clockStableThreshold_milliseconds;
		float fieldbusTimeoutDelay_milliseconds = EtherCatFieldbus::fieldbusTimeout_milliseconds;

		if(ImGui::BeginTable("##TimingTable", 2, tableFlags)){
			
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Process Interval");
			ImGui::TableNextColumn();
			ImGui::Text("Process Frequency");
			
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(widgetWidth);
			if (ImGui::InputFloat("##Process Interval", &processInterval_milliseconds, 0.1f, 1.0f, "%.1fms")) intervalEdited = true;
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(widgetWidth);
			if (ImGui::InputFloat("##Process Frequency", &processFrequency_Hertz, 0.1f, 1.0f, "%.1fHz")) frequencyEdited = true;
			
			ImGui::EndTable();
		}
		
		
		ImGui::Text("Process Data Timeout Delay");
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::InputFloat("##Process Data Timeout Delay", &processDataTimeoutDelay_milliseconds, 0.1f, 1.0f, "%.1fms");
		
		ImGui::Text("Clock Stabilisation Threshold");
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::InputFloat("##Clock Stabilisation Threshold", &stableClockThreshold_milliseconds, 0.01f, 0.1f, "%.2fms");
		
		ImGui::Text("Fieldbus Timeout Delay");
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::InputFloat("##FieldbusTimeoutDelay", &fieldbusTimeoutDelay_milliseconds, 0.01f, 0.1f, "%.2fms");

		if (frequencyEdited) EtherCatFieldbus::processInterval_milliseconds = 1000.0 / processFrequency_Hertz;
		else EtherCatFieldbus::processInterval_milliseconds = processInterval_milliseconds;

		if (processDataTimeoutDelay_milliseconds > EtherCatFieldbus::processInterval_milliseconds)
			processDataTimeoutDelay_milliseconds = EtherCatFieldbus::processInterval_milliseconds;
		
		if(fieldbusTimeoutDelay_milliseconds < EtherCatFieldbus::processInterval_milliseconds * 2.0) fieldbusTimeoutDelay_milliseconds = EtherCatFieldbus::processInterval_milliseconds * 2.0;
		
		EtherCatFieldbus::processDataTimeout_milliseconds = processDataTimeoutDelay_milliseconds;
		EtherCatFieldbus::clockStableThreshold_milliseconds = stableClockThreshold_milliseconds;
		EtherCatFieldbus::fieldbusTimeout_milliseconds = fieldbusTimeoutDelay_milliseconds;
		
		
		ImGui::EndChild();
	}
	
	
}
