#include <pch.h>

#include "AxisStateMachine.h"

#include "Motion/Interfaces.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"



void AxisStateMachine::controlsGui() {
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	widgetGui();
	
	ImGui::Text("Velocity Target: %.3f", velocityTarget);
}
void AxisStateMachine::settingsGui() {
	minNegativeVelocity->gui(Fonts::sansBold15);
	maxNegativeVelocity->gui(Fonts::sansBold15);
	minPositiveVelocity->gui(Fonts::sansBold15);
	maxPositiveVelocity->gui(Fonts::sansBold15);
}
void AxisStateMachine::axisGui() {}
void AxisStateMachine::deviceGui() {}
void AxisStateMachine::metricsGui() {}








void AxisStateMachine::ControlWidget::gui(){
	machine->widgetGui();
}


void AxisStateMachine::widgetGui(){
	
	glm::vec2 commandButtonSize(ImGui::GetTextLineHeight() * 6.0, ImGui::GetTextLineHeight() * 1.6);

	glm::vec2 headerCursor = reserveSpaceForMachineHeaderGui();

	ImGui::BeginGroup();
	
	
	bool b_disableControls = !isEnabled() || b_halted;
	ImGui::BeginDisabled(b_disableControls);
	if(b_disableControls) ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	
	ImGui::BeginGroup();
	
	if(getState() == DeviceState::OFFLINE){
		float rounding = ImGui::GetStyle().FrameRounding;
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .1f));
		ImGui::BeginDisabled();
		customButton(" ", commandButtonSize, Colors::blue, rounding, ImDrawFlags_RoundCornersTop);
		customButton(" ", commandButtonSize, Colors::blue, rounding, ImDrawFlags_RoundCornersBottom);
		ImGui::EndDisabled();
		ImGui::PopStyleVar();
		
	}else{
        
        ImColor notStateColor = ImGui::GetColorU32(ImGuiCol_Button);
        ImVec4 movingColor = Colors::yellow;
        ImVec4 reachedColor = Colors::green;
		bool blink = Timing::getBlink(0.5);
        float rounding = ImGui::GetStyle().FrameRounding;
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .1f));
        
        if(actualState == State::AT_POSITIVE_LIMIT){
            ImGui::BeginDisabled();
            customButton("At Upper Limit", commandButtonSize, reachedColor, rounding, ImDrawFlags_RoundCornersTop);
            ImGui::EndDisabled();
        }
        else if(actualState == State::MOVING_TO_POSITIVE_LIMIT){
            ImGui::BeginDisabled();
            customButton("Moving Up", commandButtonSize, blink ? movingColor : reachedColor, rounding, ImDrawFlags_RoundCornersTop);
            ImGui::EndDisabled();
        }else if(requestedState == State::MOVING_TO_POSITIVE_LIMIT){
            if(customButton("Move Up", commandButtonSize, movingColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::AT_POSITIVE_LIMIT);
        }else{
            if(customButton("Move Up", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::AT_POSITIVE_LIMIT);
        }
        
        if(actualState == State::AT_NEGATIVE_LIMIT){
            ImGui::BeginDisabled();
            customButton("At Lower Limit", commandButtonSize, reachedColor, rounding, ImDrawFlags_RoundCornersBottom);
            ImGui::EndDisabled();
        }else if(actualState == State::MOVING_TO_NEGATIVE_LIMIT){
            ImGui::BeginDisabled();
            customButton("Moving Down", commandButtonSize, blink ? movingColor : reachedColor, rounding, ImDrawFlags_RoundCornersBottom);
            ImGui::EndDisabled();
        }else if(requestedState == State::MOVING_TO_NEGATIVE_LIMIT){
            if(customButton("Move Down", commandButtonSize, movingColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::AT_NEGATIVE_LIMIT);
        }else{
            if(customButton("Move Down", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::AT_NEGATIVE_LIMIT);
        }
        
		ImGui::PopStyleVar();
	}
	
	
	if(b_halted) {
		ImGui::PushFont(Fonts::sansBold15);
		backgroundText("STOPPED", commandButtonSize, Colors::red, Colors::black);
		ImGui::PopFont();
	}
	else{
		if(ImGui::Button("STOP", commandButtonSize)) requestState(State::STOPPED);
	}
	

	ImGui::EndGroup();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .1f));
	ImGui::SameLine();
	ImGui::PopStyleVar();
	
	glm::vec2 sliderSize(ImGui::GetTextLineHeight() * 2.0, ImGui::GetItemRectSize().y);
	static double min = -1.0;
	static double max = 1.0;
	if(ImGui::VSliderScalar("##ManualVelocity", sliderSize, ImGuiDataType_Double, &manualVelocitySliderValue, &min, &max, "")){
		requestVelocityNormalized(manualVelocitySliderValue);
	}
	else if(ImGui::IsItemDeactivatedAfterEdit()) {
		requestVelocityNormalized(0.0);
		manualVelocitySliderValue = 0.0;
	}
		
	if(b_disableControls) ImGui::PopStyleColor();
	ImGui::EndDisabled();

	
	ImGui::EndGroup();
	float widgetWidth = ImGui::GetItemRectSize().x;
	machineHeaderGui(headerCursor, widgetWidth);
	machineStateControlGui(widgetWidth);
}

