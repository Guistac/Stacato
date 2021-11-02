#include <pch.h>

#include "HoodedLiftStateMachine.h"

#include "Motion/Subdevice.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Framework/Fonts.h"
#include "Gui/Framework/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

void HoodedLiftStateMachine::controlsGui() {

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Flip Controls");
	ImGui::PopFont();

	powerControlGui();

	machineSpecificMiniatureGui();
	
	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
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

	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Actual State:");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text(getState(actualState)->displayName);
	ImGui::SameLine();
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Requested State:");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text(getState(requestedState)->displayName);

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Status Signals");
	ImGui::PopFont();

	if (inputsAreValid) {
		if (emergencyStopActive) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Emergency Stop", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (remoteControlEnabled) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Remote Active", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (liftMotorFuseBurned) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Lift Motor Fuse", quadButtonSize);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (hoodMotorFuseBurned) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Hood Motor Fuse", quadButtonSize);
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

float HoodedLiftStateMachine::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 6.0;
}

void HoodedLiftStateMachine::machineSpecificMiniatureGui() {
	glm::vec2 commandButtonSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 3.0);

	bool inputsAreValid = isGpioDeviceConnected() && areGpioSignalsReady();

	if (inputsAreValid) {

		bool disableManualCommandButtons = !isEnabled();
		if(disableManualCommandButtons)BEGIN_DISABLE_IMGUI_ELEMENT

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
				ImGui::Button("", commandButtonSize);
				ImGui::Button("Unknown State", commandButtonSize);
				ImGui::Button("", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
				break;
			case MachineState::State::UNEXPECTED_STATE:
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
				ImGui::Button("", commandButtonSize);
				ImGui::Button("Error", commandButtonSize);
				ImGui::Button("", commandButtonSize);
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
				break;
		}

		if(disableManualCommandButtons) END_DISABLE_IMGUI_ELEMENT

	}
	else {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
		ImGui::Button("", commandButtonSize);
		ImGui::Button("IO is not ready", commandButtonSize);
		ImGui::Button("", commandButtonSize);
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();
	}
}