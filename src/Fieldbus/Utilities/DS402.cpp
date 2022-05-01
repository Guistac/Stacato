#include <pch.h>
#include "DS402.h"

namespace DS402 {
		
	int8_t getOperatingModeInteger(OperatingMode mode) {
		switch(mode){
			case OperatingMode::NONE:												return 0;
			case OperatingMode::PROFILE_POSITION:									return 1;
			case OperatingMode::VELOCITY:											return 2;
			case OperatingMode::PROFILE_VELOCITY:									return 3;
			case OperatingMode::PROFILE_TORQUE:										return 4;
			case OperatingMode::HOMING:												return 6;
			case OperatingMode::INTERPOLATED_POSITION:								return 7;
			case OperatingMode::CYCLIC_SYNCHRONOUS_POSITION:						return 8;
			case OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY:						return 9;
			case OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE:							return 10;
			case OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE: 	return 11;
			case OperatingMode::UNKNOWN:											return 127;
		}
	}

	OperatingMode getOperatingModeFromInteger(int8_t i) {
		switch(i){
			case 0: return OperatingMode::NONE;
			case 1: return OperatingMode::PROFILE_POSITION;
			case 2: return OperatingMode::VELOCITY;
			case 3: return OperatingMode::PROFILE_VELOCITY;
			case 4: return OperatingMode::PROFILE_TORQUE;
			case 6: return OperatingMode::HOMING;
			case 7: return OperatingMode::INTERPOLATED_POSITION;
			case 8: return OperatingMode::CYCLIC_SYNCHRONOUS_POSITION;
			case 9: return OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY;
			case 10: return OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE;
			case 11: return OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE;
			default: return OperatingMode::UNKNOWN;
		}
	}



	PowerState Status::getPowerState() {
		bool readyToSwitchOn =	(statusWord >> 0) & 0x1;
		bool switchedOn =		(statusWord >> 1) & 0x1;
		bool operationEnabled = (statusWord >> 2) & 0x1;
		bool fault =			(statusWord >> 3) & 0x1;
		bool voltageEnabled =	(statusWord >> 4) & 0x1;
		bool quickstop =		(statusWord >> 5) & 0x1;
		bool switchOnDisabled = (statusWord >> 6) & 0x1;
		if (readyToSwitchOn) {
			if (fault) return PowerState::FAULT_REACTION_ACTIVE;
			else if (!quickstop) return PowerState::QUICKSTOP_ACTIVE;
			else if (operationEnabled) return PowerState::OPERATION_ENABLED;
			else if (switchedOn) return PowerState::SWITCHED_ON;
			else return PowerState::READY_TO_SWITCH_ON;
		}
		else {
			if (fault) return PowerState::FAULT;
			else if (switchOnDisabled) return PowerState::SWITCH_ON_DISABLED;
			else return PowerState::NOT_READY_TO_SWITCH_ON;
		}
	}

	OperatingMode Status::getOperatingMode() {
		return getOperatingModeFromInteger(operatingMode);
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




	void Control::setOperatingMode(OperatingMode m) {
		operatingMode = getOperatingModeInteger(m);
	}

	void Control::setPowerState(PowerState requestedState, PowerState currentState) {
		switch (requestedState) {
			case PowerState::READY_TO_SWITCH_ON:
				switch (currentState) {
					case PowerState::OPERATION_ENABLED:
					case PowerState::SWITCHED_ON:
					case PowerState::SWITCH_ON_DISABLED:
						powerStateControlBits = 0b0110;
						break;
					case PowerState::QUICKSTOP_ACTIVE:
						powerStateControlBits = 0b0000;
						break;
					default:
						break;
				}
				break;
			case PowerState::OPERATION_ENABLED:
				switch (currentState) {
					case PowerState::SWITCHED_ON:
					case PowerState::QUICKSTOP_ACTIVE:
						powerStateControlBits = 0b1111;
						break;
					case PowerState::READY_TO_SWITCH_ON:
						powerStateControlBits = 0b0111;
						break;
					case PowerState::SWITCH_ON_DISABLED:
						powerStateControlBits = 0b0110;
						break;
					default:
						break;
				}
				break;
			case PowerState::QUICKSTOP_ACTIVE:
				switch (currentState) {
					case PowerState::OPERATION_ENABLED:
					case PowerState::QUICKSTOP_ACTIVE:
						powerStateControlBits = 0b1011;
						break;
					default:
						powerStateControlBits = 0b0000;
						break;
				}
				break;
			default: break;
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
		controlWord |= powerStateControlBits;
		controlWord |= b4 << 4;
		controlWord |= b5 << 5;
		controlWord |= b6 << 6;
		if(faultResetBit && !wasFaultResetHigh) {
			controlWord |= 0x1 << 7;
			faultResetBit = false;
		}
		controlWord |= haltBit << 8;
		controlWord |= b9 << 9;
		controlWord |= b11 << 11;
		controlWord |= b12 << 12;
		controlWord |= b13 << 13;
		controlWord |= b14 << 14;
		controlWord |= b15 << 15;
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
