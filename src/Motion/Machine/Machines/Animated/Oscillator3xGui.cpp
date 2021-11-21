#include <pch.h>

#include "Oscillator3x.h"

#include <imgui.h>
#include <implot.h>

#include "Motion/Axis/PositionControlledAxis.h"

#include "Utilities/CircularBuffer.h"

void Oscillator3x::controlsGui() {

	std::vector<std::shared_ptr<PositionControlledAxis>> axes;
	if (linearAxis1Pin->isConnected()) axes.push_back(linearAxis1Pin->getConnectedPins().front()->getPositionControlledAxis());
	if (linearAxis2Pin->isConnected()) axes.push_back(linearAxis2Pin->getConnectedPins().front()->getPositionControlledAxis());
	if (linearAxis3Pin->isConnected()) axes.push_back(linearAxis3Pin->getConnectedPins().front()->getPositionControlledAxis());

	if (axes.empty()) return;

	double lowestNormalizedVelocity = std::numeric_limits<double>::infinity();
	double lowestNormalizedAcceleration = std::numeric_limits<double>::infinity();

	for (auto& axis : axes) {
		double highLimit = axis->getHighPositionLimit();
		double lowLimit = axis->getLowPositionLimit();
		double velocityLimit = axis->getVelocityLimit_axisUnitsPerSecond();
		double accelerationLimit = axis->getAccelerationLimit_axisUnitsPerSecondSquared();

		double motionRange = highLimit - lowLimit;
		double normalizedVelocityLimit = velocityLimit / motionRange;
		double normalizedAccelerationLimit = accelerationLimit / motionRange;

		lowestNormalizedVelocity = std::min(normalizedVelocityLimit, lowestNormalizedVelocity);
		lowestNormalizedAcceleration = std::min(lowestNormalizedAcceleration, normalizedAccelerationLimit);
	}

	double maxFrequencyByVelocity = lowestNormalizedVelocity / (2.0 * M_PI);
	double maxFrequencyByAcceleration = std::sqrt(lowestNormalizedAcceleration / (4.0 * std::pow(M_PI, 2.0)));
	double maxFrequency = std::min(maxFrequencyByVelocity, maxFrequencyByAcceleration);

	if (ImGui::TreeNode("Axis Info")) {

		for (auto& axis : axes) {
			double highLimit = axis->getHighPositionLimit();
			double lowLimit = axis->getLowPositionLimit();
			double velocityLimit = axis->velocityLimit_axisUnitsPerSecond;
			double accelerationLimit = axis->accelerationLimit_axisUnitsPerSecondSquared;

			ImGui::Text("Axis: '%s'", axis->getName());
			ImGui::Text("Low Limit: %.3f", lowLimit);
			ImGui::Text("High Limit: %.3f", highLimit);
			ImGui::Text("Velocity Limit: %.3f", velocityLimit);
			ImGui::Text("Acceleration Limit: %.3f", accelerationLimit);

			double motionRange = highLimit - lowLimit;
			double normalizedVelocityLimit = velocityLimit / motionRange;
			double normalizedAccelerationLimit = accelerationLimit / motionRange;

			double maxFrequencyByVelocity = normalizedVelocityLimit / (2.0 * M_PI);
			double maxFrequencyByAcceleration = std::sqrt(normalizedAccelerationLimit / (4.0 * std::pow(M_PI, 2.0)));

			ImGui::Text("Max Frequency (v) : %.6f", maxFrequencyByVelocity);
			ImGui::Text("Max Frequency (a) : %.6f", maxFrequencyByAcceleration);

			ImGui::Separator();
		}
		ImGui::TreePop();
	}


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

}


void Oscillator3x::settingsGui() {}


void Oscillator3x::axisGui() {

	std::shared_ptr<Axis> axis1 = linearAxis1Pin->getConnectedPins().front()->getPositionControlledAxis();
	std::shared_ptr<Axis> axis2 = linearAxis2Pin->getConnectedPins().front()->getPositionControlledAxis();
	std::shared_ptr<Axis> axis3 = linearAxis3Pin->getConnectedPins().front()->getPositionControlledAxis();

	if (ImGui::BeginTabBar("AxisTabBar")) {
	
		if (ImGui::BeginTabItem("Axis 1")) {
			if (axis1) {
				if (ImGui::BeginTabBar("AxisTabs")) {
					axis1->nodeSpecificGui();
					ImGui::EndTabBar();
				}
			}
			else ImGui::Text("No Axis Connected.");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Axis 2")) {
			if (axis2) {
				if (ImGui::BeginTabBar("AxisTabs")) {
					axis2->nodeSpecificGui();
					ImGui::EndTabBar();
				}
			}
			else ImGui::Text("No Axis Connected.");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Axis 3")) {
			if (axis3) {
				if (ImGui::BeginTabBar("AxisTabs")) {
					axis3->nodeSpecificGui();
					ImGui::EndTabBar();
				}
			}
			else ImGui::Text("No Axis Connected.");
			ImGui::EndTabItem();
		}


	
		ImGui::EndTabBar();
	}

}



void Oscillator3x::deviceGui() {}
void Oscillator3x::metricsGui() {}
float Oscillator3x::getMiniatureWidth() {
	return 50.0;
}
void Oscillator3x::machineSpecificMiniatureGui() {}