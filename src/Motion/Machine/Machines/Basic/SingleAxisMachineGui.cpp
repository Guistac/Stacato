#include <pch.h>

#include "SingleAxisMachine.h"
#include "Motion/Axis/Axis.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Framework/Fonts.h"
#include "Gui/Framework/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"


void SingleAxisMachine::controlsGui() {

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Machine Control");
	ImGui::PopFont();

	powerControlGui();

	PositionUnit::Unit machinePositionUnit = PositionUnit::Unit::DEGREE;
	double accelerationLimit_machineUnits = 0;
	double velocityLimit_machineUnits = 0;
	if (isAxisConnected()) {
		std::shared_ptr<Axis> axis = getAxis();
		machinePositionUnit = axis->axisPositionUnit;
		accelerationLimit_machineUnits = axis->accelerationLimit_axisUnitsPerSecondSquared;
		velocityLimit_machineUnits = axis->velocityLimit_axisUnitsPerSecond;
	}


	//------------------- MASTER MANUAL ACCELERATION ------------------------

	ImGui::Text("Acceleration for manual controls :");
	static char accelerationString[32];
	sprintf(accelerationString, u8"%.3f %s/s²", manualControlAcceleration_machineUnitsPerSecondSquared, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##TargetAcceleration", &manualControlAcceleration_machineUnitsPerSecondSquared, 0.0, 0.0, accelerationString);
	clampValue(manualControlAcceleration_machineUnitsPerSecondSquared, 0.0, accelerationLimit_machineUnits);
	ImGui::Separator();

	//------------------- VELOCITY CONTROLS ------------------------

	float widgetWidth = ImGui::GetContentRegionAvail().x;

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Velocity Control");
	ImGui::PopFont();

	ImGui::SetNextItemWidth(widgetWidth);
	static char velocityTargetString[32];
	sprintf(velocityTargetString, "%.3f %s/s", manualVelocityTarget_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));

	float manualVelocityTarget = manualVelocityTarget_machineUnitsPerSecond;
	if (ImGui::SliderFloat("##Velocity", &manualVelocityTarget, -velocityLimit_machineUnits, velocityLimit_machineUnits, velocityTargetString));
	clampValue(manualVelocityTarget, -velocityLimit_machineUnits, velocityLimit_machineUnits);
	if (ImGui::IsItemActive()) setVelocity(manualVelocityTarget);
	else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocity(manualVelocityTarget = 0.0);

	ImGui::Separator();

	//------------------------- POSITION CONTROLS --------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Position Control");
	ImGui::PopFont();

	float tripleWidgetWidth = (widgetWidth - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
	glm::vec2 tripleButtonSize(tripleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", targetPosition_machineUnits, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##TargetPosition", &targetPosition_machineUnits, 0.0, 0.0, targetPositionString);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char targetVelocityString[32];
	sprintf(targetVelocityString, "%.3f %s/s", targetVelocity_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##TargetVelocity", &targetVelocity_machineUnitsPerSecond, 0.0, 0.0, targetVelocityString);
	clampValue(targetVelocity_machineUnitsPerSecond, 0.0, velocityLimit_machineUnits);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputDouble("##TargetTime", &targetTime_seconds, 0.0, 0.0, "%.3f s");
	if (ImGui::Button("Fast Move", tripleButtonSize)) {
		moveToPositionWithVelocity(targetPosition_machineUnits, velocityLimit_machineUnits, manualControlAcceleration_machineUnitsPerSecondSquared);
	}
	ImGui::SameLine();
	if (ImGui::Button("Velocity Move", tripleButtonSize)) {
		moveToPositionWithVelocity(targetPosition_machineUnits, targetPosition_machineUnits, manualControlAcceleration_machineUnitsPerSecondSquared);
	}
	ImGui::SameLine();
	if (ImGui::Button("Timed Move", tripleButtonSize)) {
		moveToPositionInTime(targetPosition_machineUnits, targetTime_seconds, manualControlAcceleration_machineUnitsPerSecondSquared);
	}

	if (ImGui::Button("Stop##Target", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) {
		setVelocity(0.0);
	}

	ImGui::Separator();


	std::shared_ptr<Axis> axis;
	PositionUnit::Unit positionUnit = PositionUnit::Unit::DEGREE;
	double velocityLimit = 0.0;
	//actual position in range
	double minPosition = 0.0;
	double maxPosition = 0.0;
	double positionProgress = 0.0;
	static char positionString[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		positionProgress = 1.0;
		sprintf(positionString, "Machine Disabled");
	}
	else {
		axis = getAxis();
		minPosition = axis->getLowPositionLimit();
		maxPosition = axis->getHighPositionLimit();
		positionProgress = axis->getPositionProgress();
		positionUnit = axis->axisPositionUnit;
		velocityLimit = getAxis()->velocityLimit_axisUnitsPerSecond;

		if (positionProgress <= 1.0 && positionProgress >= 0.0) {
			sprintf(positionString, "%.2f %s", actualVelocity_machineUnits, getPositionUnit(positionUnit)->shortForm);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		}
		else {
			positionProgress = 1.0;
			sprintf(positionString, "Axis out of limits : %.2f %s", actualVelocity_machineUnits, getPositionUnit(positionUnit)->shortForm);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		}
	}
	ImGui::Text("Current Position : (in range from %.2f %s to %.2f %s)", minPosition, getPositionUnitStringShort(positionUnit), maxPosition, getPositionUnitStringShort(positionUnit));
	ImGui::ProgressBar(positionProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), positionString);
	ImGui::PopStyleColor();


	//actual velocity
	float velocityProgress;
	static char velocityString[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		velocityProgress = 1.0;
		sprintf(velocityString, "Machine Disabled");
	}
	else {
		velocityProgress = std::abs(actualVelocity_machineUnits) / velocityLimit;
		sprintf(velocityString, "%.2f %s/s", actualVelocity_machineUnits, getPositionUnitStringShort(positionUnit));
		if (std::abs(actualVelocity_machineUnits) > std::abs(velocityLimit))
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}

	ImGui::Text("Current Velocity : (max %.2f%s/s)", velocityLimit, getPositionUnitStringShort(positionUnit));
	ImGui::ProgressBar(velocityProgress, ImVec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), velocityString);
	ImGui::PopStyleColor();


	//target movement progress
	float targetProgress;
	double movementSecondsLeft = 0.0;
	static char movementProgressChar[8];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "Machine Disabled");
		targetProgress = 1.0;
	}
	else if (controlMode != ControlMode::Mode::MANUAL_POSITION_TARGET) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "No Target Movement");
		targetProgress = 1.0;
	}
	else if (targetIntepolation->getProgressAtTime(profileTime_seconds) >= 1.0) {
		targetProgress = 1.0;
		sprintf(movementProgressChar, "Movement Finished");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	else {
		targetProgress = targetIntepolation->getProgressAtTime(profileTime_seconds);
		movementSecondsLeft = targetIntepolation->outTime - profileTime_seconds;
		if (movementSecondsLeft < 0.0) movementSecondsLeft = 0.0;
		sprintf(movementProgressChar, "%.2fs", movementSecondsLeft);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
	}
	ImGui::Text("Movement Time Remaining :");
	ImGui::ProgressBar(targetProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), movementProgressChar);
	ImGui::PopStyleColor();


	miniatureGui();
}


void SingleAxisMachine::settingsGui() {}
void SingleAxisMachine::axisGui() {}
void SingleAxisMachine::deviceGui() {}
void SingleAxisMachine::metricsGui() {}

float SingleAxisMachine::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 8.0;
}

void SingleAxisMachine::machineSpecificMiniatureGui() {
		float bottomControlsHeight = ImGui::GetTextLineHeight() * 3.3;
		float sliderHeight = ImGui::GetContentRegionAvail().y - bottomControlsHeight;
		float tripleWidgetWidth = (ImGui::GetContentRegionAvail().x - 2.0 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
		glm::vec2 verticalSliderSize(tripleWidgetWidth, sliderHeight);

		std::shared_ptr<Axis> axis;
		PositionUnit::Unit positionUnit = PositionUnit::Unit::DEGREE;
		float positionProgress = 1.0;
		float velocityProgress = 1.0;
		float velocityLimit = 0.0;
		if (isEnabled()) {
			axis = getAxis();
			positionUnit = axis->axisPositionUnit;
			velocityLimit = axis->velocityLimit_axisUnitsPerSecond;
			positionProgress = axis->getPositionProgress();
			velocityProgress = std::abs((actualVelocity_machineUnits / velocityLimit));
			if (velocityProgress > 1.0) velocityProgress = 1.0;
		}

		float manualVelocityTarget = manualVelocityTarget_machineUnitsPerSecond;
		ImGui::VSliderFloat("##ManualVelocity", verticalSliderSize, &manualVelocityTarget, -velocityLimit, velocityLimit, "%.3f m/s");
		clampValue(manualVelocityTarget, -velocityLimit, velocityLimit);
		if (ImGui::IsItemActive()) setVelocity(manualVelocityTarget);
		else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocity(manualVelocityTarget = 0.0);

		ImGui::SameLine();

		verticalProgressBar(velocityProgress, verticalSliderSize);
		ImGui::SameLine();
		verticalProgressBar(positionProgress, verticalSliderSize);


		float framePaddingX = ImGui::GetStyle().FramePadding.x;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(framePaddingX, ImGui::GetTextLineHeight() * 0.1));

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		static char targetPositionString[32];
		sprintf(targetPositionString, "%.1f %s", targetPosition_machineUnits, getPositionUnitStringShort(positionUnit));
		ImGui::InputDouble("##TargetPosition", &targetPosition_machineUnits, 0.0, 0.0, targetPositionString);

		float motionProgress = targetIntepolation->getProgressAtTime(profileTime_seconds);
		if (motionProgress > 0.0 && motionProgress < 1.0) {
			glm::vec2 targetmin = ImGui::GetItemRectMin();
			glm::vec2 targetmax = ImGui::GetItemRectMax();
			glm::vec2 targetsize = ImGui::GetItemRectSize();
			glm::vec2 progressBarMax(targetmin.x + targetsize.x * motionProgress, targetmax.y);
			ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
		}

		ImGui::PopStyleVar();

		float doubleWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
		glm::vec2 doubleButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);

		if (ImGui::Button("Move", doubleButtonSize)) {
			moveToPositionWithVelocity(targetPosition_machineUnits, targetVelocity_machineUnitsPerSecond, manualControlAcceleration_machineUnitsPerSecondSquared);
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop", doubleButtonSize)) {
			setVelocity(0.0);
		}
}
