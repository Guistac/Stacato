#include "Gui.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include <iostream>


void etherCatGui() {

	ImGui::Begin("EtherCAT Fieldbus Test Program", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	bool disableButton = EtherCatFieldbus::b_processRunning || EtherCatFieldbus::b_processStarting;
	if (disableButton) {ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0)); }
	if (ImGui::Button("Start")) {
		EtherCatFieldbus::start();
		ImGui::OpenPopup("Starting EtherCAT Fieldbus");
	}
	if (disableButton) { ImGui::PopItemFlag(); ImGui::PopStyleColor(); }
	ImGui::SameLine();

	disableButton = !EtherCatFieldbus::b_processRunning;
	if (disableButton) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0)); }
	if (ImGui::Button("Stop")) EtherCatFieldbus::stop();
	if (disableButton) { ImGui::PopItemFlag(); ImGui::PopStyleColor(); }
	
	ImGui::Separator();

	if (ImGui::BeginTabBar("MainTabBar")) {
		if (ImGui::BeginTabItem("EtherCAT Slaves")) {
			etherCatSlaves();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Fieldbus Metrics")) {
			etherCatMetrics();
			ImGui::EndTabItem();
		}
		static bool isTabOpen = false;
		bool wasTabOpen = isTabOpen;
		if (ImGui::BeginTabItem("Fieldbus Parameters")) {
			isTabOpen = true;
			etherCatParameters(isTabOpen && !wasTabOpen);
			ImGui::EndTabItem();
		}
		else {
			isTabOpen = false;
		}

		ImGui::EndTabBar();
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5, 0.5));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetTextLineHeight() * 30.0, 0), ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Starting EtherCAT Fieldbus")) {
		etherCatFieldbusConfigurationModal();
		ImGui::EndPopup();
	}

	
	ImGui::End();
}

void etherCatFieldbusConfigurationModal() {
	static bool firstSuccess;
	static long long successTime_milliseconds;
	static long long successCloseDelay_milliseconds = 1000;
	if (EtherCatFieldbus::b_clockStable && EtherCatFieldbus::b_allOperational) {
		//display a full progress bar and close the popup after some time (1 second)
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3, 0.8, 0.0, 1.0));
		ImGui::ProgressBar(1.0);
		ImGui::PopStyleColor();
		ImGui::Text(EtherCatFieldbus::configurationStatus);
		using namespace std::chrono;
		if (firstSuccess) {
			firstSuccess = false;
			successTime_milliseconds = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		}
		else if (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - successTime_milliseconds > successCloseDelay_milliseconds) {
			ImGui::CloseCurrentPopup();
		}
	}
	else if (EtherCatFieldbus::b_configurationError) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7, 0.0, 0.0, 1.0));
		ImGui::ProgressBar(1.0);
		ImGui::PopStyleColor();
		ImGui::Text("Error: %s", EtherCatFieldbus::configurationStatus);
	}
	else {
		firstSuccess = true; // set static variable to true, so success time will be measured once

		int progress = EtherCatFieldbus::i_configurationProgress;
		int maxProgress = EtherCatFieldbus::slaves.size() + 5;
		int clockProgress = maxProgress - 1;

		static float progressSmooth;
		if (progress == 0) progressSmooth = 0.0;

		float progress_f = 0.0;
		if (progress < clockProgress) progress_f = ((float)progress / (float)maxProgress) / 2.0;
		else {
			float maxValue = EtherCatFieldbus::processInterval_milliseconds / 2.0;
			float minValue = EtherCatFieldbus::clockStableThreshold_milliseconds;
			float value = EtherCatFieldbus::metrics.averageDcTimeError_milliseconds;
			float percentage = (value - minValue) / (maxValue - minValue);
			progress_f = 0.5 + (1.0 - percentage) / 2.0;
		}

		progressSmooth = progressSmooth * 0.8 + progress_f * 0.2;

		ImGui::ProgressBar(progressSmooth);
		ImGui::Text(EtherCatFieldbus::configurationStatus);
	}

	bool disableCancelButton = EtherCatFieldbus::b_processStarting;
	if (disableCancelButton) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0)); }
	if (ImGui::Button("Cancel")) {
		EtherCatFieldbus::stop();
		ImGui::CloseCurrentPopup();
	}
	if (disableCancelButton) { ImGui::PopItemFlag(); ImGui::PopStyleColor(); }
	else if (EtherCatFieldbus::b_configurationError) {
		ImGui::SameLine();
		if (ImGui::Button("Retry")) {
			EtherCatFieldbus::start();
		}
	}


}
