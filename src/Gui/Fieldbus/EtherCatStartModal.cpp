#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Utilities/ProgressIndicator.h"

void etherCatStartModal() {
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5, 0.5));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetTextLineHeight() * 30.0, 0), ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Starting EtherCAT Fieldbus")) {
		
		ProgressIndicator& progress = EtherCatFieldbus::startupProgress;
		
		if(progress.b_finished && progress.b_succeeded){
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3, 0.8, 0.0, 1.0));
			ImGui::ProgressBar(1.0);
			ImGui::PopStyleColor();
			ImGui::Text("%s", progress.getProgressString());
			if(progress.getTimeSinceCompletion() > 0.5) ImGui::CloseCurrentPopup();
		}else if(progress.b_finished && !progress.b_succeeded){
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7, 0.0, 0.0, 1.0));
			ImGui::ProgressBar(1.0);
			ImGui::PopStyleColor();
			ImGui::Text("%s", progress.getProgressString());
		}else{
			ImGui::ProgressBar(progress.progress);
			ImGui::Text("%s", progress.getProgressString());
		}
	
		if(progress.failed()){
			if (ImGui::Button("Retry")) EtherCatFieldbus::start();
		}else{
			bool disableCancelButton = EtherCatFieldbus::isCyclicExchangeStarting();
			if (disableCancelButton) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::Button("Cancel")) {
				EtherCatFieldbus::stop();
				ImGui::CloseCurrentPopup();
			}
			if (disableCancelButton) END_DISABLE_IMGUI_ELEMENT
		}
		 
		ImGui::EndPopup();
	}
}
