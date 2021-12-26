#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"

void etherCatParameters(bool resetNicLists) {

	if (ImGui::BeginChild("EtherCatParameters")) {

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Network Hardware");
		ImGui::PopFont();

		std::vector<NetworkInterfaceCard>& nics = EtherCatFieldbus::networkInterfaceCards;
		static int primarySelectedNic = -1;
		static int secondarySelectedNic = -1;

		//this triggers when the tab is opened and resets the nic selections to the actual values
		if (resetNicLists) {
			primarySelectedNic = -1;
			secondarySelectedNic = -1;
			for (int i = 0; i < nics.size(); i++) {
				if (strcmp(EtherCatFieldbus::networkInterfaceCard.description, nics[i].description) == 0) primarySelectedNic = i;
				if (strcmp(EtherCatFieldbus::redundantNetworkInterfaceCard.description, nics[i].description) == 0) secondarySelectedNic = i;
			}
		}

		ImGui::Text("Primary Network Interface Card");
		const char* primaryLabel = (primarySelectedNic == -1 || primarySelectedNic >= nics.size()) ? "Select NIC" : nics[primarySelectedNic].description;
		if (ImGui::BeginCombo("##NetworkInterfaceCard", primaryLabel)) {
			for (int i = 0; i < nics.size(); i++) {
				bool selected = i == primarySelectedNic;
				if (ImGui::Selectable(nics[i].description, selected)) {
					primarySelectedNic = i;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Text("Redundant Network Interface Card (optional)");
		const char* secondaryLabel = secondarySelectedNic == -1 ? "None" : (secondarySelectedNic >= nics.size() ? "Select NIC" : nics[secondarySelectedNic].description);
		if (ImGui::BeginCombo("##RedundantNetworkInterfaceCard", secondaryLabel)) {
			bool selected = secondarySelectedNic == -1;
			if (ImGui::Selectable("None", selected)) secondarySelectedNic = -1;
			for (int i = 0; i < nics.size(); i++) {
				selected = i == secondarySelectedNic;
				bool disableSelection = false;
				if (i == primarySelectedNic) disableSelection = true;
				if (disableSelection) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::Selectable(nics[i].description, selected)) {
					secondarySelectedNic = i;
				}
				if (disableSelection) END_DISABLE_IMGUI_ELEMENT

			}
			ImGui::EndCombo();
		}

		if (EtherCatFieldbus::isNetworkInitialized()) {
			if (!EtherCatFieldbus::b_redundant) ImGui::Text("Network is Open on Interface '%s'", EtherCatFieldbus::networkInterfaceCard.description);
			else ImGui::Text("Network is Open on Interface '%s' with redundancy on '%s'", EtherCatFieldbus::networkInterfaceCard.description, EtherCatFieldbus::redundantNetworkInterfaceCard.description);
		}
		else ImGui::Text("Network is Closed");

		if (ImGui::Button("Refresh Device List")) EtherCatFieldbus::updateNetworkInterfaceCardList();
		ImGui::SameLine();
		if (ImGui::Button("Open Network")) {
			if (secondarySelectedNic == -1) {
				if (primarySelectedNic != -1 && primarySelectedNic < nics.size()) {
					if (EtherCatFieldbus::init(nics[primarySelectedNic])) {
					}
				}
			}
			else {
				if (primarySelectedNic != -1 && primarySelectedNic < nics.size() && secondarySelectedNic != primarySelectedNic && secondarySelectedNic < nics.size()) {
					EtherCatFieldbus::init(nics[primarySelectedNic], nics[secondarySelectedNic]);
				}
			}
		}
		ImGui::SameLine();
		if(ImGui::Button("Auto Setup")) EtherCatFieldbus::autoInit();

		ImGui::Separator();

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("EtherCAT Timing");
		ImGui::PopFont();

		bool intervalEdited = false;
		bool frequencyEdited = false;
		float processInterval_milliseconds = EtherCatFieldbus::processInterval_milliseconds;
		float processFrequency_Hertz = 1000.0 / processInterval_milliseconds;
		float processDataTimeoutDelay_milliseconds = EtherCatFieldbus::processDataTimeout_milliseconds;
		float stableClockThreshold_milliseconds = EtherCatFieldbus::clockStableThreshold_milliseconds;

		ImGui::Text("Process Interval");
		if (ImGui::InputFloat("##Process Interval", &processInterval_milliseconds, 0.1f, 1.0f, "%.1fms")) intervalEdited = true;
		ImGui::Text("Process Frequency");
		if (ImGui::InputFloat("##Process Frequency", &processFrequency_Hertz, 0.1f, 1.0f, "%.1fHz")) frequencyEdited = true;
		ImGui::Text("Process Data Timeout Delay");
		ImGui::InputFloat("##Process Data Timeout Delay", &processDataTimeoutDelay_milliseconds, 0.1f, 1.0f, "%.1fms");
		ImGui::Text("Clock Stabilisation Threshold");
		ImGui::InputFloat("##Clock Stabilisation Threshold", &stableClockThreshold_milliseconds, 0.01f, 0.1f, "%.2fms");

		if (frequencyEdited) EtherCatFieldbus::processInterval_milliseconds = 1000.0 / processFrequency_Hertz;
		else EtherCatFieldbus::processInterval_milliseconds = processInterval_milliseconds;

		if (processDataTimeoutDelay_milliseconds > EtherCatFieldbus::processInterval_milliseconds)
			processDataTimeoutDelay_milliseconds = EtherCatFieldbus::processInterval_milliseconds;

		EtherCatFieldbus::processDataTimeout_milliseconds = processDataTimeoutDelay_milliseconds;
		EtherCatFieldbus::clockStableThreshold_milliseconds = stableClockThreshold_milliseconds;

		ImGui::Text("These parameters become active the next time the fieldbus is started");
		
		
		ImGui::EndChild();
	}
	
	
}
