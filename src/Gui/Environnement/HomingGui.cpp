#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"

#include "EnvironnementGui.h"

namespace Environnement::Gui{

void SetupWindow::drawContent(){

	/*
	
	//=================== NETWORK SETUP =====================
	
	glm::vec2 environnementBoxSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 4.2);
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::veryDarkGray);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0);
	ImGui::BeginChild("NetworkSetup", environnementBoxSize, true);
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("EtherCAT Network");
	ImGui::PopFont();
	
	glm::vec2 initButtonSize(ImGui::GetTextLineHeight() * 5.0, ImGui::GetFrameHeight());
	bool disableSetupButton = EtherCatFieldbus::isRunning() | EtherCatFieldbus::isAutoInitRunning();
	ImGui::BeginDisabled(disableSetupButton);
	if(EtherCatFieldbus::isAutoInitRunning()) {
		ImGui::Button("Initializing...", initButtonSize);
	} else {
		if(ImGui::Button("Auto Setup", initButtonSize)) EtherCatFieldbus::autoInit();
	}
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	
	static char networkStatusString[256];
	if (EtherCatFieldbus::hasNetworkInterface()) {
		if (!EtherCatFieldbus::hasRedundantInterface())
			sprintf(networkStatusString,
					"Network is Open on Interface '%s'",
					EtherCatFieldbus::getActiveNetworkInterfaceCard()->description);
		else
			sprintf(networkStatusString,
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
	
	static char deviceCountString[128];
	sprintf(deviceCountString, "%i Device%s Detected", (int)EtherCatFieldbus::getDevices().size(), EtherCatFieldbus::getDevices().size() == 1 ? "" : "s");
	glm::vec4 deviceCountButtonColor;
	if(EtherCatFieldbus::getDevices().empty()) deviceCountButtonColor = Colors::blue;
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
		
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	 
	 */
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	float boxWidth = ImGui::GetContentRegionAvail().x;
	glm::vec2 boxPadding(ImGui::GetTextLineHeight() * .4f, ImGui::GetTextLineHeight() * .2f);
	
	int id = 0;
	for(auto machine : Environnement::getMachines()){
				
		if(!machine->hasSetupGui()) continue;
		
		ImGui::PushID(id++);
		
		ImDrawListSplitter layers;
		layers.Split(drawing, 2);
		layers.SetCurrentChannel(drawing, 1);
		
		ImGui::BeginGroup();
		glm::vec2 cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos(cursor + boxPadding);
		ImGui::BeginGroup();
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("%s", machine->getName());
		ImGui::PopFont();
		machine->setupGui();
		
		ImGui::EndGroup();
		ImGui::SetCursorPosX(cursor.x + ImGui::GetItemRectSize().x + boxPadding.x * 2.0);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY());
		ImGui::EndGroup();
		
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		
		layers.SetCurrentChannel(drawing, 0);
		drawing->AddRectFilled(ImGui::GetItemRectMin(),
							   ImGui::GetItemRectMax(),
							   ImColor(Colors::veryDarkGray),
							   ImGui::GetStyle().FrameRounding,
							   ImDrawFlags_RoundCornersAll);
		drawing->AddRect(ImGui::GetItemRectMin(),
						 ImGui::GetItemRectMax(),
						 ImColor(Colors::darkGray),
						 ImGui::GetStyle().FrameRounding,
						 ImDrawFlags_RoundCornersAll,
						 ImGui::GetTextLineHeight() * .05f);
		layers.Merge(drawing);
		
		ImGui::PopID();
	}
}

}
