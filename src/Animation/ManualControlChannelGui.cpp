#include <pch.h>
#include "ManualControlChannel.h"
#include "Machine/Machine.h"
#include "Gui/Utilities/CustomWidgets.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

#include "Animation/Animatable.h"

std::string getPopupName(std::string& channelName){
	return channelName + "/MappingList";
}

void ManualControlChannel::openMappingList(){
	ImGui::OpenPopup(getPopupName(name).c_str());
}

void ManualControlChannel::mappingList(){
	if (ImGui::BeginPopup(getPopupName(name).c_str())) {
		ImGui::BeginDisabled();
		ImGui::MenuItem("Add Mapping");
		ImGui::MenuItem("Machine List :");
		ImGui::EndDisabled();
		ImGui::Separator();
		for (auto& machine : Environnement::getMachines()) {
			if(machine->getAnimatables().empty()) continue;
			
			if(machine->getAnimatables().size() == 1){
				auto animatable = machine->getAnimatables().front();
				if (animatable->hasParentComposite()) continue;
				bool b_selected = animatable->subscribedManualControlChannel == shared_from_this();
				if(ImGui::MenuItem(machine->getName(), nullptr, b_selected)){
					if(b_selected) removeSubscriber(animatable);
					else addSubscriber(animatable);
				}
			}else if (ImGui::BeginMenu(machine->getName())) {
				for (auto& animatable : machine->getAnimatables()) {
					if (animatable->hasParentComposite()) continue;
					bool b_selected = animatable->subscribedManualControlChannel == shared_from_this();
					if (ImGui::MenuItem(animatable->getName(), nullptr)) {
						if(b_selected) removeSubscriber(animatable);
						else addSubscriber(animatable);
					}
				}
				ImGui::EndMenu();
			}

		}
		ImGui::Separator();
		if(ImGui::MenuItem("Clear All")) clearSubscribers();
		ImGui::EndPopup();
	}
}

void ManualControlChannel::mappingListTooltip(){
	if(ImGui::BeginTooltip()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", name.c_str());
		ImGui::PopFont();
		if(subscribers.empty()) ImGui::Text("Click to add mappings");
		else ImGui::Text("Current Mappings: (click to edit)");
		ImGui::PopStyleColor();
		for (auto& machine : Environnement::getMachines()) {
			if(machine->getAnimatables().size() == 1){
				auto animatable = machine->getAnimatables().front();
				if (animatable->hasParentComposite()) continue;
				if(animatable->subscribedManualControlChannel == shared_from_this()){
					ImGui::Text("%s", machine->getName());
				}
			}else {
				for (auto& animatable : machine->getAnimatables()) {
					if (animatable->hasParentComposite()) continue;
					if(animatable->subscribedManualControlChannel == shared_from_this()){
						ImGui::Text("%s: %s", machine->getName(), animatable->getName());
					}
				}
			}
		}
		ImGui::EndTooltip();
	}
}


void ManualControlChannel::fullGui(){
	bool b_updated = false;
	if(ImGui::SliderFloat("##sliderX", &controlValue.x, -1.0, 1.0, "X: %.2f")) b_updated = true;
	if(ImGui::IsItemDeactivatedAfterEdit()) {
		controlValue.x = 0.0;
		b_updated = true;
	}
	if(ImGui::SliderFloat("##sliderY", &controlValue.y, -1.0, 1.0, "Y: %.2f")) b_updated = true;
	if(ImGui::IsItemDeactivatedAfterEdit()) {
		controlValue.y = 0.0;
		b_updated = true;
	}
	if(ImGui::SliderFloat("##slider2", &controlValue.z, -1.0, 1.0, "Z: %.2f")) b_updated = true;
	if(ImGui::IsItemDeactivatedAfterEdit()) {
		controlValue.z = 0.0;
		b_updated = true;
	}
	
	std::shared_ptr<Animatable> removedSubscriber = nullptr;
	for(int i = 0; i < subscribers.size(); i++){
		auto subscriber = subscribers[i];
		std::string fullName = std::string(subscriber->getMachine()->getName()) + ": " + std::string(subscriber->getName());
		if(buttonCross("##Delete")) removedSubscriber = subscriber;
		ImGui::SameLine();
		ImGui::Text("%s", fullName.c_str());
	}
	if(removedSubscriber) removeSubscriber(removedSubscriber);
	
	
	if (ImGui::Button("Add Mapping")) openMappingList();
	mappingList();
	
	if(b_updated) updateSubscribers();
}

void ManualControlsWindow::onDraw(){
	ImGui::PushID(1);
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Control Channel 1");
	ImGui::PopFont();
	Environnement::controlChannel1->fullGui();
	ImGui::PopID();
	ImGui::Separator();
	ImGui::PushID(2);
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Control Channel 2");
	ImGui::PopFont();
	Environnement::controlChannel2->fullGui();
	ImGui::PopID();
}
