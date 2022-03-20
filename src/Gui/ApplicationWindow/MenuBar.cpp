#include <pch.h>

#include "ApplicationWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include <GLFW/glfw3.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Project/Environnement.h"
#include "Project/Plot.h"
#include "Gui/Utilities/FileDialog.h"
#include "Project/Project.h"

namespace ApplicationWindow {

	bool imguiDemoWindowOpen = false;
	bool imguiMetricsWindowOpen = false;
	bool implotDemoWindowOpen = false;

	void unlockEnvironnementEditorModal();

	void drawMainMenuBar() {

		bool openImguiDemoWindow = false;
		bool openImguiMetricsWindow = false;
		bool openImplotDemoWindow = false;
		bool openAboutPopup = false;
		bool b_openUnlockEditorModal = false;

		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("Stacato")) {
			if (ImGui::MenuItem("About")) openAboutPopup = true;
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) ApplicationWindow::requestClose();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("File")) {
			
			if (ImGui::MenuItem("New Project")) Project::createNew();
			if (ImGui::MenuItem("Open Project...")) {
				FileDialog::FilePath path;
				FileDialog::FileTypeFilter filter("Stacato Project File", "stacato");
				if (FileDialog::load(path, filter)) {
					Project::load(path.path);
				}
			}
			
			ImGui::Separator();
			 
			ImGui::BeginDisabled(!Project::b_hasFilePath);
			if (ImGui::MenuItem("Save")) Project::save();
			ImGui::EndDisabled();
			
			if (ImGui::MenuItem("Save As...")) {
				FileDialog::FilePath path;
				FileDialog::FileTypeFilter filter("Stacato Project File", "stacato");
				if (FileDialog::save(path, filter, "project")) {
					Project::saveAs(path.path);
				}
			}
			 
			ImGui::Separator();
			
			ImGui::BeginDisabled(!Project::b_hasFilePath);
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

		
		if (openAboutPopup) ImGui::OpenPopup("About");
		aboutModal();

		if(b_openUnlockEditorModal) ImGui::OpenPopup("Unlock Environnement Editor");
		unlockEnvironnementEditorModal();
		
		if (ApplicationWindow::isCloseRequested()) ImGui::OpenPopup("Quitting Application");
		quitApplicationModal();

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

	void quitApplicationModal() {
		glm::vec2 center = ImGui::GetMainViewport()->GetCenter();
		bool quitApplication = false;
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Quitting Application", nullptr, popupFlags)) {
			ImGui::Text("Do you really want to exit the application ?");
			ImGui::Text("Proceeding will stop motion and discard any unsaved changes");
			if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) {
				ApplicationWindow::cancelCloseRequest();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Quit without Saving")) quitApplication = true;
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2, 0.5, 0.0, 1.0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2, 0.4, 0.1, 1.0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.6, 0.2, 1.0));
			if (ImGui::Button("Save and Quit") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
                quitApplication = Project::save();
			}
			ImGui::PopStyleColor(3);
			ImGui::EndPopup();
		}
		if (quitApplication) {
			ApplicationWindow::close();
		}
	}

	void aboutModal() {
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("About", nullptr, popupFlags)) {
			ImGui::PushFont(Fonts::robotoBold42);
				ImGui::Text("Stacato");
				ImGui::PopFont();
				ImGui::PushFont(Fonts::robotoBold20);
				ImGui::Text("Stage Control Automation Toolbox");
				ImGui::PopFont();
				ImGui::Text("Leo Becker - L'Atelier Artefact - 2021");
			ImGui::Separator();
			if (ImGui::Button("Close") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

	void unlockEnvironnementEditorModal(){
		glm::vec2 center = ImGui::GetMainViewport()->GetCenter();
		bool quitApplication = false;
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Unlock Environnement Editor", nullptr, popupFlags)) {
			ImGui::Text("Enter password to unlock the environnement editor.\n"
						"Only proceed if you know what you're doing.");
			
			glm::vec4 inputFieldColor;
			static double wrongInputTime_seconds = 0;
			double timeSinceWrongInput = Timing::getProgramTime_seconds() - wrongInputTime_seconds;
			if(timeSinceWrongInput < 0.5 && fmod(timeSinceWrongInput, 0.2) < 0.1) inputFieldColor = Colors::red;
			else inputFieldColor = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
			
			static char passwordBuffer[256];
			ImGui::PushStyleColor(ImGuiCol_FrameBg, inputFieldColor);
			ImGui::InputText("##password", passwordBuffer, 256, ImGuiInputTextFlags_Password);
			ImGui::PopStyleColor();
			
			if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) {
				sprintf(passwordBuffer, "");
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Enter") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
				if(Environnement::checkEditorPassword(passwordBuffer)){
					Environnement::showEditor();
					sprintf(passwordBuffer, "");
					ImGui::CloseCurrentPopup();
				}else{
					wrongInputTime_seconds = Timing::getProgramTime_seconds();
				}
			}
			ImGui::EndPopup();
		}
		if (quitApplication) {
			ApplicationWindow::close();
		}
	}

}
