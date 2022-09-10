#include <pch.h>

#include "AxisStateMachine.h"

#include "Motion/SubDevice.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

void AxisStateMachine::controlsGui() {

	/*
	//TODO: missing manual controls gui (previously was widget gui copy)

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Limit Switch Signals");
	ImGui::PopFont();

	float quadWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3.0) / 4.0;
	glm::vec2 quadButtonSize(quadWidgetWidth, ImGui::GetTextLineHeight() * 2.0);

	bool inputsAreValid = state != MotionState::OFFLINE;

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (inputsAreValid) {
		if (*liftLoweredSignal) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		ImGui::Button("Lift Lowered", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (*liftRaisedSignal) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		ImGui::Button("Lift Raised", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (*hoodShutSignal) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		ImGui::Button("Hood Shut", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (*hoodOpenSignal) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		ImGui::Button("Hood Open", quadButtonSize);
		ImGui::PopStyleColor();
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
		ImGui::Button("Lift Lowered", quadButtonSize);
		ImGui::SameLine();
		ImGui::Button("Lift Raised", quadButtonSize);
		ImGui::SameLine();
		ImGui::Button("Hood Shut", quadButtonSize);
		ImGui::SameLine();
		ImGui::Button("Hood Open", quadButtonSize);
		ImGui::PopStyleColor();
	}

	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Actual State:");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text("%s", Enumerator::getDisplayString(actualState));
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Requested State:");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text("%s", Enumerator::getDisplayString(requestedState));

	ImGui::Separator();

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Status Signals");
	ImGui::PopFont();

	if (inputsAreValid) {
		if (*emergencyStopClearSignal) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Emergency Stop", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (*localControlEnabledSignal) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Remote Active", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (*liftMotorCircuitBreakerSignal) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Lift Motor Circuit Breaker", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (*hoodMotorCircuitBreakerSignal) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Hood Motor Circuit Breaker", quadButtonSize);
		ImGui::PopStyleColor();
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
		ImGui::Button("Emergency Stop", quadButtonSize);
		ImGui::SameLine();
		ImGui::Button("Remote Active", quadButtonSize);
		ImGui::SameLine();
		ImGui::Button("Lift Motor Fuse", quadButtonSize);
		ImGui::SameLine();
		ImGui::Button("Hood Motor Fuse", quadButtonSize);
		ImGui::PopStyleColor();
	}

	ImGui::PopItemFlag();
	 */

}
void AxisStateMachine::settingsGui() {}
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
	
	if(getState() == MotionState::OFFLINE){
		float rounding = ImGui::GetStyle().FrameRounding;
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .1f));
		ImGui::BeginDisabled();
		customButton("", commandButtonSize, Colors::blue, rounding, ImDrawFlags_RoundCornersTop);
		customButton("", commandButtonSize, Colors::blue, rounding, ImDrawFlags_RoundCornersBottom);
		ImGui::EndDisabled();
		ImGui::PopStyleVar();
		
	}else{

		ImColor notStateColor = ImGui::GetColorU32(ImGuiCol_Button);
		ImVec4 movingColor = Colors::yellow;
		ImVec4 reachedColor = Colors::green;
		
		float rounding = ImGui::GetStyle().FrameRounding;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .1f));
		
		float movingBlinkTime = .25f;
		bool blink = fmod(Timing::getProgramTime_seconds(), movingBlinkTime) > movingBlinkTime * .5f;
		
		switch (actualState) {
				
			case State::UNKNOWN:
				ImGui::BeginDisabled();
				customButton("", commandButtonSize, Colors::darkYellow, rounding, ImDrawFlags_RoundCornersTop);
				customButton("", commandButtonSize, Colors::darkYellow, rounding, ImDrawFlags_RoundCornersBottom);
				ImGui::EndDisabled();
				break;
			case State::STOPPED:
                if(customButton("Move Up", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::AT_POSITIVE_LIMIT);
                if(customButton("Move Down", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::AT_NEGATIVE_LIMIT);
				break;
			case State::MOVING_TO_POSITIVE_LIMIT:
				ImGui::BeginDisabled();
				customButton("Moving Up", commandButtonSize, movingColor, rounding, ImDrawFlags_RoundCornersTop);
				ImGui::EndDisabled();
                if(customButton("Move Down", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::AT_NEGATIVE_LIMIT);
				break;
			case State::MOVING_TO_NEGATIVE_LIMIT:
                if(customButton("Move Up", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::AT_POSITIVE_LIMIT);
				ImGui::BeginDisabled();
				customButton("Moving Down", commandButtonSize, movingColor, rounding, ImDrawFlags_RoundCornersBottom);
				ImGui::EndDisabled();
				break;
			case State::AT_POSITIVE_LIMIT:
				ImGui::BeginDisabled();
				customButton("At Upper Limit", commandButtonSize, reachedColor, rounding, ImDrawFlags_RoundCornersTop);
                ImGui::EndDisabled();
                if(customButton("Move Down", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::AT_NEGATIVE_LIMIT);
				break;
			case State::AT_NEGATIVE_LIMIT:
                if(customButton("Move Up", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::AT_POSITIVE_LIMIT);
                ImGui::BeginDisabled();
				customButton("At Lower Limit", commandButtonSize, reachedColor, rounding, ImDrawFlags_RoundCornersBottom);
				ImGui::EndDisabled();
				break;
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
	ImGui::VSliderScalar("##ManualVelocity", sliderSize, ImGuiDataType_Double, &manualVelocitySliderValue, &min, &max, "");
	if(ImGui::IsItemActive()) {}
	else if(ImGui::IsItemDeactivatedAfterEdit()) {
		manualVelocitySliderValue = 0.0;
	}
		
	if(b_disableControls) ImGui::PopStyleColor();
	ImGui::EndDisabled();

	
	ImGui::EndGroup();
	float widgetWidth = ImGui::GetItemRectSize().x;
	machineHeaderGui(headerCursor, widgetWidth);
	machineStateControlGui(widgetWidth);
}
