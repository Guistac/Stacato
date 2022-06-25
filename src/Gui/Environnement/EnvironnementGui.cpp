#include <pch.h>

#include <imgui.h>
#include <GLFW/glfw3.h>

#include "EnvironnementGui.h"
#include "Gui/Fieldbus/EtherCatGui.h"
#include "Gui/Utilities/Log.h"

#include "Environnement/Environnement.h"

#include "Environnement/NodeGraph/NodeGraph.h"

#include "Visualizer/Visualizer.h"


namespace Environnement::Gui{

	void VisualizerScriptWindow::drawContent() {
		StageVisualizer::editor(ImGui::GetContentRegionAvail());
	}

	void EtherCATWindow::drawContent() {
		etherCatGui();
	}
	
	void LogWindow::drawContent() {
		Utilies::Gui::log();
	}

	void VisualizerWindow::drawContent() {
		StageVisualizer::canvas(ImGui::GetContentRegionAvail(), 1.0, ImGui::GetStyle().FrameRounding);
	}

	void UnlockEditorPopup::drawContent(){
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
			close();
		}
		ImGui::SameLine();
		
		if (ImGui::Button("Enter") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
			if(Environnement::checkEditorPassword(passwordBuffer)){
				Environnement::unlockEditor();
				sprintf(passwordBuffer, "");
				close();
			}else{
				wrongInputTime_seconds = Timing::getProgramTime_seconds();
			}
		}
	}

}
