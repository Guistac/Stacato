#include <pch.h>

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Environnement/Environnement.h"
#include "Utilities/ProgressIndicator.h"

#include <GLFW/glfw3.h>

#include "EtherCatGui.h"

glm::vec2 EtherCatStartPopup::getSize(){
	return glm::vec2(ImGui::GetTextLineHeight() * 30.0, .0f);
}

void EtherCatStartPopup::drawContent(){
	ProgressIndicator& progress = EtherCatFieldbus::startupProgress;
	
	if(progress.succeeded()){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		ImGui::ProgressBar(1.0);
		ImGui::PopStyleColor();
		ImGui::Text("%s", progress.getProgressString());
		if(progress.getTimeSinceCompletion() > 0.5) close();
	}else if(progress.failed()){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		ImGui::ProgressBar(1.0);
		ImGui::PopStyleColor();
		ImGui::Text("%s", progress.getProgressString());
	}else{
		progress.updateSmoothProgress();
		ImGui::ProgressBar(progress.progressSmooth);
		ImGui::Text("%s", progress.getProgressString());
	}

	
	bool disableCancelButton = !EtherCatFieldbus::canStop();
	ImGui::BeginDisabled(disableCancelButton);
	if (ImGui::Button("Cancel")) {
		EtherCatFieldbus::stop();
		close();
	}
	ImGui::EndDisabled();
	
	if(progress.failed()){
		ImGui::SameLine();
		if (ImGui::Button("Retry")) Environnement::start();
	}
	
	if(!disableCancelButton && ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)){
		Environnement::stop();
		close();
	}
}
