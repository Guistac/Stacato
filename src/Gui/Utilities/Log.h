#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Utilities/Logger.h"

namespace Utilies::Gui{

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
		ImGui::BeginChild(ImGui::GetID("LogMessages"));

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

			size_t messageCount = 0;
			for (int i = 0; i < 6; i++) {
				if (i >= level) messageCount += Logger::getLevelMessageCount()[i];
			}
			static size_t previousMessageCount = 0;

			if (messageCount > 0) {
				
				for (Logger::Message& message : Logger::getMessages()) {
					if (message.level >= level) {
						if (message.isTrace())			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
						else if (message.isDebug())		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 1.0f, 1.0f));
						else if (message.isInfo())		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
						else if (message.isWarn())		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.3f, 1.0f));
						else if (message.isError())		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
						else if (message.isCritical())	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 1.0f, 1.0f));
						const char* string = message.getString();
						ImGui::Text("%s", string);
						ImGui::PopStyleColor();
					}
				}
			}

			ImGui::PopStyleVar();
			if(previousMessageCount != messageCount && b_autoScroll) ImGui::SetScrollHereY(1.0f);
			previousMessageCount = messageCount;
		
			ImGui::EndChild();
		
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

}
