#include <pch.h>
#include "Brake.h"

#include "Gui/Utilities/CustomWidgets.h"

#include <imgui.h>

/*
void Brake::widgetGui(bool b_withTitle){
	
	glm::vec2 blockSize(ImGui::GetTextLineHeight() * 7.0, ImGui::GetTextLineHeight() * 1.5);
	
	if(b_withTitle){
		ImGui::PushFont(Fonts::sansBold20);
		centeredText(getName(), glm::vec2(blockSize.x, ImGui::GetTextLineHeight()));
		ImGui::PopFont();
	}
	
	ImGui::PushFont(Fonts::sansBold15);
	
	const char* upperButtonString;
	const char* lowerButtonString;
	ImVec4 upperButtonColor;
	ImVec4 lowerButtonColor;
	bool upperButtonDisabled;
	bool lowerButtonDisabled;
	switch(actualState){
		case State::OFFLINE:
			upperButtonString = "Offline";
			lowerButtonString = "Offline";
			upperButtonColor = Colors::blue;
			lowerButtonColor = Colors::blue;
			upperButtonDisabled = true;
			lowerButtonDisabled = true;
			break;
		case State::UNKNOWN:
			upperButtonString = "Open";
			lowerButtonString = "Close";
			upperButtonColor = Colors::red;
			lowerButtonColor = Colors::red;
			upperButtonDisabled = false;
			lowerButtonDisabled = false;
			break;
		case State::OPEN:
			upperButtonString = "Fully Opened";
			lowerButtonString = "Close";
			upperButtonColor = Colors::green;
			lowerButtonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
			upperButtonDisabled = true;
			lowerButtonDisabled = false;
			break;
		case State::CLOSED:
			upperButtonString = "Open";
			lowerButtonString = "Closed";
			upperButtonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
			lowerButtonColor = Colors::green;
			upperButtonDisabled = false;
			lowerButtonDisabled = true;
			break;
		case State::STOPPED:
		case State::IN_BETWEEN:
			upperButtonString = "Open";
			lowerButtonString = "Close";
			upperButtonColor = Colors::yellow;
			lowerButtonColor = Colors::yellow;
			upperButtonDisabled = false;
			lowerButtonDisabled = false;
			break;
	}
	
	ImGui::BeginDisabled(b_halted);
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(1.f, 1.f));
	ImGui::BeginDisabled(upperButtonDisabled);
	bool upperButtonPressed = customButton(upperButtonString, blockSize, upperButtonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTop);
	ImGui::EndDisabled();
	ImGui::BeginDisabled(lowerButtonDisabled);
	bool lowerButtonPressed = customButton(lowerButtonString, blockSize, lowerButtonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersBottom);
	ImGui::EndDisabled();
	ImGui::PopStyleVar();
	
	ImGui::EndDisabled();
	
	
	if(actualState != State::OFFLINE){
		if(upperButtonPressed) requestState(State::OPEN);
		else if(lowerButtonPressed) requestState(State::CLOSED);
	}

	ImGui::PopFont();
}

void Brake::ControlWidget::gui(){
	brake->widgetGui(true);
}


void Brake::metricsGui(){}
void Brake::controlsGui(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Controls");
	ImGui::PopFont();
	
	widgetGui(false);
	
	switch(actualState){
		case State::OFFLINE:
			ImGui::Text("Brake is Offline");
			break;
		case State::UNKNOWN:
			ImGui::Text("Brake is in unknown state");
			break;
		case State::OPEN:
			ImGui::Text("Brake is Fully Opened");
			break;
		case State::CLOSED:
			ImGui::Text("Brake is Fully Closed");
			break;
		case State::STOPPED:
		case State::IN_BETWEEN:
			ImGui::Text("Brake is not closed or open");
			break;
	}
}
void Brake::settingsGui(){}

void Brake::axisGui(){}
void Brake::deviceGui(){}
*/
