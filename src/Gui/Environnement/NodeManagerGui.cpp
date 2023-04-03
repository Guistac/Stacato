#include <pch.h>

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/ReorderableList.h"

#include "EnvironnementGui.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"
#include "Networking/NetworkDevice.h"

#include "Fieldbus/EtherCatDevice.h"

#include "Environnement/NodeGraph/NodeGraph.h"
#include "Gui/Environnement/NodeGraph/NodeGraphGui.h"

#include "Gui/Utilities/CustomWidgets.h"

namespace Environnement::Gui{

void NodeManagerWindow::onDraw(){

	static float listWidth = ImGui::GetTextLineHeight() * 20.0;
	static float minlistWidth = ImGui::GetTextLineHeight() * 15.0;
	
	ImGui::BeginChild("NodeSelectionList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y));
	if(ImGui::BeginTabBar("NodeTypeSelector")){
		
		/*
		if(ImGui::BeginTabItem("Machines")){
			
			
			std::shared_ptr<Machine> clickedMachine = nullptr;

			std::vector<std::shared_ptr<Machine>>& machines = Environnement::getMachines();
			if (ReorderableList::begin("MachineList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y))) {
				for (int i = 0; i < machines.size(); i++) {
					std::shared_ptr<Machine> machine = machines[i];
					
					if(ReorderableList::beginItem(ImGui::GetTextLineHeightWithSpacing())){
						
						if(ImGui::IsItemClicked()) clickedMachine = machine;
						
						ImGui::PushFont(Fonts::sansBold15);
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

						ImGui::PushFont(Fonts::sansBold15);
						ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
						ImGui::Text("%s", machine->shortName);
						ImGui::PopFont();

						ImGui::SameLine();
						ImGui::Text("%s", machine->getName());
						
						ReorderableList::endItem();
					}
				}
				ReorderableList::end();
			}
			int fromIndex, toIndex;
			if (ReorderableList::wasReordered(fromIndex, toIndex)) {
				std::shared_ptr<Machine> tmp = machines[fromIndex];
				machines.erase(machines.begin() + fromIndex);
				machines.insert(machines.begin() + toIndex, tmp);
			}
			if(clickedMachine) Environnement::NodeGraph::Gui::selectNode(clickedMachine);
			
			ImGui::EndTabItem();
		}
		
		*/
		
		if(ImGui::BeginTabItem("EtherCAT Devices")){
			
			
			std::vector<std::shared_ptr<EtherCatDevice>>& etherCatDevices = Environnement::getEtherCatDevices();
			std::shared_ptr<EtherCatDevice> clickedDevice = nullptr;
			
			if (ReorderableList::begin("EtherCatDeviceList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y))) {
				for (int i = 0; i < etherCatDevices.size(); i++) {
					std::shared_ptr<EtherCatDevice> etherCatDevice = etherCatDevices[i];
					if (ReorderableList::beginItem(ImGui::GetTextLineHeightWithSpacing())){
						
						if(ImGui::IsItemClicked()) clickedDevice = etherCatDevice;
						
						ImGui::PushFont(Fonts::sansBold15);
						static char numberString[16];
						if(!etherCatDevice->isDetected()) sprintf(numberString, "#-");
						else sprintf(numberString, "#%i", etherCatDevice->getSlaveIndex());
						
						float headerStripWidth = ImGui::CalcTextSize(numberString).x + ImGui::GetStyle().ItemSpacing.x;
						glm::vec2 min = ImGui::GetWindowPos();
						glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
						glm::vec4 headerStripColor;
						
						if(etherCatDevice->isConnected()) headerStripColor = Colors::green;
						else if(etherCatDevice->isDetected()) headerStripColor = Colors::yellow;
						else headerStripColor = Colors::blue;
						
						ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);
					
						ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
						int slaveIndex = etherCatDevice->getSlaveIndex();
						if(!etherCatDevice->isDetected()) ImGui::PushStyleColor(ImGuiCol_Text, Colors::darkGray);
						else ImGui::PushStyleColor(ImGuiCol_Text, Colors::white);
						ImGui::Text("%s", numberString);
						ImGui::PopStyleColor();
						ImGui::PopFont();

						ImGui::SameLine();
						ImGui::Text("%s", etherCatDevice->getName().c_str());
						
						
						ReorderableList::endItem();
					}
				}
				ReorderableList::end();
			}
			if(clickedDevice) Environnement::NodeGraph::Gui::selectNode(clickedDevice);
			
			ImGui::EndTabItem();
		}
		
		if(ImGui::BeginTabItem("Network Devices")){
			
			
			std::vector<std::shared_ptr<NetworkDevice>>& networkDevices = Environnement::getNetworkDevices();
			std::shared_ptr<Node> clickedDevice = nullptr;
			
			if (ReorderableList::begin("NetworkDeviceList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y))) {
				for (int i = 0; i < networkDevices.size(); i++) {
					std::shared_ptr<NetworkDevice> networkDevice = networkDevices[i];
					if (ReorderableList::beginItem(ImGui::GetTextLineHeightWithSpacing())) {
						
						if(ImGui::IsItemClicked()) clickedDevice = networkDevice;
						
						ImGui::PushFont(Fonts::sansBold15);
						
						float headerStripWidth = ImGui::GetFrameHeight() / 2.0;
						glm::vec2 min = ImGui::GetWindowPos();
						glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
						glm::vec4 headerStripColor;
						
						if(networkDevice->isConnected()) headerStripColor = Colors::green;
						else if(networkDevice->isDetected()) headerStripColor = Colors::yellow;
						else headerStripColor = Colors::blue;
						
						ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);
	

						ImGui::SameLine();
						ImGui::Text("%s", networkDevice->getName().c_str());
						
						
						ReorderableList::endItem();
					}
				}
				ReorderableList::end();
			}
			if(clickedDevice) Environnement::NodeGraph::Gui::selectNode(clickedDevice);
			
			ImGui::EndTabItem();
		}
		
		
		
		ImGui::EndTabBar();
	}
	ImGui::EndChild();
	
	
	
	ImGui::SameLine();
	listWidth += verticalSeparator(ImGui::GetTextLineHeight() * 0.5);
	if (listWidth < minlistWidth) listWidth = minlistWidth;
	
	
	
	ImGui::BeginChild("##SelectedNode", ImGui::GetContentRegionAvail());
		
	auto& selectedNodes = Environnement::NodeGraph::getSelectedNodes();
	
	if(ImGui::BeginTabBar("SelectedNodes")){
		if(selectedNodes.size() > 0){
			for(auto& selectedNode : selectedNodes){
				ImGui::PushID(selectedNode->getUniqueID());
				
				if(ImGui::BeginTabItem(selectedNode->getName().c_str())){
					if(ImGui::BeginTabBar(selectedNode->getName().c_str())){
						selectedNode->nodeSpecificGui();
						ImGui::EndTabBar();
					}
					ImGui::EndTabItem();
				}
				
				ImGui::PopID();
			}
		}else{
			if(ImGui::BeginTabItem("Node Inspector")){
				ImGui::BeginDisabled();
				ImGui::Text("No Selection");
				ImGui::EndDisabled();
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	 
	ImGui::EndChild();
	
}


}
