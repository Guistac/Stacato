#include <pch.h>

#include "SingleAxisMachine.h"
#include "NodeGraph/DeviceNode.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

void SingleAxisMachine::process() {

	//get devices
	std::vector<std::shared_ptr<ActuatorDevice>> actuators;
	for (auto& pin : actuatorDeviceLinks->getConnectedPins()) actuators.push_back(pin->getActuatorDevice());
	std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevice = nullptr;
	if (feedbackDeviceLink->isConnected()) positionFeedbackDevice = feedbackDeviceLink->getConnectedPins().front()->getPositionFeedbackDevice();
	std::vector<std::shared_ptr<GpioDevice>> referenceDevices;
	for (auto& pin : referenceDeviceLinks->getConnectedPins()) referenceDevices.push_back(pin->getGpioDevice());

	//handle device state transitions
	if (b_enabled) {
		for (auto actuator : actuators) {
			if (!actuator->b_enabled) disable();
		}
		if (!positionFeedbackDevice->b_ready) disable();
		for (auto referenceDevice : referenceDevices) {
			if (!referenceDevice->b_ready) disable();
		}
	}

	//TODO: the machine should get timing information from the actuator object
	currentProfilePointTime_seconds = EtherCatFieldbus::getReferenceClock_seconds();
	currentProfilePointDeltaT_seconds = currentProfilePointTime_seconds - previousProfilePointTime_seconds;
	previousProfilePointTime_seconds = currentProfilePointTime_seconds;

	if (b_enabled) {
		switch (controlMode) {
		case ControlMode::VELOCITY_TARGET: velocityTargetControl(); break;
		case ControlMode::POSITION_TARGET: positionTargetControl(); break;
		case ControlMode::FOLLOW_CURVE: followCurveControl(); break;
		case ControlMode::HOMING: homingControl(); break;
		}
		actuatorCommand->set(profilePosition_degrees);
	}
	else {
		actuatorCommand->set(positionFeedback->getLinks().front()->getInputData()->getReal());
	}

	std::shared_ptr<PositionFeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getPositionFeedbackDevice();
	std::shared_ptr<ActuatorDevice> actuatorDevice = actuatorDeviceLinks->getLinks().front()->getInputData()->getActuatorDevice();

	double actualPosition = feedbackDevice->getPosition();
	double positionError = profilePosition_degrees - feedbackDevice->getPosition();

	positionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profilePosition_degrees));
	actualPositionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actualPosition));
	positionErrorHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, positionError));
	velocityHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileVelocity_degreesPerSecond));
	accelerationHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileAcceleration_degreesPerSecondSquared));
	loadHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actuatorDevice->getLoad()));
}



void SingleAxisMachine::enable() {
	std::thread machineEnabler([this]() {
		//enable devices
		enableAllActuators();
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		//wait for devices to be enabled
		while (std::chrono::system_clock::now() - start < std::chrono::milliseconds(500)) {
			if (areAllDevicesEnabled()) break;
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		//start machine or return feedback about failure mode
		Logger::info("Enabling machine {}", getName());
		bool canMachineBeEnabled = true;
		if (actuatorDeviceLinks->isConnected()) {
			for (auto link : actuatorDeviceLinks->getLinks()) {
				std::shared_ptr<ActuatorDevice> actuatorDevice = link->getInputData()->getActuatorDevice();
				if (!actuatorDevice->isEnabled()) {
					canMachineBeEnabled = false;
					Logger::warn("Actuator subdevice '{}' of device '{}' is not enabled", actuatorDevice->getName(), actuatorDevice->parentDevice->getName());
				}
			}
		}
		else {
			canMachineBeEnabled = false;
			Logger::warn("No Actuators are connected to machine '{}'", getName());
		}
		if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
			if (feedbackDeviceLink->isConnected()) {
				std::shared_ptr<PositionFeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getPositionFeedbackDevice();
				if (!feedbackDevice->isReady()) {
					canMachineBeEnabled = false;
					Logger::warn("Position feedback subdevice '{}' of device '{}' is not ready", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());
				}
			}
			else {
				canMachineBeEnabled = false;
				Logger::warn("No Position Feedback device is connected to machine '{}'", getName());
			}
		}
		if (positionReferenceType != PositionReference::Type::NO_LIMIT) {
			if (referenceDeviceLinks->isConnected()) {
				for (auto link : referenceDeviceLinks->getLinks()) {
					std::shared_ptr<GpioDevice> gpioDevice = link->getInputData()->getGpioDevice();
					if (!gpioDevice->isReady()) {
						canMachineBeEnabled = false;
						Logger::warn("Position reference subdevice '{}' of device '{}' is not ready", gpioDevice->getName(), gpioDevice->parentDevice->getName());
					}
				}
			}
			else {
				canMachineBeEnabled = false;
				Logger::warn("No Position reference device is connected to machine '{}'", getName());
			}
		}
		if (!canMachineBeEnabled) Logger::warn("Machine '{}' cannot be enabled", getName());
		else {
			onEnable();
			Logger::info("Machine '{}' was enabled", getName());
		}
		});
	machineEnabler.detach();
}

void SingleAxisMachine::onEnable() {
	profilePosition_degrees = positionFeedback->getLinks().front()->getInputData()->getReal();
	targetCurveProfile = MotionCurve::CurveProfile();
	profileVelocity_degreesPerSecond = 0.0;
	profileAcceleration_degreesPerSecondSquared = 0.0;
	b_enabled = true;
}

void SingleAxisMachine::disable() {
	b_enabled = false;
	onDisable();
	Logger::info("Machine {} disabled", getName());
}

void SingleAxisMachine::onDisable() {
	disableAllActuators();
	targetCurveProfile = MotionCurve::CurveProfile();
	manualVelocityTarget_degreesPerSecond = 0.0;
	profileVelocity_degreesPerSecond = 0.0;
	profileAcceleration_degreesPerSecondSquared = 0.0;
}

bool SingleAxisMachine::isEnabled() {
	return b_enabled;
}

bool SingleAxisMachine::areAllDevicesReady() {
	if (actuatorDeviceLinks->isConnected()) {
		for (auto pin : actuatorDeviceLinks->getConnectedPins()) {
			std::shared_ptr<ActuatorDevice> actuatorDevice = pin->getActuatorDevice();
			if (!actuatorDevice->isReady()) return false;
		}
	}
	else return false;
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		if (feedbackDeviceLink->isConnected()) {
			std::shared_ptr<PositionFeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getPositionFeedbackDevice();
			if (!feedbackDevice->isReady()) return false;
		}
		else return false;
	}
	if (positionReferenceType != PositionReference::Type::NO_LIMIT) {
		if (referenceDeviceLinks->isConnected()) {
			for (auto pin : referenceDeviceLinks->getConnectedPins()) {
				std::shared_ptr<GpioDevice> gpioDevice = pin->getGpioDevice();
				if (!gpioDevice->isReady()) return false;
			}
		}
		else return false;
	}
	return true;
}

bool SingleAxisMachine::areAllDevicesEnabled() {
	if (!areAllDevicesReady()) return false;
	for (auto pin : actuatorDeviceLinks->getConnectedPins()) {
		std::shared_ptr<ActuatorDevice> actuatorDevice = pin->getActuatorDevice();
		if (!actuatorDevice->isEnabled()) return false;
	}
	return true;
}

void SingleAxisMachine::enableAllActuators() {
	for (auto pin : actuatorDeviceLinks->getConnectedPins()) {
		std::shared_ptr<ActuatorDevice> actuatorDevice = pin->getActuatorDevice();
		actuatorDevice->enable();
	}
}

void SingleAxisMachine::disableAllActuators() {
	for (auto pin : actuatorDeviceLinks->getConnectedPins()) {
		std::shared_ptr<ActuatorDevice> actuatorDevice = pin->getActuatorDevice();
		actuatorDevice->disable();
	}
}

//================================= MANUAL CONTROLS ===================================

void SingleAxisMachine::setVelocity(double velocity_machineUnits) {
	manualVelocityTarget_degreesPerSecond = velocity_machineUnits;
	if (controlMode == ControlMode::POSITION_TARGET) {
		targetCurveProfile = MotionCurve::CurveProfile();
	}
	controlMode = ControlMode::VELOCITY_TARGET;
}

void SingleAxisMachine::velocityTargetControl() {
	if (profileVelocity_degreesPerSecond != manualVelocityTarget_degreesPerSecond) {
		double deltaV_degreesPerSecond = manualControlAcceleration_degreesPerSecond * currentProfilePointDeltaT_seconds;
		if (profileVelocity_degreesPerSecond < manualVelocityTarget_degreesPerSecond) {
			profileVelocity_degreesPerSecond += deltaV_degreesPerSecond;
			profileAcceleration_degreesPerSecondSquared = manualControlAcceleration_degreesPerSecond;
			if (profileVelocity_degreesPerSecond > manualVelocityTarget_degreesPerSecond) profileVelocity_degreesPerSecond = manualVelocityTarget_degreesPerSecond;
		}
		else {
			profileVelocity_degreesPerSecond -= deltaV_degreesPerSecond;
			profileAcceleration_degreesPerSecondSquared = -manualControlAcceleration_degreesPerSecond;
			if (profileVelocity_degreesPerSecond < manualVelocityTarget_degreesPerSecond) profileVelocity_degreesPerSecond = manualVelocityTarget_degreesPerSecond;
		}
	}
	else profileAcceleration_degreesPerSecondSquared = 0.0;
	double deltaP_degrees = profileVelocity_degreesPerSecond * currentProfilePointDeltaT_seconds;
	profilePosition_degrees += deltaP_degrees;
}

void SingleAxisMachine::moveToPositionWithVelocity(double position_machineUnits, double velocity_machineUnits, double acceleration_machineUnits) {
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_degrees, acceleration_machineUnits, profileVelocity_degreesPerSecond);
	MotionCurve::CurvePoint endPoint(0.0, position_machineUnits, acceleration_machineUnits, 0.0);
	if (MotionCurve::getFastestVelocityConstrainedProfile(startPoint, endPoint, velocity_machineUnits, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_degreesPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void SingleAxisMachine::moveToPositionInTime(double position_machineUnits, double movementTime_seconds, double acceleration_machineUnits) {
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_degrees, acceleration_machineUnits, profileVelocity_degreesPerSecond);
	MotionCurve::CurvePoint endPoint(currentProfilePointTime_seconds + movementTime_seconds, position_machineUnits, acceleration_machineUnits, 0.0);
	if (MotionCurve::getTimeConstrainedProfile(startPoint, endPoint, velocityLimit_degreesPerSecond, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_degreesPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void SingleAxisMachine::positionTargetControl() {
	if (MotionCurve::isInsideCurveTime(currentProfilePointTime_seconds, targetCurveProfile)) {
		MotionCurve::CurvePoint curvePoint = MotionCurve::getCurvePointAtTime(currentProfilePointTime_seconds, targetCurveProfile);
		profilePosition_degrees = curvePoint.position;
		profileVelocity_degreesPerSecond = curvePoint.velocity;
		profileAcceleration_degreesPerSecondSquared = curvePoint.acceleration;
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

	XMLElement* machineTypeXML = xml->InsertNewChildElement("Machine");
	machineTypeXML->SetAttribute("Type", getAxisType(machineUnitType)->saveName);
	machineTypeXML->SetAttribute("Unit", getPositionUnitType(machinePositionUnit)->saveName);

	XMLElement* feedbackXML = xml->InsertNewChildElement("Feedback");
	feedbackXML->SetAttribute("Type", getPositionFeedbackType(positionFeedbackType)->saveName);
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		feedbackXML->SetAttribute("Unit", getPositionUnitType(feedbackPositionUnit)->saveName);
		feedbackXML->SetAttribute("UnitsPerMachineUnit", feedbackUnitsPerMachineUnits);
	}

	XMLElement* commandXML = xml->InsertNewChildElement("Command");
	commandXML->SetAttribute("Type", getCommandType(commandType)->saveName);
	commandXML->SetAttribute("Unit", getPositionUnitType(commandPositionUnit)->saveName);
	commandXML->SetAttribute("UnitsPerMachineUnit", commandUnitsPerMachineUnits);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReference");
	positionReferenceXML->SetAttribute("Type", getPositionReferenceType(positionReferenceType)->saveName);
	switch (positionReferenceType) {
	case PositionReference::Type::LOW_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", allowedPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		break;
	case PositionReference::Type::HIGH_LIMIT:
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", allowedNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		break;
	case PositionReference::Type::LOW_AND_HIGH_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", allowedPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", allowedNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", getHomingDirectionType(homingDirectionType)->saveName);
	case PositionReference::Type::POSITION_REFERENCE:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", allowedPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", allowedNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", getHomingDirectionType(homingDirectionType)->saveName);
	case PositionReference::Type::NO_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", allowedPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", allowedNegativeDeviationFromReference_degrees);
	}

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_degreesPerSecond", velocityLimit_degreesPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_degreesPerSecondSquared", accelerationLimit_degreesPerSecondSquared);

	XMLElement* defaultManualParametersXML = xml->InsertNewChildElement("DefaultManualParameters");
	defaultManualParametersXML->SetAttribute("Velocity_degreesPerSecond", defaultManualVelocity_degreesPerSecond);
	defaultManualParametersXML->SetAttribute("Acceleration_degreesPerSecondSquared", defaultManualAcceleration_degreesPerSecondSquared);

	return false;
}



bool SingleAxisMachine::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* machineXML = xml->FirstChildElement("Machine");
	if (!machineXML) return Logger::warn("Could not load Machine Attributes");
	const char* machineUnitTypeString;


	if (machineXML->QueryStringAttribute("Type", &machineUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Machine Type");
	if (getAxisType(machineUnitTypeString) == nullptr) return Logger::warn("Could not read Machine Type");
	machineUnitType = getAxisType(machineUnitTypeString)->unitType;
	const char* machineUnitString;
	if (machineXML->QueryStringAttribute("Unit", &machineUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	if (getPositionUnitType(machineUnitString) == nullptr) return Logger::warn("Could not read Machine Unit");
	machinePositionUnit = getPositionUnitType(machineUnitString)->unit;


	XMLElement* feedbackXML = xml->FirstChildElement("Feedback");
	if (!feedbackXML) return Logger::warn("Could not load Machine Feedback");
	const char* feedbackTypeString;
	if (feedbackXML->QueryStringAttribute("Type", &feedbackTypeString) != XML_SUCCESS) return Logger::warn("Could not load Feedback Type");
	if (getPositionFeedbackType(feedbackTypeString) == nullptr) return Logger::warn("Could not read Feedback Type");
	positionFeedbackType = getPositionFeedbackType(feedbackTypeString)->type;
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		const char* feedbackUnitString;
		if (feedbackXML->QueryStringAttribute("Unit", &feedbackUnitString) != XML_SUCCESS) return Logger::warn("Could not load Feedback Unit");
		if (getPositionUnitType(feedbackUnitString) == nullptr) return Logger::warn("Could not read Feedback Unit");
		feedbackPositionUnit = getPositionUnitType(feedbackUnitString)->unit;
		if (feedbackXML->QueryDoubleAttribute("UnitsPerMachineUnit", &feedbackUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Feedback Units Per Machine Unit");
	}


	XMLElement* commandXML = xml->FirstChildElement("Command");
	if (!commandXML) return Logger::warn("Could not load Machine Command Attribute");
	const char* commandTypeString;
	if (commandXML->QueryStringAttribute("Type", &commandTypeString) != XML_SUCCESS) return Logger::warn("Could not load Command Type");
	if (getCommandType(commandTypeString) == nullptr) return Logger::warn("Could not read Command Type");
	commandType = getCommandType(commandTypeString)->type;
	const char* commandUnitString;
	if (commandXML->QueryStringAttribute("Unit", &commandUnitString) != XML_SUCCESS) return Logger::warn("Could not load Command Unit");
	if (getPositionUnitType(commandUnitString) == nullptr) return Logger::warn("Could not read Command Unit");
	commandPositionUnit = getPositionUnitType(commandUnitString)->unit;
	if (commandXML->QueryDoubleAttribute("UnitsPerMachineUnit", &commandUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Command Units Per Machine Units");


	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReference");
	if (!positionReferenceXML) return Logger::warn("Could not load Machine Position Reference");
	const char* positionReferenceTypeString;
	if (positionReferenceXML->QueryStringAttribute("Type", &positionReferenceTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	if (getPositionReferenceType(positionReferenceTypeString) == nullptr) return Logger::warn("Could not read Position Reference Type");
	positionReferenceType = getPositionReferenceType(positionReferenceTypeString)->type;
	switch (positionReferenceType) {
	case PositionReference::Type::LOW_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVelocity_degreesPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReference::Type::HIGH_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVelocity_degreesPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReference::Type::LOW_AND_HIGH_LIMIT:
	case PositionReference::Type::POSITION_REFERENCE:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVelocity_degreesPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		const char* homingDirectionString;
		if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirectionString)) return Logger::warn("Could not load homing direction");
		homingDirectionType = getHomingDirectionType(homingDirectionString)->type;
		if (homingDirectionType == HomingDirection::Type::UNKNOWN) return Logger::warn("Could not identify homing direction");
		break;
	case PositionReference::Type::NO_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
	}


	XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
	if (!kinematicLimitsXML) return Logger::warn("Could not load Machine Kinematic Kimits");
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit_degreesPerSecond", &velocityLimit_degreesPerSecond)) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit_degreesPerSecondSquared", &accelerationLimit_degreesPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");

	XMLElement* defaultManualParametersXML = xml->FirstChildElement("DefaultManualParameters");
	if (!defaultManualParametersXML) return Logger::warn("Could not load default movement parameters");
	if (defaultManualParametersXML->QueryDoubleAttribute("Velocity_degreesPerSecond", &defaultManualVelocity_degreesPerSecond) != XML_SUCCESS) Logger::warn("Could not load default movement velocity");
	if (defaultManualParametersXML->QueryDoubleAttribute("Acceleration_degreesPerSecondSquared", &defaultManualAcceleration_degreesPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load default movement acceleration");
	manualControlAcceleration_degreesPerSecond = defaultManualAcceleration_degreesPerSecondSquared;
	targetVelocity = defaultManualVelocity_degreesPerSecond;

	return true;
}
