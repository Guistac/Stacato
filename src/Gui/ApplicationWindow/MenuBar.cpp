#include <pch.h>

#include "ApplicationWindow.h"

#include <imgui.h>
#include <implot.h>
#include <GLFW/glfw3.h>

#include "Gui.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Project/ProjectGui.h"
#include "Gui/Environnement/EnvironnementGui.h"

#include "Environnement/Environnement.h"
#include "Plot/Plot.h"
#include "Project/Project.h"

namespace Gui {

	bool imguiDemoWindowOpen = false;
	bool imguiMetricsWindowOpen = false;
	bool implotDemoWindowOpen = false;



	void menuBar() {

		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("Stacato")) {
			if (ImGui::MenuItem("About")) Gui::openAboutPopup();
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "Cmd Q")) ApplicationWindow::requestQuit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("File")) {
			
			if (ImGui::MenuItem("New Project", "Cmd N")) Project::createNew();
			if (ImGui::MenuItem("Open Project...", "Cmd O")) Project::Gui::load();
			
			ImGui::Separator();
			 
			ImGui::BeginDisabled(!Project::hasFilePath());
			if (ImGui::MenuItem("Save", "Cmd S")) Project::save();
			ImGui::EndDisabled();
			
			if (ImGui::MenuItem("Save As...", "Cmd Shift S")) Project::Gui::saveAs();
			 
			ImGui::Separator();
			
			ImGui::BeginDisabled(!Project::hasFilePath());
			if (ImGui::MenuItem("Reload Saved", "Cmd Shift R")) Project::reloadSaved();
			ImGui::EndDisabled();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if(Environnement::isEditorLocked()) {if(ImGui::MenuItem("Show Environnement Editor", "Cmd Shift U")) Environnement::requestEditorUnlock();}
			else if(ImGui::MenuItem("Hide Environnement Editor", "Cmd Shift U")) Environnement::lockEditor();
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("View")){
			if(ImGui::MenuItem("Reset Layout")) Gui::resetDefaultLayout();
			ImGui::EndMenu();
		}
		
		
		if (ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT) && ImGui::IsKeyDown(GLFW_KEY_LEFT_SUPER)) {
			if (ImGui::BeginMenu("Utilities")) {
				if (ImGui::MenuItem("ImGui Demo Window", nullptr, &imguiDemoWindowOpen))		imguiDemoWindowOpen = true;
				if (ImGui::MenuItem("ImGui Metrics Window", nullptr, &imguiMetricsWindowOpen))	imguiMetricsWindowOpen = true;
				if (ImGui::MenuItem("ImPlot Demo Window", nullptr, &implotDemoWindowOpen))		implotDemoWindowOpen = true;
				ImGui::EndMenu();
			}
		}
		ImGui::EndMenuBar();
		
		
		
		//Keyboard ShortCuts
		if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SUPER) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SUPER)){
			if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT)){
				if(ImGui::IsKeyPressed(GLFW_KEY_S)) Project::Gui::saveAs();
				else if(ImGui::IsKeyPressed(GLFW_KEY_R)) Project::reloadSaved();
				else if(ImGui::IsKeyPressed(GLFW_KEY_U)) {
					if(Environnement::isEditorLocked()) Environnement::requestEditorUnlock();
					else Environnement::lockEditor();
				}
			}else{
				if(ImGui::IsKeyPressed(GLFW_KEY_Q)) ApplicationWindow::requestQuit();
				else if(ImGui::IsKeyPressed(GLFW_KEY_N)) Project::createNew();
				else if(ImGui::IsKeyPressed(GLFW_KEY_O)) Project::Gui::load();
				else if(ImGui::IsKeyPressed(GLFW_KEY_S)) Project::Gui::save();
			}
		}

		
		//utility windows
		if (imguiDemoWindowOpen) ImGui::ShowDemoWindow(&imguiDemoWindowOpen);
		if (imguiMetricsWindowOpen) ImGui::ShowMetricsWindow(&imguiMetricsWindowOpen);
		if (implotDemoWindowOpen) ImPlot::ShowDemoWindow(&implotDemoWindowOpen);

	}


}
