#include <pch.h>

#include "DeadMansSwitch.h"

#include <imgui.h>

void DeadMansSwitch::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Controls")){
		
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Settings")){
		
		ImGui::EndTabItem();
	}
}

void DeadMansSwitch::controlsGui(){}
void DeadMansSwitch::settingsGui(){}


void DeadMansSwitch::ControlWidget::gui(){
	ImGui::Text("Dead Mans Switch Widget");
}

glm::vec2 DeadMansSwitch::ControlWidget::getFixedContentSize(){
	float lineHeight = ImGui::GetTextLineHeight();
	return glm::vec2(lineHeight * 10.0, lineHeight * 5.0);
}

