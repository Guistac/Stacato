#include <pch.h>

#include "MicroFlexE190.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>


void MicroFlex_e190::onConnection() {}

void MicroFlex_e190::onDisconnection() {}

void MicroFlex_e190::initialize() {
	
	auto thisMicroflex = std::static_pointer_cast<MicroFlex_e190>(shared_from_this());
	
	servo = std::make_shared<MicroFlexServoMotor>(thisMicroflex);
	servoPin->assignData(std::static_pointer_cast<ActuatorInterface>(servo));
	
	gpio = std::make_shared<MicroFlexGpio>(thisMicroflex);
	gpioPin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	addNodePin(servoPin);
	addNodePin(position_Pin);
	addNodePin(velocity_Pin);
	addNodePin(load_Pin);
	addNodePin(gpioPin);
	addNodePin(digitalIn0_Pin);
	addNodePin(digitalIn1_Pin);
	addNodePin(digitalIn2_Pin);
	addNodePin(digitalIn3_Pin);
	addNodePin(digitalOut0_Pin);
	addNodePin(digitalOut1_Pin);
	addNodePin(digitalOut2_Pin);
	addNodePin(analogIn0_Pin);
	addNodePin(analogOut0_Pin);
	
	velocityLimit_parameter->addEditCallback([this](){ configureSubmodules(); });
	accelerationLimit_parameter->addEditCallback([this](){ configureSubmodules(); });
	configureSubmodules();
	
	auto thisDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	axis = DS402Axis::make(thisDevice);
	
	axis->processDataConfiguration.enableCyclicSynchronousPositionMode();
	axis->processDataConfiguration.enableCyclicSynchronousVelocityMode();
	axis->processDataConfiguration.enableHomingMode();
	axis->processDataConfiguration.positionFollowingErrorActualValue = true;
	axis->processDataConfiguration.errorCode = true;
	axis->processDataConfiguration.currentActualValue = true;
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	axis->configureProcessData();
	txPdoAssignement.addEntry(0x4020, 0x1, 32, "Digital Inputs", &digitalInputs);
	rxPdoAssignement.addEntry(0x4021, 0x1, 32, "Digital Outputs", &digitalOutputs);
	txPdoAssignement.addEntry(0x4022, 0x1, 16, "Analog Input 0", &AI0);
	rxPdoAssignement.addEntry(0x4023, 0x1, 16, "Analog Output 0", &AO0);
	
}


void MicroFlex_e190::updateServoConfiguration(){
	/*
	servo->velocityLimit = velocityLimit_parameter->value;
	servo->accelerationLimit = accelerationLimit_parameter->value;
	servo->decelerationLimit = accelerationLimit_parameter->value;
	servo->b_decelerationLimitEqualsAccelerationLimit = true;
	servo->maxFollowingError = maxFollowingError_parameter->value;
	servo->minWorkingRange = 0.0;
	servo->maxWorkingRange = 0.0;
	servoPin->updateConnectedPins();
	*/
}




//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool MicroFlex_e190::startupConfiguration() {
	
	//———— General Drive Control
	
	//set drive control to DS402
	int16_t controlRefSource = 1;
	if(!writeSDO_S16(0x5002, 0x0, controlRefSource)) return false;
	
	//interpolation
	//TODO: how do we set this? it seem the drive works best at 10ms cycle time and stutters at smaller cycle times
	//if(!axis->setInterpolationTimePeriod(EtherCatFieldbus::processInterval_milliseconds)) return false;
	
	//———— Error Reactions
	
	//ERRORDECEL
	//uint32_t errorDeceleration = accelerationLimit_parameter->value * incrementsPerAccelerationUnit;
	uint32_t errorDeceleration = 100;
	if(!axis->setQuickstopDeceleration(errorDeceleration)) return false;
	
	//STOPMODE (Quickstop Reaction or stop input)
	//1 ERROR_DECEL
	if(!axis->setQuickstopOptionCode(1)) return false;
	
	//ABORTMODE (when leaving State Operation Enabled Manually?)
	//0 IGNORE (no error triggered)
	if(!axis->setShutdownOptionCode(0)) return false;
	
	//DS402 Fault Reaction
	//0 Disable
	//2 Quick ramp (ERRORDECEL)
	if(!axis->setFaultReactionOptionCode(0)) return false;
	
	//500D.0 LIMITMODE (when a limit signal is triggered)
	//500E.0 FOLERRORMODE (when following error happens)
	
	//———— PDO Assignement

	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex())) return false;
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex())) return false;
	
	//———— Synchronisation
	
	uint32_t cycleTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1'000'000;
	uint32_t shiftTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 500'000;
	ec_dcsync0(getSlaveIndex(), true, cycleTime_nanoseconds, shiftTime_nanoseconds);
	
	return true;
}



//======================= READING INPUTS =======================

void MicroFlex_e190::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
	
	
	//Drive Status
	bool b_estop = !isStateNone() && !axis->hasVoltage();
	bool b_isReady = !isStateNone() && isStateOperational() && !b_estop && axis->isRemoteControlActive();
	bool b_isEnabled = axis->isEnabled() && !b_waitingForEnable && axis->isRemoteControlActive();
	
	//fault logging
	if(previousErrorCode != axis->getErrorCode()){
		if(axis->getErrorCode() == 0x0) Logger::info("Fault cleared.");
		else Logger::error("fault {:x} {}", axis->getErrorCode(), getErrorCodeString());
	}
	previousErrorCode = axis->getErrorCode();
	
	//status word:
	//b10 : Target Reached (quickstop, halt ???)
	//b12 :	homing mode 1 = homing attained
	//		cyclic modes 0 = target ignored
	//b13 : homing mode 1 = homing error
	//		cyclic position 1 = following error
	bool b_targetReached = axis->getOperatingModeSpecificStatusWordBit_10();
	bool b_homingAttainedOrFollowingTarget = axis->getOperatingModeSpecificStatusWordBit_12();
	bool b_homingErrorOrFollowingError = axis->getOperatingModeSpecificStatusWordBit_13();
	
	//convert DS402 increments to our units
	double actualPosition = (double(axis->getActualPosition()) / incrementsPerPositionUnit) + servo->positionOffset;
	double actualVelocity = double(axis->getActualVelocity()) / incrementsPerVelocityUnit;
	double actualLoad = double(axis->getActualCurrent()) / 1000.0;
	actualPositionFollowingError = axis->getActualPositionFollowingError() / incrementsPerPositionUnit;
	
	//update device modules data
	if(isStateNone()) servo->state = DeviceState::OFFLINE;
	else if(b_isEnabled) servo->state = DeviceState::ENABLED;
	else if(b_isReady) servo->state = DeviceState::READY;
	else servo->state = DeviceState::NOT_READY;
	
	if(isOffline()) gpio->state = DeviceState::OFFLINE;
	else if(isStateSafeOperational()) gpio->state = DeviceState::READY;
	else if(isStateOperational()) gpio->state = DeviceState::ENABLED;
	else gpio->state = DeviceState::NOT_READY;
	
	servo->actuatorProcessData.b_isEmergencyStopActive = b_estop;
	auto& fbPd = servo->feedbackProcessData;
	fbPd.positionActual = actualPosition;
	fbPd.velocityActual = actualVelocity;
	fbPd.forceActual = 0.0;
	
	auto& acPd = servo->actuatorProcessData;
	acPd.effortActual = actualLoad;
	
	//update node pins
	*position_Value = actualPosition;
	*velocity_Value = actualVelocity;
	*load_Value = actualLoad;
	*digitalIn0_Value = digitalInputs & (0x1 << 0);
	*digitalIn1_Value = digitalInputs & (0x1 << 1);
	*digitalIn2_Value = digitalInputs & (0x1 << 2);
	*digitalIn3_Value = digitalInputs & (0x1 << 3);
	
}


//====================== PREPARING OUTPUTS =====================

void MicroFlex_e190::writeOutputs() {
	
	long long now_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	
	//handle enabling & disabling
	if(servo->actuatorProcessData.b_disable){
		servo->actuatorProcessData.b_disable = false;
		servo->actuatorProcessData.b_enable = false;
		b_waitingForEnable = false;
		axis->disable();
	}
	else if(servo->actuatorProcessData.b_enable){
		if(axis->hasFault()) axis->doFaultReset();
		else {
			servo->actuatorProcessData.b_enable = false;
			b_waitingForEnable = true;
			enableRequestTime_nanoseconds = now_nanoseconds;
			axis->enable();
		}
	}
	else if(b_waitingForEnable){
		static const long long maxEnableTime_nanoseconds = 100'000'000; //100ms
		if(axis->isEnabled()) {
			Logger::info("Drive Enabled");
			b_waitingForEnable = false;
		}
		else if(now_nanoseconds - enableRequestTime_nanoseconds > maxEnableTime_nanoseconds){
			Logger::warn("Enable request timed out");
			b_waitingForEnable = false;
			axis->disable();
		}
	}
	
	//fault handling
	if(axis->hasFault()){
		//auto clear fault during enable
		if(b_waitingForEnable) axis->doFaultReset();
		else axis->disable();
	}
	
	

	
	
	//control word:
	//b4 : 	Homing = Start homing
	//b11 : reset warnings
	//axis->setOperatingModeSpecificControlWorldBit_4(b_startHoming);
	//axis->setManufacturerSpecificControlWordBit_11(b_resetWarning);
	
	if(servo->feedbackProcessData.b_overridePosition){
		servo->feedbackProcessData.b_overridePosition = false;
		double overrideTarget = servo->feedbackProcessData.positionOverride;
		double positionRaw = axis->getActualPosition() / double(incrementsPerPositionUnit);
		servo->positionOffset = overrideTarget - positionRaw;
		servo->feedbackProcessData.b_positionOverrideBusy = false;
		servo->feedbackProcessData.b_positionOverrideSucceeded = true;
	}
	
	/*
	if(servoMotor->feedbackProcessData.b_overridePosition){
		servoMotor->feedbackProcessData.b_overridePosition = false;
		double overrideTargetPosition = servoMotor->feedbackProcessData.positionOverride;
		double positionRaw = (double)_p_act / (double)positionUnitsPerRevolution;
		servoMotor->positionOffset_revolutions = overrideTargetPosition - positionRaw;
		updateEncoderWorkingRange();
		servoMotor->feedbackProcessData.b_positionOverrideBusy = false;
		servoMotor->feedbackProcessData.b_positionOverrideSucceeded = true;
	}
	*/
	
	
	
	
	//if the drive is disabled, we copy the actual values to the profiler to be able to restart motion smoothly
	if(!axis->isEnabled()){
		profiler_velocity = double(axis->getActualVelocity()) / incrementsPerVelocityUnit;
		profiler_position = double(axis->getActualPosition()) / incrementsPerPositionUnit;
		axis->setPosition(axis->getActualPosition());
	}
	//copy the values from the servo pin
	else if(servoPin->isConnected()){
		switch(servo->actuatorProcessData.controlMode){
			case ActuatorInterface::ControlMode::VELOCITY:
				axis->setVelocity(servo->actuatorProcessData.velocityTarget * incrementsPerVelocityUnit);
				break;
			case ActuatorInterface::ControlMode::POSITION:
				axis->setPosition((servo->actuatorProcessData.positionTarget - servo->positionOffset) * incrementsPerPositionUnit);
				break;
			case ActuatorInterface::ControlMode::FORCE:
				axis->setVelocity(0);
				break;
		}
	}
	//if the servo pin is not connected, we allow manual velocity control of the axis
	else{
		double deltaT_s = EtherCatFieldbus::getCycleTimeDelta_seconds();
		double acceleration = accelerationLimit_parameter->value;
		double velocityLimit = velocityLimit_parameter->value;
		double deltaV = acceleration * deltaT_s;
		if(manualVelocityTarget > profiler_velocity) {
			profiler_velocity += deltaV;
			if(profiler_velocity > manualVelocityTarget) profiler_velocity = manualVelocityTarget;
		}
		else if(manualVelocityTarget < profiler_velocity) {
			profiler_velocity -= deltaV;
			if(profiler_velocity < manualVelocityTarget) profiler_velocity = manualVelocityTarget;
		}
		profiler_velocity = std::clamp(profiler_velocity, -velocityLimit, velocityLimit);
		double deltaP = profiler_velocity * deltaT_s;
		profiler_position += deltaP;
		axis->setVelocity(profiler_velocity * incrementsPerVelocityUnit);
	}
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}



//============================= SAVING AND LOADING DEVICE DATA ============================

bool MicroFlex_e190::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	if(!axisParameters.save(xml)) return false;
    return true;
}


bool MicroFlex_e190::loadDeviceData(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;
	if(!axisParameters.load(xml)) return false;
	velocityLimit_parameter->onEdit();
	accelerationLimit_parameter->onEdit();
	maxFollowingError_parameter->onEdit();
    return true;
}


//—————— Config Upload


void MicroFlex_e190::uploadDriveConfiguration(){
	
	bool success = true;
	
	//Current Limitation as .1% increments of drive rated current
	success &= axis->setMaxCurrent(currentLimit_parameter->value * 10.0);

	//Max Following Error
	success &= axis->setPositionFollowingErrorWindow(maxFollowingError_parameter->value * incrementsPerPositionUnit);

	//Drive Polarity
	success &= axis->setPolarity(invertDirection_parameter->value);

	Logger::trace("[{}] Storing Drive Parameters...", getName());
	
	//CoE Standart Store Parameters Object 0x1010:1
	//uint32_t write 0x65766173 to save, read 0x1 to confirm parameters were saved
	
	if(writeSDO_U32(0x1010, 0x1, 0x65766173)){
		uint32_t storeParametersStatus = 0;
		double storeStartTime = Timing::getProgramTime_seconds();
		while(true){
			readSDO_U32(0x1010, 0x1, storeParametersStatus);
			if(Timing::getProgramTime_seconds() - storeStartTime > 3.0){
				success = false;
				Logger::trace("[{}] Paramater storage timed out", getName());
				break;
			}
			else if(storeParametersStatus == 0x1){
				success &= true;
				Logger::info("[{}] Configuration saved on drive", getName());
				break;
			}
			else{
				Logger::trace("[{}] Waiting for storage confirmation", getName());
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	
}
