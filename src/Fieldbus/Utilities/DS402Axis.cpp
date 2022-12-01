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



void DS402Axis::configureProcessData(){

	///Configure Mandatory Objects for each configured operating mode
	if(processDataConfiguration.operatingModes.frequency){
		processDataConfiguration.targetFrequency = true;
		processDataConfiguration.frequencyActualValue = true;
	}
	if(processDataConfiguration.operatingModes.cyclicSynchronousPosition){
		processDataConfiguration.targetPosition = true;
		processDataConfiguration.positionActualValue = true;
	}
	if(processDataConfiguration.operatingModes.cyclicSynchronousVelocity){
		processDataConfiguration.targetVelocity = true;
		processDataConfiguration.velocityActualValue = true;
	}
	if(processDataConfiguration.operatingModes.cyclicSynchronousTorque){
		processDataConfiguration.targetTorque = true;
		processDataConfiguration.torqueActualValue = true;
	}
	//if(processDataConfiguration.operatingModes.cyclicSycnhronousTorqueWithCommutationAngle){
	//	processDataConfiguration.targetTorque = true;
	//	processDataConfiguration.commutationAngle = true;
	//}
	
	//———— Drive Operation
	
	//mandatory objects
	parentDevice->rxPdoAssignement.addEntry(0x6040, 0x0, 16, "DS402 Control Word", &processData.controlWord);
	parentDevice->rxPdoAssignement.addEntry(0x6060, 0x0, 8, "DS402 Operating Mode Selection", &processData.operatingModeSelection);
	parentDevice->txPdoAssignement.addEntry(0x6041, 0x0, 16, "DS402 Status Word", &processData.statusWord);
	parentDevice->txPdoAssignement.addEntry(0x6061, 0x0, 8, "DS402 Operating Mode Display", &processData.operatingModeDisplay);
	
	///603F.0 Error Code
	if(processDataConfiguration.errorCode)
		parentDevice->txPdoAssignement.addEntry(0x603F, 0x0, 16, "DS402 Error Code", &processData.errorCode);
	
	//———— Frequency Converter
	
	///6042.0 Target Velocity (frequency converter mode)
	if(processDataConfiguration.targetFrequency)
		parentDevice->rxPdoAssignement.addEntry(0x6042, 0x0, 16, "DS402 Target Frequency", &processData.targetFrequency);
	
	///6044.0 Velocity Actual Value (frequency converter mode)
	if(processDataConfiguration.frequencyActualValue)
		parentDevice->txPdoAssignement.addEntry(0x6044, 0x0, 16, "DS402 Frequency Actual Value", &processData.frequencyActualValue);
	
	//———— Position
	
	///607A.0 Target Position
	if(processDataConfiguration.targetPosition)
		parentDevice->rxPdoAssignement.addEntry(0x607A, 0x0, 32, "DS402 Target Position", &processData.targetPosition);
	
	///6064.0 Position Actual Value
	if(processDataConfiguration.positionActualValue)
		parentDevice->txPdoAssignement.addEntry(0x6064, 0x0, 32, "DS402 Position Actual Value", &processData.positionActualValue);
	
	///60F4.0 Following Error Actual Value (int32, position units)
	if(processDataConfiguration.positionFollowingErrorActualValue)
		parentDevice->txPdoAssignement.addEntry(0x60F4, 0x0, 32, "DS402 Position Following Error Actual Value", &processData.positionFollowingErrorActualValue);
	
	//———— Velocity
	
	///60FF.0 Target Velocity
	if(processDataConfiguration.targetVelocity)
		parentDevice->rxPdoAssignement.addEntry(0x60FF, 0x0, 32, "DS402 Target Velocity", &processData.targetVelocity);
	
	///606C.0 Velocity Actual Value
	if(processDataConfiguration.velocityActualValue)
		parentDevice->txPdoAssignement.addEntry(0x606C, 0x0, 32, "DS402 Velocity Actual Value", &processData.velocityActualValue);
	
	//———— Torque
	
	///6071.0 Target Torque
	if(processDataConfiguration.targetTorque)
		parentDevice->rxPdoAssignement.addEntry(0x6071, 0x0, 16, "DS402 Target Torque", &processData.targetTorque);
	
	///6077.0 Torque Actual Value
	if(processDataConfiguration.torqueActualValue)
		parentDevice->txPdoAssignement.addEntry(0x6077, 0x0, 16, "DS402 Torque Actual Value", &processData.torqueActualValue);
	
	///6078.0 Current Actual Value
	if(processDataConfiguration.currentActualValue)
		parentDevice->txPdoAssignement.addEntry(0x6078, 0x0, 16, "DS402 Current Actual Value", &processData.currentActualValue);
	
	///60EA.0 Commutation Angle
	//bool commutationAngle = false;
	
	///60FA Control Effort
	//bool controlEffort = false;
	
	//———— Inputs and Outputs
	
	///60FD.0 Digital Inputs
	if(processDataConfiguration.digitalInputs)
		parentDevice->txPdoAssignement.addEntry(0x60FD, 0x0, 32, "DS402 Digital Inputs", &processData.digitalInputs);
	
	///60FE.1 Digital Outputs
	if(processDataConfiguration.digitalOutputs)
		parentDevice->rxPdoAssignement.addEntry(0x60FE, 0x1, 32, "DS402 Digital Outputs", &processData.digitalOutputs);
	
}

void DS402Axis::updateInputs(){
	
	//=== update actual power state
	bool readyToSwitchOn_bit =	(processData.statusWord >> 0) & 0x1;
	bool switchedOn_bit =		(processData.statusWord >> 1) & 0x1;
	bool operationEnabled_bit = (processData.statusWord >> 2) & 0x1;
	bool fault_bit =			(processData.statusWord >> 3) & 0x1;
	bool quickstop_bit =		(processData.statusWord >> 5) & 0x1;
	bool switchOnDisabled_bit = (processData.statusWord >> 6) & 0x1;
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
	operatingModeActual = getOperatingMode(processData.operatingModeDisplay);
	
	//=== update axis state
	b_isReady					= readyToSwitchOn_bit;
	b_isEnabled					= operationEnabled_bit;
	b_hasFault					= fault_bit;
	b_voltageEnabled 			= (processData.statusWord >> 4) & 0x1;
	b_isQuickstop				= !quickstop_bit;
	b_hasWarning 				= (processData.statusWord >> 7) & 0x1;
	b_isControlledRemotely 		= (processData.statusWord >> 9) & 0x1;
	b_internalLimitActive 		= (processData.statusWord >> 11) & 0x1;
	b_isFollowingCommandValue 	= (processData.statusWord >> 12) & 0x1;
}

void DS402Axis::updateOutput(){

	//=== reset control word
	processData.controlWord = 0x0;
	
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
		processData.controlWord |= (0x1 << 7);
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
					processData.controlWord |= 0b1011;
					break;
				default:
					//go to SWITCH_ON_DISABLED
					processData.controlWord |= 0b0000;
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
					processData.controlWord |= 0b1111;
					break;
				case PowerState::READY_TO_SWITCH_ON:
					//go to SWITCHED_ON
					processData.controlWord |= 0b0111;
					break;
				default:
					//go to READY_TO_SWITCH_ON
					processData.controlWord |= 0b0110;
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
					processData.controlWord |= 0b0110;
					break;
				default:
					//go to SWITCH_ON_DISABLED
					processData.controlWord |= 0b0000;
					break;
			}
		}break;
	}
	
	//update application specific control word bits
	if(statusOpB4) processData.controlWord |= 0x1 << 4;
	if(statusOpB5) processData.controlWord |= 0x1 << 5;
	if(statusOpB6) processData.controlWord |= 0x1 << 6;
	if(statusOpB9) processData.controlWord |= 0x1 << 9;
	if(statusManB11) processData.controlWord |= 0x1 << 11;
	if(statusManB12) processData.controlWord |= 0x1 << 12;
	if(statusManB13) processData.controlWord |= 0x1 << 13;
	if(statusManB14) processData.controlWord |= 0x1 << 14;
	if(statusManB15) processData.controlWord |= 0x1 << 15;
	
	//=== update operating mode target
	processData.operatingModeSelection = getOperatingModeInteger(operatingModeTarget);
}


