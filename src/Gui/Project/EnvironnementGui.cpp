#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "ProjectGui.h"

#include "Project/Environnement.h"
#include "Motion/Machine/Machine.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Gui/Utilities/DraggableList.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Gui/Fieldbus/EtherCatGui.h"

#include "Gui/Utilities/Log.h"


void environnementGui() {
	if (ImGui::BeginTabBar("##EnvironnementTabBar")) {
		if (ImGui::BeginTabItem("Editor")) {
			EnvironnementNodeEditorGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Machine Manager")) {
			machineManagerGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Device Manager")) {
			deviceManagerGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Stage View Editor")) {
			ImGui::Text("3D Scene Graph / Geometry importer / Machine movement assignement");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Fieldbus")) {
			etherCatGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Log")) {
			log();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}


void machineManagerGui(){
	static DraggableList machineList;
	if (machineList.beginList("##MachineList", glm::vec2(ImGui::GetTextLineHeight() * 15.0, ImGui::GetContentRegionAvail().y), 4.0)) {
		std::vector<std::shared_ptr<Machine>>& machines = Environnement::getMachines();
		for (int i = 0; i < machines.size(); i++) {
			std::shared_ptr<Machine> machine = machines[i];
			if (machineList.beginItem(glm::vec2(ImGui::GetTextLineHeight() * 14.0, 100), i == 1)) {
				ImGui::PushFont(Fonts::robotoBold20);
				ImGui::Text("%i", i + 1);
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushFont(Fonts::robotoRegular20);
				ImGui::Text(machine->getName());
				ImGui::PopFont();
				if (machineList.endItem()) Logger::warn("Item {} Clicked", i + 1);
			}
		}
		machineList.endList();
		if (machineList.wasReordered()) {
			int oldIndex, newIndex;
			machineList.getReorderedItemIndex(oldIndex, newIndex);
			std::shared_ptr<Machine> tmp = machines[oldIndex];
			machines.erase(machines.begin() + oldIndex);
			machines.insert(machines.begin() + newIndex, tmp);
		}
	}
}


void deviceManagerGui() {
	static DraggableList etherCatDeviceList;
	if (etherCatDeviceList.beginList("##EtherCatDeviceList", glm::vec2(ImGui::GetTextLineHeight() * 15.0, ImGui::GetContentRegionAvail().y), 4.0)) {
		std::vector<std::shared_ptr<EtherCatDevice>>& etherCatDevices = Environnement::getEtherCatDevices();
		for (int i = 0; i < etherCatDevices.size(); i++) {
			std::shared_ptr<EtherCatDevice> etherCatDevice = etherCatDevices[i];
			if (etherCatDeviceList.beginItem(glm::vec2(ImGui::GetTextLineHeight() * 14.0, 100))) {
				ImGui::PushFont(Fonts::robotoBold20);
				ImGui::Text("%i", i + 1);
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushFont(Fonts::robotoRegular20);
				ImGui::Text(etherCatDevice->getName());
				ImGui::PopFont();
				etherCatDeviceList.endItem();
			}
		}
		etherCatDeviceList.endList();
		if (etherCatDeviceList.wasReordered()) {
			int oldIndex, newIndex;
			etherCatDeviceList.getReorderedItemIndex(oldIndex, newIndex);
			std::shared_ptr<EtherCatDevice> tmp = etherCatDevices[oldIndex];
			etherCatDevices.erase(etherCatDevices.begin() + oldIndex);
			etherCatDevices.insert(etherCatDevices.begin() + newIndex, tmp);
		}
	}
}

