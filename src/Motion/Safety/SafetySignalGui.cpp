#include <pch.h>

#include "SafetySignal.h"
#include "Gui/Utilities/CustomWidgets.h"

#include <imgui.h>

void SafetySignal::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Dead Mans Switch")){
		
		ImGui::EndTabItem();
	}
}

void SafetySignal::widgetGui(){
		
	glm::vec2 contentSize = controlWidget->getFixedContentSize();
	glm::vec2 buttonSize(contentSize.x, (contentSize.y - ImGui::GetStyle().ItemSpacing.y) / 2.f);
	
	ImGui::PushFont(Fonts::sansBold20);
	centeredText(getName(), ImVec2(contentSize.x, ImGui::GetTextLineHeight()));
	ImGui::PopFont();
	
	
}

void SafetySignal::ControlWidget::gui(){
	safetySignal->widgetGui();
}

glm::vec2 SafetySignal::ControlWidget::getFixedContentSize(){
	float lineHeight = ImGui::GetTextLineHeight();
	return glm::vec2(lineHeight * 10.0, lineHeight * 5.0);
}

