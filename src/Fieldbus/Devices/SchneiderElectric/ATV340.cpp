#include <pch.h>
#include "ATV340.h"

#include "Fieldbus/EtherCatFieldbus.h"

ATV340::ParameterOptions ATV340::options;

void ATV340::onConnection() {}

void ATV340::onDisconnection() {}

void ATV340::initialize() {
	
	//create submodules
	axis = DS402Axis::make(std::static_pointer_cast<EtherCatDevice>(shared_from_this()));
	motor = std::make_shared<ATV340_Motor>(std::static_pointer_cast<ATV340>(shared_from_this()));
	motor_pin->assignData(std::static_pointer_cast<ActuatorDevice>(motor));
	gpio = std::make_shared<ATV340_GPIO>(std::static_pointer_cast<ATV340>(shared_from_this()));
	gpio_pin->assignData(std::static_pointer_cast<GpioDevice>(gpio));
	
	//add node pins
	addNodePin(motor_pin);
	addNodePin(gpio_pin);
	addNodePin(digitalInput1_Pin);
	addNodePin(digitalInput2_Pin);
	addNodePin(digitalInput3_Pin);
	addNodePin(digitalInput4_Pin);
	addNodePin(digitalInput5_Pin);
	addNodePin(analogInput1_pin);
	addNodePin(analogInput2_pin);
	addNodePin(digitalOutput1_Pin);
	addNodePin(digitalOutput2_Pin);
	addNodePin(relaisOutput1_Pin);
	addNodePin(relaisOutput2_Pin);
	
	//configure parameter callbacks
	pdo_digitalIn->addEditCallback([this](){
		digitalInput1_Pin->setVisible(pdo_digitalIn->value);
		digitalInput2_Pin->setVisible(pdo_digitalIn->value);
		digitalInput3_Pin->setVisible(pdo_digitalIn->value);
		digitalInput4_Pin->setVisible(pdo_digitalIn->value);
		digitalInput5_Pin->setVisible(pdo_digitalIn->value);
	});
	pdo_digitalOut->addEditCallback([this](){
		digitalOutput1_Pin->setVisible(pdo_digitalOut->value);
		digitalOutput2_Pin->setVisible(pdo_digitalOut->value);
		relaisOutput1_Pin->setVisible(pdo_digitalOut->value);
		relaisOutput2_Pin->setVisible(pdo_digitalOut->value);
	});
	pdo_readAnalogIn1->addEditCallback([this](){
		analogInput1_pin->setVisible(pdo_readAnalogIn1->value);
	});
	pdo_readAnalogIn2->addEditCallback([this](){
		analogInput2_pin->setVisible(pdo_readAnalogIn2->value);
	});
	
	for(auto parameter : pdoConfigParameters.get()){
		parameter->addEditCallback([this](){
			configureProcessData();
		});
		parameter->onEdit();
	}
	
	motorParameterChoice_Param->addEditCallback([this](){
		if(motorParameterChoice_Param->value == options.NominalPower.getInt()){
			cosinusPhi_Param->setDisabled(true);
			nominalMotorPower_Param->setDisabled(false);
		}else if(motorParameterChoice_Param->value == options.CosinusPhi.getInt()){
			cosinusPhi_Param->setDisabled(false);
			nominalMotorPower_Param->setDisabled(true);
		}else{
			cosinusPhi_Param->setDisabled(false);
			nominalMotorPower_Param->setDisabled(false);
		}
	});
	motorParameterChoice_Param->onEdit();
	
	embeddedEncoderType_Param->addEditCallback([this](){
		if(embeddedEncoderType_Param->value == options.EmbeddedEncoderTypeAB.getInt()){
			embeddedEncoderVoltage_Param->setDisabled(false);
			embeddedEncoderPulsesPerRevolution_Param->setDisabled(false);
			embeddedEncoderInvertDirection_Param->setDisabled(false);
			embeddedEncoderUsage_Param->setDisabled(false);
		}else{
			embeddedEncoderVoltage_Param->setDisabled(true);
			embeddedEncoderPulsesPerRevolution_Param->setDisabled(true);
			embeddedEncoderInvertDirection_Param->setDisabled(true);
			embeddedEncoderUsage_Param->setDisabled(true);
		}
	});
	embeddedEncoderType_Param->onEdit();
	
	brakeOutputAssignement_Param->addEditCallback([this](){
		if(brakeOutputAssignement_Param->value == options.NoDigitalOutput.getInt()){
			brakeMovementType_Param->setDisabled(true);
		}else{
			brakeMovementType_Param->setDisabled(false);
		}
	});
	brakeOutputAssignement_Param->onEdit();
	
	analogInput1Type_Param->addEditCallback([this](){
		if(analogInput1Type_Param->value == options.AnalogInputTypeVoltage.getInt()){
			analogInput1MinValue_Param->setSuffix("V");
			analogInput1MaxValue_Param->setSuffix("V");
			analogInput1MinValue_Param->setDisabled(false);
			analogInput1MaxValue_Param->setDisabled(false);
		}else if(analogInput1Type_Param->value == options.AnalogInputTypeCurrent.getInt()){
			analogInput1MinValue_Param->setSuffix("mA");
			analogInput1MaxValue_Param->setSuffix("mA");
			analogInput1MinValue_Param->setDisabled(false);
			analogInput1MaxValue_Param->setDisabled(false);
		}else{
			analogInput1MinValue_Param->setSuffix("");
			analogInput1MaxValue_Param->setSuffix("");
			analogInput1MinValue_Param->setDisabled(true);
			analogInput1MaxValue_Param->setDisabled(true);
		}
	});
	analogInput1Type_Param->onEdit();
	
	analogInput2Type_Param->addEditCallback([this](){
		if(analogInput2Type_Param->value == options.AnalogInputTypeVoltage.getInt()){
			analogInput2MinValue_Param->setSuffix("V");
			analogInput2MaxValue_Param->setSuffix("V");
		}else if(analogInput2Type_Param->value == options.AnalogInputTypeCurrent.getInt()){
			analogInput2MinValue_Param->setSuffix("mA");
			analogInput2MaxValue_Param->setSuffix("mA");
		}else{
			analogInput2MinValue_Param->setSuffix("");
			analogInput2MaxValue_Param->setSuffix("");
		}
	});
	analogInput2Type_Param->onEdit();
}

void ATV340::configureProcessData(){
	
	if(EtherCatFieldbus::isRunning()) return;
	
	rxPdoAssignement.clear();
	txPdoAssignement.clear();
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	axis->processDataConfiguration.enableFrequencyMode();
	axis->processDataConfiguration.frequencyActualValue = false;
	
	if(pdo_digitalIn->value){
		txPdoAssignement.addEntry(0x2016, 0x2, 16, "Logic Inputs Physical Image", &logicInputs);
	}
	if(pdo_digitalOut->value){
		rxPdoAssignement.addEntry(0x2016, 0xD, 16, "Logic Outputs States", &logicOutputs);
	}
	if(pdo_motorPower->value){
		txPdoAssignement.addEntry(0x2002, 0xC, 16, "MotorPower", &motorPower);
	}
	if(pdo_readMotorSpeed->value){
		axis->processDataConfiguration.frequencyActualValue = true;
	}
	if(pdo_readAnalogIn1->value){
		txPdoAssignement.addEntry(0x2016, 0x2B, 16, "AI1 Input Physical Image", &analogInput1);
	}
	if(pdo_readAnalogIn2->value){
		txPdoAssignement.addEntry(0x2016, 0x2C, 16, "AI2 Input Physical Image", &analogInput2);
	}
	
	//might be smarter to thread request of this data once the fault flag is up
	txPdoAssignement.addEntry(0x2029, 0x16, 16, "LastFaultCode", &lastFaultCode);
	txPdoAssignement.addEntry(0x207B, 0x17, 16, "Safe torque Off function Status", &stoState);
	axis->configureProcessData();
}

//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool ATV340::startupConfiguration() {
	
	
	if(!axis->setOperatingMode(DS402Axis::OperatingMode::VELOCITY)) {
		return Logger::error("failed to set operating mode");
	}
	
	//———— Control Profile Settings
	
	//[fr1] configuration for reference frequency channel 1
	uint16_t ref1configuration = 169; //169 = Communication Module
	if(!writeSDO_U16(0x2036, 0xE, ref1configuration)) {
		return Logger::error("failed to set velocity reference");
	}

	//[rfc] Frequency Switching assignement
	uint16_t frequencySwitchingAssigment = 96; //96 = Fixed Reference Frequency Channel 1
	if(!writeSDO_U16(0x2036, 0xC, frequencySwitchingAssigment)) {
		return Logger::error("failed to set reference switching assignement");
	}
	
	//[chcf] Control Mode
	uint16_t controlMode = 1; //1=combined mode, ethercat has complete control of the drive
	if(!writeSDO_U16(0x2036, 0x2, controlMode)) {
		return Logger::error("failed to set control mode");
	}
	
	//———— Dynamics and Limits
	
	//[lsp] low speed (0.1Hz Increments)
	uint16_t lowSpeed = 0;
	if(!writeSDO_U16(0x2001, 0x6, lowSpeed)) return false;
	
	//[hsp] high speed (0.1Hz Increments)
	uint16_t highSpeed = 500;
	if(!writeSDO_U16(0x2001, 0x5, highSpeed)) return false;
	
	//[inr] Ramp Time Increment: 0.01 second increments
	uint16_t rampIncrement = 0; //(0=0.01, 1=0.1, 2=1.0)
	if(!writeSDO_U16(0x203C, 0x15, rampIncrement)) return false;
	
	//[acc] acceleration ramp time (0.01 second increments)
	uint16_t accelerationRampTime = accelerationRampTime_Param->value * 100;
	if(!writeSDO_U16(0x203C, 0x2, accelerationRampTime)) return false;
	
	//[dec] deceleration ramp time (0.01 second increments)
	uint16_t decelerationRampTime = decelerationRampTime_Param->value * 100;
	if(!writeSDO_U16(0x203C, 0x3, decelerationRampTime)) return false;
	
	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex())) return false;
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex())) return false;
	
	return true;
}

//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void ATV340::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
	
	bool b_estop = !isOffline() && stoState != 0;
	bool b_isReady = !isOffline() && isStateOperational() && !b_estop;// && axis->isRemoteControlActive();
	bool b_isEnabled = axis->isEnabled() && !b_waitingForEnable;// && axis->isRemoteControlActive();
	
	if(motor->isEnabled() && !axis->isEnabled()) {
		axis->disable();
	}
	
	//update servo state
	motor->b_emergencyStopActive = b_estop;
	if(isStateNone()) motor->state = MotionState::OFFLINE;
	else if(!axis->hasVoltage()) motor->state = MotionState::NOT_READY;
	else if(b_isEnabled) motor->state = MotionState::ENABLED;
	else if(b_isReady) motor->state = MotionState::READY;
	else motor->state = MotionState::NOT_READY;
	
	//these are unconfirmed...
	*digitalInput1_Signal = logicInputs & (0x1 << 0);
	*digitalInput2_Signal = logicInputs & (0x1 << 1);
	*digitalInput3_Signal = logicInputs & (0x1 << 2);
	*digitalInput4_Signal = logicInputs & (0x1 << 3);
	*digitalInput5_Signal = logicInputs & (0x1 << 4);
	
	//here there should be range remapping
	*analogInput1_value = analogInput1;
	*analogInput2_value = analogInput2;
	
	//servo->position = actualPosition;
	//servo->velocity = actualVelocity;
	//servo->load = actualLoad;
}

//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void ATV340::writeOutputs() {
	
	long long now_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	
	//handle enabling & disabling
	if(motor->b_disable){
		motor->b_disable = false;
		motor->b_enable = false;
		b_waitingForEnable = false;
		axis->disable();
	}
	else if(motor->b_enable){
		if(axis->hasFault()) axis->doFaultReset();
		else {
			motor->b_enable = false;
			b_waitingForEnable = true;
			enableRequestTime_nanoseconds = now_nanoseconds;
			axis->enable();
		}
	}
	else if(b_waitingForEnable){
		static const long long maxEnableTime_nanoseconds = 1000'000'000; //100ms
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
	
	axis->setFrequency(manualVelocityTarget_rpm);
	
	
	
	logicOutputs = 0x0;
	if(*relaisOutput1_Signal)	logicOutputs |= 0x1 << 0;
	if(*relaisOutput2_Signal)	logicOutputs |= 0x1 << 1;
	if(*digitalOutput1_Signal)	logicOutputs |= 0x1 << 8;
	if(*digitalOutput2_Signal)	logicOutputs |= 0x1 << 9;
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}

//============================= SAVING AND LOADING DEVICE DATA ============================

bool ATV340::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	if(!pdoConfigParameters.save(xml)) return false;
	if(!motorNameplateParameters.save(xml)) return false;
	if(!brakeLogicParameters.save(xml)) return false;
	if(!embeddedEncoderParameters.save(xml)) return false;
	if(!motorControlParameters.save(xml)) return false;
	if(!ioConfigParameters.save(xml)) return false;
	
	return true;
}

bool ATV340::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	if(!pdoConfigParameters.load(xml)) return false;
	if(!motorNameplateParameters.load(xml)) return false;
	if(!brakeLogicParameters.load(xml)) return false;
	if(!embeddedEncoderParameters.load(xml)) return false;
	if(!motorControlParameters.load(xml)) return false;
	if(!ioConfigParameters.load(xml)) return false;
	
	for(auto parameter : pdoConfigParameters.get()) parameter->onEdit();
	for(auto parameter : motorNameplateParameters.get()) parameter->onEdit();
	for(auto parameter : brakeLogicParameters.get()) parameter->onEdit();
	for(auto parameter : embeddedEncoderParameters.get()) parameter->onEdit();
	for(auto parameter : motorControlParameters.get()) parameter->onEdit();
	for(auto parameter : ioConfigParameters.get()) parameter->onEdit();
	
	return true;
}
























bool ATV340::configureMotor(){
	
	if(isOffline()) return;
	
	//———— Unit Scaling Selection
	
	uint16_t powerScalingCode;
	if(!readSDO_U16(0x2044, 0x3, powerScalingCode)) return false;
	double powerScaling;
	switch(powerScalingCode){
		case 20: powerScaling = 0.001; break;
		case 30: powerScaling = 0.01; break;
		case 40: powerScaling = 0.1; break;
		case 50: powerScaling = 1.0; break;
		case 60: powerScaling = 10.0; break;
		default: powerScaling = 0.0; break;
	}
	
	//Current Scaling: 0.01 Ampere increments
	uint16_t currentScalingCode = 30; //(20=0.001, 30=0.01, 40=0.1, 50=1.0, 60=10.0)
	if(!readSDO_U16(0x2044, 0x2, currentScalingCode)) return false;
	double currentScaling;
	switch(currentScalingCode){
		case 20: currentScaling = 0.001; break;
		case 30: currentScaling = 0.01; break;
		case 40: currentScaling = 0.1; break;
		case 50: currentScaling = 1.0; break;
		case 60: currentScaling = 10.0; break;
		default: currentScaling = 0.0; break;
	}
	
	//———— Motor Standard
	
	//[bfr] {Async} motor standard frequency
	uint16_t motorStandard = motorStandartFrequency_Param->value;
	if(!writeSDO_U16(0x2000, 0x10, motorStandard)) return false;
	
	
	//———— Motor Nameplate
	
	//[mpc] motor parameter choice (0=NominalPower,1=NominalCosinusPhi)
	uint16_t motorParameterChoice = motorParameterChoice_Param->value;
	if(!writeSDO_U16(0x2042, 0xF, motorParameterChoice)) return false;
	
	if(motorParameterChoice == options.NominalPower.getInt()){
		//[npr] {Async} nominal motor power
		uint16_t nominalMotorPower = nominalMotorPower_Param->value / powerScaling;
		if(!writeSDO_U16(0x2042, 0xE, nominalMotorPower)) return false;
	}else if(motorParameterChoice == options.CosinusPhi.getInt()){
		//[cos] {Async} motor 1 cosinus phi (0.01 increments)
		uint16_t motor1CosinusPhi = cosinusPhi_Param->value * 100.0;
		if(!writeSDO_U16(0x2042, 0x7, motor1CosinusPhi)) return false;
	}
	
	//[uns] {Async} nominal motor voltage (1v increments)
	uint16_t nominalMotorVoltage = nominalMotorVoltage_Param->value * 100.0;
	if(!writeSDO_U16(0x2042, 0x2, nominalMotorVoltage)) return false;
	
	//[ncr] {Async} nominal motor current
	uint16_t nominalMotorCurrent = nominalMotorCurrent_Param->value / currentScaling;
	if(!writeSDO_U16(0x2042, 0x4, nominalMotorCurrent)) return false;
	
	//[frs] {Async} nominal motor frequency (0.1Hz increments)
	uint16_t nominalMotorFrequency = nominalMotorFrequency_Param->value * 10.0;
	if(!writeSDO_S16(0x2042, 0x3, nominalMotorFrequency)) return false;
	
	//[nsp] {Async} nominal motor speed (rpm)
	uint16_t nominalMotorSpeed = nominalMotorSpeed_Param->value;
	if(!writeSDO_U16(0x2042, 0x5, nominalMotorSpeed)) return false;
	
	//[ith] Motor Thermal Current
	uint16_t motorThermalCurrent = motorThermalCurrent_Param->value / currentScaling;
	if(!writeSDO_U16(0x2042, 0x17, motorThermalCurrent)) return false;
	
	
	//———— Maximum Motor Frequency
	
	//[tfr] Motor Maximum Frequency (0.1 Hz increments)
	uint16_t motorMaximumFrequency = motorMaximumFrequency_Param->value * 10.0;
	if(!writeSDO_U16(0x2001, 0x4, motorMaximumFrequency)) return false;

	//———— Brake Logic Control
	
	//[blc] brake assignement
	uint16_t brakeAssignement = brakeOutputAssignement_Param->value;
	if(!writeSDO_U16(0x2046, 0x2, brakeAssignement)) return false;
	
	//[bst] movement type
	uint16_t brakeMovementType = brakeMovementType_Param->value;
	if(!writeSDO_U16(0x2046, 0x9, brakeMovementType)) return false;
	
	
	//———— Embedded Encoder
	
	//[eecp] embedded encoder etype (0=None, 1=AB, 2=SinCos)
	uint16_t encoderType = embeddedEncoderType_Param->value;
	if(!writeSDO_U16(0x201A, 0x47, encoderType)) return false;
	
	if(encoderType == options.EmbeddedEncoderTypeAB.getInt()){
		//[eecv] embedded encoder supply voltage (5=5V, 12=12V, 24=24V)
		uint16_t encoderSupplyVoltage = embeddedEncoderVoltage_Param->value;
		if(!writeSDO_U16(0x201A, 0x50, encoderSupplyVoltage)) return false;
		
		//[epg] pulses per encoder revolution
		uint16_t pulsesPerEncoderRevolution = embeddedEncoderPulsesPerRevolution_Param->value;
		if(!writeSDO_U16(0x201A, 0x48, pulsesPerEncoderRevolution)) return false;
		
		//[eeri] emebedded encoder revolution inversion (0=No, 1=Yes)
		uint16_t encoderinvertion = embeddedEncoderInvertDirection_Param->value ? 1 : 0;
		if(!writeSDO_U16(0x201A, 0x4F, encoderinvertion)) return false;
		
		//[eenu] embedded encoder usage (0=None, 1=SpeedMonitoring, 2=SpeedRegulation, 3=SpeedReference)
		uint16_t embeddedEncoderUsage = embeddedEncoderUsage_Param->value;
		if(!writeSDO_U16(0x201A, 0x4E, embeddedEncoderUsage)) return false;
	}
		
	//———— Motor Control Type
	
	//[ctt] Motor Control Type
	uint16_t motorControlType = motorControlType_Param->value;
	//0 = Sensorless flux vector V (no encoder feedback, multiple identical motors supported)
	//2 = Full flux Vector (encoder feedback necessary)
	if(!writeSDO_U16(0x2042, 0x8, motorControlType)) return false;
	
	return saveToEEPROM();
}

bool ATV340::startMotorTuning(){
	//———— Standstill motor tune after motor parameter assignement
	
	//[tun] autotuning (0=NoAction; 1=ApplyAutotuning; 2=EraseAutotuning)
	uint16_t autotuning = 1;
	if(!writeSDO_U16(0x2042, 0x9, autotuning)) return false;

	//[stun] tune selection (0=Default; 1=Measure; 2=Custom)
	uint16_t autotuningSelection = 1;
	if(!writeSDO_U16(0x2042, 0x12, autotuningSelection)) return false;
	
	Timing::Timer autotuneTimer;
	autotuneTimer.setExpirationSeconds(5.0);
	
	while(true){
		//[tus] autotuning status (0=NotDone; 1=Pending; 2=InProgress; 3=Fail; 4=Done)
		uint16_t autotuningStatus = 0;
		readSDO_U16(0x2042, 0xA, autotuningStatus);
		if(autotuningStatus == 3) {
			Logger::info("auto tune failed");
			return false;
		}
		else if(autotuningStatus == 4) {
			Logger::info("auto tune succeeded");
			return saveToEEPROM();
		}
		else if(autotuneTimer.isExpired()) {
			Logger::warn("auto tune timed out (took more than 5 seconds)");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	return false;
}

bool ATV340::saveToEEPROM(){
	
	//———— Method 1
	uint16_t saveConfiguration = 1; //save to congiration #0
	if(!writeSDO_U16(0x2032, 0x2, saveConfiguration)) return false;
	
	Timing::Timer saveTimer;
	saveTimer.setExpirationSeconds(5.0);
	
	while(true){
		uint16_t savestate;
		if(readSDO_U16(0x2032, 0x2, savestate) && savestate == 0x0) {
			Logger::info("Motor Parameters saved on drive");
			return true;
		}
		else if(saveTimer.isExpired()) {
			Logger::warn("configuration save timed out (took more than 5 seconds)");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	/*
	//———— Method 2
	uint16_t extendedCommandWord = 0x2;
	return writeSDO_U16(0x2037, 0x5, extendedCommandWord);
	*/
}

bool ATV340::resetFactorySettings(){
	uint16_t extendedCommandWord = 0x1;
	return writeSDO_U16(0x2037, 0x5, extendedCommandWord);
}
