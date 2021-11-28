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
		if (ImGui::BeginTabItem("Stage Editor")) {
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
	std::shared_ptr<Machine> clickedMachine = nullptr;
	
	ImGui::BeginGroup();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Machine Manager");
	ImGui::PopFont();
	
	std::vector<std::shared_ptr<Machine>>& machines = Environnement::getMachines();
	if (machineList.beginList("##MachineList", glm::vec2(ImGui::GetTextLineHeight() * 15.0, ImGui::GetContentRegionAvail().y), 4.0)) {
		for (int i = 0; i < machines.size(); i++) {
			std::shared_ptr<Machine> machine = machines[i];
			if (machineList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()), machine == Environnement::selectedMachine)) {
				
				ImGui::BeginGroup();

				ImGui::PushFont(Fonts::robotoBold15);
				float shortNameWidth = ImGui::CalcTextSize(machine->shortName).x;
				ImGui::PopFont();
				
				float headerStripWidth = shortNameWidth + ImGui::GetStyle().ItemSpacing.x;
				glm::vec2 min = ImGui::GetWindowPos();
				glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
				glm::vec4 headerStripColor;
				
				if(machine->isEnabled()) headerStripColor = Colors::green;
				else if(machine->isReady()) headerStripColor = Colors::yellow;
				else headerStripColor = Colors::blue;
				
				ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);

				ImGui::PushFont(Fonts::robotoBold15);
				ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
				ImGui::Text("%s", machine->shortName);
				ImGui::PopFont();

				ImGui::EndGroup();

				ImGui::SameLine();
				ImGui::Text("%s", machine->getName());
				if (machineList.endItem()) clickedMachine = machine;
			}
		}
		machineList.endList();
	}
	if (machineList.wasReordered()) {
		int oldIndex, newIndex;
		machineList.getReorderedItemIndex(oldIndex, newIndex);
		std::shared_ptr<Machine> tmp = machines[oldIndex];
		machines.erase(machines.begin() + oldIndex);
		machines.insert(machines.begin() + newIndex, tmp);
	}
	if(clickedMachine) Environnement::selectedMachine = clickedMachine;
	
	ImGui::EndGroup();
	
	
	ImGui::SameLine();
	
	ImGui::BeginChild("##selectedmachine", ImGui::GetContentRegionAvail());
		
	if(Environnement::selectedMachine){
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("%s", Environnement::selectedMachine->getName());
		ImGui::PopFont();
		if(ImGui::BeginTabBar("selectedMachineTabBar")){
			Environnement::selectedMachine->nodeSpecificGui();
			ImGui::EndTabBar();
		}
	}else{
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("No Machine Selected.");
		ImGui::PopFont();
	}
	
	ImGui::EndChild();
	
}


void deviceManagerGui() {
	
	static DraggableList etherCatDeviceList;
	std::vector<std::shared_ptr<EtherCatDevice>>& etherCatDevices = Environnement::getEtherCatDevices();
	
	std::shared_ptr<EtherCatDevice> clickedDevice = nullptr;
	
	ImGui::BeginGroup();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("EtherCat Device Manager");
	ImGui::PopFont();
	
	if (etherCatDeviceList.beginList("##EtherCatDeviceList", glm::vec2(ImGui::GetTextLineHeight() * 15.0, ImGui::GetContentRegionAvail().y), 4.0)) {
		for (int i = 0; i < etherCatDevices.size(); i++) {
			std::shared_ptr<EtherCatDevice> etherCatDevice = etherCatDevices[i];
			if (etherCatDeviceList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()))) {
				
				ImGui::BeginGroup();
				
				float headerStripWidth = ImGui::GetTextLineHeight() * 2.0;
				glm::vec2 min = ImGui::GetWindowPos();
				glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
				glm::vec4 headerStripColor;
				
				if(etherCatDevice->isReady()) headerStripColor = Colors::green;
				else if(etherCatDevice->isOnline()) headerStripColor = Colors::yellow;
				else if(etherCatDevice->isDetected()) headerStripColor = Colors::orange;
				else headerStripColor = Colors::blue;
				
				ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);

				ImGui::PushFont(Fonts::robotoBold15);
				ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
				int slaveIndex = etherCatDevice->getSlaveIndex();
				if(!etherCatDevice->isDetected()){
					ImGui::PushStyleColor(ImGuiCol_Text, Colors::darkGray);
					ImGui::Text("--");
					ImGui::PopStyleColor();
				}
				else ImGui::Text("#%i", etherCatDevice->getSlaveIndex());
				ImGui::PopFont();

				ImGui::EndGroup();

				ImGui::SameLine();
				ImGui::Text("%s", etherCatDevice->getName());
				
				
				if(etherCatDeviceList.endItem()) clickedDevice = etherCatDevice;
			}
		}
		etherCatDeviceList.endList();
	}
	if(clickedDevice) Environnement::selectedEtherCatDevice = clickedDevice;
	
	ImGui::EndGroup();
	
	ImGui::SameLine();
	
	ImGui::BeginChild("##selectedDevice", ImGui::GetContentRegionAvail());
		
	if(Environnement::selectedEtherCatDevice){
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("%s", Environnement::selectedEtherCatDevice->getName());
		ImGui::PopFont();
		if(ImGui::BeginTabBar("selectedDeviceTabBar")){
			Environnement::selectedEtherCatDevice->nodeSpecificGui();
			ImGui::EndTabBar();
		}
	}else{
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("No Device Selected.");
		ImGui::PopFont();
	}
	
	ImGui::EndChild();
	
}

