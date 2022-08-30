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
	glm::vec2 commandButtonSize(contentSize.x, ImGui::GetTextLineHeight() * 1.5);

	
	machineHeaderGui(contentSize.x);
		
	if(getState() == MotionState::OFFLINE){
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		float rounding = ImGui::GetStyle().FrameRounding;
		for(int i = 0; i < 4; i++){
			ImGui::InvisibleButton("nope", commandButtonSize);
			drawing->AddRectFilled(ImGui::GetItemRectMin(),
								   ImGui::GetItemRectMax(),
								   ImColor(Colors::blue),
								   rounding,
								   ImDrawFlags_RoundCornersAll);
		}
	}else{
	
	ImGui::BeginDisabled(!isEnabled() || b_halted);

        ImColor defaultButtonColor = ImGui::GetColorU32(ImGuiCol_Button);
        
		switch (actualState) {
			case State::CLOSED:
                
				if (ImGui::Button("Open & Raise", commandButtonSize)) requestState(State::RAISED);
				if (ImGui::Button("Open", commandButtonSize)) requestState(State::OPEN_LOWERED);
				ImGui::BeginDisabled();
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				ImGui::Button("Lowered & Shut", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::EndDisabled();
				break;
			case State::OPENING_CLOSING:
				if (ImGui::Button("Open & Raise", commandButtonSize)) requestState(State::RAISED);
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Open", commandButtonSize)) requestState(State::OPEN_LOWERED);
				if (ImGui::Button("Shut", commandButtonSize)) requestState(State::CLOSED);
				ImGui::PopStyleColor();
				break;
			case State::OPEN_LOWERED:
				if (ImGui::Button("Raise", commandButtonSize)) requestState(State::RAISED);
				ImGui::BeginDisabled();
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				ImGui::Button("Open & Lowered", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::EndDisabled();
				if (ImGui::Button("Shut", commandButtonSize)) requestState(State::CLOSED);
				break;
			case State::LOWERING_RAISING:
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Raise", commandButtonSize)) requestState(State::RAISED);
				if (ImGui::Button("Lower", commandButtonSize)) requestState(State::OPEN_LOWERED);
				ImGui::PopStyleColor();
				if (ImGui::Button("Lower & Shut", commandButtonSize)) requestState(State::CLOSED);
				break;
			case State::RAISED:
				ImGui::BeginDisabled();
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				ImGui::Button("Open & Raised", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::EndDisabled();
				if (ImGui::Button("Lower", commandButtonSize)) requestState(State::OPEN_LOWERED);
				if (ImGui::Button("Lower & Shut", commandButtonSize)) requestState(State::CLOSED);
				break;
			default:
				ImGui::BeginDisabled();
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
				ImGui::Button("##", commandButtonSize);
				ImGui::Button("Unknown State", commandButtonSize);
				ImGui::Button("##", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::EndDisabled();
				break;
		}
		
		if(b_halted) {
			ImGui::PushFont(Fonts::sansBold15);
			backgroundText("STOPPED", commandButtonSize, Colors::red, Colors::black);
			ImGui::PopFont();
		}
		else{
			if(ImGui::Button("STOP", commandButtonSize)) requestState(State::STOPPED);
		}
			
		ImGui::EndDisabled();

	}
		
	machineStateControlGui(contentSize.x);
}
