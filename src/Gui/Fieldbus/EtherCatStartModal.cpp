#include <pch.h>

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Environnement/Environnement.h"
#include "Utilities/ProgressIndicator.h"

#include <GLFW/glfw3.h>

bool b_openEtherCatStartModal = false;

void openEtherCatStartModal(){
	b_openEtherCatStartModal = true;
}

void etherCatStartModal() {
	
	if(b_openEtherCatStartModal) {
		ImGui::OpenPopup("Starting Environnement");
		b_openEtherCatStartModal = false;
	}
	
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5, 0.5));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetTextLineHeight() * 30.0, 0), ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Starting Environnement")) {
		
		ProgressIndicator& progress = EtherCatFieldbus::startupProgress;
		
		if(progress.succeeded()){
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
			ImGui::ProgressBar(1.0);
			ImGui::PopStyleColor();
			ImGui::Text("%s", progress.getProgressString());
			if(progress.getTimeSinceCompletion() > 0.5) ImGui::CloseCurrentPopup();
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
	
		
		bool disableCancelButton = EtherCatFieldbus::isCyclicExchangeStarting();
		ImGui::BeginDisabled(disableCancelButton);
		if (ImGui::Button("Cancel")) {
			EtherCatFieldbus::stop();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		
		if(progress.failed()){
			ImGui::SameLine();
			if (ImGui::Button("Retry")) Environnement::start();
		}
		
		if(!disableCancelButton && ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)){
			Environnement::stop();
			ImGui::CloseCurrentPopup();
		}
			
		ImGui::EndPopup();
	}
}
