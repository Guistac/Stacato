#include <pch.h>

#include "MicroFlexE190.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>


void MicroFlex_e190::onConnection() {}

void MicroFlex_e190::onDisconnection() {}

void MicroFlex_e190::onConstruction() {
	
	EtherCatDevice::onConstruction();
	
	setName("MicroFlex e190");
	
	auto thisMicroflex = downcasted_shared_from_this<MicroFlex_e190>();
	
	servoPin = NodePin::createInstance(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Servo Motor", "ServoMotor");
	gpioPin = NodePin::createInstance(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Gpio", "Gpio");
	
	position_Pin = NodePin::createInstance(position_Value, NodePin::Direction::NODE_OUTPUT, "Position", "Position", NodePin::Flags::DisableDataField);
	velocity_Pin = NodePin::createInstance(velocity_Value, NodePin::Direction::NODE_OUTPUT, "Velocity", "Velocity", NodePin::Flags::DisableDataField);
	load_Pin = NodePin::createInstance(load_Value, NodePin::Direction::NODE_OUTPUT, "Load", "Load", NodePin::Flags::DisableDataField);
	
	digitalIn0_Pin = NodePin::createInstance(digitalIn0_Value, NodePin::Direction::NODE_OUTPUT, "Digital Input 0", "DigitalInput0", NodePin::Flags::DisableDataField);
	digitalIn1_Pin = NodePin::createInstance(digitalIn1_Value, NodePin::Direction::NODE_OUTPUT, "Digital Input 1", "DigitalInput1", NodePin::Flags::DisableDataField);
	digitalIn2_Pin = NodePin::createInstance(digitalIn2_Value, NodePin::Direction::NODE_OUTPUT, "Digital Input 2", "DigitalInput2", NodePin::Flags::DisableDataField);
	digitalIn3_Pin = NodePin::createInstance(digitalIn3_Value, NodePin::Direction::NODE_OUTPUT, "Digital Input 3", "DigitalInput3", NodePin::Flags::DisableDataField);
	
	digitalOut0_Pin = NodePin::createInstance(digitalOut0_Value, NodePin::Direction::NODE_INPUT, "Digital Output 0", "DigitalOutput0");
	digitalOut1_Pin = NodePin::createInstance(digitalOut1_Value, NodePin::Direction::NODE_INPUT, "Digital Output 1", "DigitalOutput1");
	digitalOut2_Pin = NodePin::createInstance(digitalOut2_Value, NodePin::Direction::NODE_INPUT, "Digital Output 2", "DigitalOutput2");
	
	analogIn0_Pin = NodePin::createInstance(analogIn0_Value, NodePin::Direction::NODE_OUTPUT, "Analog Input 0", "AnalogInput0", NodePin::Flags::DisableDataField);
	analogOut0_Pin = NodePin::createInstance(analogOut0_Value, NodePin::Direction::NODE_INPUT, "Analog Output 0", "AnalogOutput0");
	
	
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
	
	velocityLimit_parameter = Legato::NumberParameter<double>::createInstance(10.0, "Velocity Limit", "VelocityLimit");
	velocityLimit_parameter->setUnit(Units::AngularDistance::Revolution);
	velocityLimit_parameter->allowNegatives(false);
	velocityLimit_parameter->setSuffix("/s");

	accelerationLimit_parameter = Legato::NumberParameter<double>::createInstance(10.0, "Acceleration Limit", "AccelerationLimit");
	accelerationLimit_parameter->setUnit(Units::AngularDistance::Revolution);
	accelerationLimit_parameter->allowNegatives(false);
	accelerationLimit_parameter->setSuffix("/s\xc2\xb2");
																					
	invertMotor_parameter = Legato::BooleanParameter::createInstance(false, "Invert Direction", "InvertDirection");
	currentLimit_parameter = Legato::NumberParameter<double>::createInstance(100.0, "Max Current", "MaxCurrent");
	currentLimit_parameter->setUnit(Units::Fraction::Percent);
	currentLimit_parameter->allowNegatives(false);
																			   
	maxFollowingError_parameter = Legato::NumberParameter<double>::createInstance(1.0, "Max Following Error", "MaxFollowingError");
	maxFollowingError_parameter->setUnit(Units::AngularDistance::Revolution);
	maxFollowingError_parameter->allowNegatives(false);
	
	axisParameters = Legato::ParameterGroup::createInstance("Axis",{
		velocityLimit_parameter,
		accelerationLimit_parameter,
		invertMotor_parameter,
		currentLimit_parameter,
		maxFollowingError_parameter
	});
	axisParameters->setName("Servo Parameters");
	axisParameters->setSaveString("ServoParameters");
	
	
	velocityLimit_parameter->addEditCallback([this](){ configureSubmodules(); });
	accelerationLimit_parameter->addEditCallback([this](){ configureSubmodules(); });
	configureSubmodules();
	
	auto thisDevice = downcasted_shared_from_this<EtherCatDevice>();
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
	
	//———— Current Limitation
	
	//as .1% increments of drive rated current
	uint16_t maxCurrent = currentLimit_parameter->getValue() * 10.0;
	if(!axis->setMaxCurrent(maxCurrent)) return false;
	
	//———— Max Following Error
	
	uint32_t maxFollowingError = maxFollowingError_parameter->getValue() * incrementsPerPositionUnit;
	if(!axis->setPositionFollowingErrorWindow(maxFollowingError)) return false;
	
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
	double actualPosition = double(axis->getActualPosition()) / incrementsPerPositionUnit;
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
	if(servo->b_disableRequest){
		servo->b_disableRequest = false;
		servo->b_enableRequest = false;
		servo->b_waitingForEnable = false;
		axis->disable();
	}
	else if(servo->b_enableRequest){
		if(axis->hasFault()) axis->doFaultReset();
		else {
			servo->b_enableRequest = false;
			servo->b_waitingForEnable = true;
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
	bool b_startHoming = false;
	bool b_resetWarning = false;
	axis->setOperatingModeSpecificControlWorldBit_4(b_startHoming);
	axis->setManufacturerSpecificControlWordBit_11(b_resetWarning);
	
	//if the drive is disabled, we copy the actual values to the profiler to be able to restart motion smoothly
	if(!axis->isEnabled()){
		profiler_velocity = double(axis->getActualVelocity()) / incrementsPerVelocityUnit;
		profiler_position = double(axis->getActualPosition()) / incrementsPerPositionUnit;
		axis->setPosition(axis->getActualPosition());
	}
	//copy the values from the servo pin
	else if(servoPin->isConnected()){
		axis->setPosition(servo->actuatorProcessData.positionTarget * incrementsPerPositionUnit);
	}
	//if the servo pin is not connected, we allow manual velocity control of the axis
	else{
		double deltaT_s = EtherCatFieldbus::getCycleTimeDelta_seconds();
		double acceleration = accelerationLimit_parameter->getValue();
		double velocityLimit = velocityLimit_parameter->getValue();
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
		//axis->setVelocity(profiler_velocity * incrementsPerVelocityUnit);
		axis->setPosition(profiler_position * incrementsPerPositionUnit);
	}
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}



//============================= SAVING AND LOADING DEVICE DATA ============================

bool MicroFlex_e190::onSerialization(){
	bool success = true;
	success &= EtherCatDevice::onSerialization();
	success &= axisParameters->serializeIntoParent(this);
	return success;
}


bool MicroFlex_e190::onDeserialization(){
	bool success = true;
	success &= EtherCatDevice::onDeserialization();
	success &= axisParameters->deserializeFromParent(this);
	return success;
}
