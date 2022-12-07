#include "DS402Axis.h"

#include "EtherCatPDO.h"



int8_t DS402Axis::getOperatingModeCode(DS402Axis::OperatingMode mode) {
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

DS402Axis::OperatingMode DS402Axis::getOperatingMode(int8_t code) {
	switch(code){
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

	int operatingModeCount = 0;
	
	///Configure Mandatory Objects for each configured operating mode
	if(processDataConfiguration.operatingModes.frequency){
		processDataConfiguration.targetFrequency = true;
		processDataConfiguration.frequencyActualValue = true;
		operatingModeCount++;
	}
	if(processDataConfiguration.operatingModes.cyclicSynchronousPosition){
		processDataConfiguration.targetPosition = true;
		processDataConfiguration.positionActualValue = true;
		operatingModeCount++;
	}
	if(processDataConfiguration.operatingModes.cyclicSynchronousVelocity){
		processDataConfiguration.targetVelocity = true;
		processDataConfiguration.velocityActualValue = true;
		operatingModeCount++;
	}
	if(processDataConfiguration.operatingModes.cyclicSynchronousTorque){
		processDataConfiguration.targetTorque = true;
		processDataConfiguration.torqueActualValue = true;
		operatingModeCount++;
	}
	if(processDataConfiguration.operatingModes.cyclicSycnhronousTorqueWithCommutationAngle){
		assert("Cyclic Synchrinous Torque With Commutation Angle is not supported yet");
		//processDataConfiguration.targetTorque = true;
		//processDataConfiguration.commutationAngle = true;
		operatingModeCount++;
	}
	if(processDataConfiguration.operatingModes.homing){
		operatingModeCount++;
	}
	
	//———— Drive Operation
	
	//mandatory objects
	parentDevice->rxPdoAssignement.addEntry(0x6040, 0x0, 16, "DS402 Control Word", &processData.controlWord);
	parentDevice->txPdoAssignement.addEntry(0x6041, 0x0, 16, "DS402 Status Word", &processData.statusWord);
	
	if(operatingModeCount > 1){
		parentDevice->rxPdoAssignement.addEntry(0x6060, 0x0, 8, "DS402 Operating Mode Selection", &processData.operatingModeSelection);
		parentDevice->txPdoAssignement.addEntry(0x6061, 0x0, 8, "DS402 Operating Mode Display", &processData.operatingModeDisplay);
	}
	
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
	
	bool b_readyToSwitchOn		= (processData.statusWord >> 0) & 0x1;
	bool b_switchedOn 			= (processData.statusWord >> 1) & 0x1;
	bool b_operationEnabled 	= (processData.statusWord >> 2) & 0x1;
	b_hasFault					= (processData.statusWord >> 3) & 0x1;
	b_voltageEnabled 			= (processData.statusWord >> 4) & 0x1;
	bool b_quickstop 			= (processData.statusWord >> 5) & 0x1;
	bool b_switchOnDisabled 	= (processData.statusWord >> 6) & 0x1;
	b_hasWarning 				= (processData.statusWord >> 7) & 0x1;
	statusWord_ManSpecBit_8 	= (processData.statusWord >> 8) & 0x1;
	b_remoteControlActive 		= (processData.statusWord >> 9) & 0x1;
	statusWord_OpSpecBit_10 	= (processData.statusWord >> 10) & 0x1;
	b_internalLimitReached 		= (processData.statusWord >> 11) & 0x1;
	statusWord_OpSpecBit_12 	= (processData.statusWord >> 12) & 0x1;
	statusWord_OpSpecBit_13 	= (processData.statusWord >> 13) & 0x1;
	statusWord_ManSpecBit_14 	= (processData.statusWord >> 14) & 0x1;
	statusWord_ManSpecBit_15 	= (processData.statusWord >> 15) & 0x1;
	
	//Logger::warn("statusword: {:b}", processData.statusWord);
	
	
	//——— old power state solver, we should compare that with future drives
	//if (b_readyToSwitchOn) {
	//	if (b_hasFault) 				powerStateActual = DS402Axis::PowerState::FAULT_REACTION_ACTIVE;
	//	else if (!b_quickstop)			powerStateActual = PowerState::QUICKSTOP_ACTIVE;
	//	else if (b_operationEnabled) 	powerStateActual = DS402Axis::PowerState::OPERATION_ENABLED;
	//	else if (b_switchedOn) 			powerStateActual = DS402Axis::PowerState::SWITCHED_ON;
	//	else 							powerStateActual = DS402Axis::PowerState::READY_TO_SWITCH_ON;
	//}
	//else {
	//	if (b_hasFault) 				powerStateActual = DS402Axis::PowerState::FAULT;
	//	else if (b_switchOnDisabled) 	powerStateActual = DS402Axis::PowerState::SWITCH_ON_DISABLED;
	//	else 							powerStateActual = DS402Axis::PowerState::NOT_READY_TO_SWITCH_ON;
	//}
	

	
	//=== update actual power state
	PowerState previousPowerState = powerStateActual;
	if(b_hasFault && (!b_operationEnabled || !b_switchedOn || !b_readyToSwitchOn))
		powerStateActual = PowerState::FAULT;
	else if(b_hasFault)
		powerStateActual = PowerState::FAULT_REACTION_ACTIVE;
	else if(b_switchOnDisabled)
		powerStateActual = PowerState::SWITCH_ON_DISABLED;
	else if(!b_quickstop)
		powerStateActual = PowerState::QUICKSTOP_ACTIVE;
	else if(b_operationEnabled)
		powerStateActual = PowerState::OPERATION_ENABLED;
	else if(b_switchedOn)
		powerStateActual = PowerState::SWITCHED_ON;
	else if(b_readyToSwitchOn)
		powerStateActual = PowerState::READY_TO_SWITCH_ON;
	else
		powerStateActual = PowerState::NOT_READY_TO_SWITCH_ON;
	
	if(previousPowerState != powerStateActual)
		Logger::warn("{} : Power state changed to {}", parentDevice->getName(), Enumerator::getDisplayString(powerStateActual));
	
	//=== update actual operating mode
	operatingModeActual = getOperatingMode(processData.operatingModeDisplay);
	
	//Logger::warn("status word: {:b}", processData.statusWord);
}

void DS402Axis::updateOutput(){

	//=== reset control word
	processData.controlWord = 0x0;
	
	//=== react to fault reset command
	if(b_doFaultReset && !b_faultResetBusy){
		processData.controlWord |= (0x1 << 7);
		b_faultResetBusy = true;
		b_doFaultReset = false;
	}else if(b_faultResetBusy){
		b_faultResetBusy = false;
	}
	
	
	//Shutdown 			0110
	//Switch On 		0111
	//Disable Voltage	0100
	//Quick Stop		0010
	//Disable Operation	0111
	//Enable Operation	1111
	
	//=== update power state target
	switch(powerStateTarget){
			
		case TargetPowerState::DISABLED:{ //target: READY_TO_SWITCH_ON
			switch(powerStateActual){
				case PowerState::OPERATION_ENABLED: 	//transition 8
				case PowerState::SWITCHED_ON:			//transition 6
				case PowerState::SWITCH_ON_DISABLED:	//transition 2
				case PowerState::READY_TO_SWITCH_ON:	//stay in state
				case PowerState::QUICKSTOP_ACTIVE: 		//transition 12
					processData.controlWord |= 0b0110; 	//Shutdown
					break;
				case PowerState::NOT_READY_TO_SWITCH_ON://transition 1
				default:
					processData.controlWord |= 0b0100;	//Disable Voltage
					break;
			}
		}break;
			
		case TargetPowerState::ENABLED:{ //target: OPERATION_ENABLED
			switch(powerStateActual){
				case PowerState::SWITCHED_ON:			//transition 4
				case PowerState::QUICKSTOP_ACTIVE:		//transition 16
				case PowerState::OPERATION_ENABLED:		//stay in state
					processData.controlWord |= 0b1111; 	//Enable operation
					break;
				case PowerState::READY_TO_SWITCH_ON:	//transition 3
					processData.controlWord |= 0b0111;	//Switch On
					break;
				case PowerState::SWITCH_ON_DISABLED:	//transition 2
				case PowerState::NOT_READY_TO_SWITCH_ON://transition 1
				default:
					processData.controlWord |= 0b0110;  //Shutdown
					break;
			}
		}break;
			
		case TargetPowerState::QUICKSTOP_ACTIVE:{ //target: QUICKSTOP
			switch(powerStateActual){
				case PowerState::OPERATION_ENABLED:		//transition 11
				case PowerState::QUICKSTOP_ACTIVE:		//stay in state
					processData.controlWord |= 0b0010;	//Quickstop
					break;
				default:
					//can't quickstop, go to READY_TO_SWITCH_ON
					processData.controlWord |= 0b0110;
					break;
			}
		}break;
	}
	
	
	
	if(controlWord_OpSpecBit_4) 	processData.controlWord |= (0x1 << 4);
	if(controlWord_OpSpecBit_5) 	processData.controlWord |= (0x1 << 5);
	if(controlWord_OpSpecBit_6) 	processData.controlWord |= (0x1 << 6);
	if(controlWord_OpSpecBit_9) 	processData.controlWord |= (0x1 << 9);
	if(controlWord_ManSpecBit_11) 	processData.controlWord |= (0x1 << 11);
	if(controlWord_ManSpecBit_12) 	processData.controlWord |= (0x1 << 12);
	if(controlWord_ManSpecBit_13) 	processData.controlWord |= (0x1 << 13);
	if(controlWord_ManSpecBit_14) 	processData.controlWord |= (0x1 << 14);
	if(controlWord_ManSpecBit_15) 	processData.controlWord |= (0x1 << 15);
	
	//Logger::error("control word: {:b}", processData.controlWord);
	
	//=== update operating mode target
	processData.operatingModeSelection = getOperatingModeCode(operatingModeTarget);
}


