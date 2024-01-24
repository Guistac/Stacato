#include <pch.h>
#include "ManualControlChannel.h"
#include "Machine/Machine.h"
#include "Gui/Utilities/CustomWidgets.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

void ManualControlChannel::gui(){
	
	const char* mappingPreviewString = "No Mapping Active";
	if(activeChannelPreset) mappingPreviewString = activeChannelPreset->nameParameter->value.c_str();
	ImGui::PushFont(Fonts::sansBold20);
	ImVec2 headerSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing());
	backgroundText(mappingPreviewString, headerSize, Colors::darkGray, Colors::white);
	ImGui::PopFont();
	
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::BeginDisabled(activeChannelPreset == nullptr);
	if(ImGui::SliderFloat("##controlSliderX", &controlSliderValueX, -1.0, 1.0)) setControlValue(controlSliderValueX, 0.0);
	else if(ImGui::IsItemDeactivatedAfterEdit()) setControlValue(0.0, 0.0);
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if(ImGui::SliderFloat("##controlSliderY", &controlSliderValueY, -1.0, 1.0)) setControlValue(0.0, controlSliderValueY);
	else if(ImGui::IsItemDeactivatedAfterEdit()) setControlValue(0.0, 0.0);
	ImGui::EndDisabled();
	

	
	ImGui::Separator();

	ImVec2 buttonSize(0, ImGui::GetFrameHeight());
	ImVec4 buttonColor = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
	float buttonRounding = ImGui::GetStyle().FrameRounding;
	
	if(b_editLocked){
		if(customButton("Edit", buttonSize, buttonColor, buttonRounding, ImDrawFlags_RoundCornersAll)) b_editLocked = false;
	}
	else{
		if(customButton("Lock", buttonSize, buttonColor, buttonRounding, ImDrawFlags_RoundCornersLeft)) b_editLocked = true;
		ImGui::SameLine(0.0, 1.0);
		if(customButton("New", buttonSize, buttonColor, buttonRounding, ImDrawFlags_RoundCornersNone)) createChannelPreset();
		ImGui::SameLine(0.0, 1.0);
		ImGui::BeginDisabled(activeChannelPreset == nullptr);
		if(customButton("Remove", buttonSize, buttonColor, buttonRounding, ImDrawFlags_RoundCornersRight)) removeChannelPreset(activeChannelPreset);
		ImGui::EndDisabled();
	}
	
	ImGui::SameLine();
	
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if(ImGui::BeginCombo("##MappingSelector", mappingPreviewString)){
		for(int i = 0; i < channelPresets.size(); i++){
			ImGui::PushID(i);
			auto mapping = channelPresets[i];
			const char* name = mapping->nameParameter->value.c_str();
			if(ImGui::Selectable(name, mapping == activeChannelPreset)) setActiveChannelPreset(mapping);
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	
	
	if(activeChannelPreset == nullptr) return;
		
	if(!b_editLocked){
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Preset Name:");
		ImGui::PopFont();
		activeChannelPreset->nameParameter->gui();
	}
	
	if(!activeChannelPreset->getMappings().empty()){
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Mappings:");
		ImGui::PopFont();
	}
	else{
		ImGui::PushStyleColor(ImGuiCol_Tab, Colors::darkGray);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("No Mappings:");
		ImGui::PopFont();
		ImGui::PopStyleColor();
	}
	
	if(ImGui::BeginTable("##AnimatableMappings", 2)){
		
		for(int i = 0; i < activeChannelPreset->getMappings().size(); i++){
			ImGui::PushID(i);
			auto mapping = activeChannelPreset->getMappings()[i];
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			backgroundText(mapping->animatable->getMachine()->getName(), ImVec2(0,0), Colors::darkGray, Colors::white, ImDrawFlags_RoundCornersLeft);
			ImGui::SameLine(0.0, 0.0);
			backgroundText(mapping->animatable->getName(), ImVec2(0,0), Colors::veryDarkGray, Colors::white, ImDrawFlags_RoundCornersRight);
			if(!b_editLocked){
				ImGui::TableSetColumnIndex(1);
				mapping->multiplier->gui();
			}
			ImGui::PopID();
		}
		
		if(!b_editLocked){
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if(ImGui::Button("Edit Mappings")) ImGui::OpenPopup("AnimatableAdderPopup");
		}
			
		if (ImGui::BeginPopup("AnimatableAdderPopup")) {
			for (auto& machine : Environnement::getMachines()) {
				if(machine->getAnimatables().empty()) continue;
				
				if(machine->getAnimatables().size() == 1){
					auto animatable = machine->getAnimatables().front();
					if (animatable->hasParentComposite()) continue;
					bool mappingHasAnimatable = activeChannelPreset->hasAnimatable(animatable);
					if(ImGui::MenuItem(machine->getName(), nullptr, mappingHasAnimatable)){
						if(mappingHasAnimatable) activeChannelPreset->removeAnimatable(animatable);
						else activeChannelPreset->addAnimatable(animatable);
					}
				}else if (ImGui::BeginMenu(machine->getName())) {
					for (auto& animatable : machine->getAnimatables()) {
						if (animatable->hasParentComposite()) continue;
						bool mappingHasAnimatable = activeChannelPreset->hasAnimatable(animatable);
						if (ImGui::MenuItem(animatable->getName(), nullptr, mappingHasAnimatable)) {
							if(mappingHasAnimatable) activeChannelPreset->removeAnimatable(animatable);
							else activeChannelPreset->addAnimatable(animatable);
						}
					}
					ImGui::EndMenu();
				}

			}
			ImGui::EndPopup();
		}
			
		ImGui::EndTable();
	}
	
	
	
}


void ManualControlsWindow::onDraw(){
	Environnement::getManualControlChannel()->gui();
}
