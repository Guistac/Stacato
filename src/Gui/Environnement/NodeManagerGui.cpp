#include <pch.h>

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/DraggableList.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"
#include "Networking/NetworkDevice.h"

#include "Fieldbus/EtherCatDevice.h"

#include "Environnement/NodeGraph/NodeGraph.h"
#include "Gui/Environnement/NodeGraph/NodeGraphGui.h"

namespace Environnement::Gui{

void nodeManager(){
	static float listWidth = ImGui::GetTextLineHeight() * 20.0;
	static float minlistWidth = ImGui::GetTextLineHeight() * 15.0;
	
	ImGui::BeginChild("NodeSelectionList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y));
	if(ImGui::BeginTabBar("NodeTypeSelector")){
		
		if(ImGui::BeginTabItem("Machines")){
			
			static DraggableList machineList;
			std::shared_ptr<Machine> clickedMachine = nullptr;

			std::vector<std::shared_ptr<Machine>>& machines = Environnement::getMachines();
			if (machineList.beginList("##MachineList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y), 4.0)) {
				for (int i = 0; i < machines.size(); i++) {
					std::shared_ptr<Machine> machine = machines[i];
					if (machineList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()),
											  Environnement::NodeGraph::Gui::isNodeSelected(machine))) {

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
			if(clickedMachine) Environnement::NodeGraph::Gui::selectNode(clickedMachine);
			
			ImGui::EndTabItem();
		}
		
		
		
		if(ImGui::BeginTabItem("EtherCAT Devices")){
			
			static DraggableList etherCatDeviceList;
			std::vector<std::shared_ptr<EtherCatDevice>>& etherCatDevices = Environnement::getEtherCatDevices();
			std::shared_ptr<EtherCatDevice> clickedDevice = nullptr;
			
			if (etherCatDeviceList.beginList("##EtherCatDeviceList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y), 4.0)) {
				for (int i = 0; i < etherCatDevices.size(); i++) {
					std::shared_ptr<EtherCatDevice> etherCatDevice = etherCatDevices[i];
					if (etherCatDeviceList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()),
													 Environnement::NodeGraph::Gui::isNodeSelected(etherCatDevice))) {
						
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
						ImGui::Text("%s", etherCatDevice->getName());
						
						
						if(etherCatDeviceList.endItem()) clickedDevice = etherCatDevice;
					}
				}
				etherCatDeviceList.endList();
			}
			if(clickedDevice) Environnement::NodeGraph::Gui::selectNode(clickedDevice);
			
			ImGui::EndTabItem();
		}
		
		if(ImGui::BeginTabItem("Network Devices")){
			
			static DraggableList networkDeviceList;
			std::vector<std::shared_ptr<NetworkDevice>>& networkDevices = Environnement::getNetworkDevices();
			std::shared_ptr<Node> clickedDevice = nullptr;
			
			if (networkDeviceList.beginList("##NetworkDeviceList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y), 4.0)) {
				for (int i = 0; i < networkDevices.size(); i++) {
					std::shared_ptr<NetworkDevice> networkDevice = networkDevices[i];
					if (networkDeviceList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()),
													 Environnement::NodeGraph::Gui::isNodeSelected(networkDevice))) {
						
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
						ImGui::Text("%s", networkDevice->getName());
						
						
						if(networkDeviceList.endItem()) clickedDevice = networkDevice;
					}
				}
				networkDeviceList.endList();
			}
			if(clickedDevice) Environnement::NodeGraph::Gui::selectNode(clickedDevice);
			
			ImGui::EndTabItem();
		}
		
		
		
		ImGui::EndTabBar();
	}
	ImGui::EndChild();
	
	
	static float splitterWidth = ImGui::GetTextLineHeight() * 0.5;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::SameLine();
	ImGui::InvisibleButton("VerticalSplitter", glm::vec2(splitterWidth, ImGui::GetContentRegionAvail().y));
	if (ImGui::IsItemActive()) listWidth += ImGui::GetIO().MouseDelta.x;
	if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
	if (listWidth < minlistWidth) listWidth = minlistWidth;
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	float middle = (min.x + max.x) / 2.0;
	ImGui::GetWindowDrawList()->AddLine(glm::vec2(middle, min.y), glm::vec2(middle, max.y), ImColor(Colors::darkGray), ImGui::GetTextLineHeight() * 0.1);
	ImGui::SameLine();
	ImGui::PopStyleVar();
	ImGui::SameLine();
	
	
	
	
	
	
	
	
	
	
	ImGui::BeginChild("##SelectedNode", ImGui::GetContentRegionAvail());
		
	auto& selectedNodes = Environnement::NodeGraph::getSelectedNodes();
	
	if(ImGui::BeginTabBar("SelectedNodes")){
		if(selectedNodes.size() > 0){
			for(auto& selectedNode : selectedNodes){
				ImGui::PushID(selectedNode->getUniqueID());
				
				if(ImGui::BeginTabItem(selectedNode->getName())){
					if(ImGui::BeginTabBar(selectedNode->getName())){
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
