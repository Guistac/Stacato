#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui.h"

#include "ApplicationWindow.h"

#include "Gui/Project/ProjectGui.h"
#include "Environnement/Environnement.h"
#include "Gui/StageView/StageView.h"

#include "Gui/Environnement/EnvironnementGui.h"
#include "Gui/Plot/PlotGui.h"

namespace Gui {

	void draw() {

		//=== Draw Main Menu Bar ===
		menuBar();

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
		
		ImGui::PushStyleColor(ImGuiCol_TabActive, glm::vec4(0.6, 0.4, 0.0, 1.0));
		if (ImGui::BeginTabBar("MainTabBar")) {
			if(!Environnement::isEditorHidden()){
				if (ImGui::BeginTabItem("Environnement")) {
					Environnement::Gui::gui();
					ImGui::EndTabItem();
				}
			}
			if (ImGui::BeginTabItem("Machines")) {
				ImGui::BeginChild("Machine");
				Environnement::Gui::machineList();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Setup")){
				ImGui::BeginChild("Setup");
				Environnement::Gui::homingAndSetup();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Plot")) {
				PlotGui::editor();
				ImGui::EndTabItem();
			}
			
			//if(!Environnement::isEditorHidden()){
			//	if (ImGui::BeginTabItem("Stage View")) {
			//		StageView::draw();
			//		ImGui::EndTabItem();
			//	}
			//}
			
			ImGui::EndTabBar();
		}
		ImGui::PopStyleColor();
		
		ImGui::End();

		//=== Draw Bottom Toolbar ===
		ImGui::SetNextWindowPos(mainWindowPosition + glm::vec2(0, mainWindowSize.y));
		ImGui::SetNextWindowSize(glm::vec2(mainWindowSize.x, toolbarHeight));
		ImGui::Begin("##Toolbar", nullptr, windowFlags);
		toolbar(toolbarHeight);
		ImGui::End();

	}



}
