#include <pch.h>

#include "HoodedLiftStateMachine.h"

#include "Motion/SubDevice.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

void HoodedLiftStateMachine::controlsGui() {

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Manual Flip Controls");
	ImGui::PopFont();

	stateControlGui();

	//TODO: missing manual controls gui (previously was widget gui copy)
	
	ImGui::Separator();

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Limit Switch Signals");
	ImGui::PopFont();

	float quadWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3.0) / 4.0;
	glm::vec2 quadButtonSize(quadWidgetWidth, ImGui::GetTextLineHeight() * 2.0);

	bool inputsAreValid = isGpioDeviceConnected() && areGpioSignalsReady();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (inputsAreValid) {
		if (liftLowered) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		ImGui::Button("Lift Lowered", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (liftRaised) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		ImGui::Button("Lift Raised", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (hoodShut) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		ImGui::Button("Hood Shut", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (hoodOpen) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
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
	ImGui::Text("%s", getState(actualState)->displayName);
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Requested State:");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text("%s", getState(requestedState)->displayName);

	ImGui::Separator();

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Status Signals");
	ImGui::PopFont();

	if (inputsAreValid) {
		if (emergencyStopClear) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Emergency Stop", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (localControlEnabled) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Remote Active", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (liftMotorCircuitBreakerTripped) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Lift Motor Circuit Breaker", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (hoodMotorCircuitBreakerTripped) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
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
void HoodedLiftStateMachine::settingsGui() {}
void HoodedLiftStateMachine::axisGui() {}
void HoodedLiftStateMachine::deviceGui() {}
void HoodedLiftStateMachine::metricsGui() {}








void HoodedLiftStateMachine::ControlWidget::gui(){
	machine->widgetGui();
}
glm::vec2 HoodedLiftStateMachine::ControlWidget::getFixedContentSize(){
	float lineHeight = ImGui::GetTextLineHeight();
	return glm::vec2(lineHeight * 10.0, lineHeight * 10.0);
}

void HoodedLiftStateMachine::widgetGui(){
	glm::vec2 contentSize = controlWidget->getFixedContentSize();
	glm::vec2 commandButtonSize(contentSize.x, ImGui::GetTextLineHeight() * 3.0);

	machineHeaderGui(contentSize.x);
	
	if(isReady()){
		
		bool disableManualCommandButtons = !isEnabled();
		ImGui::BeginDisabled(disableManualCommandButtons);

		switch (actualState) {
			case MachineState::State::LIFT_LOWERED_HOOD_SHUT:
				if (ImGui::Button("Open & Raise", commandButtonSize)) requestedState = MachineState::State::LIFT_RAISED_HOOD_OPEN;
				if (ImGui::Button("Open", commandButtonSize)) requestedState = MachineState::State::LIFT_LOWERED_HOOD_OPEN;
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				ImGui::Button("Lowered & Shut", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
				break;
			case MachineState::State::LIFT_LOWERED_HOOD_MOVING:
				if (ImGui::Button("Open & Raise", commandButtonSize)) requestedState = MachineState::State::LIFT_RAISED_HOOD_OPEN;
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Open", commandButtonSize)) requestedState = MachineState::State::LIFT_LOWERED_HOOD_OPEN;
				if (ImGui::Button("Shut", commandButtonSize)) requestedState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
				ImGui::PopStyleColor();
				break;
			case MachineState::State::LIFT_LOWERED_HOOD_OPEN:
				if (ImGui::Button("Raise", commandButtonSize)) requestedState = MachineState::State::LIFT_RAISED_HOOD_OPEN;
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				ImGui::Button("Open & Lowered", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
				if (ImGui::Button("Shut", commandButtonSize)) requestedState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
				break;
			case MachineState::State::LIFT_MOVING_HOOD_OPEN:
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Raise", commandButtonSize)) requestedState = MachineState::State::LIFT_RAISED_HOOD_OPEN;
				if (ImGui::Button("Lower", commandButtonSize)) requestedState = MachineState::State::LIFT_LOWERED_HOOD_OPEN;
				ImGui::PopStyleColor();
				if (ImGui::Button("Lower & Shut", commandButtonSize)) requestedState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
				break;
			case MachineState::State::LIFT_RAISED_HOOD_OPEN:
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				ImGui::Button("Open & Raised", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
				if (ImGui::Button("Lower", commandButtonSize)) requestedState = MachineState::State::LIFT_LOWERED_HOOD_OPEN;
				if (ImGui::Button("Lower & Shut", commandButtonSize)) requestedState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
				break;
			case MachineState::State::UNKNOWN:
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
				ImGui::Button("##", commandButtonSize);
				ImGui::Button("Unknown State", commandButtonSize);
				ImGui::Button("##", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
				break;
			case MachineState::State::UNEXPECTED_STATE:
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
				ImGui::Button("##", commandButtonSize);
				ImGui::Button("Error", commandButtonSize);
				ImGui::Button("##", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
				break;
		}

		ImGui::EndDisabled();

	}else{
		
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
		ImGui::Button("##emptyButton", commandButtonSize);
		ImGui::Button("Not ready", commandButtonSize);
		ImGui::Button("##emptyButton", commandButtonSize);
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();
		
	}
	
	machineStateControlGui(contentSize.x);
}
