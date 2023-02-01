#include <pch.h>

#include "FlipStateMachine.h"

#include "Motion/SubDevice.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

void FlipStateMachine::controlsGui() {

	//TODO: missing manual controls gui (previously was widget gui copy)

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Limit Switch Signals");
	ImGui::PopFont();

	float quadWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3.0) / 4.0;
	glm::vec2 quadButtonSize(quadWidgetWidth, ImGui::GetTextLineHeight() * 2.0);

	bool inputsAreValid = state != DeviceState::OFFLINE;

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

}
void FlipStateMachine::settingsGui() {}
void FlipStateMachine::axisGui() {}
void FlipStateMachine::deviceGui() {}
void FlipStateMachine::metricsGui() {}








void FlipStateMachine::ControlWidget::gui(){
	machine->widgetGui();
}


void FlipStateMachine::widgetGui(){
	
	float lineHeight = ImGui::GetTextLineHeight();
	glm::vec2 contentSize = glm::vec2(lineHeight * 6.0, lineHeight * 6.0);
	glm::vec2 commandButtonSize(contentSize.x, ImGui::GetTextLineHeight() * 1.6);

	
	machineHeaderGui(contentSize.x);
		
    bool b_disableControls = !isEnabled() || b_halted;
	ImGui::BeginDisabled(b_disableControls);
    if(b_disableControls) ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	
	if(getState() == DeviceState::OFFLINE){
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		float rounding = ImGui::GetStyle().FrameRounding;
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .1f));
		ImGui::BeginDisabled();
		customButton("", commandButtonSize, Colors::blue, rounding, ImDrawFlags_RoundCornersTop);
		customButton("", commandButtonSize, Colors::blue, rounding, ImDrawFlags_RoundCornersNone);
		customButton("", commandButtonSize, Colors::blue, rounding, ImDrawFlags_RoundCornersBottom);
		ImGui::EndDisabled();
		ImGui::PopStyleVar();
		
	}else{

        ImColor notStateColor = ImGui::GetColorU32(ImGuiCol_Button);
        ImVec4 movingColor = Colors::yellow;
        ImVec4 reachedColor = Colors::green;
		
		float rounding = ImGui::GetStyle().FrameRounding;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .1f));
        
		switch (actualState) {
			case State::CLOSED:
				if(customButton("Open & Raise", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::RAISED);
				if(customButton("Open", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersNone)) requestState(State::OPEN_LOWERED);
				ImGui::BeginDisabled();
				customButton("Lowered & Shut", commandButtonSize, reachedColor, rounding, ImDrawFlags_RoundCornersBottom);
				ImGui::EndDisabled();
				break;
			case State::OPENING_CLOSING:
				if(customButton("Open & Raise", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::RAISED);
				if(customButton("Open", commandButtonSize, movingColor, rounding, ImDrawFlags_RoundCornersNone)) requestState(State::OPEN_LOWERED);
				if(customButton("Shut", commandButtonSize, movingColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::CLOSED);
				break;
			case State::OPEN_LOWERED:
				if(customButton("Raise", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::RAISED);
				ImGui::BeginDisabled();
				customButton("Open & Lowered", commandButtonSize, reachedColor, rounding, ImDrawFlags_RoundCornersNone);
				ImGui::EndDisabled();
				if(customButton("Shut", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::CLOSED);
				break;
			case State::LOWERING_RAISING:
				if(customButton("Raise", commandButtonSize, movingColor, rounding, ImDrawFlags_RoundCornersTop)) requestState(State::RAISED);
				if(customButton("Lower", commandButtonSize, movingColor, rounding, ImDrawFlags_RoundCornersNone)) requestState(State::OPEN_LOWERED);
				if(customButton("Lower & Shut", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::CLOSED);
				break;
			case State::RAISED:
				ImGui::BeginDisabled();
				customButton("Open & Raised", commandButtonSize, reachedColor, rounding, ImDrawFlags_RoundCornersTop);
				ImGui::EndDisabled();
				if(customButton("Lower", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersNone)) requestState(State::OPEN_LOWERED);
				if(customButton("Lower & Shut", commandButtonSize, notStateColor, rounding, ImDrawFlags_RoundCornersBottom)) requestState(State::CLOSED);
				break;
			default:
				ImGui::BeginDisabled();
				customButton("", commandButtonSize, Colors::darkYellow, rounding, ImDrawFlags_RoundCornersTop);
				customButton("Unknown State", commandButtonSize, Colors::darkYellow, rounding, ImDrawFlags_RoundCornersNone);
				customButton("", commandButtonSize, Colors::darkYellow, rounding, ImDrawFlags_RoundCornersBottom);
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
		
    if(b_disableControls) ImGui::PopStyleColor();
	ImGui::EndDisabled();

		
	machineStateControlGui(contentSize.x);
}
