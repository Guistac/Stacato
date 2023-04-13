#include <pch.h>

#include "SafetySignal.h"
#include "Gui/Utilities/CustomWidgets.h"

#include <imgui.h>

void SafetySignal::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Dead Mans Switch")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Controls");
		ImGui::PopFont();
		
		widgetGui();
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Settings");
		ImGui::PopFont();
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Fault Reset Pulse Time");
		ImGui::PopFont();
		faultResetPulseTime->gui();
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Uncleared Fault LED Blink Frequency");
		ImGui::PopFont();
		unclearedFaultLedBlinkFrequency->gui();
		
		ImGui::EndTabItem();
	}
}

void SafetySignal::widgetGui(){
	
	glm::vec2 blockSize(ImGui::GetTextLineHeight() * 7.0, ImGui::GetTextLineHeight() * 1.5);
	
	ImGui::PushFont(Fonts::sansBold20);
	centeredText(getName().c_str(), glm::vec2(blockSize.x, ImGui::GetTextLineHeight()));
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::sansBold15);
	switch(safetyState){
		case State::OFFLINE:
			backgroundText("Offline", blockSize, Colors::blue, Colors::white);
			break;
		case State::EMERGENCY_STOP:{
			double blinkTimeSeconds = 1.0 / unclearedFaultLedBlinkFrequency->getValue();
			bool blinkState = fmod(Timing::getProgramTime_seconds(), blinkTimeSeconds) < blinkTimeSeconds * 0.5;
			backgroundText("E-STOP", blockSize, blinkState ? Colors::yellow : Colors::red, Colors::black);
			}break;
		case State::UNCLEARED_SAFETY_FAULT:
			backgroundText("Uncleared Fault", blockSize, Colors::yellow, Colors::black);
			break;
		case State::CLEAR:
			backgroundText("Clear", blockSize, Colors::green, Colors::black);
			break;
	}
	ImGui::PopFont();
	
	if(resetSafetyFaultPin->isConnected()){
		if(safetyLineValidPin->isConnected()) ImGui::BeginDisabled(safetyState != State::UNCLEARED_SAFETY_FAULT);
		else ImGui::BeginDisabled(safetyState == State::CLEAR);
		if(ImGui::Button("Clear Fault", blockSize)) resetSafety();
		ImGui::EndDisabled();
	}
}

void SafetySignal::ControlWidget::gui(){
	safetySignal->widgetGui();
}
