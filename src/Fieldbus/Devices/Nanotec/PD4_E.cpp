#include <pch.h>

#include "PD4_E.h"
#include "Fieldbus/EtherCatFieldbus.h"

bool PD4_E::isDeviceReady() {
	return servoMotor->isReady();
}
void PD4_E::enable() {
	servoMotor->enable();
}
void PD4_E::disable() {
	servoMotor->disable();
}
bool PD4_E::isEnabled() {
	return servoMotor->isEnabled();
}

void PD4_E::onDisconnection() {}

void PD4_E::onConnection() {}

void PD4_E::resetData() {
	servoMotor->b_detected = false;
	servoMotor->b_enabled = false;
	servoMotor->b_emergencyStopActive = false;
	servoMotor->b_moving = false;
	servoMotor->b_online = false;
	servoMotor->b_ready = false;
	requestedPowerState = DS402::PowerState::State::READY_TO_SWITCH_ON;
	gpioDevice->b_detected = false;
	gpioDevice->b_online = false;
	gpioDevice->b_ready = false;
}

void PD4_E::assignIoData() {
	servoMotor->setParentDevice(std::dynamic_pointer_cast<Device>(shared_from_this()));
	servoActuatorDeviceLink->set(servoMotor);

	gpioDevice->setParentDevice(std::dynamic_pointer_cast<Device>(shared_from_this()));
	gpioDeviceLink->set(gpioDevice);

	addIoData(servoActuatorDeviceLink);
	addIoData(positionPin);
	addIoData(velocityPin);

	addIoData(gpioDeviceLink);
	addIoData(digitalIn1Pin);
	addIoData(digitalIn2Pin);
	addIoData(digitalIn3Pin);
	addIoData(digitalIn4Pin);
	addIoData(digitalIn5Pin);
	addIoData(digitalIn6Pin);
	addIoData(analogIn1Pin);

	addIoData(digitalOut1Pin);
	addIoData(digitalOut2Pin);

	servoMotor->positionUnit = PositionUnit::Unit::REVOLUTION;
	double maxEncoderRevolutions = 1 << encoderMultiTurnResolutionBits;
	servoMotor->rangeMin_positionUnits = -maxEncoderRevolutions / 2.0;
	servoMotor->rangeMax_positionUnits = maxEncoderRevolutions / 2.0;
	servoMotor->accelerationLimit_positionUnitsPerSecondSquared = 25.0; //25 rps/s 
	servoMotor->velocityLimit_positionUnitsPerSecond = maxVelocity_revolutionsPerSecond; //50 rps = 3000 rpm

	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(DS402::controlWordIndex,			0x0, 16, "DS402 Control Word", &ds402control.controlWord);
	rxPdoAssignement.addEntry(DS402::operatingModeControlIndex, 0x0, 8, "Operating Mode Control", &ds402control.operatingModeControl);
	rxPdoAssignement.addEntry(DS402::targetPositionIndex,		0x0, 32, "Target Position", &targetPosition);
	rxPdoAssignement.addEntry(DS402::targetVelocityIndex,		0x0, 32, "Target Velocity", &targetVelocity);
	rxPdoAssignement.addEntry(0x60FE, 0x1, 32, "Digital Outputs", &digitalOutputs);

	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(DS402::statusWordIndex,			0x0, 16, "DS402 Status Word", &ds402status.statusWord);
	txPdoAssignement.addEntry(DS402::operatingModeDisplayIndex, 0x0, 8,	"Operating Mode Display", &ds402status.operatingModeDisplay);
	txPdoAssignement.addEntry(DS402::actualPositionIndex,		0x0, 32, "Actual Position", &actualPosition);
	txPdoAssignement.addEntry(DS402::actualVelocityIndex,		0x0, 32, "Actual Velocity", &actualVelocity);
	txPdoAssignement.addEntry(0x2039, 0x2, 32, "Actual Current", &actualCurrent);
	txPdoAssignement.addEntry(0x60F4, 0x0, 32, "Following Error", &actualError);
	txPdoAssignement.addEntry(0x60FD, 0x0, 32, "Digital Inputs", &digitalInputs);
}

bool PD4_E::startupConfiguration() {

	//Set position unit to 1 encoder increment per unit
	if (!writeSDO_U32(0x60A8, 0x0, 0x00B50000)) return false;
	
	//Set Velocity Unit to 0.01 revolution per second per unit
	if (!writeSDO_U32(0x60A9, 0x0, 0xFEB40300)) return false;

	uint32_t maxMotorSpeed_motorVelocityUnits = maxVelocity_revolutionsPerSecond * velocityUnitsPerRevolutionPerSecond;
	if (!writeSDO_U32(0x320E, 0x5, maxMotorSpeed_motorVelocityUnits)) return false;

	//set max motor current
	uint32_t maxCurrent_milliamperes = maxCurrent_amperes * 1000.0;
	if (!writeSDO_U32(0x2031, 0x0, maxCurrent_milliamperes)) return false;

	//set the cycle time to be in milliseconds (^-3) and equal to the fieldbus cycle time
	uint8_t processCycle_milliseconds = EtherCatFieldbus::processInterval_milliseconds;
	if (!writeSDO_U8(0x60C2, 0x1, processCycle_milliseconds)) return false;
	if (!writeSDO_S8(0x60C2, 0x2, -3)) return false;

	//Quickstop Behavior (deceleration ramp and transition to power state quickstop)
	if (!writeSDO_S16(0x605A, 0x0, 5)) return false;

	int motorUnitsPerRevolution = 0x1 << encoderSingleTurnResolutionBits;
	uint32_t maxPositionError_motorUnits = maxFollowingError_revolutions * motorUnitsPerRevolution;
	if (!writeSDO_U32(0x6065, 0x0, maxPositionError_motorUnits)) return false;

	uint16_t followingErrorTimeout = followingErrorTimeout_milliseconds;
	if (!writeSDO_U16(0x6066, 0x0, followingErrorTimeout)) return false;

	//set reaction to following error timeout to Quickstop Ramp
	if (!writeSDO_S16(0x3700, 0x0, 2)) return false;

	//set position controller proportional gain to 500 (quick fix)
	uint32_t proportionalGain = positionControllerProportionalGain;
	if (!writeSDO_U32(0x3210, 0x1, proportionalGain)) return false;

	//set the negative and positive limit switch functions
	uint32_t digitalInputSpecialFunctionEnable = 0;
	if (negativeLimitSwitchOnDigitalIn1) digitalInputSpecialFunctionEnable |= 0x1;
	if (positiveLimitSwitchOnDigitalIn2) digitalInputSpecialFunctionEnable |= 0x2;
	if (!writeSDO_U32(0x3240, 0x1, digitalInputSpecialFunctionEnable)) return false;

	//set the inversion of digital signals
	uint32_t digitalInputFunctionInverted = 0;
	if (invertDigitalInput1) digitalInputFunctionInverted |= 0x1;
	if (invertDigitalInput2) digitalInputFunctionInverted |= 0x2;
	if (invertDigitalInput3) digitalInputFunctionInverted |= 0x4;
	if (invertDigitalInput4) digitalInputFunctionInverted |= 0x8;
	if (invertDigitalInput5) digitalInputFunctionInverted |= 0x10;
	if (invertDigitalInput6) digitalInputFunctionInverted |= 0x20;
	if (!writeSDO_U32(0x3240, 0x2, digitalInputFunctionInverted)) return false;

	//set limit switch reaction to quickstop ramp & change to power state quickstop
	int16_t limitSwitchReaction = 6;
	if (!writeSDO_S16(0x3701, 0x0, limitSwitchReaction)) return false;

	if (!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) Logger::warn("RxPDO assignement failed");
	if (!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) Logger::warn("TxPDO assignement failed");

	//sync signal interval has to be 1ms = 1000000ns
	uint32_t sync0Interval_nanoseconds = 1000000;
	//uint32_t sync0Offset_nanoseconds = sync0Interval_nanoseconds / 2;
	uint32_t sync0offset_nanoseconds = 0;
	ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);

	return true;
}


void PD4_E::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);

	DS402::PowerState::State previousPowerState = actualPowerState;

	actualOperatingMode = ds402status.getOperatingMode();
	actualPowerState = ds402status.getPowerState();

	if (actualPowerState != previousPowerState) {
		Logger::warn("Power state changed to {}", DS402::getPowerState(actualPowerState)->displayName);
	}

	//mandatory DS402 bits
	warning = ds402status.hasWarning();
	fault = ds402status.hasFault();
	isRemoteControlled = ds402status.isRemoteControlled();
	targetReached = ds402status.getOperationModeSpecificByte10();
	internalLimit = ds402status.isInternalLimitSet();

	//nanotec drive specific bits
	controllerSyncedWithFieldbus = ds402status.getManufacturerSpecificByte8();
	controllerFollowsTarget = ds402status.getOperationModeSpecificByte12();
	followingError = ds402status.getOperationModeSpecificByte13();
	closedLoopActive = ds402status.getManufacturerSpecificByte15();

	if (servoMotor->isEnabled()) {
		if (followingError) servoMotor->disable();
		if (actualPowerState != DS402::PowerState::State::OPERATION_ENABLED) servoMotor->disable();
	}

	int encoderIncrementsPerRevolution = 0x1 << encoderSingleTurnResolutionBits;
	actualPosition_revolutions = (double)actualPosition / (double)encoderIncrementsPerRevolution;
	actualFollowingError_revolutions = (double)actualError / (double)encoderIncrementsPerRevolution;
	actualVelocity_revolutionsPerSecond = (double)actualVelocity / velocityUnitsPerRevolutionPerSecond;
	actualCurrent_amperes = std::abs((double)actualCurrent / 1000.0);
	positionPin->set(actualPosition_revolutions);
	velocityPin->set(actualFollowingError_revolutions);

	digitalIn1 = (digitalInputs >> 16) & 0x1;
	digitalIn2 = (digitalInputs >> 17) & 0x1;
	digitalIn3 = (digitalInputs >> 18) & 0x1;
	digitalIn4 = (digitalInputs >> 19) & 0x1;
	digitalIn5 = (digitalInputs >> 20) & 0x1;
	digitalIn6 = (digitalInputs >> 21) & 0x1;

	digitalIn1Pin->set(digitalIn1);
	digitalIn2Pin->set(digitalIn2);
	digitalIn3Pin->set(digitalIn3);
	digitalIn4Pin->set(digitalIn4);
	digitalIn5Pin->set(digitalIn5);
	digitalIn6Pin->set(digitalIn6);

	if (negativeLimitSwitchOnDigitalIn1 && digitalIn1 && servoMotor->isEnabled() && actualVelocity_revolutionsPerSecond < 0.0) {
		Logger::warn("Hit Negative Limit Switch");
		servoMotor->b_enabled = false;
	}
	if (positiveLimitSwitchOnDigitalIn2 && digitalIn2 && servoMotor->isEnabled() && actualVelocity_revolutionsPerSecond > 0.0) {
		Logger::warn("Hit Positive Limit Switch");
		servoMotor->b_enabled = false;
	}

	servoMotor->positionRaw_positionUnits = actualPosition_revolutions;
	servoMotor->velocity_positionUnitsPerSecond = actualVelocity_revolutionsPerSecond;
	servoMotor->load = actualCurrent_amperes / maxCurrent_amperes;
	servoMotor->commandRequestTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	servoMotor->b_online = true;
	servoMotor->b_detected = true;
	servoMotor->b_emergencyStopActive = false;
	servoMotor->b_parked = false;
	servoMotor->b_moving = servoMotor->getVelocity() > 0.001;
	switch (actualPowerState) {
	case DS402::PowerState::State::OPERATION_ENABLED:
	case DS402::PowerState::State::QUICKSTOP_ACTIVE:
	case DS402::PowerState::State::READY_TO_SWITCH_ON:
	case DS402::PowerState::State::SWITCHED_ON:
		servoMotor->b_ready = true;
		break;
	default:
		servoMotor->b_ready = false;
		break;
	}
	if (actualControlMode != ControlMode::Mode::EXTERNAL_CONTROL) servoMotor->b_ready = false;
	switch (actualPowerState) {
	case DS402::PowerState::State::OPERATION_ENABLED:
		servoMotor->b_enabled = true;
		break;
	default:
		servoMotor->b_enabled = false;
		break;
	}
	gpioDevice->b_detected = true;
	gpioDevice->b_online = true;
	gpioDevice->b_ready = true;

}

void PD4_E::prepareOutputs() {

	double time_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	double profileTimeDelta_seconds = time_seconds - profileTime_seconds;
	profileTime_seconds = time_seconds;
	switch (actualControlMode) {
	case ControlMode::Mode::MANUAL_VELOCITY_CONTROL: {
			double deltaV = manualAcceleration_revolutionPerSecondSquared * profileTimeDelta_seconds;
			if (profileVelocity_revolutions < manualVelocityTarget_revolutionsPerSecond) {
				profileVelocity_revolutions += deltaV;
				if (profileVelocity_revolutions > manualVelocityTarget_revolutionsPerSecond) profileVelocity_revolutions = manualVelocityTarget_revolutionsPerSecond;
			}
			else if (profileVelocity_revolutions > manualVelocityTarget_revolutionsPerSecond) {
				profileVelocity_revolutions -= deltaV;
				if (profileVelocity_revolutions < manualVelocityTarget_revolutionsPerSecond) profileVelocity_revolutions = manualVelocityTarget_revolutionsPerSecond;
			}
			double deltaP = profileVelocity_revolutions * profileTimeDelta_seconds;
			profilePosition_revolutions += deltaP;
		}break;
		case ControlMode::Mode::EXTERNAL_CONTROL: {
			double previousProfilePosition_revolutions = profilePosition_revolutions;
			if (servoActuatorDeviceLink->isConnected()) profilePosition_revolutions = servoMotor->getCommand();
			profileVelocity_revolutions = (profilePosition_revolutions - previousProfilePosition_revolutions) / profileTimeDelta_seconds;
			}break;
	}
	int encoderIncrementsPerRevolution = 0x1 << encoderSingleTurnResolutionBits;
	targetPosition = profilePosition_revolutions * encoderIncrementsPerRevolution;

	if (digitalOut1Pin->isConnected()) digitalOut1Pin->set(digitalOut1Pin->getConnectedPins().front()->getBoolean());
	if (digitalOut2Pin->isConnected()) digitalOut2Pin->set(digitalOut2Pin->getConnectedPins().front()->getBoolean());
	digitalOut1 = digitalOut1Pin->getBoolean();
	digitalOut2 = digitalOut2Pin->getBoolean();
	digitalOutputs = 0;
	if (digitalOut1) digitalOutputs |= 0x1 << 16;
	if (digitalOut2) digitalOutputs |= 0x1 << 17;

	if (servoMotor->b_setEnabled) {
		servoMotor->b_setEnabled = false;
		profilePosition_revolutions = actualPosition_revolutions;
		profileVelocity_revolutions = actualVelocity_revolutionsPerSecond;
		requestedPowerState = DS402::PowerState::State::OPERATION_ENABLED;
	}

	if (servoMotor->b_setDisabled) {
		servoMotor->b_setDisabled = false;
		requestedPowerState = DS402::PowerState::State::READY_TO_SWITCH_ON;
	}

	if (servoMotor->b_setQuickstop) {
		servoMotor->b_setQuickstop = false;
		requestedPowerState = DS402::PowerState::State::QUICKSTOP_ACTIVE;
	}

	if (ds402status.hasFault()) ds402control.performFaultReset();
	ds402control.setPowerState(requestedPowerState, actualPowerState);
	ds402control.setOperatingMode(requestedOperatingMode);
	ds402control.updateControlWord();

	rxPdoAssignement.pushDataTo(identity->outputs);
}

bool PD4_E::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool PD4_E::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

std::vector<PD4_E::ControlMode> controlModes = {
	{PD4_E::ControlMode::Mode::MANUAL_VELOCITY_CONTROL, "Manual Velocity Control"},
	{PD4_E::ControlMode::Mode::EXTERNAL_CONTROL, "External Control"}
};
std::vector<PD4_E::ControlMode>& PD4_E::getControlModes() {
	return controlModes;
}
PD4_E::ControlMode* PD4_E::getControlMode(PD4_E::ControlMode::Mode m) {
	for (auto& controlMode : controlModes) {
		if (m == controlMode.mode) return &controlMode;
	}
	return nullptr;
}