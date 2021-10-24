#include <pch.h>

#include "PD4_E.h"


bool PD4_E::isDeviceReady() {
	return isStateOperational();
}
void PD4_E::enable() {}
void PD4_E::disable() {}
bool PD4_E::isEnabled() { return false; }
void PD4_E::onDisconnection() {}
void PD4_E::onConnection() {}
void PD4_E::resetData() {}

void PD4_E::assignIoData() {
	servoActuatorDevice->setParentDevice(std::dynamic_pointer_cast<Device>(shared_from_this()));

	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(0x6040, 0x0, 16, "DS402 Control Word", &DS402controlWord);
	rxPdoAssignement.addEntry(0x6060, 0x0, 8, "Operating Mode Control", &operatingModeControl);
	rxPdoAssignement.addEntry(0x60FF, 0x0, 32, "Target Velocity", &targetVelocity);

	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6041, 0x0, 16, "DS402 Status Word", &DS402statusWord);
	txPdoAssignement.addEntry(0x6061, 0x0, 8, "Operating Mode Display", &operatingModeDisplay);
	txPdoAssignement.addEntry(0x6064, 0x0, 32, "Actual Position", &actualPosition);
	txPdoAssignement.addEntry(0x606C, 0x0, 32, "Actual Velocity", &actualVelocity);
}

bool PD4_E::startupConfiguration() {


	//Quickstop transition (deceleration ramp and transition to state quickstop)
	if (!writeSDO_S16(0x605A, 0x0, 5)) return false;

	
	if (!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) Logger::warn("RxPDO assignement failed");
	if (!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) Logger::warn("TxPDO assignement failed");
	return true;
}


void PD4_E::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);

	currentOperatingMode = getOperatingMode(operatingModeDisplay)->mode;

	//======= DS402 Status Word ========
	bool readyToSwitchOn = (DS402statusWord >> 0) & 0x1;
	bool switchedOn = (DS402statusWord >> 1) & 0x1;
	bool operationEnabled = (DS402statusWord >> 2) & 0x1;
	bool fault = (DS402statusWord >> 3) & 0x1;
	bool voltageEnabled = (DS402statusWord >> 4) & 0x1;
	bool quickstop = (DS402statusWord >> 5) & 0x1;
	bool switchOnDisabled = (DS402statusWord >> 6) & 0x1;
	if (readyToSwitchOn) {
		if (fault) powerState = DS402PowerState::State::FAULT_REACTION_ACTIVE;
		else if (!quickstop) powerState = DS402PowerState::State::QUICKSTOP_ACTIVE;
		else if (operationEnabled) powerState = DS402PowerState::State::OPERATION_ENABLED;
		else if (switchedOn) powerState = DS402PowerState::State::SWITCHED_ON;
		else powerState = DS402PowerState::State::READY_TO_SWITCH_ON;
	}
	else {
		if (fault) powerState = DS402PowerState::State::FAULT;
		else if (switchOnDisabled) powerState = DS402PowerState::State::SWITCH_ON_DISABLED;
		else powerState = DS402PowerState::State::NOT_READY_TO_SWITCH_ON;
	}
	bool warning = (DS402statusWord >> 7) & 0x1;
	bool sync = (DS402statusWord >> 8) & 0x1;
	bool remote = (DS402statusWord >> 9) & 0x1;
	bool targetReached = (DS402statusWord >> 10) & 0x1;
	bool internalLimitActive = (DS402statusWord >> 11) & 0x1;
	bool operatingModeSpecific1 = (DS402statusWord >> 12) & 0x1;
	bool operatingModeSpecific2 = (DS402statusWord >> 13) & 0x1;
	bool closedLoopActive = (DS402statusWord >> 15) & 0x1;


	switch (powerState) {
		case DS402PowerState::State::OPERATION_ENABLED:
		case DS402PowerState::State::QUICKSTOP_ACTIVE:
		case DS402PowerState::State::READY_TO_SWITCH_ON:
		case DS402PowerState::State::SWITCHED_ON:
			servoActuatorDevice->b_ready = true;
			break;
		default:
			servoActuatorDevice->b_ready = false;
			break;
	}

	switch (powerState) {
		case DS402PowerState::State::OPERATION_ENABLED:
			servoActuatorDevice->b_enabled = true;
			break;
		default:
			servoActuatorDevice->b_enabled = false;
			break;
	}

}

void PD4_E::prepareOutputs() {


	switch (requestedPowerState) {
		case DS402PowerState::State::READY_TO_SWITCH_ON:
			switch (powerState) {
				case DS402PowerState::State::OPERATION_ENABLED:
				case DS402PowerState::State::SWITCHED_ON:
				case DS402PowerState::State::SWITCH_ON_DISABLED:
					DS402controlWord = 0b0110;
					break;
				case DS402PowerState::State::QUICKSTOP_ACTIVE:
					DS402controlWord = 0b0000;
					break;
			}
			break;
		case DS402PowerState::State::OPERATION_ENABLED:
			switch (powerState) {
				case DS402PowerState::State::SWITCHED_ON:
				case DS402PowerState::State::QUICKSTOP_ACTIVE:
					DS402controlWord = 0b1111;
					break;
				case DS402PowerState::State::READY_TO_SWITCH_ON:
					DS402controlWord = 0b0111;
					break;
				case DS402PowerState::State::SWITCH_ON_DISABLED:
					DS402controlWord = 0b0110;
					break;
			}
			break;
		case DS402PowerState::State::QUICKSTOP_ACTIVE:
			switch (powerState) {
				case DS402PowerState::State::OPERATION_ENABLED:
				case DS402PowerState::State::QUICKSTOP_ACTIVE:
					DS402controlWord = 0b1011;
					break;
				default:
					requestedPowerState: DS402PowerState::State::READY_TO_SWITCH_ON;
					break;
			}
			break;
	}

	if (performFaultReset) {
		performFaultReset = false;
		b_faultReset = true;
	}
	if (b_faultReset) {
		DS402controlWord |= 0b10000000;
		b_faultReset = false;
	}

	operatingModeControl = getOperatingMode(requestedOperatingMode)->value;

	rxPdoAssignement.pushDataTo(identity->outputs);
}

bool PD4_E::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool PD4_E::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }




//====================== CiA DS402 Power State Machine ============================

std::vector<PD4_E::DS402PowerState> DS402PowerStates = {
	{PD4_E::DS402PowerState::State::NOT_READY_TO_SWITCH_ON,	"Not Ready To Switch On"},
	{PD4_E::DS402PowerState::State::SWITCH_ON_DISABLED,		"Switch On Disabled"},
	{PD4_E::DS402PowerState::State::READY_TO_SWITCH_ON,		"Ready To Switch On"},
	{PD4_E::DS402PowerState::State::SWITCHED_ON,			"Switched On"},
	{PD4_E::DS402PowerState::State::OPERATION_ENABLED,		"Operation Enabled"},
	{PD4_E::DS402PowerState::State::QUICKSTOP_ACTIVE,		"Quickstop Active"},
	{PD4_E::DS402PowerState::State::FAULT_REACTION_ACTIVE,	"Fault Reaction Active"},
	{PD4_E::DS402PowerState::State::FAULT,					"Fault"},
	{PD4_E::DS402PowerState::State::UNKNOWN,				"Unknown Power State"}
};
std::vector<PD4_E::DS402PowerState>& PD4_E::getDS402PowerStates() {
	return DS402PowerStates;
}
PD4_E::DS402PowerState* PD4_E::getDS402PowerState(PD4_E::DS402PowerState::State s) {
	for (auto& state : DS402PowerStates) {
		if (s == state.state) return &state;
	}
	return nullptr;
}


//========================= Operation Modes ==============================

struct OperatingMode {
	enum class Mode {
		AUTO_SETUP,
		NO_MODE_CHANGE_OR_NONE_ASSIGNED,
		PROFILE_POSITION,
		VELOCITY,
		PROFILE_VELOCITY,
		PROFILE_TORQUE,
		HOMING,
		INTERPOLATED_POSITION,
		CYCLIC_SYNCHRONOUS_POSITION,
		CYCLIC_SYNCHRONOUS_VELOCITY,
		CYCLIC_SYNCHRONOUS_TORQUE
	};
	int8_t value;
	Mode mode;
	const char displayName[64];
};

std::vector<PD4_E::OperatingMode> operatingModes = {
		{PD4_E::OperatingMode::Mode::AUTO_SETUP, -2, "Auto Setup"},
		{PD4_E::OperatingMode::Mode::NO_MODE_CHANGE_OR_NONE_ASSIGNED, 0, "None"},
		{PD4_E::OperatingMode::Mode::PROFILE_POSITION, 1, "Profile Position"},
		{PD4_E::OperatingMode::Mode::VELOCITY, 2, "Velocity"},
		{PD4_E::OperatingMode::Mode::PROFILE_VELOCITY, 3, "Profile Velocity"},
		{PD4_E::OperatingMode::Mode::PROFILE_TORQUE, 4, "Profile Torque"},
		{PD4_E::OperatingMode::Mode::HOMING, 6, "Homing"},
		{PD4_E::OperatingMode::Mode::INTERPOLATED_POSITION, 7, "Interpolated Position"},
		{PD4_E::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION, 8, "Cyclic Synchronous Position"},
		{PD4_E::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY, 9, "Cyclic Synchronous Velocity"},
		{PD4_E::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_TORQUE, 10, "Cyclic Synchronous Torque"}
};

std::vector<PD4_E::OperatingMode>& PD4_E::getOperatingModes() {
	return operatingModes;
}
PD4_E::OperatingMode* PD4_E::getOperatingMode(int8_t value) {
	for (auto& mode : operatingModes) {
		if (value == mode.value) return &mode;
	}
	return nullptr;
}
PD4_E::OperatingMode* PD4_E::getOperatingMode(OperatingMode::Mode m) {
	for (auto& mode : operatingModes) {
		if (m == mode.mode) return &mode;
	}
	return nullptr;
}