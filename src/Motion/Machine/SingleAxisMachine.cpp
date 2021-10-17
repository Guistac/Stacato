#include <pch.h>

#include "SingleAxisMachine.h"
#include "NodeGraph/Device.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

void SingleAxisMachine::process() {

	//check connection requirements and abort processing if the requirements are not met
	bool needsFeedback = needsPositionFeedbackDevice();
	bool needsReference = needsReferenceDevice();
	bool needsActuator = needsActuatorDevice();
	bool needsServo = needsServoActuatorDevice();
	if (needsFeedback && !isPositionFeedbackDeviceConnected()) return;
	if (needsReference && !isReferenceDeviceConnected()) return;
	if (needsActuator && !isActuatorDeviceConnected()) return;
	if (needsServo && !isServoActuatorDeviceConnected()) return;

	//get devices
	std::shared_ptr<ActuatorDevice> actuatorDevice;
	std::shared_ptr<GpioDevice> referenceDevice;
	std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevice;
	std::shared_ptr<ServoActuatorDevice> servoActuatorDevice;
	if (needsFeedback) positionFeedbackDevice = getPositionFeedbackDevice();
	if (needsReference) referenceDevice = getReferenceDevice();
	if (needsActuator) actuatorDevice = getActuatorDevice();
	if (needsServo) servoActuatorDevice = getServoActuatorDevice();

	//get reference signals
	if (needsReference) updateReferenceSignals();

	//handle device state transitions
	if (b_enabled) {
		if (needsActuator && !actuatorDevice->b_enabled) disable();
		if (needsFeedback && !positionFeedbackDevice->b_ready) disable();
		if (needsReference && !referenceDevice->b_ready) disable();
		if (needsServo && (!servoActuatorDevice->b_ready || !servoActuatorDevice->b_enabled)) {
			if(!isHoming())	disable();
		}
	}

	if (needsFeedback) {
		//convert feedback units to axis units
		actualPosition_machineUnits = positionFeedbackDevice->getPosition() / feedbackUnitsPerMachineUnits;
		actualVelocity_machineUnitsPerSecond = positionFeedbackDevice->getVelocity() / feedbackUnitsPerMachineUnits;
	}
	else if (needsServo) {
		actualPosition_machineUnits = servoActuatorDevice->getPosition() / actuatorUnitsPerMachineUnits;
		actualVelocity_machineUnitsPerSecond = servoActuatorDevice->getVelocity() / actuatorUnitsPerMachineUnits;
	}

	//update timing
	//TODO: the machine should get timing information from the actuator object or servo actuator object
	currentProfilePointTime_seconds = EtherCatFieldbus::getReferenceClock_seconds();
	currentProfilePointDeltaT_seconds = currentProfilePointTime_seconds - previousProfilePointTime_seconds;
	previousProfilePointTime_seconds = currentProfilePointTime_seconds;

	//update profile generator
	if (b_enabled) {
		if (b_isHoming) homingControl();
		switch (controlMode) {
			case ControlMode::VELOCITY_TARGET: velocityTargetControl(); break;
			case ControlMode::POSITION_TARGET: positionTargetControl(); break;
			case ControlMode::FOLLOW_CURVE: followCurveControl(); break;
		}
	}
	else if (needsFeedback || needsServo){
		//if the axis is disabled, copy position feedback data to profile generator
		profilePosition_machineUnits = actualPosition_machineUnits;
		profileVelocity_machineUnitsPerSecond = actualVelocity_machineUnitsPerSecond;
	}

	if (!needsFeedback && !needsServo) {
		//if there is no position feedback, we assume the position of the axis based on the profile generator
		actualPosition_machineUnits = profilePosition_machineUnits;
		actualVelocity_machineUnitsPerSecond = profileVelocity_machineUnitsPerSecond;
	}

	switch (motionControl) {
		case MotionControl::Type::SERVO_CONTROL:
			servoActuatorDevice->setPositionCommand(profilePosition_machineUnits * actuatorUnitsPerMachineUnits);
			break;
		case MotionControl::Type::CLOSED_LOOP_CONTROL:
		case MotionControl::Type::OPEN_LOOP_CONTROL:
			Logger::critical("Velocity Commands are not supported yet");
			//actuatorDevice->setCommand(profileVelocity_machineUnitsPerSecond * actuatorUnitsPerMachineUnits);
			//todo:
			//for closed loop, pid control with adjustable gains
			//for open loop, just send velocity values with unit scaling
			break;
	}

	position->set(actualPosition_machineUnits);
	velocity->set(actualVelocity_machineUnitsPerSecond);

	//prepare metrics data
	double positionError = profilePosition_machineUnits - actualPosition_machineUnits / feedbackUnitsPerMachineUnits;
	double actualLoad;
	if (needsActuator) actualLoad = actuatorDevice->getLoad();
	else if (needsServo) actualLoad = servoActuatorDevice->getLoad();
	positionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profilePosition_machineUnits));
	actualPositionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actualPosition_machineUnits));
	positionErrorHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, positionError));
	velocityHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileVelocity_machineUnitsPerSecond));
	accelerationHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileAcceleration_machineUnitsPerSecondSquared));
	loadHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actualLoad));
}

//=================================== STATE CONTROL ============================================

void SingleAxisMachine::enable() {
	std::thread machineEnabler([this]() {
		//enable actuator
		if (needsActuatorDevice()) getServoActuatorDevice()->enable();
		else if (needsServoActuatorDevice()) getServoActuatorDevice()->enable();
		//wait for actuator to be enabled
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		if (needsActuatorDevice()) {
			while (!getActuatorDevice()->isEnabled()) {
				if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(500)) return Logger::warn("Could not enable Machine '{}', actuator did not enable on time", getName());
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
		else if (needsServoActuatorDevice()) {
			while (!getServoActuatorDevice()->isEnabled()) {
				if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(500)) return Logger::warn("Could not enable Machine '{}', servo actuator did not enable on time", getName());
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
		if (!isReady()) Logger::warn("Machine '{}' cannot be enabled", getName());
		else onEnable();
	});
	machineEnabler.detach();
}

void SingleAxisMachine::onEnable() {
	targetCurveProfile = MotionCurve::CurveProfile();
	manualVelocityTarget_machineUnitsPerSecond = 0.0;
	profileVelocity_machineUnitsPerSecond = 0.0;
	profileAcceleration_machineUnitsPerSecondSquared = 0.0;
	b_enabled = true;
	Logger::info("Machine '{}' was enabled", getName());
}

void SingleAxisMachine::disable() {
	b_enabled = false;
	if (needsActuatorDevice()) getActuatorDevice()->disable();
	else if (needsServoActuatorDevice()) getServoActuatorDevice()->disable();
	targetCurveProfile = MotionCurve::CurveProfile();
	manualVelocityTarget_machineUnitsPerSecond = 0.0;
	profileVelocity_machineUnitsPerSecond = 0.0;
	profileAcceleration_machineUnitsPerSecondSquared = 0.0;
	Logger::info("Machine {} disabled", getName());
}

bool SingleAxisMachine::isEnabled() {
	return b_enabled;
}

bool SingleAxisMachine::isReady() {
	//checks if all connected devices are connected and ready
	if (needsActuatorDevice() && (!isActuatorDeviceConnected() || !getActuatorDevice()->isReady())) return false;
	else if (needsServoActuatorDevice() && (!isServoActuatorDeviceConnected() || !getServoActuatorDevice()->isReady())) return false;
	if (needsPositionFeedbackDevice() && (!isPositionFeedbackDeviceConnected() || !getPositionFeedbackDevice()->isReady())) return false;
	if (needsReferenceDevice() && (!isReferenceDeviceConnected() || !getReferenceDevice()->isReady())) return false;
	return true;
}

//========================== IO DEVICES =============================

bool SingleAxisMachine::needsPositionFeedbackDevice() {
	switch (motionControl) {
		case MotionControl::Type::CLOSED_LOOP_CONTROL:
			return true;
		case MotionControl::Type::OPEN_LOOP_CONTROL:
		case MotionControl::Type::SERVO_CONTROL:
			return false;
	}
}

bool SingleAxisMachine::isPositionFeedbackDeviceConnected() {
	return positionFeedbackDeviceLink->isConnected();
}

std::shared_ptr<PositionFeedbackDevice> SingleAxisMachine::getPositionFeedbackDevice() {
	return positionFeedbackDeviceLink->getConnectedPins().front()->getPositionFeedbackDevice();
}



bool SingleAxisMachine::needsReferenceDevice() {
	switch (positionReference) {
		case PositionReference::Type::LOW_LIMIT_SIGNAL:
		case PositionReference::Type::HIGH_LIMIT_SIGNAL:
		case PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
		case PositionReference::Type::REFERENCE_SIGNAL:
			return true;
		default:
			return false;
	}
}

bool SingleAxisMachine::isReferenceDeviceConnected() {
	return referenceDeviceLink->isConnected();
}

std::shared_ptr<GpioDevice> SingleAxisMachine::getReferenceDevice() {
	return referenceDeviceLink->getConnectedPins().front()->getGpioDevice();
}

void SingleAxisMachine::updateReferenceSignals() {
	switch (positionReference) {
		case PositionReference::Type::LOW_LIMIT_SIGNAL:
			previousLowLimitSignal = lowLimitSignal;
			if (lowLimitSignalPin->isConnected()) lowLimitSignalPin->set(lowLimitSignalPin->getConnectedPins().front()->getBoolean());
			lowLimitSignal = lowLimitSignalPin->getBoolean();
			break;
		case PositionReference::Type::HIGH_LIMIT_SIGNAL:
			previousHighLimitSignal = highLimitSignal;
			if (highLimitSignalPin->isConnected()) highLimitSignalPin->set(highLimitSignalPin->getConnectedPins().front()->getBoolean());
			highLimitSignal = highLimitSignalPin->getBoolean();
			break;
		case PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
			previousLowLimitSignal = lowLimitSignal;
			if (lowLimitSignalPin->isConnected()) lowLimitSignalPin->set(lowLimitSignalPin->getConnectedPins().front()->getBoolean());
			lowLimitSignal = lowLimitSignalPin->getBoolean();
			previousHighLimitSignal = highLimitSignal;
			if (highLimitSignalPin->isConnected()) highLimitSignalPin->set(highLimitSignalPin->getConnectedPins().front()->getBoolean());
			highLimitSignal = highLimitSignalPin->getBoolean();
			break;
		case PositionReference::Type::REFERENCE_SIGNAL:
			previousReferenceSignal = referenceSignal;
			if (referenceSignalPin->isConnected()) referenceSignalPin->set(referenceSignalPin->getConnectedPins().front()->getBoolean());
			referenceSignal = referenceSignalPin->getBoolean();
			break;
		default: break;
	}
}

bool SingleAxisMachine::needsActuatorDevice() {
	switch (motionControl) {
		case MotionControl::Type::CLOSED_LOOP_CONTROL:
		case MotionControl::Type::OPEN_LOOP_CONTROL:
			return true;
		case MotionControl::Type::SERVO_CONTROL:
			return false;
	}
}

bool SingleAxisMachine::isActuatorDeviceConnected() {
	return actuatorDeviceLink->isConnected();
}
std::shared_ptr<ActuatorDevice> SingleAxisMachine::getActuatorDevice() {
	return actuatorDeviceLink->getConnectedPins().front()->getActuatorDevice();
}

void SingleAxisMachine::getPositionRange(double& lowLimit, double& highLimit) {
	if (needsPositionFeedbackDevice() && !isPositionFeedbackDeviceConnected()) {
		lowLimit = 0.0;
		highLimit = 0.0;
		return;
	}
	else if (needsServoActuatorDevice() && !isServoActuatorDeviceConnected()) {
		lowLimit = 0.0;
		highLimit = 0.0;
		return;
	}
	switch (positionReference) {
		case PositionReference::Type::LOW_LIMIT_SIGNAL:
		case PositionReference::Type::HIGH_LIMIT_SIGNAL:
		case PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
		case PositionReference::Type::REFERENCE_SIGNAL:
		case PositionReference::Type::FEEDBACK_REFERENCE:
		case PositionReference::Type::NO_LIMIT:
			if (needsPositionFeedbackDevice()) {
				lowLimit = getPositionFeedbackDevice()->rangeMin_positionUnits / feedbackUnitsPerMachineUnits;
				highLimit = getPositionFeedbackDevice()->rangeMax_positionUnits / feedbackUnitsPerMachineUnits;
			}
			else if (needsServoActuatorDevice()) {
				lowLimit = getServoActuatorDevice()->rangeMin_positionUnits / feedbackUnitsPerMachineUnits;
				highLimit = getServoActuatorDevice()->rangeMax_positionUnits / feedbackUnitsPerMachineUnits;
			}
			break;
	}
}



bool SingleAxisMachine::needsServoActuatorDevice() {
	switch (motionControl) {
		case MotionControl::Type::CLOSED_LOOP_CONTROL:
		case MotionControl::Type::OPEN_LOOP_CONTROL:
			return false;
		case MotionControl::Type::SERVO_CONTROL:
			return true;
	}
}

bool SingleAxisMachine::isServoActuatorDeviceConnected() {
	return servoActuatorDeviceLink->isConnected();
}

std::shared_ptr<ServoActuatorDevice> SingleAxisMachine::getServoActuatorDevice() {
	return servoActuatorDeviceLink->getConnectedPins().front()->getServoActuatorDevice();
}


//================================= MANUAL CONTROLS ===================================

void SingleAxisMachine::setVelocity(double velocity_machineUnits) {
	manualVelocityTarget_machineUnitsPerSecond = velocity_machineUnits;
	if (controlMode == ControlMode::POSITION_TARGET) {
		targetCurveProfile = MotionCurve::CurveProfile();
	}
	controlMode = ControlMode::VELOCITY_TARGET;
}

void SingleAxisMachine::velocityTargetControl() {
	if (profileVelocity_machineUnitsPerSecond != manualVelocityTarget_machineUnitsPerSecond) {
		double deltaV_machineUnitsPerSecond = manualControlAcceleration_machineUnitsPerSecond * currentProfilePointDeltaT_seconds;
		if (profileVelocity_machineUnitsPerSecond < manualVelocityTarget_machineUnitsPerSecond) {
			profileVelocity_machineUnitsPerSecond += deltaV_machineUnitsPerSecond;
			profileAcceleration_machineUnitsPerSecondSquared = manualControlAcceleration_machineUnitsPerSecond;
			if (profileVelocity_machineUnitsPerSecond > manualVelocityTarget_machineUnitsPerSecond) profileVelocity_machineUnitsPerSecond = manualVelocityTarget_machineUnitsPerSecond;
		}
		else {
			profileVelocity_machineUnitsPerSecond -= deltaV_machineUnitsPerSecond;
			profileAcceleration_machineUnitsPerSecondSquared = -manualControlAcceleration_machineUnitsPerSecond;
			if (profileVelocity_machineUnitsPerSecond < manualVelocityTarget_machineUnitsPerSecond) profileVelocity_machineUnitsPerSecond = manualVelocityTarget_machineUnitsPerSecond;
		}
	}
	else profileAcceleration_machineUnitsPerSecondSquared = 0.0;
	double deltaP_machineUnits = profileVelocity_machineUnitsPerSecond * currentProfilePointDeltaT_seconds;
	profilePosition_machineUnits += deltaP_machineUnits;
}

void SingleAxisMachine::moveToPositionWithVelocity(double position_machineUnits, double velocity_machineUnits, double acceleration_machineUnits) {
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_machineUnits, acceleration_machineUnits, profileVelocity_machineUnitsPerSecond);
	MotionCurve::CurvePoint endPoint(0.0, position_machineUnits, acceleration_machineUnits, 0.0);
	if (MotionCurve::getFastestVelocityConstrainedProfile(startPoint, endPoint, velocity_machineUnits, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_machineUnitsPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void SingleAxisMachine::moveToPositionInTime(double position_machineUnits, double movementTime_seconds, double acceleration_machineUnits) {
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_machineUnits, acceleration_machineUnits, profileVelocity_machineUnitsPerSecond);
	MotionCurve::CurvePoint endPoint(currentProfilePointTime_seconds + movementTime_seconds, position_machineUnits, acceleration_machineUnits, 0.0);
	if (MotionCurve::getTimeConstrainedProfile(startPoint, endPoint, velocityLimit_machineUnitsPerSecond, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_machineUnitsPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void SingleAxisMachine::positionTargetControl() {
	if (MotionCurve::isInsideCurveTime(currentProfilePointTime_seconds, targetCurveProfile)) {
		MotionCurve::CurvePoint curvePoint = MotionCurve::getCurvePointAtTime(currentProfilePointTime_seconds, targetCurveProfile);
		profilePosition_machineUnits = curvePoint.position;
		profileVelocity_machineUnitsPerSecond = curvePoint.velocity;
		profileAcceleration_machineUnitsPerSecondSquared = curvePoint.acceleration;
	}
	else {
		profileVelocity_machineUnitsPerSecond = 0.0;
		profileAcceleration_machineUnitsPerSecondSquared = 0.0;
	}
}

//=================================================================


void SingleAxisMachine::followCurveControl() {}


void SingleAxisMachine::startHoming() {
	b_isHoming = true;
	homingStep = Homing::Step::NOT_STARTED;
	homingError = Homing::Error::NONE;
}

void SingleAxisMachine::cancelHoming() {
	b_isHoming = false;
	homingError = Homing::Error::HOMING_CANCELED;
	controlMode = ControlMode::VELOCITY_TARGET;
	setVelocity(0.0);
}

bool SingleAxisMachine::isHoming() {
	return b_isHoming;
}

bool SingleAxisMachine::didHomingSucceed() {
	return !isHoming() && homingStep == Homing::Step::FINISHED;
}

bool SingleAxisMachine::didHomingFail() {
	return homingError != Homing::Error::NONE;
}

void SingleAxisMachine::onHomingSuccess() {
	b_isHoming = false;
	controlMode = ControlMode::VELOCITY_TARGET;
	homingStep = Homing::Step::FINISHED;
	setVelocity(0.0);
}

void SingleAxisMachine::onHomingError() {
	b_isHoming = false;
	homingStep = Homing::Step::NOT_STARTED;
	disable();
}

void SingleAxisMachine::homingControl() {

	using namespace Homing;

	switch (positionReference) {



		case PositionReference::Type::LOW_LIMIT_SIGNAL:

			switch (homingStep) {
				case Step::NOT_STARTED:
					homingStep = Step::SEARCHING_LOW_LIMIT_COARSE;
					break;
				case Step::SEARCHING_LOW_LIMIT_COARSE:
					setVelocity(-homingVelocity_machineUnitsPerSecond);
					//look for a limit signal transition from low to high
					if (!previousHighLimitSignal && highLimitSignal) {
						homingError = Error::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousLowLimitSignal && lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_COARSE;
					break;
				case Step::FOUND_LOW_LIMIT_COARSE:
					setVelocity(0.0);
					if (actualVelocity_machineUnitsPerSecond == 0.0) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_machineUnits = actualPosition_machineUnits;
							profileVelocity_machineUnitsPerSecond = 0.0;
							profileAcceleration_machineUnitsPerSecondSquared = 0.0;
						}
						else homingStep = Step::SEARCHING_LOW_LIMIT_FINE;
					}
					break;
				case Step::SEARCHING_LOW_LIMIT_FINE:
					setVelocity(homingVelocity_machineUnitsPerSecond / 10.0);
					//look for a limit signal transition from high to low
					if (previousLowLimitSignal && !lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_FINE;
					break;
				case Step::FOUND_LOW_LIMIT_FINE:
					setVelocity(0.0);
					if (actualVelocity_machineUnitsPerSecond == 0.0) homingStep = Step::RESETTING_POSITION_FEEDBACK;
					break;
				case Step::RESETTING_POSITION_FEEDBACK:
					if (needsServoActuatorDevice()) {
						std::shared_ptr<ServoActuatorDevice> servo = getServoActuatorDevice();
						servo->setEncoderPosition(0.0);
						profilePosition_machineUnits = servo->getPosition() / feedbackUnitsPerMachineUnits;
					}
					else if (needsPositionFeedbackDevice()) {
						std::shared_ptr<PositionFeedbackDevice> feedback = getPositionFeedbackDevice();
						feedback->setPosition(0.0);
						profilePosition_machineUnits = feedback->getPosition() / feedbackUnitsPerMachineUnits;
					}
					onHomingSuccess();
					break;
			}
			break;
			


		case PositionReference::Type::HIGH_LIMIT_SIGNAL:
		


		case PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
			


			switch (homingStep) {
				case Step::NOT_STARTED:
					homingStep = Step::SEARCHING_LOW_LIMIT_COARSE;
					break;
				case Step::SEARCHING_LOW_LIMIT_COARSE:
					setVelocity(-homingVelocity_machineUnitsPerSecond);
					//look for a limit signal transition from low to high
					if (!previousHighLimitSignal && highLimitSignal) {
						homingError = Error::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousLowLimitSignal && lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_COARSE;
					break;
				case Step::FOUND_LOW_LIMIT_COARSE:
					setVelocity(0.0);
					if (actualVelocity_machineUnitsPerSecond == 0.0) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_machineUnits = actualPosition_machineUnits;
							profileVelocity_machineUnitsPerSecond = 0.0;
							profileAcceleration_machineUnitsPerSecondSquared = 0.0;
						}
						else homingStep = Step::SEARCHING_LOW_LIMIT_FINE;
					}
					break;
				case Step::SEARCHING_LOW_LIMIT_FINE:
					setVelocity(homingVelocity_machineUnitsPerSecond / 10.0);
					//look for a limit signal transition from high to low
					if (previousLowLimitSignal && !lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_FINE;
					break;
				case Step::FOUND_LOW_LIMIT_FINE:
					setVelocity(0.0);
					if (actualVelocity_machineUnitsPerSecond == 0.0) homingStep = Step::RESETTING_POSITION_FEEDBACK;
					break;
				case Step::RESETTING_POSITION_FEEDBACK:
					if (needsServoActuatorDevice()) {
						std::shared_ptr<ServoActuatorDevice> servo = getServoActuatorDevice();
						servo->setEncoderPosition(0.0);
						profilePosition_machineUnits = servo->getPosition() / feedbackUnitsPerMachineUnits;
					}
					else if (needsPositionFeedbackDevice()) {
						std::shared_ptr<PositionFeedbackDevice> feedback = getPositionFeedbackDevice();
						feedback->setPosition(0.0);
						profilePosition_machineUnits = feedback->getPosition() / feedbackUnitsPerMachineUnits;
					}
					homingStep = Step::SEARCHING_HIGH_LIMIT_COARSE;
					break;
				case Step::SEARCHING_HIGH_LIMIT_COARSE:
					setVelocity(homingVelocity_machineUnitsPerSecond);
					//look for a limit signal transition from low to high
					if (!previousLowLimitSignal && lowLimitSignal) {
						homingError = Error::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousHighLimitSignal && highLimitSignal) homingStep = Step::FOUND_HIGH_LIMIT_COARSE;
					break;
				case Step::FOUND_HIGH_LIMIT_COARSE:
					setVelocity(0.0);
					if (actualVelocity_machineUnitsPerSecond == 0.0) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_machineUnits = actualPosition_machineUnits;
							profileVelocity_machineUnitsPerSecond = 0.0;
							profileAcceleration_machineUnitsPerSecondSquared = 0.0;
						}
						else homingStep = Step::SEARCHING_HIGH_LIMIT_FINE;
					}
					break;
				case Step::SEARCHING_HIGH_LIMIT_FINE:
					setVelocity(-homingVelocity_machineUnitsPerSecond / 10.0);
					//look for a limit signal transition from high to low
					if (previousHighLimitSignal && !highLimitSignal) homingStep = Step::FOUND_HIGH_LIMIT_FINE;
					break;
				case Step::FOUND_HIGH_LIMIT_FINE:
					setVelocity(0.0);
					if (actualVelocity_machineUnitsPerSecond == 0.0) homingStep = Step::SETTING_HIGH_LIMIT;
					break;
				case Step::SETTING_HIGH_LIMIT:
					//set high limit
					homingStep = Step::FINISHED;
					onHomingSuccess();
					break;
			}
			break;



		case PositionReference::Type::REFERENCE_SIGNAL:
			//touch off reference from one side, touch off reference from other side, go to middle of two touches, set encoder to zero
		case PositionReference::Type::FEEDBACK_REFERENCE:
			//no homing
		case PositionReference::Type::NO_LIMIT:
			//no homing
			break;
	}
}


//==================================== SAVING AND LOADING =========================================


bool SingleAxisMachine::save(tinyxml2::XMLElement* xml) {

	using namespace tinyxml2;

	XMLElement* machineXML = xml->InsertNewChildElement("Machine");
	machineXML->SetAttribute("UnitType", getPositionUnitType(machinePositionUnitType)->saveName);
	machineXML->SetAttribute("Unit", getPositionUnit(machinePositionUnit)->saveName);
	machineXML->SetAttribute("MotionControl", getMotionControlType(motionControl)->saveName);

	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("FeedbackUnitsPerMachineUnit", feedbackUnitsPerMachineUnits);
	unitConversionXML->SetAttribute("ActuatorUnitsPerMachineUnit", actuatorUnitsPerMachineUnits);
	unitConversionXML->SetAttribute("FeedbackAndActuatorConversionIdentical", feedbackAndActuatorConversionIdentical);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_machineUnitsPerSecond", velocityLimit_machineUnitsPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_machineUnitsPerSecondSquared", accelerationLimit_machineUnitsPerSecondSquared);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReference");
	positionReferenceXML->SetAttribute("Type", getPositionReference(positionReference)->saveName);
	switch (positionReference) {
		case PositionReference::Type::LOW_LIMIT_SIGNAL:
			positionReferenceXML->SetAttribute("MaxPositiveDeviation_machineUnits", allowedPositiveDeviationFromReference_machineUnits);
			positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
			break;
		case PositionReference::Type::HIGH_LIMIT_SIGNAL:
			positionReferenceXML->SetAttribute("MaxNegativeDeviation_machineUnits", allowedNegativeDeviationFromReference_machineUnits);
			positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
			break;
		case PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
			positionReferenceXML->SetAttribute("MaxPositiveDeviation_machineUnits", allowedPositiveDeviationFromReference_machineUnits);
			positionReferenceXML->SetAttribute("MaxNegativeDeviation_machineUnits", allowedNegativeDeviationFromReference_machineUnits);
			positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
			positionReferenceXML->SetAttribute("HomingDirection", getHomingDirection(homingDirection)->saveName);
			break;
		case PositionReference::Type::REFERENCE_SIGNAL:
			positionReferenceXML->SetAttribute("MaxPositiveDeviation_machineUnits", allowedPositiveDeviationFromReference_machineUnits);
			positionReferenceXML->SetAttribute("MaxNegativeDeviation_machineUnits", allowedNegativeDeviationFromReference_machineUnits);
			positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
			positionReferenceXML->SetAttribute("HomingDirection", getHomingDirection(homingDirection)->saveName);
			break;
		case PositionReference::Type::FEEDBACK_REFERENCE:
			positionReferenceXML->SetAttribute("MaxPositiveDeviation_machineUnits", allowedPositiveDeviationFromReference_machineUnits);
			positionReferenceXML->SetAttribute("MaxNegativeDeviation_machineUnits", allowedNegativeDeviationFromReference_machineUnits);
			break;
		case PositionReference::Type::NO_LIMIT:
			break;
	}

	XMLElement* defaultManualParametersXML = xml->InsertNewChildElement("DefaultManualParameters");
	defaultManualParametersXML->SetAttribute("Velocity_machineUnitsPerSecond", defaultManualVelocity_machineUnitsPerSecond);
	defaultManualParametersXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", defaultManualAcceleration_machineUnitsPerSecondSquared);

	return false;
}



bool SingleAxisMachine::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* machineXML = xml->FirstChildElement("Machine");
	if (!machineXML) return Logger::warn("Could not load Machine Attributes");
	const char* machineUnitTypeString;
	if (machineXML->QueryStringAttribute("UnitType", &machineUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit Type");
	if (getPositionUnitType(machineUnitTypeString) == nullptr) return Logger::warn("Could not read Machine Unit Type");
	machinePositionUnitType = getPositionUnitType(machineUnitTypeString)->type;
	const char* machineUnitString;
	if (machineXML->QueryStringAttribute("Unit", &machineUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	if (getPositionUnit(machineUnitString) == nullptr) return Logger::warn("Could not read Machine Unit");
	machinePositionUnit = getPositionUnit(machineUnitString)->unit;
	const char* motionControlTypeString;
	if (machineXML->QueryStringAttribute("MotionControl", &motionControlTypeString) != XML_SUCCESS) return Logger::warn("Could not load Motion Control Type");
	if (getMotionControlType(motionControlTypeString) == nullptr) return Logger::warn("Could not read Motion Control Type");
	setMotionControlType(getMotionControlType(motionControlTypeString)->type);

	XMLElement* unitConversionXML = xml->FirstChildElement("UnitConversion");
	if (!unitConversionXML) return Logger::warn("Could not load Unit Conversion");
	if (unitConversionXML->QueryDoubleAttribute("FeedbackUnitsPerMachineUnit", &feedbackUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Feedback Units Per Machine Unit");
	if (unitConversionXML->QueryDoubleAttribute("ActuatorUnitsPerMachineUnit", &actuatorUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Actuator Units Per Machine Unit");
	if (unitConversionXML->QueryBoolAttribute("FeedbackAndActuatorConversionIdentical", &feedbackAndActuatorConversionIdentical) != XML_SUCCESS) return Logger::warn("Could not load FeedbackAndActuatorConversionIdentical");

	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReference");
	if (!positionReferenceXML) return Logger::warn("Could not load Machine Position Reference");
	const char* positionLimitTypeString;
	if (positionReferenceXML->QueryStringAttribute("Type", &positionLimitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	if (getPositionReference(positionLimitTypeString) == nullptr) return Logger::warn("Could not read Position Reference Type");
	setPositionReferenceType(getPositionReference(positionLimitTypeString)->type);

	switch (positionReference) {
		case PositionReference::Type::LOW_LIMIT_SIGNAL:
			if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation_machineUnits", &allowedPositiveDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
			if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_machineUnitsPerSecond", &homingVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
			break;
		case PositionReference::Type::HIGH_LIMIT_SIGNAL:
			if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation_machineUnits", &allowedNegativeDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
			if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_machineUnitsPerSecond", &homingVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
			break;
		case PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
		case PositionReference::Type::REFERENCE_SIGNAL:
			if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation_machineUnits", &allowedPositiveDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
			if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation_machineUnits", &allowedNegativeDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
			if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_machineUnitsPerSecond", &homingVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
			const char* homingDirectionString;
			if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirectionString)) return Logger::warn("Could not load homing direction");
			if (getHomingDirection(homingDirectionString) == nullptr) return Logger::warn("Could not read homing direction");
			homingDirection = getHomingDirection(homingDirectionString)->type;
			break;
		case PositionReference::Type::FEEDBACK_REFERENCE:
			if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation_machineUnits", &allowedPositiveDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
			if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation_machineUnits", &allowedNegativeDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
			break;
		case PositionReference::Type::NO_LIMIT:
			break;
	}

	XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
	if (!kinematicLimitsXML) return Logger::warn("Could not load Machine Kinematic Kimits");
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit_machineUnitsPerSecond", &velocityLimit_machineUnitsPerSecond)) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit_machineUnitsPerSecondSquared", &accelerationLimit_machineUnitsPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");

	XMLElement* defaultManualParametersXML = xml->FirstChildElement("DefaultManualParameters");
	if (!defaultManualParametersXML) return Logger::warn("Could not load default movement parameters");
	if (defaultManualParametersXML->QueryDoubleAttribute("Velocity_machineUnitsPerSecond", &defaultManualVelocity_machineUnitsPerSecond) != XML_SUCCESS) Logger::warn("Could not load default movement velocity");
	if (defaultManualParametersXML->QueryDoubleAttribute("Acceleration_machineUnitsPerSecondSquared", &defaultManualAcceleration_machineUnitsPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load default movement acceleration");
	manualControlAcceleration_machineUnitsPerSecond = defaultManualAcceleration_machineUnitsPerSecondSquared;
	targetVelocity_machineUnitsPerSecond = defaultManualVelocity_machineUnitsPerSecond;

	return true;
}



void SingleAxisMachine::setMotionControlType(MotionControl::Type type) {
	//remove all pins
	//we then add them all again to assure the same pin order each time
	removeIoData(positionFeedbackDeviceLink);
	removeIoData(servoActuatorDeviceLink);
	removeIoData(actuatorDeviceLink);
	removeIoData(referenceDeviceLink);
	removeIoData(lowLimitSignalPin);
	removeIoData(highLimitSignalPin);
	removeIoData(referenceSignalPin);
	switch (type) {
		case MotionControl::Type::OPEN_LOOP_CONTROL:
			addIoData(actuatorDeviceLink);
			break;
		case MotionControl::Type::CLOSED_LOOP_CONTROL:
			addIoData(actuatorDeviceLink);
			addIoData(positionFeedbackDeviceLink);
			break;
		case MotionControl::Type::SERVO_CONTROL:
			addIoData(servoActuatorDeviceLink);
			break;
	}
	motionControl = type;
	setPositionReferenceType(positionReference);
}

void SingleAxisMachine::setPositionReferenceType(PositionReference::Type type) {
	removeIoData(referenceDeviceLink);
	removeIoData(lowLimitSignalPin);
	removeIoData(highLimitSignalPin);
	removeIoData(referenceSignalPin);
	switch (type) {
		case PositionReference::Type::LOW_LIMIT_SIGNAL:
			addIoData(referenceDeviceLink);
			addIoData(lowLimitSignalPin);
			break;
		case PositionReference::Type::HIGH_LIMIT_SIGNAL:
			addIoData(referenceDeviceLink);
			addIoData(highLimitSignalPin);
			break;
		case PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
			addIoData(referenceDeviceLink);
			addIoData(highLimitSignalPin);
			addIoData(lowLimitSignalPin);
			break;
		case PositionReference::Type::REFERENCE_SIGNAL:
			addIoData(referenceDeviceLink);
			addIoData(referenceSignalPin);
			break;
		case PositionReference::Type::FEEDBACK_REFERENCE:
		case PositionReference::Type::NO_LIMIT:
			removeIoData(referenceDeviceLink);
			removeIoData(lowLimitSignalPin);
			removeIoData(highLimitSignalPin);
			removeIoData(referenceSignalPin);
			break;
	}
	positionReference = type;
}