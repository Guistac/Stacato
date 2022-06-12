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

#include "Layout.h"

#include "Gui/Fieldbus/EtherCatGui.h"

#include "Gui/Assets/Images.h"

namespace Gui{

	void QuitApplicationPopup::drawContent(){
		ImGui::Text("Do you really want to exit the application ?");
		ImGui::Text("Proceeding will stop motion and discard any unsaved changes");
		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) close();
		ImGui::SameLine();
		if (ImGui::Button("Quit without Saving")) {
			close();
			ApplicationWindow::quit();
		}
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2, 0.5, 0.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2, 0.4, 0.1, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.6, 0.2, 1.0));
		if (ImGui::Button("Save and Quit") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
			if(Project::Gui::save()) ApplicationWindow::quit();
			close();
		}
		ImGui::PopStyleColor(3);
	}

	void AboutPopup::drawContent(){
				
		float iconSize = ImGui::GetTextLineHeight() * 7.0;
		float textSize = Fonts::sansBold42->FontSize + Fonts::sansBold20->FontSize + ImGui::GetTextLineHeight() + 2.0 * ImGui::GetStyle().ItemSpacing.y;
		
		glm::vec2 cursorPos = ImGui::GetCursorPos();
		ImGui::Image(Images::StacatoIcon.getID(), glm::vec2(iconSize));
		ImGui::SameLine();
		
		ImGui::BeginGroup();
		ImGui::SetCursorPosY(cursorPos.y + iconSize - textSize);
		
		ImGui::PushFont(Fonts::sansBold42);
		ImGui::Text("Stacato");
		ImGui::PopFont();
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Stage Control Automation Toolbox");
		ImGui::PopFont();
		ImGui::Text("Leo Becker - L'Atelier Artefact - 2021");
		ImGui::EndGroup();
	}
	
}
