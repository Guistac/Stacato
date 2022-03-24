#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Machine/Machine.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void Machine::nodeSpecificGui() {

	if (ImGui::BeginTabItem("Controls")) {
		ImGui::BeginChild("Controls");
		stateControlGui();
		ImGui::Separator();
		controlsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Settings")) {
		ImGui::BeginChild("Settings");
		generalSettingsGui();
		ImGui::Separator();
		settingsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Axis")) {
		ImGui::BeginChild("Axis");
		axisGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Devices")) {
		ImGui::BeginChild("Devices");
		deviceGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Metrics")) {
		ImGui::BeginChild("Metrics");
		metricsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
}

void Machine::stateControlGui() {
	glm::vec2 buttonSize;
	int buttonCount = 2;
	buttonSize.x = (ImGui::GetContentRegionAvail().x - (buttonCount - 1) * ImGui::GetStyle().ItemSpacing.x) / buttonCount;
	buttonSize.y = ImGui::GetTextLineHeight() * 2.0;

	bool readyToEnable = isReady();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Machine Enabled", buttonSize);
	}
	else if (readyToEnable) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Machine Ready", buttonSize);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Machine Not Ready", buttonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();

	ImGui::SameLine();
	ImGui::BeginDisabled(!readyToEnable);
	if (isEnabled()) {
		if (ImGui::Button("Disable Machine", buttonSize)) disable();
	}
	else {
		if (ImGui::Button("Enable Machine", buttonSize)) enable();
	}
	ImGui::EndDisabled();
}

void Machine::generalSettingsGui() {
	ImGui::Text("Machine Name :");
	int nameBufferSize;
	char* nameBuffer = getNameBuffer(nameBufferSize);
	ImGui::InputText("##name", nameBuffer, nameBufferSize);

	ImGui::Text("Short Name :");
	ImGui::InputText("##shortName", shortName, 16);
}

void Machine::miniatureGui() {

	glm::vec2 miniatureSize(getMiniatureWidth(), ImGui::GetTextLineHeight() * 20.0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetTextLineHeight() * 0.2);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::almostBlack);
	if (ImGui::BeginChild(getName(), miniatureSize, true)) {

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0, -ImGui::GetTextLineHeight() * 0.2));

		float availableWidth = ImGui::GetContentRegionAvail().x;
		ImGui::PushFont(Fonts::robotoBold20);
		float bigNameWidth = ImGui::CalcTextSize(getShortName()).x;
		ImGui::SameLine((availableWidth - bigNameWidth) * 0.5 + ImGui::GetStyle().WindowPadding.x);
		ImGui::Text("%s", getShortName());
		ImGui::PopFont();

		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.3));

		ImGui::NewLine();
		ImGui::PushFont(Fonts::robotoBold15);
		float smallNameWidth = ImGui::CalcTextSize(getName()).x;
		ImGui::SameLine((availableWidth - smallNameWidth) * 0.5 + ImGui::GetStyle().WindowPadding.x);
		ImGui::Text("%s", getName());
		ImGui::PopFont();

		float bottomSectionHeight = ImGui::GetTextLineHeight() * 1.8;
		glm::vec2 miniatureContentSize = ImGui::GetContentRegionAvail();
		miniatureContentSize.y -= bottomSectionHeight;

		if (ImGui::BeginChild(getName(), miniatureContentSize, false)) {
			machineSpecificMiniatureGui();
		}
		ImGui::EndChild();

		glm::vec2 singleButtonSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5);
		if (isEnabled()) {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			if (ImGui::Button("Disable", singleButtonSize)) {
				disable();
			}
		}
		else if (isReady()) {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
			if (ImGui::Button("Enable", singleButtonSize)) {
				enable();
			}
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::Button("Not Ready", singleButtonSize);
			ImGui::PopItemFlag();
		}
		ImGui::PopStyleColor();

		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

}
