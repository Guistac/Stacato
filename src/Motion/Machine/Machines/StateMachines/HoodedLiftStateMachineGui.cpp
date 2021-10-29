#include <pch.h>

#include "HoodedLiftStateMachine.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Framework/Fonts.h"
#include "Gui/Framework/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

void HoodedLiftStateMachine::controlsGui() {
	ImGui::Text("Actual State: %s", getState(actualState)->displayName);
	ImGui::Text("Requested State: %s", getState(requestedState)->displayName);

	miniatureGui();
}
void HoodedLiftStateMachine::settingsGui() {}
void HoodedLiftStateMachine::axisGui() {}
void HoodedLiftStateMachine::deviceGui() {}
void HoodedLiftStateMachine::metricsGui() {}

float HoodedLiftStateMachine::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 6.0;

}

void HoodedLiftStateMachine::machineSpecificMiniatureGui() {
		glm::vec2 commandButtonSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 2.0);

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
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
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
}