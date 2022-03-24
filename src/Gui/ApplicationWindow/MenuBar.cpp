#include <pch.h>

#include "ApplicationWindow.h"

#include <imgui.h>
#include <implot.h>
#include <GLFW/glfw3.h>

#include "Gui.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Project/ProjectGui.h"

#include "Environnement/Environnement.h"
#include "Plot/Plot.h"
#include "Project/Project.h"


namespace Gui {

	bool imguiDemoWindowOpen = false;
	bool imguiMetricsWindowOpen = false;
	bool implotDemoWindowOpen = false;



	void menuBar() {

		bool openImguiDemoWindow = false;
		bool openImguiMetricsWindow = false;
		bool openImplotDemoWindow = false;
		bool openAboutPopup = false;
		bool b_openUnlockEditorModal = false;

		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("Stacato")) {
			if (ImGui::MenuItem("About")) openAboutPopup = true;
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) ApplicationWindow::requestQuit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("File")) {
			
			if (ImGui::MenuItem("New Project")) Project::createNew();
			if (ImGui::MenuItem("Open Project...")) Project::Gui::load();
			
			ImGui::Separator();
			 
			ImGui::BeginDisabled(!Project::hasFilePath());
			if (ImGui::MenuItem("Save")) Project::save();
			ImGui::EndDisabled();
			
			if (ImGui::MenuItem("Save As...")) Project::Gui::saveAs();
			 
			ImGui::Separator();
			
			ImGui::BeginDisabled(!Project::hasFilePath());
			if (ImGui::MenuItem("Reload Saved")) Project::reloadSaved();
			ImGui::EndDisabled();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			/*
			BEGIN_DISABLE_IMGUI_ELEMENT
				static char currentPlotString[256];
			sprintf(currentPlotString, "Current Plot: %s", Project::currentPlot->name);
			ImGui::MenuItem(currentPlotString);
			END_DISABLE_IMGUI_ELEMENT

				if (ImGui::BeginMenu("Plots")) {
					for (auto plot : Project::plots) {
						if (ImGui::MenuItem(plot->name, nullptr, plot == Project::currentPlot)) {
							Project::currentPlot = plot;
						}
					}
					ImGui::EndMenu();
				}
			
			ImGui::Separator();
			*/
			if(Environnement::isEditorHidden()){
				if(ImGui::MenuItem("Show Environnement Editor")) b_openUnlockEditorModal = true;
			}
			else {
				if(ImGui::MenuItem("Hide Environnement Editor")) Environnement::hideEditor();
			}
			
			ImGui::EndMenu();
		}
		if (ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT) && ImGui::IsKeyDown(GLFW_KEY_LEFT_SUPER)) {
			if (ImGui::BeginMenu("Utilities")) {
				if (ImGui::MenuItem("ImGui Demo Window", nullptr, &imguiDemoWindowOpen))		openImguiDemoWindow = true;
				if (ImGui::MenuItem("ImGui Metrics Window", nullptr, &imguiMetricsWindowOpen))	openImguiMetricsWindow = true;
				if (ImGui::MenuItem("ImPlot Demo Window", nullptr, &implotDemoWindowOpen))		openImplotDemoWindow = true;
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();

		
		
		//display popup windows
		if (openAboutPopup) ImGui::OpenPopup("About");
		Gui::aboutModal();

		if(b_openUnlockEditorModal) ImGui::OpenPopup("Unlock Environnement Editor");
		unlockEnvironnementEditorModal();
		
		if (ApplicationWindow::isQuitRequested()) ImGui::OpenPopup("Quitting Application");
		Gui::quitApplicationModal();

		if(Project::isNewProjectRequested()) ImGui::OpenPopup("Closing Current Project");
		Project::Gui::closePopup();
		
		
		
		
		//utility windows
		if (openImguiDemoWindow) {
			ImGui::SetNextWindowFocus();
			imguiDemoWindowOpen = true;
		}
		if (imguiDemoWindowOpen) ImGui::ShowDemoWindow(&imguiDemoWindowOpen);

		if (openImguiMetricsWindow) {
			ImGui::SetNextWindowFocus();
			imguiMetricsWindowOpen = true;
		}
		if (imguiMetricsWindowOpen) ImGui::ShowMetricsWindow(&imguiMetricsWindowOpen);

		if (openImplotDemoWindow) {
			ImGui::SetNextWindowFocus();
			implotDemoWindowOpen = true;
		}
		if (implotDemoWindowOpen) ImPlot::ShowDemoWindow(&implotDemoWindowOpen);

	}


}
