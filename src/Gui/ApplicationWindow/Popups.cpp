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

namespace Gui{


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



void aboutPopup() {
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

}
