#include <pch.h>
#include "Brake.h"

#include "Gui/Utilities/CustomWidgets.h"

#include <imgui.h>

void Brake::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Brake")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Controls");
		ImGui::PopFont();
		
		widgetGui(false);
		
		switch(state){
			case State::OFFLINE:
				ImGui::Text("Brake is Offline");
				break;
			case State::ERROR:
				ImGui::Text("Brake is Offline");
				break;
			case State::OPEN:
				ImGui::Text("Brake is Fully Opened");
				break;
			case State::CLOSED:
				ImGui::Text("Brake is Fully Closed");
				break;
			case State::NOT_CLOSED:
				ImGui::Text("Brake is not closed");
				break;
		}
		
		ImGui::EndTabItem();
	}
}

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
	switch(state){
		case State::OFFLINE:
			upperButtonString = "Offline";
			lowerButtonString = "Offline";
			upperButtonColor = Colors::blue;
			lowerButtonColor = Colors::blue;
			upperButtonDisabled = true;
			lowerButtonDisabled = true;
			break;
		case State::ERROR:
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
		case State::NOT_CLOSED:
			upperButtonString = "Open";
			lowerButtonString = "Close";
			upperButtonColor = Colors::yellow;
			lowerButtonColor = Colors::yellow;
			upperButtonDisabled = false;
			lowerButtonDisabled = false;
			break;
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(1.f, 1.f));
	ImGui::BeginDisabled(upperButtonDisabled);
	bool upperButtonPressed = customButton(upperButtonString, blockSize, upperButtonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTop);
	ImGui::EndDisabled();
	ImGui::BeginDisabled(lowerButtonDisabled);
	bool lowerButtonPressed = customButton(lowerButtonString, blockSize, lowerButtonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersBottom);
	ImGui::EndDisabled();
	ImGui::PopStyleVar();
	
	if(upperButtonPressed){
		switch(state){
			case State::OFFLINE:
				break;
			case State::ERROR:
			case State::OPEN:
			case State::CLOSED:
			case State::NOT_CLOSED:
				target = Target::OPEN;
				break;
		}
	}
	
	if(lowerButtonPressed){
		switch(state){
			case State::OFFLINE:
				break;
			case State::ERROR:
			case State::OPEN:
			case State::CLOSED:
			case State::NOT_CLOSED:
				target = Target::CLOSED;
				break;
		}
	}

	ImGui::PopFont();
}

void Brake::ControlWidget::gui(){
	brake->widgetGui(true);
}
