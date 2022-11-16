#include "DS402Axis.h"

#include "EtherCatPDO.h"

int8_t getOperatingModeInteger(DS402Axis::OperatingMode mode) {
	switch(mode){
		case DS402Axis::OperatingMode::NONE:												return 0;
		case DS402Axis::OperatingMode::PROFILE_POSITION:									return 1;
		case DS402Axis::OperatingMode::VELOCITY:											return 2;
		case DS402Axis::OperatingMode::PROFILE_VELOCITY:									return 3;
		case DS402Axis::OperatingMode::PROFILE_TORQUE:										return 4;
		case DS402Axis::OperatingMode::HOMING:												return 6;
		case DS402Axis::OperatingMode::INTERPOLATED_POSITION:								return 7;
		case DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION:							return 8;
		case DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY:							return 9;
		case DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE:							return 10;
		case DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE: 	return 11;
	}
}

DS402Axis::OperatingMode getOperatingMode(int8_t i) {
	switch(i){
		case 0: return DS402Axis::OperatingMode::NONE;
		case 1: return DS402Axis::OperatingMode::PROFILE_POSITION;
		case 2: return DS402Axis::OperatingMode::VELOCITY;
		case 3: return DS402Axis::OperatingMode::PROFILE_VELOCITY;
		case 4: return DS402Axis::OperatingMode::PROFILE_TORQUE;
		case 6: return DS402Axis::OperatingMode::HOMING;
		case 7: return DS402Axis::OperatingMode::INTERPOLATED_POSITION;
		case 8: return DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION;
		case 9: return DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY;
		case 10: return DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE;
		case 11: return DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE;
		default: return DS402Axis::OperatingMode::NONE;
	}
}



void DS402Axis::configureProcessData(EtherCatPdoAssignement& rxPdo, EtherCatPdoAssignement& txPdo){
	
	rxPdo.addEntry(0x6040, 0x0, 16, "DS402_controlWord", &controlWord);
	rxPdo.addEntry(0x6060, 0x0, 8, "DS402_operatingModeSelection", &operatingModeSelection);
	
	txPdo.addEntry(0x6041, 0x0, 16, "DS402_statusWord", &statusWord);
	txPdo.addEntry(0x6061, 0x0, 8, "DS402_operatingModeDisplay", &operatingModeDisplay);
	
	if(configure_Velocity){
		rxPdo.addEntry(0x6042, 0x0, 16, "vl_velocityTarget", &vl_velocityTarget);
		txPdo.addEntry(0x6044, 0x0, 16, "vl_velocityActual", &vl_velocityActual);
	}
	if(configure_CyclicSynchronousPosition){
		rxPdo.addEntry(0x607A, 0x0, 32, "csp_positionTarget", &csp_positionTarget);
	}
	if(configure_CyclicSynchronousVelocity){
		rxPdo.addEntry(0x60FF, 0x0, 32, "csv_velocityTarget", &csv_velocityTarget);
	}
	if(configure_CyclicSynchronousTorque){
		rxPdo.addEntry(0x6071, 0x0, 32, "cst_torqueTarget", &cst_torqueTarget);
	}
	if(configure_CyclicSynchronousPosition
	   || configure_CyclicSynchronousVelocity
	   || configure_CyclicSynchronousTorque){
		txPdo.addEntry(0x6064, 0x0, 32, "csp_positionActual", &csp_positionActual);
		txPdo.addEntry(0x606C, 0x0, 32, "csv_velocityActual", &csv_velocityActual);
		txPdo.addEntry(0x6077, 0x0, 32, "cst_torqueActual", &cst_torqueActual);
	}
}

void DS402Axis::updateInputs(){
	
	//=== update actual power state
	bool readyToSwitchOn_bit =	(statusWord >> 0) & 0x1;
	bool switchedOn_bit =		(statusWord >> 1) & 0x1;
	bool operationEnabled_bit = (statusWord >> 2) & 0x1;
	bool fault_bit =			(statusWord >> 3) & 0x1;
	bool quickstop_bit =		(statusWord >> 5) & 0x1;
	bool switchOnDisabled_bit = (statusWord >> 6) & 0x1;
	if (readyToSwitchOn_bit) {
		if (fault_bit) 					powerStateActual = DS402Axis::PowerState::FAULT_REACTION_ACTIVE;
		else if (!quickstop_bit) 		powerStateActual = DS402Axis::PowerState::QUICKSTOP_ACTIVE;
		else if (operationEnabled_bit) 	powerStateActual = DS402Axis::PowerState::OPERATION_ENABLED;
		else if (switchedOn_bit) 		powerStateActual = DS402Axis::PowerState::SWITCHED_ON;
		else 							powerStateActual = DS402Axis::PowerState::READY_TO_SWITCH_ON;
	}
	else {
		if (fault_bit) 					powerStateActual = DS402Axis::PowerState::FAULT;
		else if (switchOnDisabled_bit) 	powerStateActual = DS402Axis::PowerState::SWITCH_ON_DISABLED;
		else 							powerStateActual = DS402Axis::PowerState::NOT_READY_TO_SWITCH_ON;
	}
	
	//TODO: we should react to power state changes and change the state target accordingly
	
	//=== update actual operating mode
	operatingModeActual = getOperatingMode(operatingModeDisplay);
	
	//=== update axis state
	b_isReady					= readyToSwitchOn_bit;
	b_isEnabled					= operationEnabled_bit;
	b_hasFault					= fault_bit;
	b_voltageEnabled 			= (statusWord >> 4) & 0x1;
	b_isQuickstop				= !quickstop_bit;
	b_hasWarning 				= (statusWord >> 7) & 0x1;
	b_isControlledRemotely 		= (statusWord >> 9) & 0x1;
	b_internalLimitActive 		= (statusWord >> 11) & 0x1;
	b_isFollowingCommandValue 	= (statusWord >> 12) & 0x1;
}

void DS402Axis::updateOutput(){

	//=== reset control word
	controlWord = 0x0;
	
	//=== react to power stage commands
	if(b_shouldEnable){
		b_shouldEnable = false;
		powerStateTarget = PowerState::OPERATION_ENABLED;
	}
	if(b_shouldDisable){
		b_shouldDisable = false;
		powerStateTarget = PowerState::READY_TO_SWITCH_ON;
	}
	if(b_shouldQuickstop){
		b_shouldQuickstop = false;
		powerStateTarget = PowerState::QUICKSTOP_ACTIVE;
	}
	
	//=== react to fault reset command
	if(b_shouldFaultReset && !b_faultResetBusy){
		controlWord |= (0x1 << 7);
		b_faultResetBusy = true;
		b_shouldFaultReset = false;
	}else if(b_faultResetBusy){
		b_faultResetBusy = false;
	}
	
	//=== update power state target
	switch(powerStateTarget){
		case PowerState::QUICKSTOP_ACTIVE:{
			//target: QUICKSTOP
			switch(powerStateActual){
				case PowerState::OPERATION_ENABLED:
				case PowerState::QUICKSTOP_ACTIVE:
					//go to QUICKSTOP_ACTIVE
					controlWord |= 0b1011;
					break;
				default:
					//go to SWITCH_ON_DISABLED
					controlWord |= 0b0000;
					break;
			}
		}break;
		case PowerState::OPERATION_ENABLED:{
			//target: OPERATION_ENABLED
			switch(powerStateActual){
				case PowerState::SWITCHED_ON:
				case PowerState::QUICKSTOP_ACTIVE:
				case PowerState::OPERATION_ENABLED:
					//go to OPERATION_ENABLED
					controlWord |= 0b1111;
					break;
				case PowerState::READY_TO_SWITCH_ON:
					//go to SWITCHED_ON
					controlWord |= 0b0111;
					break;
				default:
					//go to READY_TO_SWITCH_ON
					controlWord |= 0b0110;
					break;
			}
		}break;
		case PowerState::READY_TO_SWITCH_ON:
		default:{
			//target: READY_TO_SWITCH_ON
			switch(powerStateActual){
				case PowerState::OPERATION_ENABLED:
				case PowerState::SWITCHED_ON:
				case PowerState::SWITCH_ON_DISABLED:
				case PowerState::READY_TO_SWITCH_ON:
					//go to READY_TO_SWITCH_ON
					controlWord |= 0b0110;
					break;
				default:
					//go to SWITCH_ON_DISABLED
					controlWord |= 0b0000;
					break;
			}
		}break;
	}
	
	//update application specific control word bits
	if(statusOpB4) controlWord |= 0x1 << 4;
	if(statusOpB5) controlWord |= 0x1 << 5;
	if(statusOpB6) controlWord |= 0x1 << 6;
	if(statusOpB9) controlWord |= 0x1 << 9;
	if(statusManB11) controlWord |= 0x1 << 11;
	if(statusManB12) controlWord |= 0x1 << 12;
	if(statusManB13) controlWord |= 0x1 << 13;
	if(statusManB14) controlWord |= 0x1 << 14;
	if(statusManB15) controlWord |= 0x1 << 15;
	
	//=== update operating mode target
	operatingModeSelection = getOperatingModeInteger(operatingModeTarget);
	
	//update cyclic targets if the modes are configured and not currently in use
	if(configure_CyclicSynchronousPosition && operatingModeActual != OperatingMode::CYCLIC_SYNCHRONOUS_POSITION){
		csp_positionTarget = csp_positionActual;
	}
	if(configure_CyclicSynchronousVelocity && operatingModeActual != OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY){
		csv_velocityTarget = csv_velocityActual;
	}
	if(configure_CyclicSynchronousTorque && operatingModeActual != OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE){
		cst_torqueTarget = cst_torqueActual;
	}
}

void DS402Axis::setPosition(int32_t position){
	csp_positionTarget = position;
	operatingModeTarget = OperatingMode::CYCLIC_SYNCHRONOUS_POSITION;
}

void DS402Axis::setVelocity(int32_t velocity){
	csv_velocityTarget = velocity;
	operatingModeTarget = OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY;
}

void DS402Axis::setTorque(int32_t torque){
	cst_torqueTarget = torque;
	operatingModeTarget = OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE;
}

void DS402Axis::setFrequency(int16_t frequency){
	vl_velocityTarget = frequency;
	operatingModeTarget = OperatingMode::VELOCITY;
}


