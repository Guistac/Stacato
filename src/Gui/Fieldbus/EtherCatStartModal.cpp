#include <pch.h>

#include "Gui/Gui.h"

#include "Fieldbus/EtherCatFieldbus.h"

void etherCatStartModal() {
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5, 0.5));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetTextLineHeight() * 30.0, 0), ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Starting EtherCAT Fieldbus")) {
		static bool firstSuccess;
		static long long successTime_milliseconds;
		static long long successCloseDelay_milliseconds = 500;
		if (EtherCatFieldbus::isCyclicExchangeStartSuccessfull()) {
			//display a full progress bar and close the popup after some time (1 second)
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3, 0.8, 0.0, 1.0));
			ImGui::ProgressBar(1.0);
			ImGui::PopStyleColor();
			ImGui::Text(EtherCatFieldbus::startupStatusString);
			using namespace std::chrono;
			if (firstSuccess) {
				firstSuccess = false;
				successTime_milliseconds = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
			}
			else if (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - successTime_milliseconds > successCloseDelay_milliseconds) {
				ImGui::CloseCurrentPopup();
			}
		}
		else if (EtherCatFieldbus::b_startupError) {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7, 0.0, 0.0, 1.0));
			ImGui::ProgressBar(1.0);
			ImGui::PopStyleColor();
			ImGui::Text("Error: %s", EtherCatFieldbus::startupStatusString);
		}
		else {
			firstSuccess = true; // set static variable to true, so success time will be measured once

			int progress = EtherCatFieldbus::i_startupProgress;
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
			ImGui::Text(EtherCatFieldbus::startupStatusString);
		}

		bool disableCancelButton = EtherCatFieldbus::isCyclicExchangeStarting();
		if (disableCancelButton) BEGIN_DISABLE_IMGUI_ELEMENT
		if (ImGui::Button("Cancel")) {
			EtherCatFieldbus::stop();
			ImGui::CloseCurrentPopup();
		}
		if (disableCancelButton) END_DISABLE_IMGUI_ELEMENT
		else if (EtherCatFieldbus::b_startupError) {
			ImGui::SameLine();
			if (ImGui::Button("Retry")) {
				EtherCatFieldbus::start();
			}
		}
		ImGui::EndPopup();
	}
}