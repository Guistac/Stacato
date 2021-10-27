#include <pch.h>

#include "SingleAxisLinearMachine.h"

#include "Motion/Axis/Axis.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Framework/Fonts.h"
#include "Gui/Framework/Colors.h"


void SingleAxisLinearMachine::assignIoData() {
	addIoData(axisLink);
}

bool SingleAxisLinearMachine::isEnabled() {
	return b_enabled;
}

bool SingleAxisLinearMachine::isReady() {
	if (!isAxisConnected()) return false;
	std::shared_ptr<Axis> axis = getAxis();
	if (axis->hasManualControlsEnabled()) return false;
	if (axis->getCommandType() != MotionCommand::Type::POSITION_COMMAND) return false;
	if (axis->axisPositionUnitType != PositionUnit::Type::LINEAR) return false;
	return true;
}

bool SingleAxisLinearMachine::isMoving() {
	if (isAxisConnected()) return getAxis()->isMoving();
	return false;
}

void SingleAxisLinearMachine::enable() {
	if (isReady()) {
		std::thread machineEnabler([this]() {
			using namespace std::chrono;
			std::shared_ptr<Axis> axis = getAxis();
			axis->enable();
			time_point enableRequestTime = system_clock::now();
			while (duration(system_clock::now() - enableRequestTime) < milliseconds(100)) {
				std::this_thread::sleep_for(milliseconds(10));
				if (axis->isEnabled()) {
					b_enabled = true;
					break;
				}
			}
			});
		machineEnabler.detach();
	}
}

void SingleAxisLinearMachine::disable() {
	b_enabled = false;
	if (isAxisConnected()) getAxis()->disable();
}

void SingleAxisLinearMachine::process() {
	if (!isAxisConnected()) return;
	std::shared_ptr<Axis> axis = getAxis();

	//Get Realtime values from axis
	actualPosition_machineUnits = axis->actualPosition_axisUnits;
	actualVelocity_machineUnits = axis->actualVelocity_axisUnitsPerSecond;

	//Update Time
	profileTime_seconds = axis->currentProfilePointTime_seconds;
	profileDeltaTime_seconds = axis->currentProfilePointDeltaT_seconds;

	//Handle state changes
	if (b_enabled) {
		if (!axis->isEnabled()) disable();
	}

	if (b_enabled) {
		//if the machine is enabled
		//its drives the profile generator values of the axis
		//it also sends commands to the actuators of the axis
		switch (controlMode) {
			case ControlMode::Mode::MANUAL_VELOCITY_TARGET:
				velocityTargetControl();
				break;
			case ControlMode::Mode::MANUAL_POSITION_TARGET:
				positionTargetControl();
				break;
			case ControlMode::Mode::FOLLOW_PARAMETER:
				break;
		}
		axis->profilePosition_axisUnits = profilePosition_machineUnits;
		axis->profileVelocity_axisUnitsPerSecond = profileVelocity_machineUnitsPerSecond;
		axis->setActuatorCommands();
	}
	else {
		//if the machine is disabled
		//we only update the profile generator by copying axis values
		profilePosition_machineUnits = axis->profilePosition_axisUnits;
		profileVelocity_machineUnitsPerSecond = axis->profileVelocity_axisUnitsPerSecond;
	}
}

bool SingleAxisLinearMachine::isAxisConnected() {
	return axisLink->isConnected();
}

std::shared_ptr<Axis> SingleAxisLinearMachine::getAxis() {
	return axisLink->getConnectedPins().front()->getAxis();
}

void SingleAxisLinearMachine::moveToParameter() {}









//================================= MANUAL VELOCITY TARGET CONTROL ===================================

void SingleAxisLinearMachine::setVelocity(double velocity_axisUnits) {
	manualVelocityTarget_machineUnitsPerSecond = velocity_axisUnits;
	if (controlMode == ControlMode::Mode::MANUAL_POSITION_TARGET) targetCurveProfile = MotionCurve::CurveProfile();
	controlMode = ControlMode::Mode::MANUAL_VELOCITY_TARGET;
}

void SingleAxisLinearMachine::velocityTargetControl() {
	std::shared_ptr<Axis> axis = getAxis();
	double lowPositionLimit = axis->getLowPositionLimit();
	double highPositionLimit = axis->getHighPositionLimit();
	double velocityLimit = axis->velocityLimit_axisUnitsPerSecond;

	if (profileVelocity_machineUnitsPerSecond != manualVelocityTarget_machineUnitsPerSecond) {
		double deltaV_axisUnitsPerSecond = manualControlAcceleration_machineUnitsPerSecondSquared * profileDeltaTime_seconds;
		if (profileVelocity_machineUnitsPerSecond < manualVelocityTarget_machineUnitsPerSecond) {
			profileVelocity_machineUnitsPerSecond += deltaV_axisUnitsPerSecond;
			if (profileVelocity_machineUnitsPerSecond > manualVelocityTarget_machineUnitsPerSecond) profileVelocity_machineUnitsPerSecond = manualVelocityTarget_machineUnitsPerSecond;
		}
		else {
			profileVelocity_machineUnitsPerSecond -= deltaV_axisUnitsPerSecond;
			if (profileVelocity_machineUnitsPerSecond < manualVelocityTarget_machineUnitsPerSecond) profileVelocity_machineUnitsPerSecond = manualVelocityTarget_machineUnitsPerSecond;
		}
	}
	double deltaPosition_machineUnits = profileVelocity_machineUnitsPerSecond * profileDeltaTime_seconds;
	profilePosition_machineUnits += deltaPosition_machineUnits;
}

//================================= MANUAL POSITION TARGET CONTROL ===================================

void SingleAxisLinearMachine::moveToPositionWithVelocity(double position_machineUnits, double velocity_machineUnits, double acceleration_machineUnits) {
	std::shared_ptr<Axis> axis = getAxis();
	double lowPositionLimit = axis->getLowPositionLimit();
	double highPositionLimit = axis->getHighPositionLimit();
	double velocityLimit = axis->velocityLimit_axisUnitsPerSecond;

	if (position_machineUnits > highPositionLimit) position_machineUnits = highPositionLimit;
	else if (position_machineUnits < lowPositionLimit) position_machineUnits = lowPositionLimit;

	MotionCurve::CurvePoint startPoint(profileTime_seconds, profilePosition_machineUnits, acceleration_machineUnits, profileVelocity_machineUnitsPerSecond);
	MotionCurve::CurvePoint endPoint(0.0, position_machineUnits, acceleration_machineUnits, 0.0);
	if (MotionCurve::getFastestVelocityConstrainedProfile(startPoint, endPoint, velocity_machineUnits, targetCurveProfile)) {
		controlMode = ControlMode::Mode::MANUAL_POSITION_TARGET;
		manualVelocityTarget_machineUnitsPerSecond = 0.0;
	}
	else setVelocity(0.0);
}

void SingleAxisLinearMachine::moveToPositionInTime(double position_machineUnits, double movementTime_seconds, double acceleration_machineUnits) {
	std::shared_ptr<Axis> axis = getAxis();
	double lowPositionLimit = axis->getLowPositionLimit();
	double highPositionLimit = axis->getHighPositionLimit();
	double velocityLimit = axis->velocityLimit_axisUnitsPerSecond;

	if (position_machineUnits > highPositionLimit) position_machineUnits = highPositionLimit;
	else if (position_machineUnits < lowPositionLimit) position_machineUnits = lowPositionLimit;

	MotionCurve::CurvePoint startPoint(profileTime_seconds, profilePosition_machineUnits, acceleration_machineUnits, profileVelocity_machineUnitsPerSecond);
	MotionCurve::CurvePoint endPoint(profileTime_seconds + movementTime_seconds, position_machineUnits, acceleration_machineUnits, 0.0);
	if (MotionCurve::getTimeConstrainedProfile(startPoint, endPoint, velocityLimit, targetCurveProfile)) {
		controlMode = ControlMode::Mode::MANUAL_POSITION_TARGET;
		manualVelocityTarget_machineUnitsPerSecond = 0.0;
	}
	else setVelocity(0.0);
}

void SingleAxisLinearMachine::positionTargetControl() {
	if (MotionCurve::isInsideCurveTime(profileTime_seconds, targetCurveProfile)) {
		MotionCurve::CurvePoint curvePoint = MotionCurve::getCurvePointAtTime(profileTime_seconds, targetCurveProfile);
		profilePosition_machineUnits = curvePoint.position;
		profileVelocity_machineUnitsPerSecond = curvePoint.velocity;
	}
	else profileVelocity_machineUnitsPerSecond = 0.0;
}




























void SingleAxisLinearMachine::controlsGui() {

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Machine Control");
	ImGui::PopFont();

	glm::vec2 buttonSize;
	int buttonCount = 2;
	buttonSize.x = (ImGui::GetContentRegionAvail().x - (buttonCount - 1) * ImGui::GetStyle().ItemSpacing.x) / buttonCount;
	buttonSize.y = ImGui::GetTextLineHeight() * 2.0;

	bool readyToEnable = isReady();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Machine Enabled", buttonSize);
	}
	else if (readyToEnable) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Machine Ready", buttonSize);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Machine Not Ready", buttonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();

	ImGui::SameLine();
	if (!readyToEnable) BEGIN_DISABLE_IMGUI_ELEMENT
		if (isEnabled()) {
			if (ImGui::Button("Disable Machine", buttonSize)) disable();
		}
		else {
			if (ImGui::Button("Enable Machine", buttonSize)) enable();
		}
	if (!readyToEnable) END_DISABLE_IMGUI_ELEMENT

		ImGui::Separator();


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
	clamp(manualControlAcceleration_machineUnitsPerSecondSquared, 0.0, accelerationLimit_machineUnits);
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
	clamp(manualVelocityTarget, -velocityLimit_machineUnits, velocityLimit_machineUnits);
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
	clamp(targetVelocity_machineUnitsPerSecond, 0.0, velocityLimit_machineUnits);
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




}


void SingleAxisLinearMachine::settingsGui() {}
void SingleAxisLinearMachine::axisGui() {}
void SingleAxisLinearMachine::deviceGui() {}
void SingleAxisLinearMachine::metricsGui() {}
void SingleAxisLinearMachine::miniatureGui() {}
