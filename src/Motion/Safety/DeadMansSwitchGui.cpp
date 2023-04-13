#include <pch.h>

#include "DeadMansSwitch.h"
#include "Gui/Utilities/CustomWidgets.h"

#include <imgui.h>

void DeadMansSwitch::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Dead Mans Switch")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Controls");
		ImGui::PopFont();
		
		widgetGui();
	
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



void DeadMansSwitch::widgetGui(){
	
	float width = ImGui::GetTextLineHeight() * 8.0;
	glm::vec2 buttonSize(width, ImGui::GetTextLineHeight() * 1.5);
	
	ImGui::PushFont(Fonts::sansBold20);
	centeredText(getName().c_str(), ImVec2(width, ImGui::GetTextLineHeight()));
	ImGui::PopFont();
    
    glm::vec2 statusSize(width, ImGui::GetTextLineHeight() * 3.0);
    ImGui::PushFont(Fonts::sansBold15);
		
	switch(state){
		case State::NOT_CONNECTED:
			backgroundText("Not Connected", statusSize, Colors::blue);
			break;
		case State::NOT_PRESSED:
			backgroundText("Not Pressed", statusSize, Colors::red);
			break;
		case State::PRESS_REQUESTED:
			backgroundText("Press Requested", statusSize, Colors::yellow);
		{
			ImDrawList* drawing = ImGui::GetWindowDrawList();
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			glm::vec2 size = ImGui::GetItemRectSize();
			float timeoutProgress = timeSincePressRequest_seconds / requestTimeoutDelay->getValue();
			max.x -= size.x * timeoutProgress;
			drawing->AddRectFilled(min, max, ImColor(1.f, 1.f, 1.f, .3f));
		}
			break;
		case State::PRESSED:
            if(b_shouldKeepPressing) backgroundText("Keep Pressing !", statusSize, Timing::getBlink(1.0 / requestBlinkFrequency->getValue()) ? Colors::green : Colors::yellow);
            else backgroundText("Pressed", statusSize, Colors::green);
			break;
	}
    
    ImGui::PopFont();
	
	ImGui::BeginDisabled(state == State::NOT_CONNECTED || state == State::PRESSED);
	if(ImGui::Button("Request Press", buttonSize)) requestPress();
	ImGui::EndDisabled();
}

