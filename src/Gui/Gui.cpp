#include <pch.h>

#include "Gui.h"

void gui(bool closeWindowRequest) {
	mainMenuBar(closeWindowRequest);

	float toolbarHeight = ImGui::GetTextLineHeight() * 3.2;
	glm::vec2 mainWindowPosition = ImGui::GetMainViewport()->WorkPos;
	glm::vec2 mainWindowSize = ImGui::GetMainViewport()->WorkSize;
	mainWindowSize.y -= toolbarHeight;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
	ImGui::SetNextWindowPos(mainWindowPosition);
	ImGui::SetNextWindowSize(mainWindowSize);
	ImGui::Begin("Main Window", nullptr, windowFlags);
	mainWindow();
	ImGui::End();

	ImGui::SetNextWindowPos(mainWindowPosition + glm::vec2(0, mainWindowSize.y));
	ImGui::SetNextWindowSize(glm::vec2(mainWindowSize.x, toolbarHeight));
	ImGui::Begin("##Toolbar", nullptr, windowFlags);
	toolbar(toolbarHeight);
	ImGui::End();
}

