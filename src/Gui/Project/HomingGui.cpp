#include <pch.h>

#include "ProjectGui.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Project/Environnement.h"
#include "Motion/Machine/Machine.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"

void setupGui(){
	
	glm::vec2 machineHomingListSize(ImGui::GetTextLineHeight() * 30.0, ImGui::GetContentRegionAvail().y);
	
	ImGui::BeginChild("Homing", machineHomingListSize, false);
	
	glm::vec2 machineHomingBoxSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 4.2);
	glm::vec2 homingControlButtonSize(ImGui::GetTextLineHeight() * 5.0, ImGui::GetFrameHeight());
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::veryDarkGray);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0);
	for(auto machine : Environnement::getMachines()){
		if(!machine->isHomeable()) continue;
	
		ImGui::BeginChild(machine->getName(), machineHomingBoxSize, true);
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("%s", machine->getName());
		ImGui::PopFont();
		
		bool disableHomingButton = !machine->isEnabled();
		if(disableHomingButton) BEGIN_DISABLE_IMGUI_ELEMENT
		if(machine->isHoming()){
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
			if(ImGui::Button("Stop Homing", homingControlButtonSize)) machine->stopHoming();
			ImGui::PopStyleColor();
		}else{
			if(ImGui::Button("Start Homing", homingControlButtonSize)) machine->startHoming();
		}
		if(disableHomingButton) END_DISABLE_IMGUI_ELEMENT
		
		glm::vec4 progressIndicatorColor = Colors::darkGray;
		if(machine->isHoming()) progressIndicatorColor = Colors::orange;
		else if(machine->didHomingSucceed()) progressIndicatorColor = Colors::green;
		else if(machine->didHomingFail()) progressIndicatorColor = Colors::red;
		
		
		ImGui::SameLine();
		
		glm::vec2 homingProgressIndicatorSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
		
		BackgroundText::draw(machine->getHomingStateString(),
							 homingProgressIndicatorSize,
							 Colors::darkGray);
		
		if(machine->isHoming()){
			glm::vec2 min = ImGui::GetItemRectMin();
			float width = ImGui::GetItemRectSize().x;
			float height = ImGui::GetItemRectSize().y;
			double homingProgress = machine->getHomingProgress();
			homingProgress = std::min(std::max(homingProgress, 0.0), 1.0);
			double maxX = min.x + width * homingProgress;
			glm::vec2 progressMax(maxX, min.y + height);
			ImGui::GetWindowDrawList()->AddRectFilled(min, progressMax, ImColor(Colors::transparentWhite), ImGui::GetStyle().FrameRounding);
		}
		
		ImGui::EndChild();
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	
	ImGui::EndChild();

	
	
	
	
	
	
	ImGui::SameLine();
	
	glm::vec2 environnementBoxSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 4.2);
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::veryDarkGray);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0);
	ImGui::BeginChild("NetworkSetup", environnementBoxSize, true);
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Network");
	ImGui::PopFont();
	
	glm::vec2 initButtonSize(ImGui::GetTextLineHeight() * 5.0, ImGui::GetFrameHeight());
	bool disableSetupButton = EtherCatFieldbus::isRunning() | EtherCatFieldbus::isAutoInitRunning();
	if(disableSetupButton) BEGIN_DISABLE_IMGUI_ELEMENT
	if(EtherCatFieldbus::isAutoInitRunning()) {
		ImGui::Button("Initializing...", initButtonSize);
	} else {
		if(ImGui::Button("Auto Setup", initButtonSize)) EtherCatFieldbus::autoInit();
	}
	if(disableSetupButton) END_DISABLE_IMGUI_ELEMENT
	
	ImGui::SameLine();
	
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
	
	static char deviceCountString[128];
	sprintf(deviceCountString, "%i Device%s Detected", (int)EtherCatFieldbus::slaves.size(), EtherCatFieldbus::slaves.size() == 1 ? "" : "s");
	glm::vec4 deviceCountButtonColor;
	if(EtherCatFieldbus::slaves.empty()) deviceCountButtonColor = Colors::blue;
	else deviceCountButtonColor = Colors::green;
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleColor(ImGuiCol_Button, deviceCountButtonColor);
	ImGui::Button(deviceCountString);
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Detected Devices:");
		ImGui::PopFont();
		for(auto& device : EtherCatFieldbus::slaves){
			ImGui::Text("%s", device->getName());
		}
		ImGui::EndTooltip();
	}
		
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	
}
