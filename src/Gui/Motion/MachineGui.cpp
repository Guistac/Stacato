#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Motion/Machine/Machine.h"
#include "Gui/Framework/Colors.h"
#include "Gui/Framework/Fonts.h"

void Machine::nodeSpecificGui() {
	if (ImGui::BeginTabItem("Controls")) {
		if (ImGui::BeginChild("Controls")) {
			controlsGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Settings")) {
		if (ImGui::BeginChild("Settings")) {
			settingsGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Devices")) {
		if (ImGui::BeginChild("Devices")) {
			axisGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Metrics")) {
		if (ImGui::BeginChild("Metrics")) {
			metricsGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
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
		ImGui::Text(getShortName());
		ImGui::PopFont();

		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.3));

		ImGui::NewLine();
		ImGui::PushFont(Fonts::robotoBold15);
		float smallNameWidth = ImGui::CalcTextSize(getName()).x;
		ImGui::SameLine((availableWidth - smallNameWidth) * 0.5 + ImGui::GetStyle().WindowPadding.x);
		ImGui::Text(getName());
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
