#include <pch.h>

#include "SingleAxisMachine.h"
#include "NodeGraph/Device.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

void SingleAxisMachine::process() {

	bool needsFeedback = needsPositionFeedbackDevice();
	bool needsReference = needsReferenceDevice();

	//get devices
	std::shared_ptr<ActuatorDevice> actuatorDevice = getActuatorDevice();
	std::shared_ptr<GpioDevice> referenceDevice;
	std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevice;
	if (needsFeedback) positionFeedbackDevice = getPositionFeedbackDevice();
	if (needsReference) referenceDevice = getReferenceDevice();

	//handle device state transitions
	if (b_enabled) {
		if (!actuatorDevice->b_enabled) disable();
		if (needsFeedback && !positionFeedbackDevice->b_ready) disable();
		if (needsReference && !referenceDevice->b_ready) disable();
	}

	if (needsFeedback) {
		//convert feedback units to axis units
		actualPosition_machineUnits = positionFeedbackDevice->getPosition() / feedbackUnitsPerMachineUnits;
		actualVelocity_machineUnitsPerSecond = positionFeedbackDevice->getVelocity() / feedbackUnitsPerMachineUnits;
	}

	//update timing
	//TODO: the machine should get timing information from the actuator object
	currentProfilePointTime_seconds = EtherCatFieldbus::getReferenceClock_seconds();
	currentProfilePointDeltaT_seconds = currentProfilePointTime_seconds - previousProfilePointTime_seconds;
	previousProfilePointTime_seconds = currentProfilePointTime_seconds;

	//update profile generator
	if (b_enabled) {
		switch (controlMode) {
			case ControlMode::VELOCITY_TARGET: velocityTargetControl(); break;
			case ControlMode::POSITION_TARGET: positionTargetControl(); break;
			case ControlMode::FOLLOW_CURVE: followCurveControl(); break;
			case ControlMode::HOMING: homingControl(); break;
		}
	}
	else if (needsFeedback){
		//if the axis is disabled, copy position feedback data to profile generator
		profilePosition_machineUnits = actualPosition_machineUnits;
		profileVelocity_machineUnitsPerSecond = actualVelocity_machineUnitsPerSecond;
	}

	if (!needsFeedback) {
		//if there is no position feedback, we assume the position of the axis based on the profile generator
		actualPosition_machineUnits = profilePosition_machineUnits;
		actualVelocity_machineUnitsPerSecond = profileVelocity_machineUnitsPerSecond;
	}

	switch (motionControlType) {
		case MotionControlType::Type::SERVO_CONTROL:
			actuatorDevice->setVelocityCommand(profilePosition_machineUnits * actuatorUnitsPerMachineUnits);
			break;
		case MotionControlType::Type::CLOSED_LOOP_CONTROL:
		case MotionControlType::Type::OPEN_LOOP_CONTROL:
			Logger::critical("Velocity Commands are not supported yet");
			//actuatorDevice->setCommand(profileVelocity_machineUnitsPerSecond * actuatorUnitsPerMachineUnits);
			//todo:
			//for closed loop, pid control with adjustable gains
			//for open loop, just send velocity values with unit scaling
			break;
	}

	//prepare metrics data
	double positionError = profilePosition_machineUnits - actualPosition_machineUnits / feedbackUnitsPerMachineUnits;
	double actualLoad = actuatorDevice->getLoad();
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
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		actuator->enable();
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		//wait for actuator to be enabled
		while (!actuator->isEnabled()) {
			if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(500)) {
				return Logger::warn("Could not enable Machine '{}', actuator did not enable on time", getName());
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
	getActuatorDevice()->disable();
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
	//checks if all connected devices are ready and compatible

	if (!isActuatorDeviceConnected()) return false;
	std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
	if (!actuator->isReady()) return false;

	if (needsPositionFeedbackDevice() && (!isPositionFeedbackDeviceConnected() || !getPositionFeedbackDevice()->isReady())) return false;
	if (needsReferenceDevice() && (!isReferenceDeviceConnected() || !getReferenceDevice()->isReady())) return false;

	return true;
}

//========================== IO DEVICES =============================

bool SingleAxisMachine::needsPositionFeedbackDevice() {
	switch (motionControlType) {
		case MotionControlType::Type::CLOSED_LOOP_CONTROL: return true;
		case MotionControlType::Type::SERVO_CONTROL: return true;
		default: return false;
	}
}

bool SingleAxisMachine::isPositionFeedbackDeviceConnected() {
	return positionFeedbackDeviceLink->isConnected();
}

std::shared_ptr<PositionFeedbackDevice> SingleAxisMachine::getPositionFeedbackDevice() {
	return positionFeedbackDeviceLink->getConnectedPins().front()->getPositionFeedbackDevice();
}

double SingleAxisMachine::getPositionFeedback() {
	return positionFeedbackDeviceLink->getConnectedPins().front()->getPositionFeedbackDevice()->getPosition();
}

bool SingleAxisMachine::needsReferenceDevice() {
	switch (positionLimitType) {
		case PositionLimitType::Type::LOW_LIMIT_SIGNAL:
		case PositionLimitType::Type::HIGH_LIMIT_SIGNAL:
		case PositionLimitType::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
		case PositionLimitType::Type::REFERENCE_SIGNAL:
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

bool SingleAxisMachine::isActuatorDeviceConnected() {
	return actuatorDeviceLink->isConnected();
}
std::shared_ptr<ActuatorDevice> SingleAxisMachine::getActuatorDevice() {
	return actuatorDeviceLink->getConnectedPins().front()->getActuatorDevice();
}

void SingleAxisMachine::getPositionRange(double& lowLimit, double& highLimit) {
	if (!isPositionFeedbackDeviceConnected())return;
	switch (positionLimitType) {
		case PositionLimitType::Type::LOW_LIMIT_SIGNAL:
		case PositionLimitType::Type::HIGH_LIMIT_SIGNAL:
		case PositionLimitType::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
		case PositionLimitType::Type::REFERENCE_SIGNAL:
		case PositionLimitType::Type::FEEDBACK_REFERENCE:
		case PositionLimitType::Type::NO_LIMIT:
			lowLimit = getPositionFeedbackDevice()->rangeMin_positionUnits / feedbackUnitsPerMachineUnits;
			highLimit = getPositionFeedbackDevice()->rangeMax_positionUnits / feedbackUnitsPerMachineUnits;
			break;
	}
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
		setVelocity(0.0);
	}
}

//=================================================================


void SingleAxisMachine::followCurveControl() {}


void SingleAxisMachine::homingControl() {}


//==================================== SAVING AND LOADING =========================================


bool SingleAxisMachine::save(tinyxml2::XMLElement* xml) {

	using namespace tinyxml2;

	XMLElement* machineXML = xml->InsertNewChildElement("Machine");
	machineXML->SetAttribute("UnitType", getPositionUnitType(machinePositionUnitType)->saveName);
	machineXML->SetAttribute("Unit", getPositionUnit(machinePositionUnit)->saveName);
	machineXML->SetAttribute("MotionControl", getMotionControlType(motionControlType)->saveName);

	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("FeedbackUnitsPerMachineUnit", feedbackUnitsPerMachineUnits);
	unitConversionXML->SetAttribute("ActuatorUnitsPerMachineUnit", actuatorUnitsPerMachineUnits);
	unitConversionXML->SetAttribute("FeedbackAndActuatorConversionIdentical", feedbackAndActuatorConversionIdentical);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_machineUnitsPerSecond", velocityLimit_machineUnitsPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_machineUnitsPerSecondSquared", accelerationLimit_machineUnitsPerSecondSquared);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReference");
	positionReferenceXML->SetAttribute("Type", getPositionLimitType(positionLimitType)->saveName);
	switch (positionLimitType) {
	case PositionLimitType::Type::LOW_LIMIT_SIGNAL:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation_machineUnits", allowedPositiveDeviationFromReference_machineUnits);
		positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
		break;
	case PositionLimitType::Type::HIGH_LIMIT_SIGNAL:
		positionReferenceXML->SetAttribute("MaxNegativeDeviation_machineUnits", allowedNegativeDeviationFromReference_machineUnits);
		positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
		break;
	case PositionLimitType::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation_machineUnits", allowedPositiveDeviationFromReference_machineUnits);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation_machineUnits", allowedNegativeDeviationFromReference_machineUnits);
		positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", getHomingDirectionType(homingDirectionType)->saveName);
		break;
	case PositionLimitType::Type::REFERENCE_SIGNAL:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation_machineUnits", allowedPositiveDeviationFromReference_machineUnits);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation_machineUnits", allowedNegativeDeviationFromReference_machineUnits);
		positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", getHomingDirectionType(homingDirectionType)->saveName);
		break;
	case PositionLimitType::Type::FEEDBACK_REFERENCE:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation_machineUnits", allowedPositiveDeviationFromReference_machineUnits);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation_machineUnits", allowedNegativeDeviationFromReference_machineUnits);
		break;
	case PositionLimitType::Type::NO_LIMIT:
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
	motionControlType = getMotionControlType(motionControlTypeString)->type;

	XMLElement* unitConversionXML = xml->FirstChildElement("UnitConversion");
	if (!unitConversionXML) return Logger::warn("Could not load Unit Conversion");
	if (unitConversionXML->QueryDoubleAttribute("FeedbackUnitsPerMachineUnit", &feedbackUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Feedback Units Per Machine Unit");
	if (unitConversionXML->QueryDoubleAttribute("ActuatorUnitsPerMachineUnit", &actuatorUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Actuator Units Per Machine Unit");
	if (unitConversionXML->QueryBoolAttribute("FeedbackAndActuatorConversionIdentical", &feedbackAndActuatorConversionIdentical) != XML_SUCCESS) return Logger::warn("Could not load FeedbackAndActuatorConversionIdentical");

	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReference");
	if (!positionReferenceXML) return Logger::warn("Could not load Machine Position Reference");
	const char* positionLimitTypeString;
	if (positionReferenceXML->QueryStringAttribute("Type", &positionLimitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	if (getPositionLimitType(positionLimitTypeString) == nullptr) return Logger::warn("Could not read Position Reference Type");
	positionLimitType = getPositionLimitType(positionLimitTypeString)->type;
	switch (positionLimitType) {
	case PositionLimitType::Type::LOW_LIMIT_SIGNAL:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation_machineUnits", &allowedPositiveDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_machineUnitsPerSecond", &homingVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionLimitType::Type::HIGH_LIMIT_SIGNAL:
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation_machineUnits", &allowedNegativeDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_machineUnitsPerSecond", &homingVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionLimitType::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
	case PositionLimitType::Type::REFERENCE_SIGNAL:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation_machineUnits", &allowedPositiveDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation_machineUnits", &allowedNegativeDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_machineUnitsPerSecond", &homingVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		const char* homingDirectionString;
		if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirectionString)) return Logger::warn("Could not load homing direction");
		if (getHomingDirectionType(homingDirectionString) == nullptr) return Logger::warn("Could not read homing direction");
		homingDirectionType = getHomingDirectionType(homingDirectionString)->type;
		break;
	case PositionLimitType::Type::FEEDBACK_REFERENCE:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation_machineUnits", &allowedPositiveDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation_machineUnits", &allowedNegativeDeviationFromReference_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
		break;
	case PositionLimitType::Type::NO_LIMIT:
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
