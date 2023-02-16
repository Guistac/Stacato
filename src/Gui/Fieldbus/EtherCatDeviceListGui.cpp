#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Fieldbus/EtherCatGui.h"
#include "Environnement/Environnement.h"
#include "Gui/Utilities/CustomWidgets.h"

void etherCatSlaves() {

	ImGui::BeginGroup();

	ImGui::BeginDisabled(!EtherCatFieldbus::canScan());
	if (ImGui::Button("Scan Network")) EtherCatFieldbus::scan();
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


void EtherCatDevicesWindow::onDraw(){
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	glm::vec2 deviceSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
	float rounding = ImGui::GetStyle().FrameRounding;
	
	for(auto etherCatDevice : Environnement::getEtherCatDevices()){
		
		glm::vec2 cursor = ImGui::GetCursorPos();
		ImGui::InvisibleButton("##ethercatDevice", deviceSize);
		glm::vec2 endCursor = ImGui::GetCursorPos();
		bool b_hovered = ImGui::IsItemHovered();
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		ImColor backgroundColor = ImColor(Colors::almostBlack);
		drawing->AddRectFilled(min, max, backgroundColor, rounding, ImDrawFlags_RoundCornersAll);
		
		//glm::vec2 textPos(cursor.x + ImGui::GetStyle().FramePadding.x, cursor.y + (deviceSize.y - ImGui::GetTextLineHeight()) * .5f);
		ImGui::SetCursorPos(cursor);
		ImGui::PushFont(Fonts::sansBold15);
		etherCatDevice->deviceIdVignette(deviceSize.y);
		ImGui::PopFont();
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
		ImGui::SetCursorPosY(cursor.y + (deviceSize.y - ImGui::GetTextLineHeight()) * .5f);
		ImGui::Text("%s", etherCatDevice->getName());
		ImGui::SetCursorPos(endCursor);
		
		if(b_hovered){
			ImGui::BeginTooltip();
			ImGui::PushFont(Fonts::sansBold15);
			
			if(etherCatDevice->isConnected()){
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::green);
				ImGui::Text("Device Online");
			}else if(etherCatDevice->isDetected()){
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
				ImGui::Text("Device Detected");
			}else{
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::blue);
				ImGui::Text("Device Offline");
			}
			ImGui::PopStyleColor();
			
			ImGui::PopFont();
			ImGui::EndTooltip();
		}
	}
}
