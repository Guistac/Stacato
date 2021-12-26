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
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("EtherCAT Network Hardware");
		ImGui::PopFont();
		
		ImGui::SameLine();
		if(beginHelpMarker("(help)")){
			ImGui::TextWrapped("Selection of the actual physical ethernet port for EtherCAT Communications."
							   "\nFor a redundant ring network topology, a second network interface card can be selected. This will make the system immune to a cable break in the network loop."
							   "\nAuto setup will select the first network interface card to which an EtherCAT device is connnected.");
			endHelpMarker();
		}

		std::shared_ptr<NetworkInterfaceCard> selectedPrimaryNic = EtherCatFieldbus::primaryNetworkInterfaceCard;
		std::shared_ptr<NetworkInterfaceCard> selectedSecondaryNic = EtherCatFieldbus::redundantNetworkInterfaceCard;
		bool b_nicSelected = false;
		
		ImGuiTableFlags tableFlags = ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
		if(ImGui::BeginTable("##nicTable", 2, tableFlags)){
			
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Primary Network Interface Card");
			ImGui::TableNextColumn();
			ImGui::Text("Redundant Network Interface Card (optional)");

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			
			ImGui::SetNextItemWidth(widgetWidth);
			const char* primaryLabel = EtherCatFieldbus::primaryNetworkInterfaceCard == nullptr ? "None" : EtherCatFieldbus::primaryNetworkInterfaceCard->description;
			if (ImGui::BeginCombo("##NetworkInterfaceCard", primaryLabel)) {
				if (ImGui::Selectable("None", EtherCatFieldbus::primaryNetworkInterfaceCard == nullptr)) {
					selectedPrimaryNic = nullptr;
					selectedSecondaryNic = nullptr;
					b_nicSelected = true;
				}
				for (auto& nic : EtherCatFieldbus::networkInterfaceCards) {
					bool selected = nic == EtherCatFieldbus::primaryNetworkInterfaceCard;
					if (ImGui::Selectable(nic->description, selected)) {
						selectedPrimaryNic = nic;
						b_nicSelected = true;
					}
				}
				ImGui::EndCombo();
			}
			
			ImGui::TableNextColumn();
			
			bool disableSecondaryNicSelection = EtherCatFieldbus::primaryNetworkInterfaceCard == nullptr;
			if(disableSecondaryNicSelection) BEGIN_DISABLE_IMGUI_ELEMENT
			
			ImGui::SetNextItemWidth(widgetWidth);
			const char* secondaryLabel = EtherCatFieldbus::redundantNetworkInterfaceCard == nullptr ? "None" : EtherCatFieldbus::redundantNetworkInterfaceCard->description;
			if (ImGui::BeginCombo("##RedundantNetworkInterfaceCard", secondaryLabel)) {
				if (ImGui::Selectable("None", EtherCatFieldbus::redundantNetworkInterfaceCard == nullptr)) {
					selectedSecondaryNic = nullptr;
					b_nicSelected = true;
				}
				for (auto& nic : EtherCatFieldbus::networkInterfaceCards) {
					bool disableSelection = nic == EtherCatFieldbus::primaryNetworkInterfaceCard;
					if (disableSelection) BEGIN_DISABLE_IMGUI_ELEMENT
					if (ImGui::Selectable(nic->description, EtherCatFieldbus::redundantNetworkInterfaceCard == nic)) {
						selectedSecondaryNic = nic;
						b_nicSelected = true;
					}
					if (disableSelection) END_DISABLE_IMGUI_ELEMENT

				}
				ImGui::EndCombo();
			}
			 
			if(disableSecondaryNicSelection) END_DISABLE_IMGUI_ELEMENT
			
			
			ImGui::EndTable();
		}
		
		
		if(b_nicSelected){
			if (selectedSecondaryNic == nullptr && selectedPrimaryNic == nullptr) EtherCatFieldbus::terminate();
			else if(selectedSecondaryNic == nullptr && selectedPrimaryNic != nullptr) EtherCatFieldbus::init(selectedPrimaryNic);
			else if (selectedPrimaryNic != nullptr) EtherCatFieldbus::init(selectedPrimaryNic, selectedSecondaryNic);
		}
		
		
		static char networkStatusString[256];
		if (EtherCatFieldbus::isNetworkInitialized()) {
			if (!EtherCatFieldbus::isNetworkRedundant()) sprintf(networkStatusString, "Network is Open on Interface '%s'", EtherCatFieldbus::primaryNetworkInterfaceCard->description);
			else sprintf(networkStatusString,"Network is Open on Interface '%s' with redundancy on '%s'", EtherCatFieldbus::primaryNetworkInterfaceCard->description, EtherCatFieldbus::redundantNetworkInterfaceCard->description);
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
		if (ImGui::Button("Refresh Device List")) EtherCatFieldbus::updateNetworkInterfaceCardList();
		ImGui::SameLine();
		if(ImGui::Button("Auto Setup")) EtherCatFieldbus::autoInit();

		ImGui::Separator();

		ImGui::PushFont(Fonts::robotoBold20);
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

		if (frequencyEdited) EtherCatFieldbus::processInterval_milliseconds = 1000.0 / processFrequency_Hertz;
		else EtherCatFieldbus::processInterval_milliseconds = processInterval_milliseconds;

		if (processDataTimeoutDelay_milliseconds > EtherCatFieldbus::processInterval_milliseconds)
			processDataTimeoutDelay_milliseconds = EtherCatFieldbus::processInterval_milliseconds;

		EtherCatFieldbus::processDataTimeout_milliseconds = processDataTimeoutDelay_milliseconds;
		EtherCatFieldbus::clockStableThreshold_milliseconds = stableClockThreshold_milliseconds;
		
		
		ImGui::EndChild();
	}
	
	
}
