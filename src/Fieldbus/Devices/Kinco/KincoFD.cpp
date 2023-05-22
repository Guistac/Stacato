#include "KincoFD.h"

#include "Fieldbus/EtherCatFieldbus.h"

void KincoFD::onDisconnection() {
	actuator->state = DeviceState::OFFLINE;
}
void KincoFD::onConnection() {}

void KincoFD::onDetection(){	
	int16_t CMD_q_Limit;
	readSDO_S16(0x60F6, 0xF, CMD_q_Limit);
	driveCurrentLimit = float(CMD_q_Limit) / float(incrementsPerAmpere);
}

void KincoFD::initialize() {
	auto thisDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	axis = DS402Axis::make(thisDevice);
	
	actuator = std::make_shared<KincoServoMotor>();
	actuator->drive = std::static_pointer_cast<KincoFD>(shared_from_this());
	actuator->positionUnit = Units::AngularDistance::Revolution;
	actuator->actuatorConfig.b_supportsEffortFeedback = true;
	actuator->actuatorConfig.b_supportsForceControl = false;
	actuator->actuatorConfig.b_canQuickstop = false;
	actuator->actuatorConfig.b_supportsVelocityControl = true;
	actuator->actuatorConfig.b_supportsHoldingBrakeControl = false;
	actuator->actuatorConfig.b_supportsPositionControl = true;
	actuator->feedbackConfig.b_supportsForceFeedback = false;
	actuator->feedbackConfig.b_supportsPositionFeedback = true;
	actuator->feedbackConfig.b_supportsVelocityFeedback = true;
	actuator->feedbackConfig.positionFeedbackType = PositionFeedbackType::INCREMENTAL;
	actuator->feedbackConfig.positionLowerWorkingRangeBound = -std::numeric_limits<double>::infinity();
	actuator->feedbackConfig.positionUpperWorkingRangeBound = std::numeric_limits<double>::infinity();
	
	maxVelocity_parameter = NumberParameter<double>::make(0.0, "Max Velocity", "MaxVelocity");
	maxVelocity_parameter->setUnit(Units::AngularDistance::Revolution);
	maxVelocity_parameter->setSuffix("/s");
	maxVelocity_parameter->addEditCallback([this](){updateActuatorInterface();});
	maxAcceleration_parameter = NumberParameter<double>::make(0.0, "Max Acceleration", "MaxAcceleration");
	maxAcceleration_parameter->setUnit(Units::AngularDistance::Revolution);
	maxAcceleration_parameter->setSuffix("/s\xc2\xb2");
	maxAcceleration_parameter->addEditCallback([this](){updateActuatorInterface();});
	
	maxCurrent_parameter = NumberParameter<double>::make(0.0, "Max Current", "MaxCurrent");
	maxCurrent_parameter->setUnit(Units::Current::Ampere);
	maxCurrent_parameter->setFormat("%.2f");
	velocityFeedforward_parameter = NumberParameter<double>::make(0.0, "Velocity Feed Forward", "VelocityFeedForward");
	velocityFeedforward_parameter->setUnit(Units::Fraction::Percent);
	velocityFeedforward_parameter->setFormat("%.1f");
	maxFollowingError_parameter = NumberParameter<double>::make(0.0, "Max Position Following Error", "MaxFollowingError");
	maxFollowingError_parameter->setUnit(Units::AngularDistance::Revolution);
	maxFollowingError_parameter->addEditCallback([this](){updateActuatorInterface();});
	followingErrorTimeout_parameter = NumberParameter<int>::make(0.0, "Position Following Error Time Out", "FollowingErrorTimeout");
	followingErrorTimeout_parameter->setUnit(Units::Time::Millisecond);
	
	brakingResistorResistance_parameter = NumberParameter<int>::make(0.0, "Braking Resistor Resistance", "BrakingResistorResistance");
	brakingResistorResistance_parameter->setUnit(Units::Resistance::Ohm);
	brakingResistorPower_parameter = NumberParameter<int>::make(0.0, "Braking Resistor Rated Power", "BrakingResistorRatedPower");
	brakingResistorPower_parameter->setUnit(Units::Power::Watt);
	
	DIN1Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN1 Function", "Din1Function");
	DIN2Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN2 Function", "Din2Function");
	DIN3Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN3 Function", "Din3Function");
	DIN4Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN4 Function", "Din4Function");
	DIN5Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN5 Function", "Din5Function");
	DIN6Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN6 Function", "Din6Function");
	DIN7Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN7 Function", "Din7Function");
	
	DIN1Polarity_parameter = BooleanParameter::make(false, "DIN1 Polarity", "DIN1Polarity");
	DIN2Polarity_parameter = BooleanParameter::make(false, "DIN2 Polarity", "DIN2Polarity");
	DIN3Polarity_parameter = BooleanParameter::make(false, "DIN3 Polarity", "DIN3Polarity");
	DIN4Polarity_parameter = BooleanParameter::make(false, "DIN4 Polarity", "DIN4Polarity");
	DIN5Polarity_parameter = BooleanParameter::make(false, "DIN5 Polarity", "DIN5Polarity");
	DIN6Polarity_parameter = BooleanParameter::make(false, "DIN6 Polarity", "DIN6Polarity");
	DIN7Polarity_parameter = BooleanParameter::make(false, "DIN7 Polarity", "DIN7Polarity");
	
	DOUT1Function_parameter = OptionParameter::make2(outputFunction_none, outputFunctionOptions, "DOUT1 Function", "Dout1Function");
	DOUT2Function_parameter = OptionParameter::make2(outputFunction_none, outputFunctionOptions, "DOUT2 Function", "Dout2Function");
	DOUT3Function_parameter = OptionParameter::make2(outputFunction_none, outputFunctionOptions, "DOUT3 Function", "Dout3Function");
	DOUT4Function_parameter = OptionParameter::make2(outputFunction_none, outputFunctionOptions, "DOUT4 Function", "Dout4Function");
	DOUT5Function_parameter = OptionParameter::make2(outputFunction_none, outputFunctionOptions, "DOUT5 Function", "Dout5Function");
	
	DOUT1Polarity_parameter = BooleanParameter::make(false, "DOUT1 Polarity", "Dout1Polarity");
	DOUT2Polarity_parameter = BooleanParameter::make(false, "DOUT2 Polarity", "Dout2Polarity");
	DOUT3Polarity_parameter = BooleanParameter::make(false, "DOUT3 Polarity", "Dout3Polarity");
	DOUT4Polarity_parameter = BooleanParameter::make(false, "DOUT4 Polarity", "Dout4Polarity");
	DOUT5Polarity_parameter = BooleanParameter::make(false, "DOUT5 Polarity", "Dout5Polarity");
	
	
	
	
	axis->processDataConfiguration.enableCyclicSynchronousPositionMode();
	axis->processDataConfiguration.enableCyclicSynchronousVelocityMode();
	axis->processDataConfiguration.enableHomingMode();
	axis->processDataConfiguration.positionFollowingErrorActualValue = true;
	axis->processDataConfiguration.errorCode = true;
	axis->processDataConfiguration.currentActualValue = true;
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	axis->configureProcessData();
	
	actuatorPin = std::make_shared<NodePin>(std::static_pointer_cast<ActuatorInterface>(actuator), NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Actuator", "Actuator");
	addNodePin(actuatorPin);
	
	updateActuatorInterface();
}

bool KincoFD::startupConfiguration() {
	//———— PDO Assignement

	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex())) return false;
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex())) return false;
	
	//Configure Synchronisation
	switch(EtherCatFieldbus::processInterval_milliseconds){
		case 1:
			if(!writeSDO_U8(0x3011, 0x1, 0, "ECAN_Sync_Cycle")) return false;
			break;
		case 2:
			if(!writeSDO_U8(0x3011, 0x1, 1, "ECAN_Sync_Cycle")) return false;
			break;
		case 4:
			if(!writeSDO_U8(0x3011, 0x1, 2, "ECAN_Sync_Cycle")) return false;
			break;
		case 8:
			if(!writeSDO_U8(0x3011, 0x1, 3, "ECAN_Sync_Cycle")) return false;
			break;
		default:
			Logger::warn("[{}] Could not set ECAN_Sync_Cycle cycle time", getName());
			Logger::warn("	Supported cycle times are 1/2/4/8ms, current fieldbus cycle time is {}ms", EtherCatFieldbus::processInterval_milliseconds);
			return false;
	}
	//ECAN_Sync_Clock (1==enabled)
	if(!writeSDO_U8(0x3011, 0x2, 1, "ECAN_Sync_Clock")) return false;
	//ECAN_Sync_Shift (1==62.5µs shift)
	//8 == .5ms / 500µs / 500000ns seems to be a good value to avoid motor jitter at high velocity in cyclic position mode
	if(!writeSDO_U8(0x3011, 0x3, 8, "ECAN_Sync_Shift")) return false;
	
	//set command type to communication control (EtherCAT)
	if(!writeSDO_U8(0x3041, 0x2, 8, "Command Type")) return false;
	//enable error on ethercat disconnection
	if(!writeSDO_S16(0x6007, 0x0, 1, "Abort Connection Mode")) return false;
	int driveTimeoutDelay_milliseconds = EtherCatFieldbus::processInterval_milliseconds * 3;
	//error happens if no process data is received for a period equivalent to 3 cycles (in milliseconds)
	if(!writeSDO_U16(0x100C, 0x0, driveTimeoutDelay_milliseconds, "Guard_Time")) return false;
	//set time multiplier to 1
	if(!writeSDO_U8(0x100D, 0x0, 1, "Life_Time_Factor")) return false;
	
	if(!axis->startupConfiguration()) return false;
	
	//———— Synchronisation
	
	uint32_t cycleTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1'000'000;
	//shift time is always half the cycle time
	ec_dcsync0(getSlaveIndex(), true, cycleTime_nanoseconds, cycleTime_nanoseconds / 2);
	
	return true;
}

void KincoFD::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
	
	if(actuator->isEnabled() && !axis->isEnabled()){
		axis->disable();
		b_waitingForEnable = false;
		Logger::warn("[{}] Driver was disabled", getName());
	}
	
	if(b_waitingForEnable){
		//enable time out delay is 200ms
		if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - enableRequestTime_nanos > 200'000'000){
			b_waitingForEnable = false;
			axis->disable();
			Logger::warn("[{}] Driver enable timed out", getName());
		}
		else if(axis->isEnabled()){
			b_waitingForEnable = false;
			Logger::info("[{}] Driver was enabled", getName());
		}
	}
	
	if(axis->isEnabled()) actuator->state = DeviceState::ENABLED;
	else if(axis->isNotReady()) actuator->state = DeviceState::NOT_READY;
	else actuator->state = DeviceState::READY;
	
	if(axis->getErrorCode() != previousErrorCode){
		previousErrorCode = axis->getErrorCode();
		Logger::warn("[{}] Error: {}", getName(), getErrorCodeString());
	}
	
	actuator->b_hasFault = axis->hasFault();
	actuator->feedbackProcessData.positionActual = double(axis->getActualPosition() + positionOffset) / incrementsPerRevolution;;
	actuator->feedbackProcessData.velocityActual = double(axis->getActualVelocity()) / incrementsPerRevolutionPerSecond;
	actuator->feedbackProcessData.forceActual = 0.0;
	actuator->actuatorProcessData.b_isEmergencyStopActive = false;
	actuator->actuatorProcessData.followingErrorActual = double(axis->getActualPositionFollowingError()) / incrementsPerRevolution;
	actuator->actuatorProcessData.effortActual = std::abs(double(axis->getActualCurrent() / incrementsPerAmpere) / maxCurrent_parameter->value);
	
}

void KincoFD::writeOutputs(){
	
	//——— Handle Actuator Interface Commands
	
	if(actuator->actuatorProcessData.b_enable){
		actuator->actuatorProcessData.b_enable = false;
		enableRequestTime_nanos = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
		b_waitingForEnable = true;
	}
	
	if(b_waitingForEnable){
		if(axis->hasFault()) axis->doFaultReset();
		else axis->enable();
	}
	
	if(actuator->actuatorProcessData.b_disable){
		actuator->actuatorProcessData.b_disable = false;
		axis->disable();
	}
	
	if(actuator->feedbackProcessData.b_overridePosition){
		actuator->feedbackProcessData.b_overridePosition = false;
		actuator->feedbackProcessData.b_positionOverrideBusy = true;
	}
	
	//——— Handle Axis Operating mode and commands
	
	if(b_isAutotuning){
		axis->setManufacturerSpecificOperatingMode(11);
	}
	else if(actuator->feedbackProcessData.b_positionOverrideBusy){
		if(axis->doHoming()){
			actuator->feedbackProcessData.b_positionOverrideBusy = false;
			actuator->feedbackProcessData.b_positionOverrideSucceeded = true;
			positionOffset = actuator->feedbackProcessData.positionOverride * incrementsPerRevolution;
		}
	}
	else{
		switch(actuator->actuatorProcessData.controlMode){
			case ActuatorInterface::ControlMode::VELOCITY:
				axis->setVelocity(actuator->actuatorProcessData.velocityTarget * incrementsPerRevolutionPerSecond);
				break;
			case ActuatorInterface::ControlMode::POSITION:
				axis->setPosition(actuator->actuatorProcessData.positionTarget * (incrementsPerRevolution - positionOffset));
				break;
			default:
				axis->setVelocity(0.0);
				break;
		}
	}
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}


bool KincoFD::saveDeviceData(tinyxml2::XMLElement* xml) {
		
	xml->SetAttribute("PositionOffset", positionOffset);
	
	maxVelocity_parameter->save(xml);
	maxAcceleration_parameter->save(xml);
	
	maxCurrent_parameter->save(xml);
	velocityFeedforward_parameter->save(xml);
	maxFollowingError_parameter->save(xml);
	followingErrorTimeout_parameter->save(xml);

	brakingResistorResistance_parameter->save(xml);
	brakingResistorPower_parameter->save(xml);
	
	DIN1Function_parameter->save(xml);
	DIN2Function_parameter->save(xml);
	DIN3Function_parameter->save(xml);
	DIN4Function_parameter->save(xml);
	DIN5Function_parameter->save(xml);
	DIN6Function_parameter->save(xml);
	DIN7Function_parameter->save(xml);
	
	DIN1Polarity_parameter->save(xml);
	DIN2Polarity_parameter->save(xml);
	DIN3Polarity_parameter->save(xml);
	DIN4Polarity_parameter->save(xml);
	DIN5Polarity_parameter->save(xml);
	DIN6Polarity_parameter->save(xml);
	DIN7Polarity_parameter->save(xml);
	
	DOUT1Function_parameter->save(xml);
	DOUT2Function_parameter->save(xml);
	DOUT3Function_parameter->save(xml);
	DOUT4Function_parameter->save(xml);
	DOUT5Function_parameter->save(xml);
	
	DOUT1Polarity_parameter->save(xml);
	DOUT2Polarity_parameter->save(xml);
	DOUT3Polarity_parameter->save(xml);
	DOUT4Polarity_parameter->save(xml);
	DOUT5Polarity_parameter->save(xml);
	
	return true;
}

bool KincoFD::loadDeviceData(tinyxml2::XMLElement* xml) {

	xml->QueryIntAttribute("PositionOffset", &positionOffset);
	
	maxVelocity_parameter->load(xml);
	maxAcceleration_parameter->load(xml);
	
	maxCurrent_parameter->load(xml);
	velocityFeedforward_parameter->load(xml);
	maxFollowingError_parameter->load(xml);
	followingErrorTimeout_parameter->load(xml);
	
	brakingResistorResistance_parameter->load(xml);
	brakingResistorPower_parameter->load(xml);
	
	DIN1Function_parameter->load(xml);
	DIN2Function_parameter->load(xml);
	DIN3Function_parameter->load(xml);
	DIN4Function_parameter->load(xml);
	DIN5Function_parameter->load(xml);
	DIN6Function_parameter->load(xml);
	DIN7Function_parameter->load(xml);
	
	DIN1Polarity_parameter->load(xml);
	DIN2Polarity_parameter->load(xml);
	DIN3Polarity_parameter->load(xml);
	DIN4Polarity_parameter->load(xml);
	DIN5Polarity_parameter->load(xml);
	DIN6Polarity_parameter->load(xml);
	DIN7Polarity_parameter->load(xml);
	
	DOUT1Function_parameter->load(xml);
	DOUT2Function_parameter->load(xml);
	DOUT3Function_parameter->load(xml);
	DOUT4Function_parameter->load(xml);
	DOUT5Function_parameter->load(xml);
	
	DOUT1Polarity_parameter->load(xml);
	DOUT2Polarity_parameter->load(xml);
	DOUT3Polarity_parameter->load(xml);
	DOUT4Polarity_parameter->load(xml);
	DOUT5Polarity_parameter->load(xml);
	
	updateActuatorInterface();
	
	return true;
}


void KincoFD::uploadConfiguration(){
	
	switch(EtherCatFieldbus::processInterval_milliseconds){
		case 1:
			if(!writeSDO_U8(0x3011, 0x1, 0, "ECAN_Sync_Cycle")) return false;
			break;
		case 2:
			if(!writeSDO_U8(0x3011, 0x1, 1, "ECAN_Sync_Cycle")) return false;
			break;
		case 4:
			if(!writeSDO_U8(0x3011, 0x1, 2, "ECAN_Sync_Cycle")) return false;
			break;
		case 8:
			if(!writeSDO_U8(0x3011, 0x1, 3, "ECAN_Sync_Cycle")) return false;
			break;
		default:
			Logger::warn("[{}] Could not set ECAN_Sync_Cycle cycle time", getName());
			Logger::warn("	Supported cycle times are 1/2/4/8ms, current fieldbus cycle time is {}ms", EtherCatFieldbus::processInterval_milliseconds);
			return;
	}
	
	//ECAN_Sync_Clock (1==enabled)
	if(!writeSDO_U8(0x3011, 0x2, 1, "ECAN_Sync_Clock")) return false;
	//ECAN_Sync_Shift (1==62.5µs shift)
	if(!writeSDO_U8(0x3011, 0x3, 10, "ECAN_Sync_Shift")) return false;
	
	axis->setProfileAcceleration(maxAcceleration_parameter->value * incrementsPerRevolutionPerSecondSquared);
	axis->setProfileDeceleration(maxAcceleration_parameter->value * incrementsPerRevolutionPerSecondSquared);
	
	axis->setMaxCurrent(maxCurrent_parameter->value * incrementsPerAmpere);
	writeSDO_S16(0x60FB, 0x2, float(velocityFeedforward_parameter->value) * 2.56);
	axis->setFollowingErrorWindow(maxFollowingError_parameter->value * incrementsPerRevolution);
	writeSDO_U16(0x2508, 0x9, followingErrorTimeout_parameter->value);
	
	writeSDO_U16(0x60F7, 0x1, brakingResistorResistance_parameter->value);
	writeSDO_U16(0x60F7, 0x2, brakingResistorPower_parameter->value);
	
	writeSDO_U16(0x2010, 0x3, uint16_t(DIN1Function_parameter->value), "DIN1 Function");
	writeSDO_U16(0x2010, 0x4, uint16_t(DIN2Function_parameter->value), "DIN2 Function");
	writeSDO_U16(0x2010, 0x5, uint16_t(DIN3Function_parameter->value), "DIN3 Function");
	writeSDO_U16(0x2010, 0x6, uint16_t(DIN4Function_parameter->value), "DIN4 Function");
	writeSDO_U16(0x2010, 0x7, uint16_t(DIN5Function_parameter->value), "DIN5 Function");
	writeSDO_U16(0x2010, 0x8, uint16_t(DIN6Function_parameter->value), "DIN6 Function");
	writeSDO_U16(0x2010, 0x9, uint16_t(DIN7Function_parameter->value), "DIN7 Function");
	
	
	uint16_t DINPolarity = 0x0;
	if(DIN1Polarity_parameter->value) DINPolarity |= (0x1 << 0);
	if(DIN2Polarity_parameter->value) DINPolarity |= (0x1 << 1);
	if(DIN3Polarity_parameter->value) DINPolarity |= (0x1 << 2);
	if(DIN4Polarity_parameter->value) DINPolarity |= (0x1 << 3);
	if(DIN5Polarity_parameter->value) DINPolarity |= (0x1 << 4);
	if(DIN6Polarity_parameter->value) DINPolarity |= (0x1 << 5);
	if(DIN7Polarity_parameter->value) DINPolarity |= (0x1 << 6);
	writeSDO_U16(0x2010, 0x1, DINPolarity);
	
	
	
	writeSDO_U16(0x2010, 0xF, uint16_t(DOUT1Function_parameter->value), "DOUT1 Function");
	writeSDO_U16(0x2010, 0x10, uint16_t(DOUT2Function_parameter->value), "DOUT2 Function");
	writeSDO_U16(0x2010, 0x11, uint16_t(DOUT3Function_parameter->value), "DOUT3 Function");
	writeSDO_U16(0x2010, 0x12, uint16_t(DOUT4Function_parameter->value), "DOUT4 Function");
	writeSDO_U16(0x2010, 0x13, uint16_t(DOUT5Function_parameter->value), "DOUT5 Function");
	
	uint16_t DOUTPolarity = 0x0;
	if(DOUT1Polarity_parameter->value) DOUTPolarity |= (0x1 << 0);
	if(DOUT2Polarity_parameter->value) DOUTPolarity |= (0x1 << 1);
	if(DOUT3Polarity_parameter->value) DOUTPolarity |= (0x1 << 2);
	if(DOUT4Polarity_parameter->value) DOUTPolarity |= (0x1 << 3);
	if(DOUT5Polarity_parameter->value) DOUTPolarity |= (0x1 << 4);
	writeSDO_U16(0x2010, 0xD, DOUTPolarity);
	
	
	
	
	
	
	
	
	
	//store drive data
	if(!writeSDO_U8(0x2FF0, 0x1, 1, "Store Data")) return false;
	//store motor data
	if(!writeSDO_U8(0x2FF0, 0x3, 1, "Store Motor Data")) return false;
	
	Logger::info("[{}] Configuration uploaded, reboot drive to make parameters valid", getName());
}

void KincoFD::updateActuatorInterface(){
	auto& actuatorConfig = actuator->actuatorConfig;
	actuatorConfig.accelerationLimit = maxAcceleration_parameter->value;
	actuatorConfig.decelerationLimit = maxAcceleration_parameter->value;
	actuatorConfig.followingErrorLimit = maxFollowingError_parameter->value;
	actuatorConfig.velocityLimit = maxVelocity_parameter->value;
	actuatorPin->updateConnectedPins();
}

bool KincoFD::startAutoTuning(){
	b_isAutotuning = true;
	
	//set safe movement distance for autotuning (in 0.01 motor rev)
	uint16_t Safe_Dist = 40;
	writeSDO_U16(0x3040, 0x6, Safe_Dist);
	
	//wait for operating mode to change to tuning
	while(true){
		if(axis->getOperatingModeActual() == 11) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	//start tuning command
	writeSDO_S8(0x3040, 0x1, 1);
	
	//wait for tuning to end
	while(true){
		int8_t Inertia_GetResult;
		if(readSDO_S8(0x3040, 0x9, Inertia_GetResult) && Inertia_GetResult == 1) break;
		int8_t Tuning_Method;
		if(readSDO_S8(0x3040, 0x1, Tuning_Method) && Tuning_Method < 0){
			Logger::warn("Tuning failed for reason {}", Tuning_Method);
			b_isAutotuning = false;
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	//store drive data
	writeSDO_U8(0x2FF0, 0x1, 1, "Store Data");
	
	b_isAutotuning = false;
	
	/**
	 If changing stiffness via communication, WriteFUN_CTL(3041.05) must be set to 1 first, and be set back to 0 after stiffness has been changed.
	 **/
	
	
	
}

std::string KincoFD::getErrorCodeString(){
	switch(axis->getErrorCode()){
		case 0x0: 	return "No Error";
		case 0x7380:
		case 0x7331:
		case 0x7122:
					//return "Wrong UVW signals or motor connection / No encoder connected or no encoder communication reply";
					return "error ABZ signals / no communication encoder connected)";
		case 0x7381:
		case 0x7320: return "error UVW signals / Invalid multiturn data, needs special procedure to reset";
		case 0x7305:
		case 0x7330: return "Counting error / Communication with encoder disturbed.";
		case 0x4210: return "Heatsink temperature too high";
		case 0x3210: return "DC-Bus over voltage";
		case 0x3220: return "DC-Bus under voltage";
		case 0x2320: return "Power stage or Motor short circuit";
		case 0x2321: return "current ADC full";
		case 0x7110: return "Overload Brake chopper resistor";
		case 0x8611: return "Max following error exceeded";
		case 0x5112: return "Logic supply voltage too low";
		case 0x2350: return "Motor or Powerstage IIt error";
		case 0x8A80: return "Pulse input frequency too high";
		case 0x4310: return "Motor Temperature sensor Alarm";
		case 0x6310: return "EEPROM Checksum fault";
		case 0x5210: return "Current sensor signal offset or ripple too big";
		case 0x6010: return "Software watchdog exception";
		case 0x6011: return "Invalid Interrupt exception";
		case 0x7400: return "Wrong MCU type detected";
		case 0x6320: return "No motor data in EEPROM / motor never configured";
		case 0x6321: return "one or more motor wire lost";
		case 0xFF01: return "Dout overload (only for JD / K10)";
		case 0xFF11: return "STO1 input error";
		case 0xFF12: return "STO2 input error";
		case 0x5443: return "DIN function \"pre_enable\" is configured, but the input is inactive when the drive is enabled or should become enabled";
		case 0x5442: return "positive Limit (after homing)";
		case 0x5441: return "negaitive Limit (after homing)";
		case 0x6012: return "Firmware internal error in SPI handling";
		case 0x8100:
		case 0x81FF: return "CAN connection aborted or Timeout in service mode";
		case 0x8A81: return "Different direction betw. Motor and Position encoder";
		case 0x7382: return "Antivalence error ABZ signals, Master enc.";
		case 0x7306: return "Counting error Master enc.";
		case 0xFF10: return "STO input error";
		case 0xFF1A: return "STO1 input abnormal behaviour";
		case 0xFF1B: return "STO2 input abnormal behaviour";
		default: {
			std::stringstream output;
			output << "Unknown Error Code : 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << axis->getErrorCode();
			return output.str();
		}
	}
}

std::string KincoFD::KincoServoMotor::getStatusString(){
	if(!isOnline()) return "Drive is offline";
	else if(hasFault()) return std::string("Fault : ") + drive->getErrorCodeString();
	else if(!isReady()){
		if(!drive->axis->hasVoltage()) return "No Motor Voltage";
		else return "Drive is not ready";
	}
	else if(isEnabled()) return "Motor is Enabled";
	else return "Motor is ready";
}

