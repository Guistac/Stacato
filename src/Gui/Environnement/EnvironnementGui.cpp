#include <pch.h>

#include <imgui.h>
#include <GLFW/glfw3.h>

#include "EnvironnementGui.h"
#include "Gui/Fieldbus/EtherCatGui.h"
#include "Gui/Utilities/Log.h"

#include "Environnement/Environnement.h"

#include "Environnement/NodeGraph/NodeGraph.h"

namespace Environnement::Gui{

	void gui() {
		if (ImGui::BeginTabBar("##EnvironnementTabBar")) {
			if (ImGui::BeginTabItem("Editor")) {
				Environnement::Gui::editor();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Node Manager")){
				nodeManager();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Stage Editor")) {
				stageEditor();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Fieldbus")) {
				etherCatGui();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Log")) {
				Utilies::Gui::log();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	void unlockEditorPopup(){
		glm::vec2 center = ImGui::GetMainViewport()->GetCenter();
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
		
			if(ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
			ImGui::InputText("##password", passwordBuffer, 256, ImGuiInputTextFlags_Password);
			ImGui::PopStyleColor();
			
			if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) {
				sprintf(passwordBuffer, "");
				Environnement::lockEditor();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Enter") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
				if(Environnement::checkEditorPassword(passwordBuffer)){
					Environnement::confirmEditorUnlock();
					sprintf(passwordBuffer, "");
					ImGui::CloseCurrentPopup();
				}else{
					wrongInputTime_seconds = Timing::getProgramTime_seconds();
				}
			}
			
			ImGui::EndPopup();
		}
	}

}
