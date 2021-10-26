#include <pch.h>
#include "DS402.h"

namespace DS402 {

	std::vector<PowerState> powerStates = {
		{PowerState::State::NOT_READY_TO_SWITCH_ON,	"Not Ready To Switch On"},
		{PowerState::State::SWITCH_ON_DISABLED,		"Switch On Disabled"},
		{PowerState::State::READY_TO_SWITCH_ON,		"Ready To Switch On"},
		{PowerState::State::SWITCHED_ON,			"Switched On"},
		{PowerState::State::OPERATION_ENABLED,		"Operation Enabled"},
		{PowerState::State::QUICKSTOP_ACTIVE,		"Quickstop Active"},
		{PowerState::State::FAULT_REACTION_ACTIVE,	"Fault Reaction Active"},
		{PowerState::State::FAULT,					"Fault"},
		{PowerState::State::UNKNOWN,				"Unknown Power State"}
	};

	std::vector<PowerState>& getPowerStates() {
		return powerStates;
	}

	PowerState* getPowerState(PowerState::State s) {
		for (auto& state : powerStates) {
			if (s == state.state) return &state;
		}
		return &powerStates.back();
	}
	
	std::vector<OperatingMode> operatingModes = {
		{OperatingMode::Mode::PROFILE_POSITION,									1, "Profile Position"},
		{OperatingMode::Mode::VELOCITY,											2, "Velocity"},
		{OperatingMode::Mode::PROFILE_VELOCITY,									3, "Profile Velocity"},
		{OperatingMode::Mode::PROFILE_TORQUE,									4, "Profile Torque"},
		{OperatingMode::Mode::HOMING,											6, "Homing"},
		{OperatingMode::Mode::INTERPOLATED_POSITION,							7, "Interpolated Position"},
		{OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION,						8, "Cyclic Synchronous Position"},
		{OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY,						9, "Cyclic Synchronous Velocity"},
		{OperatingMode::Mode::CYCLIC_SYNCHRONOUS_TORQUE,						10, "Cyclic Synchronous Torque"},
		{OperatingMode::Mode::CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE, 11, "Cyclic Synchronous Torque with Communcation Angle"},
		{OperatingMode::Mode::UNKNOWN,											127, "Unknown"}
	};

	std::vector<OperatingMode>& getOperatingModes() {
		return operatingModes;
	}
	OperatingMode* getOperatingMode(OperatingMode::Mode m) {
		for (auto& mode : operatingModes) {
			if (mode.mode == m) return &mode;
		}
		return &operatingModes.back();
	}
	OperatingMode* getOperatingMode(int8_t i) {
		for (auto& mode : operatingModes) {
			if (mode.value == i) return &mode;
		}
		return &operatingModes.back();
	}



	PowerState::State Status::getPowerState() {
		bool readyToSwitchOn =	(statusWord >> 0) & 0x1;
		bool switchedOn =		(statusWord >> 1) & 0x1;
		bool operationEnabled = (statusWord >> 2) & 0x1;
		bool fault =			(statusWord >> 3) & 0x1;
		bool voltageEnabled =	(statusWord >> 4) & 0x1;
		bool quickstop =		(statusWord >> 5) & 0x1;
		bool switchOnDisabled = (statusWord >> 6) & 0x1;
		if (readyToSwitchOn) {
			if (fault) return PowerState::State::FAULT_REACTION_ACTIVE;
			else if (!quickstop) return PowerState::State::QUICKSTOP_ACTIVE;
			else if (operationEnabled) return PowerState::State::OPERATION_ENABLED;
			else if (switchedOn) return PowerState::State::SWITCHED_ON;
			else return PowerState::State::READY_TO_SWITCH_ON;
		}
		else {
			if (fault) return PowerState::State::FAULT;
			else if (switchOnDisabled) return PowerState::State::SWITCH_ON_DISABLED;
			else return PowerState::State::NOT_READY_TO_SWITCH_ON;
		}
	}
	OperatingMode::Mode Status::getOperatingMode() {
		DS402::getOperatingMode(operatingModeDisplay)->mode;
	}
	bool Status::hasFault() { return (statusWord >> 3) & 0x1; }
	bool Status::hasWarning() { return (statusWord >> 7) & 0x1; }
	bool Status::isRemoteControlled() { return (statusWord >> 9) & 0x1; }
	bool Status::isInternalLimitSet() { return (statusWord >> 11) & 0x1; }
	bool Status::getManufacturerSpecificByte8() { return (statusWord >> 8) & 0x1; }
	bool Status::getManufacturerSpecificByte14() { return (statusWord >> 14) & 0x1; }
	bool Status::getManufacturerSpecificByte15() { return (statusWord >> 15) & 0x1; }
	bool Status::getOperationModeSpecificByte10() { return (statusWord >> 10) & 0x1; }
	bool Status::getOperationModeSpecificByte12() { return (statusWord >> 12) & 0x1; }
	bool Status::getOperationModeSpecificByte13() { return (statusWord >> 13) & 0x1; }




	void Control::setOperatingMode(OperatingMode::Mode m) {
		operatingModeControl = getOperatingMode(m)->value;
	}
	void Control::setPowerState(PowerState::State requestedState, PowerState::State currentState) {
		switch (requestedState) {
			case PowerState::State::READY_TO_SWITCH_ON: {
				switch (currentState) {
					case PowerState::State::OPERATION_ENABLED:
					case PowerState::State::SWITCHED_ON:
					case PowerState::State::SWITCH_ON_DISABLED:
						powerStateControlBits = 0b0110;
						break;
					case PowerState::State::QUICKSTOP_ACTIVE:
						powerStateControlBits = 0b0000;
						break;
					}
				}break;
			case PowerState::State::OPERATION_ENABLED: {
				switch (currentState) {
					case PowerState::State::SWITCHED_ON:
					case PowerState::State::QUICKSTOP_ACTIVE:
						powerStateControlBits = 0b1111;
						break;
					case PowerState::State::READY_TO_SWITCH_ON:
						powerStateControlBits = 0b0111;
						break;
					case PowerState::State::SWITCH_ON_DISABLED:
						powerStateControlBits = 0b0110;
						break;
					}
				}break;
			case PowerState::State::QUICKSTOP_ACTIVE: {
				switch (currentState) {
					case PowerState::State::OPERATION_ENABLED:
					case PowerState::State::QUICKSTOP_ACTIVE:
						powerStateControlBits = 0b1011;
						break;
					default:
						powerStateControlBits = 0b0000;
						break;
				}
				}break;
			}
	}
	void Control::performFaultReset() { faultResetBit = true; }
	void Control::performHalt() { haltBit = true; }
	void Control::clearHalt() { haltBit = false; }
	bool Control::isHalted() { return haltBit; }
	void Control::setOperatingModeSpecificByte4(bool state) { b4 = state; }
	void Control::setOperatingModeSpecificByte5(bool state) { b5 = state; }
	void Control::setOperatingModeSpecificByte6(bool state) { b6 = state; }
	void Control::setOperatingModeSpecificByte9(bool state) { b9 = state; }
	void Control::setManufacturerSpecificByte11(bool state) { b11 = state; }
	void Control::setManufacturerSpecificByte12(bool state) { b12 = state; }
	void Control::setManufacturerSpecificByte13(bool state) { b13 = state; }
	void Control::setManufacturerSpecificByte14(bool state) { b14 = state; }
	void Control::setManufacturerSpecificByte15(bool state) { b15 = state; }
	void Control::updateControlWord() {
		bool wasFaultResetHigh = (controlWord >> 7) & 0x1;
		controlWord = 0x0;
		controlWord &= powerStateControlBits;
		controlWord &= b4 << 4;
		controlWord &= b5 << 5;
		controlWord &= b6 << 6;
		if(faultResetBit && !wasFaultResetHigh) {
			controlWord &= 0x1 << 7;
			faultResetBit = false;
		}
		controlWord &= haltBit << 8;
		controlWord &= b9 << 9;
		controlWord &= b11 << 11;
		controlWord &= b12 << 12;
		controlWord &= b13 << 13;
		controlWord &= b14 << 14;
		controlWord &= b15 << 15;
	}





	uint16_t statusWordIndex = 0x6041;
	uint16_t controlWordIndex = 0x6040;
	uint16_t operatingModeControlIndex = 0x6060;
	uint16_t operatingModeDisplayIndex = 0x6061;

	uint16_t targetPositionIndex = 0x607A;
	uint16_t actualPositionIndex = 0x6064;
	uint16_t actualFollowingErrorIndex = 0x60F4;
	uint16_t followingErrorWindowIndex = 0x6065;
	uint16_t followingErrorTimeoutIndex = 0x6066;

	uint16_t targetVelocityIndex = 0x60FF;
	uint16_t actualVelocityIndex = 0x606C;

	uint16_t targetTorqueIndex = 0x6071;
	uint16_t actualTorqueIndex = 0x6077;

}
