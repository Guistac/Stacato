#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "ApplicationWindow.h"

#include "Gui/Project/ProjectGui.h"
#include "Project/Environnement.h"
#include "Gui/StageView/StageView.h"

namespace ApplicationWindow {

	void drawGui() {

		//=== Draw Main Menu Bar ===
		drawMainMenuBar();

		//=== Define Bottom Toolbar and main window height
		float toolbarHeight = ImGui::GetTextLineHeight() * 3.2;
		glm::vec2 mainWindowPosition = ImGui::GetMainViewport()->WorkPos;
		glm::vec2 mainWindowSize = ImGui::GetMainViewport()->WorkSize;
		mainWindowSize.y -= toolbarHeight;

		//=== Draw Main Window ===
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
		ImGui::SetNextWindowPos(mainWindowPosition);
		ImGui::SetNextWindowSize(mainWindowSize);
		ImGui::Begin("Main Window", nullptr, windowFlags);
		drawMainWindow();
		ImGui::End();

		//=== Draw Bottom Toolbar ===
		ImGui::SetNextWindowPos(mainWindowPosition + glm::vec2(0, mainWindowSize.y));
		ImGui::SetNextWindowSize(glm::vec2(mainWindowSize.x, toolbarHeight));
		ImGui::Begin("##Toolbar", nullptr, windowFlags);
		drawToolbar(toolbarHeight);
		ImGui::End();

	}


	void drawMainWindow() {
		ImGui::PushStyleColor(ImGuiCol_TabActive, glm::vec4(0.6, 0.4, 0.0, 1.0));
		if (ImGui::BeginTabBar("MainTabBar")) {
			if(!Environnement::isEditorHidden()){
				if (ImGui::BeginTabItem("Environnement")) {
					environnementGui();
					ImGui::EndTabItem();
				}
			}
			if (ImGui::BeginTabItem("Machines")) {
                ImGui::BeginChild("Machine");
				machineListGui();
                ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Plot")) {
				plotGui();
				ImGui::EndTabItem();
			}
			if(!Environnement::isEditorHidden()){
				if (ImGui::BeginTabItem("Stage View")) {
					StageView::draw();
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
		ImGui::PopStyleColor();
	}

}
