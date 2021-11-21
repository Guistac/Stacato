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
			ImGui::Text(axes[i]->getName());
			ImGui::SameLine();
			if (axes[i]->isEnabled()) ImGui::Text("Enabled");
			else if (axes[i]->isReady()) ImGui::Text("Ready");
			else ImGui::Text("Not Ready");
			float velocityTarget = 0.0;
			ImGui::SliderFloat("##AxisVelocity", &velocityTarget, -1.0, 1.0, "%.3f n/s");
			if (ImGui::IsItemActive()) setVelocityTarget(i, velocityTarget);
			else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(i, 0.0);
			float positionProgress = axes[i]->getActualPosition_normalized();
			ImGui::ProgressBar(positionProgress, glm::vec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeight()));
			float velocityProgress = std::abs(axes[i]->getActualVelocityNormalized());
			ImGui::ProgressBar(velocityProgress, glm::vec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeight()));
			ImGui::PopID();
		}
	}

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Oscillator Control");
	ImGui::PopFont();

	ImGui::SliderFloat("##Freq", &oscillatorFrequency_hertz, 0.0, maxOscillationFrequency, "%.5f Hz");
	ImGui::SliderFloat("##Phase", &oscillatorPhaseOffset_percent, 0.0, 100.0, "%.2f percent");
	ImGui::SliderFloat("##MinAmp", &oscillatorLowerAmplitude_normalized, 0.0, 1.0, "%.2f n");
	ImGui::SliderFloat("##MaxAmp", &oscillatorUpperAmplitude_normalized, 0.0, 1.0, "%.2f n");

	float widgetWidth = ImGui::GetItemRectSize().x;
	glm::vec2 doubleButtonSize((widgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);

	if (b_oscillatorActive) {
		if (ImGui::Button("Stop Oscillator")) {
			stopOscillator();
		}
	}
	else if (isOscillatorReadyToStart()) {
		if (ImGui::Button("Start Oscillator")) {
			startOscillator();
		}
	}
	else {
		if (ImGui::Button("Prime Oscillator")) {
			moveToOscillatorStart();
		}
	}
	ImGui::SameLine();


	
/*
static double previousTime_seconds = Timing::getProgramTime_seconds();
double time_seconds = Timing::getProgramTime_seconds();
double deltaT_seconds = time_seconds - previousTime_seconds;
previousTime_seconds = time_seconds;

static float frequency = 0.1;
static float minAmplitude = 0.0;
static float maxAmplitude = 1.0;
static float phaseOffset = 0.0;
static bool startAtZero = false;

static bool running = false;
static double cycleStartTime = 0.0;
static double cycleTime = 0.0;

static double xOffset = 0.0;
static std::vector<double> axisPositions(axes.size(), 0.0);
static std::vector<CircularBuffer*> axisPositionBuffers;
static bool b_initialized = false;

if (!b_initialized) {
	b_initialized = true;
	for (int i = 0; i < axes.size(); i++) {
		axisPositionBuffers.push_back(new CircularBuffer(1024));
	}
}

ImGui::SliderFloat("Frequency", &frequency, 0.0, maxFrequency);
ImGui::SliderFloat("Min Amplitude", &minAmplitude, 0.0, 1.0);
ImGui::SliderFloat("Max Amplitude", &maxAmplitude, 0.0, 1.0);
ImGui::SliderFloat("Phase Offset", &phaseOffset, 0.0, 100.0);
ImGui::Checkbox("Start At Zero", &startAtZero);
if (running) {
	if (ImGui::Button("Stop")) running = false;
	ImGui::SameLine();
	ImGui::Text("Time: %.3f", cycleTime);
}
else {
	if (ImGui::Button("Start")){
		running = true;
		cycleStartTime = time_seconds;
		for (int i = 0; i < axes.size(); i++) {
			axisPositions[i] = 0.0;
			axisPositionBuffers[i]->clear();
		}
		xOffset = 0.0;
	}
}

if (running) {
	cycleTime = time_seconds - cycleStartTime;
	xOffset += deltaT_seconds * frequency * 2.0 * M_PI;
	double phaseOffsetRadians = (phaseOffset / 100.0) * 2.0 * M_PI;
	for (int i = 0; i < axes.size(); i++) {
		double axisXOffset = xOffset - i * phaseOffsetRadians;
		if (axisXOffset < 0.0) axisXOffset = 0.0;
		double position = cos(axisXOffset);
		if (startAtZero) position *= -1.0;
		position = (position + 1.0) / 2.0;
		position = position * (maxAmplitude - minAmplitude) + minAmplitude;
		axisPositions[i] = position;
		axisPositionBuffers[i]->addPoint(glm::vec2(cycleTime, axisPositions[i]));
	}
}



ImGui::Text("XOffset: %.6f  Cycle: %.6f", xOffset, xOffset / (2.0 * M_PI));
for (int i = 0; i < axes.size(); i++) {
	ImGui::Text("Axis %i:  Position: %.6f", i, axisPositions[i]);
}

glm::vec2* firstBuffer;
axisPositionBuffers.front()->getBuffer(&firstBuffer);
int bufferSize = axisPositionBuffers.front()->size();
if (bufferSize == 0) return;

ImPlot::SetNextPlotLimits(firstBuffer[0].x, firstBuffer[bufferSize - 1].x, 0.0, 1.0, ImGuiCond_Always);
if (ImPlot::BeginPlot("oscillator plot")) {

	for (int i = 0; i < axes.size(); i++) {

		glm::vec2* buffer;
		size_t bufferSize = axisPositionBuffers[i]->size();
		axisPositionBuffers[i]->getBuffer(&buffer);

		ImPlot::PlotLine("##line", &buffer[0].x, &buffer[0].y, bufferSize, 0, sizeof(glm::vec2));

	}

	ImPlot::EndPlot();
}

for (auto position : axisPositions) {
	float p = position;
	ImGui::VSliderFloat("##axis1", glm::vec2(50, 500), &p, 0.0, 1.0);
	ImGui::SameLine();
}
*/
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
				double maxFrequencyByVelocity = maxNormalizedVelocity / (2.0 * M_PI);
				double maxFrequencyByAcceleration = std::sqrt(maxNormalizedAcceleration / (4.0 * std::pow(M_PI, 2.0)));
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

	if (ImGui::Button("Update Machine Limits")) {
		updateMachineLimits();
	}
	ImGui::SameLine();
	ImGui::Text("Max Oscillation Frequency: %.5f", maxOscillationFrequency);
	ImGui::Text("Max Velocity: %.5f", maxVelocity_normalized);
	ImGui::Text("Max Acceleration: %.5f", maxAcceleration_normalized);
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
					if (ImGui::BeginTabBar("AxisTabs")) {
						axes[i]->nodeSpecificGui();
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



void Oscillator3x::deviceGui() {}
void Oscillator3x::metricsGui() {}
float Oscillator3x::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 10.0;
}
void Oscillator3x::machineSpecificMiniatureGui() {
	glm::vec2 axisChildSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, ImGui::GetContentRegionAvail().y);
	float bottomSectionHeight = ImGui::GetTextLineHeight() * 3.0;
	glm::vec2 positionIndicatorSize(ImGui::GetTextLineHeight() * 1.0, axisChildSize.y - bottomSectionHeight);
	glm::vec2 velocitySliderSize(axisChildSize.x - positionIndicatorSize.x - ImGui::GetStyle().ItemSpacing.x, positionIndicatorSize.y);
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i)) {
			std::shared_ptr<PositionControlledAxis> axis = getAxis(i);
			ImGui::BeginChild(axis->getName(), axisChildSize);
			float velocityTarget = 0.0;
			ImGui::VSliderFloat("##Axis1", velocitySliderSize, &velocityTarget, -1.0, 1.0, "");
			ImGui::SameLine();
			float axisPositionNormalized = axis->getProfilePosition_axisUnits() / axis->getRange_axisUnits();
			verticalProgressBar(axisPositionNormalized, positionIndicatorSize);
			if (ImGui::IsItemActive()) setVelocityTarget(i, velocityTarget);
			else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(i, 0.0);
			static char positionString[32];
			if (isnan(axisPositionNormalized)) axisPositionNormalized = 0.0;
			sprintf(positionString, "%.2f", axisPositionNormalized);
			ImGui::PushFont(Fonts::robotoRegular12);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0));
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
			ImGui::Button(positionString, glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.0));
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
			ImGui::PopFont();
			ImGui::EndChild();
		}
		else {
		}
		if (i < 2) ImGui::SameLine();
	}
}