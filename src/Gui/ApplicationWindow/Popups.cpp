#include <pch.h>

#include "Gui.h"
#include <imgui.h>
#include <GLFW/glfw3.h>

#include "ApplicationWindow.h"
#include "Project/Project.h"
#include "Environnement/Environnement.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Project/ProjectGui.h"
#include "Gui/Environnement/EnvironnementGui.h"

namespace Gui{

	void popups(){
		if (isAboutPopupOpenRequested()) ImGui::OpenPopup("About");
		Gui::aboutPopup();
		
		if (ApplicationWindow::isQuitRequested()) ImGui::OpenPopup("Quitting Application");
		Gui::quitApplicationPopup();

		if(Project::isNewProjectRequested()) ImGui::OpenPopup("Closing Current Project");
		Project::Gui::closePopup();
		
		if(Environnement::isEditorUnlockRequested()) ImGui::OpenPopup("Unlock Environnement Editor");
		Environnement::Gui::unlockEditorPopup();
	}

	void quitApplicationPopup() {
		glm::vec2 center = ImGui::GetMainViewport()->GetCenter();
		bool quitApplication = false;
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Quitting Application", nullptr, popupFlags)) {
			ImGui::Text("Do you really want to exit the application ?");
			ImGui::Text("Proceeding will stop motion and discard any unsaved changes");
			if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) {
				ApplicationWindow::cancelQuitRequest();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Quit without Saving")) {
				quitApplication = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2, 0.5, 0.0, 1.0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2, 0.4, 0.1, 1.0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.6, 0.2, 1.0));
			if (ImGui::Button("Save and Quit") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
				quitApplication = Project::Gui::save();
				if(!quitApplication) {
					ApplicationWindow::cancelQuitRequest();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::PopStyleColor(3);
			ImGui::EndPopup();
		}
		if (quitApplication) {
			ApplicationWindow::quit();
		}
	}


	bool b_aboutPopupOpenRequested = false;

	void openAboutPopup(){ b_aboutPopupOpenRequested = true; }

	bool isAboutPopupOpenRequested(){ return b_aboutPopupOpenRequested; }

	void aboutPopup() {
	   ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	   ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	   ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	   if (ImGui::BeginPopupModal("About", nullptr, popupFlags)) {
		   ImGui::PushFont(Fonts::sansBold42);
			   ImGui::Text("Stacato");
			   ImGui::PopFont();
			   ImGui::PushFont(Fonts::sansBold20);
			   ImGui::Text("Stage Control Automation Toolbox");
			   ImGui::PopFont();
			   ImGui::Text("Leo Becker - L'Atelier Artefact - 2021");
		   ImGui::Separator();
		   if (ImGui::Button("Close") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
			   b_aboutPopupOpenRequested = false;
			   ImGui::CloseCurrentPopup();
		   }
		   ImGui::EndPopup();
	   }
   }

}
