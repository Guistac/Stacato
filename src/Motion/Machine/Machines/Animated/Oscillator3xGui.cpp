#include <pch.h>

#include "Oscillator3x.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Colors.h"

#include "Motion/Axis/PositionControlledAxis.h"

#include "Utilities/CircularBuffer.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Gui/Assets/Fonts.h"

void Oscillator3x::controlsGui() {

	std::vector<std::shared_ptr<PositionControlledAxis>> axes;
	getAxes(axes);

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Velocity Control");
	ImGui::PopFont();

	for (int i = 0; i < 3; i++) {
		if (axes[i] != nullptr) {
			ImGui::PushID(i);

			std::shared_ptr<PositionControlledAxis> axis = axes[i];

			ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::Button(axis->getName());
			ImGui::PopItemFlag();
			ImGui::PopStyleColor();

			ImGui::SameLine();

			if (axis->isEnabled()) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				if (ImGui::Button("Disable")) {
					axis->disable();
				}
			}
			else if (axes[i]->isReady()) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Enable")) {
					axis->enable();
				}
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::Button("Not Ready");
				ImGui::PopItemFlag();
			}
			ImGui::PopStyleColor();

			if (axis->isHoming()) {
				ImGui::SameLine();
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				ImGui::Button(getHomingStep(axis->homingStep)->displayName);
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
			}

			float velocityTarget = 0.0;
			ImGui::SliderFloat("##AxisVelocity", &velocityTarget, -maxVelocity_normalized, maxVelocity_normalized, "%.3f n/s");
			if (ImGui::IsItemActive()) setVelocityTarget(i, velocityTarget);
			else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(i, 0.0);
			float positionProgress = axes[i]->getActualPosition_normalized();
			ImGui::ProgressBar(positionProgress, glm::vec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeight()));
			float velocityProgress = std::abs(axes[i]->getActualVelocityNormalized());
			ImGui::ProgressBar(velocityProgress, glm::vec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeight()));
			ImGui::PopID();
		}
	}

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Oscillator Control");
	ImGui::PopFont();

	ImGui::SliderFloat("##Freq", &oscillatorFrequency_hertz, 0.0, maxOscillationFrequency, "%.5f Hz");
	bool disableControls = b_oscillatorActive;
	if(disableControls) BEGIN_DISABLE_IMGUI_ELEMENT
	ImGui::SliderFloat("##Phase", &oscillatorPhaseOffset_percent, 0.0, 100.0, "%.2f percent");
	ImGui::SliderFloat("##MinAmp", &oscillatorLowerAmplitude_normalized, 0.0, 1.0, "%.2f n");
	ImGui::SliderFloat("##MaxAmp", &oscillatorUpperAmplitude_normalized, 0.0, 1.0, "%.2f n");
	if(disableControls) END_DISABLE_IMGUI_ELEMENT

	float widgetWidth = ImGui::GetItemRectSize().x;
	glm::vec2 doubleButtonSize((widgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);

	if (b_oscillatorActive) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		if (ImGui::Button("Stop Oscillator")) {
			stopOscillator();
		}
	}
	else if (isOscillatorReadyToStart()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		if (ImGui::Button("Start Oscillator")) {
			startOscillator();
		}
	}
	else if(isEnabled()){
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::orange);
		if (ImGui::Button("Prime Oscillator")) {
			moveToOscillatorStart();
		}
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Button("Not Ready##Oscillator");
		ImGui::PopItemFlag();
	}
	ImGui::PopStyleColor();

	ImGui::SameLine();

	if (ImGui::Button("Stop All")) {
		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) setVelocityTarget(i, 0.0);
		}
	}

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Homing");
	ImGui::PopFont();

	if (isEnabled()) {
		if (isHoming()) {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			if (ImGui::Button("Stop Homing")) {
				stopHoming();
			}
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
			if (ImGui::Button("Start Homing")) {
				startHoming();
			}
		}
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Button("Not Ready##Homing");
		ImGui::PopItemFlag();
	}
	ImGui::PopStyleColor();


}


void Oscillator3x::settingsGui() {
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Oscillator Settings");
	ImGui::PopFont();
	
	ImGui::Checkbox("Start Oscillator From Lower Axis Limits", &b_startAtLowerLimit);

	ImGui::Text("Oscillation Frequency Limit");

	if (ImGui::BeginTable("##AxisInfo", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
	
		ImGui::TableSetupColumn("Axis");
		ImGui::TableSetupColumn("Velocity Limited Frequency");
		ImGui::TableSetupColumn("Acceleration Limit Frequency");
		ImGui::TableHeadersRow();
		
		std::vector<std::shared_ptr<PositionControlledAxis>> axes;
		getAxes(axes);

		for (int i = 0; i < axes.size(); i++) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (axes[i] != nullptr) {
				ImGui::Text(axes[i]->getName());
				ImGui::TableNextColumn();
				double maxNormalizedVelocity = axes[i]->getVelocityLimit_axisUnitsPerSecond() / axes[i]->getRange_axisUnits();
				double maxNormalizedAcceleration = axes[i]->getAccelerationLimit_axisUnitsPerSecondSquared() / axes[i]->getRange_axisUnits();
				double maxFrequencyByVelocity = maxNormalizedVelocity / M_PI;
				double maxFrequencyByAcceleration = std::sqrt(maxNormalizedAcceleration / (2.0 * std::pow(M_PI, 2.0)));
				ImGui::Text("%.6f Hz", maxFrequencyByVelocity);
				ImGui::TableNextColumn();
				ImGui::Text("%.6f Hz", maxFrequencyByAcceleration);
			}
			else {
				ImGui::Text("Axis Not Connected");
			}
		}

		ImGui::EndTable();
	}

	ImGui::Text("Max Oscillation Frequency: %.5f", maxOscillationFrequency);
	ImGui::Text("Max Velocity: %.5f", maxVelocity_normalized);
	ImGui::Text("Max Acceleration: %.5f", maxAcceleration_normalized);
	if (ImGui::Button("Update Machine Limits")) updateMachineLimits();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Rapids");
	ImGui::PopFont();

	ImGui::Text("Velocity :");
	ImGui::InputDouble("##rapidVel", &rapidVelocity_normalized, 0.0, 0.0, "%.5f n/s");
	rapidVelocity_normalized = std::min(rapidVelocity_normalized, maxVelocity_normalized);
	rapidVelocity_normalized = std::max(rapidVelocity_normalized, 0.0);
	ImGui::Text("Acceleration :");
	ImGui::InputDouble("##rapidAcc", &rapidAcceleration_normalized, 0.0, 0.0, "%.5f n/s2");
	rapidAcceleration_normalized = std::min(rapidAcceleration_normalized, maxAcceleration_normalized);
	rapidAcceleration_normalized = std::max(rapidAcceleration_normalized, 0.0);

}


void Oscillator3x::axisGui() {

	std::vector<std::shared_ptr<PositionControlledAxis>> axes;
	getAxes(axes);

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Axes");
	ImGui::PopFont();

	if (ImGui::BeginTabBar("AxisTabBar")) {
		for (int i = 0; i < axes.size(); i++) {
			if (axes[i] != nullptr) {
				ImGui::PushID(i);
				if (ImGui::BeginTabItem(axes[i]->getName())) {
						if (ImGui::BeginTabBar(axes[i]->getName())) {
						if (ImGui::BeginTabItem("Settings")) {
							ImGui::BeginChild("SettingsChild");
							axes[i]->settingsGui();
							ImGui::EndChild();
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Devices")) {
							ImGui::BeginChild("DevicesChild");
							axes[i]->devicesGui();
							ImGui::EndChild();
							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}
					ImGui::EndTabItem();
				}
				ImGui::PopID();
			}
		}
		ImGui::EndTabBar();
	}

}



void Oscillator3x::deviceGui() {
	std::vector<std::shared_ptr<PositionControlledAxis>> axes;
	getAxes(axes);

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Metrics");
	ImGui::PopFont();

	if (ImGui::BeginTabBar("AxisTabBar")) {
		for (int i = 0; i < axes.size(); i++) {
			if (axes[i] != nullptr) {
				ImGui::PushID(i);
				if (ImGui::BeginTabItem(axes[i]->getName())) {
					axes[i]->devicesGui();
					ImGui::EndTabItem();
				}
				ImGui::PopID();
			}
		}
		ImGui::EndTabBar();
	}
}


void Oscillator3x::metricsGui() {
	std::vector<std::shared_ptr<PositionControlledAxis>> axes;
	getAxes(axes);

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Metrics");
	ImGui::PopFont();

	if (ImGui::BeginTabBar("AxisTabBar")) {
		for (int i = 0; i < axes.size(); i++) {
			if (axes[i] != nullptr) {
				ImGui::PushID(i);
				if (ImGui::BeginTabItem(axes[i]->getName())) {
					axes[i]->metricsGui();
					ImGui::EndTabItem();
				}
				ImGui::PopID();
			}
		}
		ImGui::EndTabBar();
	}
}


float Oscillator3x::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 10.0;
}
void Oscillator3x::machineSpecificMiniatureGui() {
	bool b_disableAllControls = !isEnabled() || oscillatorParameterGroup->hasParameterTrack();

	if(b_disableAllControls) BEGIN_DISABLE_IMGUI_ELEMENT

	glm::vec2 axisChildSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, ImGui::GetContentRegionAvail().y);
	float sliderHeight = axisChildSize.y - ImGui::GetTextLineHeight() * 3.3;
	glm::vec2 positionIndicatorSize(ImGui::GetTextLineHeight() * 1.0, sliderHeight);
	glm::vec2 velocitySliderSize(axisChildSize.x - positionIndicatorSize.x - ImGui::GetStyle().ItemSpacing.x, sliderHeight);
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i)) {
			std::shared_ptr<PositionControlledAxis> axis = getAxis(i);
			ImGui::BeginChild(axis->getName(), axisChildSize);


			ImGui::PushFont(Fonts::robotoBold12);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0));
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
			ImGui::Button(axis->getName(), glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.0));
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
			ImGui::PopFont();

		
			ImGui::PopStyleVar();
			

			float velocityTarget = 0.0;
			ImGui::VSliderFloat("##Axis1", velocitySliderSize, &velocityTarget, -maxVelocity_normalized, maxVelocity_normalized, "");
			if (ImGui::IsItemActive()) setVelocityTarget(i, velocityTarget);
			else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(i, 0.0);
			ImGui::SameLine();
			float axisPositionNormalized = axis->getActualPosition_normalized();
			verticalProgressBar(axisPositionNormalized, positionIndicatorSize);
			static char positionString[32];
			if (isnan(axisPositionNormalized)) axisPositionNormalized = 0.0;
			sprintf(positionString, "%.2f", axisPositionNormalized);
			ImGui::PushFont(Fonts::robotoRegular12);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0));
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
			ImGui::Button(positionString, glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.0));
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
			ImGui::PopFont();

			ImGui::PushFont(Fonts::robotoLight12);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			glm::vec2 statusButtonSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.0);
			if (axis->isEnabled()) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				if (ImGui::Button("Enabled", statusButtonSize)) {
					axis->disable();
				}
			}
			else if (axis->isReady()) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Ready", statusButtonSize)) {
					axis->enable();
				}
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
				ImGui::Button("Not Ready", statusButtonSize);
			}
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
			ImGui::PopFont();
			ImGui::PopStyleColor();
			
			ImGui::EndChild();
		}
		else {
		}
		if (i < 2) ImGui::SameLine();
	}

	if(b_disableAllControls) END_DISABLE_IMGUI_ELEMENT
}