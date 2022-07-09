#include <pch.h>

#include "DeadMansSwitch.h"
#include "Gui/Utilities/CustomWidgets.h"

#include <imgui.h>

void DeadMansSwitch::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Dead Mans Switch")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Controls");
		ImGui::PopFont();
		
		controlGui(ImVec2(ImGui::GetContentRegionAvail().x * .5f, ImGui::GetTextLineHeight() * 4.f));
	
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Settings");
		ImGui::PopFont();
		
		ImGui::Text("Press request timeout delay");
		requestTimeoutDelay->gui();
		
		ImGui::Text("LED blink frequency on request");
		requestBlinkFrequency->gui();
		
		ImGui::Text("LED Idle blink frequency");
		idleBlinkFrequency->gui();
		
		ImGui::Text("LED Idle blink length");
		idleBlinkLength->gui();
		
		ImGui::EndTabItem();
	}
}

void DeadMansSwitch::controlGui(ImVec2 size){
	glm::vec2 buttonSize(size.x, (size.y - ImGui::GetStyle().ItemSpacing.y) / 2.f);
	
	switch(state){
		case State::NOT_CONNECTED:
			backgroundText("Not Connected", buttonSize, Colors::blue);
			break;
		case State::NOT_PRESSED:
			backgroundText("Not Pressed", buttonSize, Colors::red);
			break;
		case State::PRESS_REQUESTED:
			backgroundText("Press Requested", buttonSize, Colors::yellow);
		{
			ImDrawList* drawing = ImGui::GetWindowDrawList();
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			glm::vec2 size = ImGui::GetItemRectSize();
			float timeoutProgress = timeSincePressRequest_seconds / requestTimeoutDelay->value;
			max.x -= size.x * timeoutProgress;
			drawing->AddRectFilled(min, max, ImColor(1.f, 1.f, 1.f, .3f));
		}
			break;
		case State::PRESSED:
			backgroundText("Pressed", buttonSize, Colors::green);
			break;
	}
	
	ImGui::BeginDisabled(state == State::NOT_CONNECTED || state == State::PRESSED);
	if(ImGui::Button("Request Press", buttonSize)) requestPress();
	ImGui::EndDisabled();
}

void DeadMansSwitch::widgetGui(){
		
	glm::vec2 contentSize = controlWidget->getFixedContentSize();
	glm::vec2 buttonSize(contentSize.x, (contentSize.y - ImGui::GetStyle().ItemSpacing.y) / 2.f);
	
	ImGui::PushFont(Fonts::sansBold20);
	centeredText(getName(), ImVec2(contentSize.x, ImGui::GetTextLineHeight()));
	ImGui::PopFont();
	
	controlGui(ImVec2(contentSize.x, ImGui::GetTextLineHeight() * 4.f));
}

void DeadMansSwitch::ControlWidget::gui(){
	deadMansSwitch->widgetGui();
}

glm::vec2 DeadMansSwitch::ControlWidget::getFixedContentSize(){
	float lineHeight = ImGui::GetTextLineHeight();
	return glm::vec2(lineHeight * 10.0, lineHeight * 5.0);
}

