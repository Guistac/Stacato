#include <pch.h>

#include "PD4_E.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

void PD4_E::onDisconnection() {}

void PD4_E::onConnection() {}

void PD4_E::resetData() {
	servoMotor->state = DeviceState::OFFLINE;
	servoMotor->actuatorProcessData.b_isEmergencyStopActive = false;
	requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	gpioDevice->state = DeviceState::OFFLINE;
}

void PD4_E::initialize() {
	
	servoActuatorDeviceLink->assignData(std::static_pointer_cast<ActuatorInterface>(servoMotor));
	gpioDeviceLink->assignData(std::static_pointer_cast<GpioInterface>(gpioDevice));

	addNodePin(servoActuatorDeviceLink);
	addNodePin(gpioDeviceLink);
	
	positionPin->assignData(positionPinValue);
	addNodePin(positionPin);
	
	velocityPin->assignData(velocityPinValue);
	addNodePin(velocityPin);
	
	digitalIn1Pin->assignData(digitalIn1PinValue);
	addNodePin(digitalIn1Pin);
	
	digitalIn2Pin->assignData(digitalIn2PinValue);
	addNodePin(digitalIn2Pin);
	
	digitalIn3Pin->assignData(digitalIn3PinValue);
	addNodePin(digitalIn3Pin);
	
	digitalIn4Pin->assignData(digitalIn4PinValue);
	addNodePin(digitalIn4Pin);
	
	digitalIn5Pin->assignData(digitalIn5PinValue);
	addNodePin(digitalIn5Pin);
	
	digitalIn6Pin->assignData(digitalIn6PinValue);
	addNodePin(digitalIn6Pin);
	
	analogIn1Pin->assignData(analogIn1PinValue);
	addNodePin(analogIn1Pin);
	
	digitalOut1Pin->assignData(digitalOut1PinValue);
	addNodePin(digitalOut1Pin);
	
	digitalOut2Pin->assignData(digitalOut2PinValue);
	addNodePin(digitalOut2Pin);

	servoMotor->positionUnit = Units::AngularDistance::Revolution;
	double maxEncoderRevolutions = 1 << encoderMultiTurnResolutionBits;
	servoMotor->feedbackConfig.positionLowerWorkingRangeBound = -maxEncoderRevolutions / 2.0;
	servoMotor->feedbackConfig.positionUpperWorkingRangeBound = maxEncoderRevolutions / 2.0;

	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(DS402::controlWordIndex,			0x0, 16, "DS402 Control Word", &ds402control.controlWord);
	rxPdoAssignement.addEntry(DS402::operatingModeControlIndex, 0x0, 8, "Operating Mode Control", &ds402control.operatingMode);
	rxPdoAssignement.addEntry(DS402::targetPositionIndex,		0x0, 32, "Target Position", &targetPosition);
	rxPdoAssignement.addEntry(DS402::targetVelocityIndex,		0x0, 32, "Target Velocity", &targetVelocity);
	rxPdoAssignement.addEntry(0x60FE, 0x1, 32, "Digital Outputs", &digitalOutputs);

	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(DS402::statusWordIndex,			0x0, 16, "DS402 Status Word", &ds402status.statusWord);
	txPdoAssignement.addEntry(DS402::operatingModeDisplayIndex, 0x0, 8,	"Operating Mode Display", &ds402status.operatingMode);
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

	//changes the direction of motion of the motor
	uint8_t overrideFieldInversion;
	if (invertDirectionOfMotion) overrideFieldInversion = 0x80;
	else overrideFieldInversion = 0x0;
	Logger::critical("invert: {}", overrideFieldInversion);
	if (!writeSDO_U8(0x607E, 0x0, overrideFieldInversion)) return false;
	b_directionOfMotionIsInverted = invertDirectionOfMotion;

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

	DS402::PowerState previousPowerState = actualPowerState;

	if ((b_startAutoSetup || b_autoSetupActive) && ds402status.operatingMode == -2) {
		if (!b_autoSetupActive) {
			b_autoSetupActive = true;
			b_startAutoSetup = false;
		}
		if (ds402status.getOperationModeSpecificByte12()) {
			b_autoSetupComplete = true;
			b_autoSetupActive = false;
			servoMotor->disable();
		}
	}
	actualOperatingMode = ds402status.getOperatingMode();
	actualPowerState = ds402status.getPowerState();

	if (actualPowerState != previousPowerState) {
		Logger::warn("Power state changed to {}", Enumerator::getDisplayString(actualPowerState));
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
		if (actualPowerState != DS402::PowerState::OPERATION_ENABLED) servoMotor->disable();
	}

	int encoderIncrementsPerRevolution = 0x1 << encoderSingleTurnResolutionBits;
	actualPosition_revolutions = (double)actualPosition / (double)encoderIncrementsPerRevolution;
	if(b_directionOfMotionIsInverted) actualPosition_revolutions *= -1.0;

	actualFollowingError_revolutions = (double)actualError / (double)encoderIncrementsPerRevolution;
	actualVelocity_revolutionsPerSecond = (double)actualVelocity / velocityUnitsPerRevolutionPerSecond;
	actualCurrent_amperes = std::abs((double)actualCurrent / 1000.0);
	*positionPinValue = actualPosition_revolutions;
	*velocityPinValue = actualVelocity_revolutionsPerSecond;

	digitalIn1 = (digitalInputs >> 16) & 0x1;
	digitalIn2 = (digitalInputs >> 17) & 0x1;
	digitalIn3 = (digitalInputs >> 18) & 0x1;
	digitalIn4 = (digitalInputs >> 19) & 0x1;
	digitalIn5 = (digitalInputs >> 20) & 0x1;
	digitalIn6 = (digitalInputs >> 21) & 0x1;
	
	*digitalIn1PinValue = digitalIn1;
	*digitalIn2PinValue = digitalIn2;
	*digitalIn3PinValue = digitalIn3;
	*digitalIn4PinValue = digitalIn4;
	*digitalIn5PinValue = digitalIn5;
	*digitalIn6PinValue = digitalIn6;

	/*
	if (negativeLimitSwitchOnDigitalIn1 && digitalIn1 && servoMotor->isEnabled() && actualVelocity_revolutionsPerSecond < 0.0) {
		Logger::warn("Hit Negative Limit Switch");
		servoMotor->b_enabled = false;
	}
	if (positiveLimitSwitchOnDigitalIn2 && digitalIn2 && servoMotor->isEnabled() && actualVelocity_revolutionsPerSecond > 0.0) {
		Logger::warn("Hit Positive Limit Switch");
		servoMotor->b_enabled = false;
	}
	 */

	auto& actProc = servoMotor->actuatorProcessData;
	auto& fdbProc = servoMotor->feedbackProcessData;
	fdbProc.positionActual = actualPosition_revolutions;
	fdbProc.velocityActual = actualVelocity_revolutionsPerSecond;
	actProc.effortActual = actualCurrent_amperes / maxCurrent_amperes;
	switch (actualPowerState) {
		case DS402::PowerState::OPERATION_ENABLED:
			servoMotor->state = DeviceState::ENABLED;
			break;
		case DS402::PowerState::QUICKSTOP_ACTIVE:
		case DS402::PowerState::READY_TO_SWITCH_ON:
		case DS402::PowerState::SWITCHED_ON:
			servoMotor->state = DeviceState::READY;
			break;
		default:
			servoMotor->state = DeviceState::NOT_READY;
			break;
	}
	gpioDevice->state = DeviceState::ENABLED;
	/*
	if (actualControlMode != ControlMode::Mode::EXTERNAL_CONTROL) servoMotor->b_ready = false;
	switch (actualPowerState) {
	case DS402::PowerState::OPERATION_ENABLED:
		servoMotor->b_enabled = true;
		break;
	default:
		servoMotor->b_enabled = false;
		break;
	}
	 */
	//gpioDevice->b_detected = true;
	//gpioDevice->b_online = true;
	//gpioDevice->b_ready = true;

}

void PD4_E::writeOutputs() {

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
			if (servoActuatorDeviceLink->isConnected()) profilePosition_revolutions = servoMotor->actuatorProcessData.positionTarget;
			profileVelocity_revolutions = (profilePosition_revolutions - previousProfilePosition_revolutions) / profileTimeDelta_seconds;
			}break;
	}
	int encoderIncrementsPerRevolution = 0x1 << encoderSingleTurnResolutionBits;
	targetPosition = profilePosition_revolutions * encoderIncrementsPerRevolution;

	if (digitalOut1Pin->isConnected()) *digitalOut1PinValue = digitalOut1Pin->getConnectedPin()->read<bool>();
	if (digitalOut2Pin->isConnected()) *digitalOut2PinValue = digitalOut2Pin->getConnectedPin()->read<bool>();
	digitalOut1 = *digitalOut1PinValue;
	digitalOut2 = *digitalOut2PinValue;
	digitalOutputs = 0;
	if (digitalOut1) digitalOutputs |= 0x1 << 16;
	if (digitalOut2) digitalOutputs |= 0x1 << 17;

	if (servoMotor->b_enable) {
		servoMotor->b_enable = false;
		profilePosition_revolutions = actualPosition_revolutions;
		profileVelocity_revolutions = actualVelocity_revolutionsPerSecond;
		requestedPowerState = DS402::PowerState::OPERATION_ENABLED;
	}

	if (servoMotor->b_disable) {
		servoMotor->b_disable = false;
		requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	}

	/*
	if (servoMotor->b_setQuickstop) {
		servoMotor->b_setQuickstop = false;
		requestedPowerState = DS402::PowerState::QUICKSTOP_ACTIVE;
	}
	 */

	if (ds402status.hasFault()) {
		ds402control.performFaultReset();
	}
	ds402control.setPowerState(requestedPowerState, actualPowerState);
	if (b_startAutoSetup || b_autoSetupActive) {
		ds402control.operatingMode = -2;
		servoMotor->enable();
		if (servoMotor->isEnabled()) {
			ds402control.setOperatingModeSpecificByte4(true);
		}
	}
	else ds402control.setOperatingMode(requestedOperatingMode);
	ds402control.updateControlWord();

	rxPdoAssignement.pushDataTo(identity->outputs);
 
}

bool PD4_E::saveDeviceData(tinyxml2::XMLElement* xml) {
	
	using namespace tinyxml2;

	XMLElement* limitsXML = xml->InsertNewChildElement("Limits");
	limitsXML->SetAttribute("VelocityLimit_revolutionsPerSecond", servoMotor->getVelocityLimit());
	limitsXML->SetAttribute("AccelerationLimit_revolutionsPerSecondSquared", servoMotor->getAccelerationLimit());
	limitsXML->SetAttribute("CurrentLimit_amps", currentLimit_amperes);
	limitsXML->SetAttribute("MaxFollowingError_revolutions", maxFollowingError_revolutions);
	limitsXML->SetAttribute("InvertDirectionOfMotion", invertDirectionOfMotion);

	XMLElement* defaultManualAccelerationXML = xml->InsertNewChildElement("DefaultManualAcceleration");
	defaultManualAccelerationXML->SetAttribute("revolutionsPerSecondSquared", defaultManualAcceleration_revolutionsPerSecondSquared);

	XMLElement* tuningXML = xml->InsertNewChildElement("Tuning");
	tuningXML->SetAttribute("PositionControllerProportionalGain", positionControllerProportionalGain);

	XMLElement* limitSwitchesXML = xml->InsertNewChildElement("LimitSwitches");
	limitSwitchesXML->SetAttribute("UseDigitalInput1asNegativeLimit", negativeLimitSwitchOnDigitalIn1);
	limitSwitchesXML->SetAttribute("UseDigitalInput2asPositiveLimit", positiveLimitSwitchOnDigitalIn2);

	XMLElement* digitalInputInversionXML = xml->InsertNewChildElement("DigitalInputInversion");
	digitalInputInversionXML->SetAttribute("DigitalIn1", invertDigitalInput1);
	digitalInputInversionXML->SetAttribute("DigitalIn2", invertDigitalInput2);
	digitalInputInversionXML->SetAttribute("DigitalIn3", invertDigitalInput3);
	digitalInputInversionXML->SetAttribute("DigitalIn4", invertDigitalInput4);
	digitalInputInversionXML->SetAttribute("DigitalIn5", invertDigitalInput5);
	digitalInputInversionXML->SetAttribute("DigitalIn6", invertDigitalInput6);

	//XMLElement* encoderOffsetXML = xml->InsertNewChildElement("EncoderOffset");
	//encoderOffsetXML->SetAttribute("Revolutions", servoMotor->positionOffset_positionUnits);

	return true;
}
	 
bool PD4_E::loadDeviceData(tinyxml2::XMLElement* xml) { 
	
	using namespace tinyxml2;

	XMLElement* limitsXML = xml->FirstChildElement("Limits");
	if (limitsXML == nullptr) return Logger::warn("Could not find Limit attribute");
	if (limitsXML->QueryDoubleAttribute("VelocityLimit_revolutionsPerSecond", &servoMotor->actuatorConfig.velocityLimit) != XML_SUCCESS) return Logger::warn("Could not find velocity Limit attribute");
	if(limitsXML->QueryDoubleAttribute("AccelerationLimit_revolutionsPerSecondSquared", &servoMotor->actuatorConfig.velocityLimit) != XML_SUCCESS) return Logger::warn("Could not find acceleration Limit attribute");
	if(limitsXML->QueryDoubleAttribute("CurrentLimit_amps", &currentLimit_amperes) != XML_SUCCESS) return Logger::warn("Could not find current Limit attribute");
	if(limitsXML->QueryDoubleAttribute("MaxFollowingError_revolutions", &maxFollowingError_revolutions) != XML_SUCCESS) return Logger::warn("Could not find following error Limit attribute");
	if(limitsXML->QueryBoolAttribute("InvertDirectionOfMotion", &invertDirectionOfMotion) != XML_SUCCESS) return Logger::warn("Could not find Invert direction of motion attribute");

	XMLElement* defaultManualAccelerationXML = xml->FirstChildElement("DefaultManualAcceleration");
	if (defaultManualAccelerationXML == nullptr) return Logger::warn("Could not find default manual acceleration value limit");
	if (defaultManualAccelerationXML->QueryDoubleAttribute("revolutionsPerSecondSquared", &defaultManualAcceleration_revolutionsPerSecondSquared) != XML_SUCCESS) return Logger::warn("Could not load default manual acceleration attribute");

	XMLElement* tuningXML = xml->FirstChildElement("Tuning");
	if (tuningXML == nullptr) return Logger::warn("Could not find tuning attribute");
	if (tuningXML->QueryDoubleAttribute("PositionControllerProportionalGain", &positionControllerProportionalGain) != XML_SUCCESS) return Logger::warn("Could not find proportional gain attribute");

	XMLElement* limitSwitchesXML = xml->FirstChildElement("LimitSwitches");
	if (limitSwitchesXML == nullptr) return Logger::warn("Could not find limit switch attribute");
	if (limitSwitchesXML->QueryBoolAttribute("UseDigitalInput1asNegativeLimit", &negativeLimitSwitchOnDigitalIn1) != XML_SUCCESS) return Logger::warn("Could not find negative limit switch attribute");
	if (limitSwitchesXML->QueryBoolAttribute("UseDigitalInput2asPositiveLimit", &positiveLimitSwitchOnDigitalIn2) != XML_SUCCESS) return Logger::warn("Could not find negative limit switch attribute");

	XMLElement* digitalInputInversionXML = xml->FirstChildElement("DigitalInputInversion");
	if (digitalInputInversionXML == nullptr) return Logger::warn("Could not find digital input inversion attribute");
	if(digitalInputInversionXML->QueryBoolAttribute("DigitalIn1", &invertDigitalInput1) != XML_SUCCESS) return Logger::warn("Could not Find Invert Digital Input 1 Attribute");
	if(digitalInputInversionXML->QueryBoolAttribute("DigitalIn2", &invertDigitalInput2) != XML_SUCCESS) return Logger::warn("Could not Find Invert Digital Input 2 Attribute");
	if(digitalInputInversionXML->QueryBoolAttribute("DigitalIn3", &invertDigitalInput3) != XML_SUCCESS) return Logger::warn("Could not Find Invert Digital Input 3 Attribute");
	if(digitalInputInversionXML->QueryBoolAttribute("DigitalIn4", &invertDigitalInput4) != XML_SUCCESS) return Logger::warn("Could not Find Invert Digital Input 4 Attribute");
	if(digitalInputInversionXML->QueryBoolAttribute("DigitalIn5", &invertDigitalInput5) != XML_SUCCESS) return Logger::warn("Could not Find Invert Digital Input 5 Attribute");
	if(digitalInputInversionXML->QueryBoolAttribute("DigitalIn6", &invertDigitalInput6) != XML_SUCCESS) return Logger::warn("Could not Find Invert Digital Input 6 Attribute");
	
	/*
	XMLElement* encoderOffsetXML = xml->FirstChildElement("EncoderOffset");
	if (encoderOffsetXML == nullptr) return Logger::warn("Could not Find Encoder Offset Attribute");
	if (encoderOffsetXML->QueryDoubleAttribute("Revolutions", &servoMotor->positionOffset_positionUnits) != XML_SUCCESS) return Logger::warn("Could not load Encoder Offset Attribute");
	*/
	
	return true;
}

std::vector<PD4_E::ControlMode> PD4_E::controlModes = {
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
