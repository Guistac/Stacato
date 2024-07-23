#include <pch.h>

#include <imgui.h>
#include <GLFW/glfw3.h>

#include "EnvironnementGui.h"
#include "Gui/Fieldbus/EtherCatGui.h"
#include "Gui/Utilities/Log.h"

#include "Environnement/Environnement.h"

#include "Environnement/NodeGraph/NodeGraph.h"

#include "Visualizer/Visualizer.h"

#include "Environnement/EnvironnementScript.h"

#include "Stacato/StacatoEditor.h"


namespace Environnement::Gui{

	void VisualizerScriptWindow::onDraw() {
		StageVisualizer::editor(ImGui::GetContentRegionAvail());
	}

	void EtherCATWindow::onDraw() {
		etherCatGui();
	}

	void ScriptEditorWindow::onDraw(){
		ImGui::BeginDisabled(!Script::canCompile());
		if(ImGui::Button("Compile")) Script::compile();
		ImGui::SameLine();
		if(ImGui::Button("Reset")) Script::reset();
		ImGui::EndDisabled();
		Script::editor(ImGui::GetContentRegionAvail());
	}
	
	void LogWindow::onDraw() {
		Utilies::Gui::log();
	}

	void VisualizerWindow::onDraw() {
		StageVisualizer::canvas(ImGui::GetContentRegionAvail(), 1.0, ImGui::GetStyle().FrameRounding);
	}

	void UnlockEditorPopup::onDraw(){
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
		
		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			snprintf(passwordBuffer, 256, "");
			Stacato::Editor::lock();
			close();
		}
		ImGui::SameLine();
		
		if (ImGui::Button("Enter") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
			std::string pass = passwordBuffer;
			if(Stacato::Editor::checkEditorPassword(pass)){
				Stacato::Editor::unlock();
				snprintf(passwordBuffer, 256, "");
				close();
			}else{
				wrongInputTime_seconds = Timing::getProgramTime_seconds();
			}
		}
	}

}
