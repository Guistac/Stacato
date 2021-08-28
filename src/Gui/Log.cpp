#include <pch.h>

#include "Gui.h"

#include <imgui.h>

void log() {

	static int level = 2;
	static const char* levels[6] = {
		"Trace",
		"Debug",
		"Info",
		"Warn",
		"Error",
		"Critical"
	};

	if (ImGui::BeginCombo("Log Level", levels[level])) {
		for (int i = 0; i < 6; i++) {
			bool selected = i == level;
			if (ImGui::Selectable(levels[i], &selected)) level = i;
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	static bool b_autoScroll = true;
	ImGui::Checkbox("Auto-Scroll", &b_autoScroll);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0, 0.0, 0.0, 1.0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if(ImGui::BeginChild(ImGui::GetID("LogMessages"))){

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		size_t messageCount = 0;
		for (int i = 0; i < 6; i++) { 
			if (i >= level) messageCount += Logger::levelMessageCount[i];
		}
		static size_t previousMessageCount = 0;

		if (messageCount > 0) {
			
			for (int i = 0; i < Logger::messages.size(); i++) {
				if (Logger::messages[i].level >= level) {
					if (Logger::messages[i].isTrace())			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					else if (Logger::messages[i].isDebug())		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 1.0f, 1.0f));
					else if (Logger::messages[i].isInfo())		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
					else if (Logger::messages[i].isWarn())		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.3f, 1.0f));
					else if (Logger::messages[i].isError())		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
					else if (Logger::messages[i].isCritical())	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 1.0f, 1.0f));
                    const char* message = Logger::messages[i].getMessage();
                    ImGui::Text("%s", message);
					ImGui::PopStyleColor();
				}
			}
		}

		ImGui::PopStyleVar();
		if(previousMessageCount != messageCount && b_autoScroll) ImGui::SetScrollHereY(1.0f);
		previousMessageCount = messageCount;
		ImGui::EndChild();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}
