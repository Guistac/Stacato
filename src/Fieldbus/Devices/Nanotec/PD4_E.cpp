#include <pch.h>

#include "PD4_E.h"
#include "Fieldbus/EtherCatFieldbus.h"

bool PD4_E::isDeviceReady() {
	return isStateOperational();
}
void PD4_E::enable() {
	b_enabledOperation = true;
}
void PD4_E::disable() {
	b_disableOperation = true;
}
bool PD4_E::isEnabled() { 
	return actualPowerState == DS402::PowerState::State::OPERATION_ENABLED || actualPowerState == DS402::PowerState::State::SWITCHED_ON;
}
void PD4_E::onDisconnection() {}
void PD4_E::onConnection() {}
void PD4_E::resetData() {}

void PD4_E::assignIoData() {
	servoMotor->setParentDevice(std::dynamic_pointer_cast<Device>(shared_from_this()));
	servoActuatorDeviceLink->set(servoMotor);
	addIoData(servoActuatorDeviceLink);
	addIoData(positionPin);
	addIoData(velocityPin);

	double maxEncoderRevolutions = 1 << encoderMultiTurnResolutionBits;
	servoMotor->rangeMin_positionUnits = -maxEncoderRevolutions / 2.0;
	servoMotor->rangeMax_positionUnits = maxEncoderRevolutions / 2.0;
	servoMotor->accelerationLimit_positionUnitsPerSecondSquared = 25.0; //25 rps/s 
	servoMotor->velocityLimit_positionUnitsPerSecond = 42.0; //50 rps = 3000 rpm

	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(DS402::controlWordIndex,			0x0, 16, "DS402 Control Word", &ds402control.controlWord);
	rxPdoAssignement.addEntry(DS402::operatingModeControlIndex, 0x0, 8, "Operating Mode Control", &ds402control.operatingModeControl);
	rxPdoAssignement.addEntry(DS402::targetPositionIndex,		0x0, 32, "Target Position", &targetPosition);
	rxPdoAssignement.addEntry(DS402::targetVelocityIndex,		0x0, 32, "Target Velocity", &targetVelocity);

	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(DS402::statusWordIndex,			0x0, 16, "DS402 Status Word", &ds402status.statusWord);
	txPdoAssignement.addEntry(DS402::operatingModeDisplayIndex, 0x0, 8,	"Operating Mode Display", &ds402status.operatingModeDisplay);
	txPdoAssignement.addEntry(DS402::actualPositionIndex,		0x0, 32, "Actual Position", &actualPosition);
	txPdoAssignement.addEntry(DS402::actualVelocityIndex,		0x0, 32, "Actual Velocity", &actualVelocity);
	txPdoAssignement.addEntry(0x2039, 0x2, 32, "Actual Current", &actualCurrent);
	txPdoAssignement.addEntry(0x60F4, 0x0, 32, "Following Error", &actualError);
}

bool PD4_E::startupConfiguration() {

	//Set position unit to 1 encoder increment per unit
	if (!writeSDO_U32(0x60A8, 0x0, 0x00B50000)) return false;
	
	//Set Velocity Unit to 0.01 revolution per second per unit
	if (!writeSDO_U32(0x60A9, 0x0, 0xFEB40300)) return false;

	uint32_t maxMotorSpeed_motorVelocityUnits = servoMotor->getVelocityLimit() * velocityUnitsPerRevolutionPerSecond;
	if (!writeSDO_U32(0x320E, 0x5, maxMotorSpeed_motorVelocityUnits)) return false;

	//set max motor current
	uint32_t maxCurrent_milliamperes = maxCurrent_amperes * 1000.0;
	if (!writeSDO_U32(0x2031, 0x0, maxCurrent_milliamperes)) return false;

	//set the cycle time to be in milliseconds (^-3) and equal to the fieldbus cycle time
	uint8_t processCycle_milliseconds = EtherCatFieldbus::processInterval_milliseconds;
	if (!writeSDO_U8(0x60C2, 0x1, processCycle_milliseconds)) return false;
	if (!writeSDO_S8(0x60C2, 0x2, -3)) return false;

	//Quickstop transition (deceleration ramp and transition to state quickstop)
	if (!writeSDO_S16(0x605A, 0x0, 5)) return false;

	int motorUnitsPerRevolution = 0x1 << encoderSingleTurnResolutionBits;
	uint32_t maxPositionError_motorUnits = maxFollowingError_revolutions * motorUnitsPerRevolution;
	if (!writeSDO_U32(0x6065, 0x0, maxPositionError_motorUnits)) return false;

	uint16_t followingErrorTimeout = followingErrorTimeout_milliseconds;
	if (!writeSDO_U16(0x6066, 0x0, followingErrorTimeout)) return false;

	//set reaction to following error timeout to Quickstop Ramp
	if (!writeSDO_S16(0x3700, 0x0, 2)) return false;

	//set position controller proportional gain to 500 (quick fix)
	if (!writeSDO_U32(0x3210, 0x1, 500)) return false;

	if (!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) Logger::warn("RxPDO assignement failed");
	if (!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) Logger::warn("TxPDO assignement failed");

	//sync signal interval has to be 1ms = 1000000ns
	uint32_t sync0Interval_nanoseconds = 1000000;
	//uint32_t sync0offset_nanoseconds = sync0Interval_nanoseconds / 2;
	uint32_t sync0offset_nanoseconds = 0;
	ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);

	return true;
}


void PD4_E::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);

	actualOperatingMode = ds402status.getOperatingMode();
	actualPowerState = ds402status.getPowerState();

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

	int encoderIncrementsPerRevolution = 0x1 << encoderSingleTurnResolutionBits;
	double position_revolutions = (double)actualPosition / (double)encoderIncrementsPerRevolution;
	followingError_revolutions = (double)actualError / (double)encoderIncrementsPerRevolution;
	double velocity_revolutionsPerSecond = (double)actualVelocity / velocityUnitsPerRevolutionPerSecond;
	positionPin->set(position_revolutions);
	velocityPin->set(velocity_revolutionsPerSecond);

	double actualCurrent_amperes = std::abs((double)actualCurrent / 1000.0);




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

	switch (actualPowerState) {
	case DS402::PowerState::State::OPERATION_ENABLED:
		servoMotor->b_enabled = true;
		break;
	default:
		servoMotor->b_enabled = false;
		break;
	}
	servoMotor->positionRaw_positionUnits = position_revolutions;
	servoMotor->velocity_positionUnitsPerSecond = velocity_revolutionsPerSecond;
	servoMotor->load = actualCurrent_amperes / maxCurrent_amperes;
	servoMotor->commandRequestTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	servoMotor->b_online = true;
	servoMotor->b_detected = true;
	servoMotor->b_emergencyStopActive = false;
	servoMotor->b_enabled = actualPowerState == DS402::PowerState::State::OPERATION_ENABLED;
	servoMotor->b_ready = actualOperatingMode == DS402::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION
		&& (actualPowerState == DS402::PowerState::State::OPERATION_ENABLED
		|| actualPowerState == DS402::PowerState::State::READY_TO_SWITCH_ON
		|| actualPowerState == DS402::PowerState::State::SWITCHED_ON
		|| actualPowerState == DS402::PowerState::State::QUICKSTOP_ACTIVE);
	servoMotor->b_parked = false;
	servoMotor->b_moving = servoMotor->getVelocity() > 0.001;
	
}

void PD4_E::prepareOutputs() {

	if (servoMotor->b_setEnabled) {
		servoMotor->b_setEnabled = false;
		requestedPowerState = DS402::PowerState::State::OPERATION_ENABLED;
	}

	if (servoMotor->b_setDisabled) {
		servoMotor->b_setDisabled = false;
		requestedPowerState = DS402::PowerState::State::READY_TO_SWITCH_ON;
	}

	if (b_enabledOperation) {
		b_enabledOperation = false;
		requestedPowerState = DS402::PowerState::State::OPERATION_ENABLED;
	}

	if (b_disableOperation) {
		b_disableOperation = false;
		requestedPowerState = DS402::PowerState::State::READY_TO_SWITCH_ON;
	}

	if (ds402status.hasFault()) ds402control.performFaultReset();
	ds402control.setPowerState(requestedPowerState, actualPowerState);
	if (performFaultReset) {
		performFaultReset = false;
		ds402control.performFaultReset();
	}
	if (b_autoSetup) ds402control.operatingModeControl = -2;
	else ds402control.setOperatingMode(requestedOperatingMode);
	ds402control.setOperatingModeSpecificByte4(b_startSetup);


	ds402control.updateControlWord();

	int encoderIncrementsPerRevolution = 0x1 << encoderSingleTurnResolutionBits;
	targetPosition = servoMotor->getCommand() * encoderIncrementsPerRevolution;

	rxPdoAssignement.pushDataTo(identity->outputs);
}

bool PD4_E::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool PD4_E::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }
