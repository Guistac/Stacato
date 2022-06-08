#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void etherCatSlaves() {

	ImGui::BeginGroup();

	bool disableScanButton = !EtherCatFieldbus::canScan();
	ImGui::BeginDisabled(disableScanButton);
	if (ImGui::Button("Scan Network")) EtherCatFieldbus::scanNetwork();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::Text("%i Devices Found", (int)EtherCatFieldbus::getDevices().size());

	static int selectedSlaveIndex = -1;

	ImVec2 listWidth(ImGui::GetTextLineHeight() * 14, ImGui::GetContentRegionAvail().y);
	if (ImGui::BeginListBox("##DiscoveredEtherCATSlaves", listWidth)) {
		for (auto slave : EtherCatFieldbus::getDevices()) {
			bool selected = selectedSlaveIndex == slave->getSlaveIndex();
			if (ImGui::Selectable(slave->getName(), &selected)) selectedSlaveIndex = slave->getSlaveIndex();
		}
		if (EtherCatFieldbus::getDevices().empty()) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::Selectable("No Devices Detected...");
			ImGui::PopItemFlag();
		}
		ImGui::EndListBox();
	}
	ImGui::EndGroup();
	ImGui::SameLine();

    std::shared_ptr<EtherCatDevice> selectedSlave = nullptr;
	for (auto slave : EtherCatFieldbus::getDevices()) 
		if (slave->getSlaveIndex() == selectedSlaveIndex) { selectedSlave = slave; break; }

	ImGui::BeginGroup();
	ImGui::PushFont(Fonts::sansBold20);
	if (selectedSlave) ImGui::Text("%s (Node #%i) ", selectedSlave->getSaveName(), selectedSlave->getSlaveIndex());
	else ImGui::Text("No Device Selected");
	ImGui::PopFont();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::BeginChild(ImGui::GetID("SelectedSlaveDisplayWindow"))) {
		if (selectedSlave) {
            if (ImGui::BeginTabBar("DevicePropertiesTabBar")) {
                selectedSlave->nodeSpecificGui();
                ImGui::EndTabBar();
            }
		}
		ImGui::EndChild();
	}
	ImGui::PopStyleVar();
	ImGui::EndGroup();

}
