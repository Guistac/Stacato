#include <pch.h>

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Environnement/Environnement.h"
#include "Utilities/ProgressIndicator.h"

#include "Gui/Utilities/CustomWidgets.h"

#include <GLFW/glfw3.h>

#include "EtherCatGui.h"

glm::vec2 EtherCatStartPopup::getSize(){
	return glm::vec2(ImGui::GetTextLineHeight() * 30.0, .0f);
}

void EtherCatStartPopup::onDraw(){
	ProgressIndicator& progress = EtherCatFieldbus::startupProgress;
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("EtherCAT Device States:");
	ImGui::PopFont();

	if(ImGui::BeginChild("deviceView", ImVec2(ImGui::GetContentRegionAvail().x, 0.0), ImGuiChildFlags_AutoResizeY)){
		float fieldPadding = ImGui::GetStyle().FramePadding.x;
		float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
		float windowMaxX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		float previousButtonX = 0.0;
		
		for(auto device : Environnement::getEtherCatDevices()){
			float fieldWidth = ImGui::CalcTextSize(device->getName()).x + fieldPadding * 2.0 + itemSpacing;
			if(previousButtonX + fieldWidth < windowMaxX && previousButtonX != 0.0) ImGui::SameLine();
			backgroundText(device->getName(), device->getEtherCatStateColor());
			previousButtonX = ImGui::GetItemRectMax().x;

			if(ImGui::IsItemHovered()){
				if(ImGui::BeginTooltip()){
					ImGui::PushFont(Fonts::sansBold15);
					ImGui::Text("%s", device->getEtherCatStateChar());
					ImGui::PopFont();
					ImGui::EndTooltip();
				}
			}
		}

		ImGui::EndChild();
	}

	ImGui::Separator();

	ImVec2 progressBarSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
	
	if(progress.succeeded()){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		ImGui::ProgressBar(1.0, progressBarSize);
		ImGui::PopStyleColor();
		ImGui::Text("%s", progress.getProgressString());
		if(progress.getTimeSinceCompletion() > 0.5) close();
	}else if(progress.failed()){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		ImGui::ProgressBar(1.0, progressBarSize, "");
		ImGui::PopStyleColor();
		ImGui::Text("%s", progress.getProgressString());
	}else{
		progress.updateSmoothProgress();
		ImGui::ProgressBar(progress.progressSmooth, progressBarSize);
		ImGui::Text("%s", progress.getProgressString());
	}

	
	if (ImGui::Button("Cancel")) {
		EtherCatFieldbus::stop();
		close();
	}
	
	if(progress.failed()){
		ImGui::SameLine();
		if (ImGui::Button("Retry")) Environnement::start();
	}
	
	if(ImGui::IsKeyPressed(ImGuiKey_Escape)){
		Environnement::stop();
		close();
	}
}
