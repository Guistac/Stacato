#include <pch.h>

#include "HelpMarker.h"
#include <imgui.h>
#include "Gui/Framework/Colors.h"

bool beginHelpMarker(const char* displayText) {
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::Text(displayText);
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered()) {
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetTextLineHeight() * 20.0, 0));
		ImGui::BeginTooltip();
		return true;
	}
	return false;
}

void endHelpMarker() {
	ImGui::EndTooltip();
}

bool beginHelpTooltip() {
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetTextLineHeight() * 20.0, 0));
	ImGui::BeginTooltip();
	return true;
}

void endHelpTooltip() {
	ImGui::EndTooltip();
}