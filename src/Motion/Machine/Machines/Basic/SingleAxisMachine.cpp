#include <pch.h>

#include "SingleAxisMachine.h"

#include "Motion/Axis/Axis.h"
#include "Motion/Machine/AnimatableParameter.h"


void SingleAxisMachine::assignIoData() {
	addIoData(axisLink);
	std::shared_ptr<Machine> thisMachine = std::dynamic_pointer_cast<Machine>(shared_from_this());
	positionParameter = std::make_shared<AnimatableParameter>("Position", thisMachine, AnimatableParameter::Type::KINEMATIC_POSITION_CURVE);
	animatableParameters.push_back(positionParameter);
	//TODO: initialize animatable parameter
}

bool SingleAxisMachine::isEnabled() {
	return b_enabled;
}

bool SingleAxisMachine::isReady() {
	if (!isAxisConnected()) return false;
	std::shared_ptr<Axis> axis = getAxis();
	if (axis->hasManualControlsEnabled()) return false;
	if (!axis->isReady()) return false;
	if (axis->getCommandType() != MotionCommand::Type::POSITION_COMMAND) return false;
	if (axis->axisPositionUnitType != PositionUnit::Type::LINEAR) return false;
	return true;
}

bool SingleAxisMachine::isMoving() {
	if (isAxisConnected()) return getAxis()->isMoving();
	return false;
}

void SingleAxisMachine::enable() {
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

void SingleAxisMachine::disable() {
	b_enabled = false;
	if (isAxisConnected()) getAxis()->disable();
}

void SingleAxisMachine::process() {
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

bool SingleAxisMachine::isAxisConnected() {
	return axisLink->isConnected();
}

std::shared_ptr<Axis> SingleAxisMachine::getAxis() {
	return axisLink->getConnectedPins().front()->getAxis();
}

void SingleAxisMachine::moveToParameter() {}









//================================= MANUAL VELOCITY TARGET CONTROL ===================================

void SingleAxisMachine::setVelocity(double velocity_axisUnits) {
	manualVelocityTarget_machineUnitsPerSecond = velocity_axisUnits;
	if (controlMode == ControlMode::Mode::MANUAL_POSITION_TARGET) targetIntepolation = Motion::Interpolation();
	controlMode = ControlMode::Mode::MANUAL_VELOCITY_TARGET;
}

void SingleAxisMachine::velocityTargetControl() {
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

void SingleAxisMachine::moveToPositionWithVelocity(double position_machineUnits, double velocity_machineUnits, double acceleration_machineUnits) {
	std::shared_ptr<Axis> axis = getAxis();
	double lowPositionLimit = axis->getLowPositionLimit();
	double highPositionLimit = axis->getHighPositionLimit();
	double velocityLimit = axis->velocityLimit_axisUnitsPerSecond;

	if (position_machineUnits > highPositionLimit) position_machineUnits = highPositionLimit;
	else if (position_machineUnits < lowPositionLimit) position_machineUnits = lowPositionLimit;

	auto startPoint = std::make_shared<Motion::Point>(profileTime_seconds, profilePosition_machineUnits, acceleration_machineUnits, profileVelocity_machineUnitsPerSecond);
	auto endPoint= std::make_shared<Motion::Point>(0.0, position_machineUnits, acceleration_machineUnits, 0.0);
	if (Motion::getFastestVelocityConstrainedInterpolation(startPoint, endPoint, velocity_machineUnits, targetIntepolation)) {
		controlMode = ControlMode::Mode::MANUAL_POSITION_TARGET;
		manualVelocityTarget_machineUnitsPerSecond = 0.0;
	}
	else setVelocity(0.0);
}

void SingleAxisMachine::moveToPositionInTime(double position_machineUnits, double movementTime_seconds, double acceleration_machineUnits) {
	std::shared_ptr<Axis> axis = getAxis();
	double lowPositionLimit = axis->getLowPositionLimit();
	double highPositionLimit = axis->getHighPositionLimit();
	double velocityLimit = axis->velocityLimit_axisUnitsPerSecond;

	if (position_machineUnits > highPositionLimit) position_machineUnits = highPositionLimit;
	else if (position_machineUnits < lowPositionLimit) position_machineUnits = lowPositionLimit;

	auto startPoint= std::make_shared<Motion::Point>(profileTime_seconds, profilePosition_machineUnits, acceleration_machineUnits, profileVelocity_machineUnitsPerSecond);
	auto endPoint= std::make_shared<Motion::Point>(profileTime_seconds + movementTime_seconds, position_machineUnits, acceleration_machineUnits, 0.0);
	if (Motion::getTimeConstrainedInterpolation(startPoint, endPoint, velocityLimit, targetIntepolation)) {
		controlMode = ControlMode::Mode::MANUAL_POSITION_TARGET;
		manualVelocityTarget_machineUnitsPerSecond = 0.0;
	}
	else setVelocity(0.0);
}

void SingleAxisMachine::positionTargetControl() {
	if (targetIntepolation.isTimeInside(profileTime_seconds)) {
		std::shared_ptr<Motion::Point> curvePoint = targetIntepolation.getPointAtTime(profileTime_seconds);
		profilePosition_machineUnits = curvePoint->position;
		profileVelocity_machineUnitsPerSecond = curvePoint->velocity;
	}
	else profileVelocity_machineUnitsPerSecond = 0.0;
}


void SingleAxisMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}