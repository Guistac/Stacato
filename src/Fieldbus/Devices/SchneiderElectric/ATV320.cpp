#include "ATV320.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/Utilities/SDOTask.h"

//Motor standart changes the ramp times
//1 sec ramp is 1 sec to go to the nominal frequency (50 or 60Hz)

int ATV320::getPolePairCount(){
	float nomSpeed = nominalMotorSpeedParameter->getReal();
	switch(standartMotorFrequencyParameter->value){
		default:
		case HZ_50:
			if(nomSpeed > 3000.0) return 1.0;
			else return std::round(3000.0 / nomSpeed);
		case HZ_60:
			if(nomSpeed > 3600.0) return 1.0;
			else return std::round(3600.0 / nomSpeed);
	}
}

float ATV320::HzToRps(float Hz){
	return Hz / getPolePairCount();
}

float ATV320::RpsToHz(float rps){
	return rps * getPolePairCount();
}

void ATV320::onConnection() {
	
}

void ATV320::onDisconnection() {
	actuator->state = DeviceState::OFFLINE;
	actuator->actuatorProcessData.b_isEmergencyStopActive = false;
	gpio->state = DeviceState::OFFLINE;
}

void ATV320::updateActuatorInterface(){
	actuator->actuatorConfig.velocityLimit = HzToRps(highControlFrequencyParameter->getReal());
	double accRefVel;
	if(standartMotorFrequencyParameter->value == HZ_50) accRefVel = HzToRps(50.0);
	else if(standartMotorFrequencyParameter->value == HZ_60) accRefVel = HzToRps(60.0);
	actuator->actuatorConfig.accelerationLimit = accRefVel / accelerationRampTime->value;
	actuator->actuatorConfig.decelerationLimit = accRefVel / decelerationRampTime->value;
}

void ATV320::initialize() {
	
	auto thisATV320 = std::static_pointer_cast<ATV320>(shared_from_this());
	
	axis = DS402Axis::make(thisATV320);
	axis->warnPowerStateChanged(false);
	actuator = std::make_shared<ATV_Motor>(thisATV320);
	actuator->feedbackConfig.b_supportsVelocityFeedback = true;
	actuator->actuatorConfig.b_supportsEffortFeedback = true;
	actuator->actuatorConfig.b_supportsVelocityControl = true;
	gpio = std::make_shared<ATV_GPIO>(thisATV320);
	
	configurationUploadTask.setAtv320(thisATV320);
	standstillTuningTask.setAtv320(thisATV320);
	
	actuatorPin->assignData(std::static_pointer_cast<ActuatorInterface>(actuator));
	gpioPin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	addNodePin(actuatorPin);
	addNodePin(gpioPin);
	
	addNodePin(stoStatusPin);
	addNodePin(digitalInput1Pin);
	addNodePin(digitalInput2Pin);
	addNodePin(digitalInput3Pin);
	addNodePin(digitalInput4Pin);
	addNodePin(digitalInput5Pin);
	addNodePin(digitalInput6Pin);
	addNodePin(actualVelocityPin);
	addNodePin(actualLoadPin);
	
	
	//————— General Settings —————
	
	accelerationRampTime = NumberParameter<double>::make(3.0, "Acceleration Ramp", "AccelerationRamp");
	accelerationRampTime->setUnit(Units::Time::Second);
	accelerationRampTime->addEditCallback([this](){ updateActuatorInterface(); });
	
	decelerationRampTime = NumberParameter<double>::make(1.0, "Deceleration Ramp", "DecelerationRamp");
	decelerationRampTime->setUnit(Units::Time::Second);
	decelerationRampTime->addEditCallback([this](){ updateActuatorInterface(); });
	
	invertDirection = BooleanParameter::make(false, "Invert Motion Direction", "InvertMotionDirection");
	
	lowControlFrequencyParameter = NumberParameter<double>::make(0.0, "Minimum Control Frequency [lsp]", "MinControlFrequency");
	lowControlFrequencyParameter->setUnit(Units::Frequency::Hertz);
	
	highControlFrequencyParameter = NumberParameter<double>::make(50.0, "Maximum Control Frequency [hsp]", "MaxControlFrequency");
	highControlFrequencyParameter->setUnit(Units::Frequency::Hertz);
	
	maxFrequencyParameter = NumberParameter<double>::make(50.0, "Maximum Frequency [tfr]", "MaxFrequency");
	maxFrequencyParameter->setUnit(Units::Frequency::Hertz);
	
	switchingFrequencyParameter = NumberParameter<double>::make(4.0, "Switching Frequency [sfr]", "SwitchingFrequency");
	switchingFrequencyParameter->setUnit(Units::Frequency::Kilohertz);
	switchingFrequencyParameter->addEditCallback([this](){
		if(switchingFrequencyParameter->value > 16.0) switchingFrequencyParameter->overwrite(16.0);
		else if(switchingFrequencyParameter->value < 2.0) switchingFrequencyParameter->overwrite(2.0);
	});
	
	boostAmountParameter = NumberParameter<double>::make(0.0, "Boost Amount [boo]", "BoostAmount", "%.1f", Units::Fraction::Percent);
	boostThresholdParameter = NumberParameter<double>::make(0.0, "Boost Threshold [fab]", "BoostThreshold", "%.1f", Units::Frequency::Hertz);

	//————— Motor Parameters —————
	
	standartMotorFrequencyParameter = OptionParameter::make2(option_frequency50Hz, options_standartMotorFrequency, "Standart Motor Frequency [bfr]", "StandartMotorFrequency");
	
	motorControlTypeParameter = OptionParameter::make2(option_motorControlType_standardMotorLaw, options_motorControlType, "Motor Control Type [ctt]", "MotorControlType");
	
	ratedMotorPowerParameter = NumberParameter<double>::make(0.0, "Rated Motor Power [npr]", "RatedMotorPower");
	ratedMotorPowerParameter->setUnit(Units::Power::KiloWatt);
	
	nominalMotorVoltageParameter = NumberParameter<double>::make(0.0, "Rated Motor Voltage [uns]", "RatedMotorVoltage");
	nominalMotorVoltageParameter->setUnit(Units::Voltage::Volt);
	
	nominalMotorCurrentParameter = NumberParameter<double>::make(0.0, "Rated Motor Current [ncr]", "RatedMotorCurrent");
	nominalMotorCurrentParameter->setUnit(Units::Current::Ampere);
	
	nominalMotorSpeedParameter = NumberParameter<double>::make(1400.0, "Motor Rated Speed [nps]", "MotorRatedSpeed");
	nominalMotorSpeedParameter->setUnit(Units::AngularDistance::Revolution);
	nominalMotorSpeedParameter->setSuffix("/min");
	nominalMotorSpeedParameter->addEditCallback([this](){ updateActuatorInterface(); });
	
	//————— Limit signal configuration —————

	forwardStopLimitAssignementParameter = OptionParameter::make2(option_logicInput_none, options_logicInput, "Forward limit input", "ForwardStopSignal");
	
	reverseStopLimitAssignementParameter = OptionParameter::make2(option_logicInput_none, options_logicInput, "Reverse limit input", "ReverseStopSignal");
	
	stopLimitConfigurationParameter = OptionParameter::make2(option_activeHigh, options_activeLowHigh, "Stop limit configuration", "StopLimitConfiguration");
	
	
	//————— Logic input configuration —————
	
	auto clampInputOnDelay = [](NumberParam<int> onDelay){
		if(onDelay->value < 0) onDelay->overwrite(0);
		else if(onDelay->value > 200) onDelay->overwrite(200);
	};
	logicInput1OnDelayParameter = NumberParameter<int>::make(0, "LI1 On Delay", "LI1OnDelay");
	logicInput1OnDelayParameter->setUnit(Units::Time::Millisecond);
	logicInput1OnDelayParameter->addEditCallback([&,this](){ clampInputOnDelay(logicInput1OnDelayParameter); });
	logicInput2OnDelayParameter = NumberParameter<int>::make(0, "LI2 On Delay", "LI2OnDelay");
	logicInput2OnDelayParameter->setUnit(Units::Time::Millisecond);
	logicInput2OnDelayParameter->addEditCallback([&,this](){ clampInputOnDelay(logicInput2OnDelayParameter); });
	logicInput3OnDelayParameter = NumberParameter<int>::make(0, "LI3 On Delay", "LI3OnDelay");
	logicInput3OnDelayParameter->setUnit(Units::Time::Millisecond);
	logicInput3OnDelayParameter->addEditCallback([&,this](){ clampInputOnDelay(logicInput3OnDelayParameter); });
	logicInput4OnDelayParameter = NumberParameter<int>::make(0, "LI4 On Delay", "LI4OnDelay");
	logicInput4OnDelayParameter->setUnit(Units::Time::Millisecond);
	logicInput4OnDelayParameter->addEditCallback([&,this](){ clampInputOnDelay(logicInput4OnDelayParameter); });
	logicInput5OnDelayParameter = NumberParameter<int>::make(0, "LI5 On Delay", "LI5OnDelay");
	logicInput5OnDelayParameter->setUnit(Units::Time::Millisecond);
	logicInput5OnDelayParameter->addEditCallback([&,this](){ clampInputOnDelay(logicInput5OnDelayParameter); });
	logicInput6OnDelayParameter = NumberParameter<int>::make(0, "LI6 On Delay", "LI6OnDelay");
	logicInput6OnDelayParameter->setUnit(Units::Time::Millisecond);
	logicInput6OnDelayParameter->addEditCallback([&,this](){ clampInputOnDelay(logicInput6OnDelayParameter); });
	
	invertLogicInput1Parameter = BooleanParameter::make(false, "Invert Logic Input 1", "InvertLogicInput1");
	invertLogicInput2Parameter = BooleanParameter::make(false, "Invert Logic Input 2", "InvertLogicInput2");
	invertLogicInput3Parameter = BooleanParameter::make(false, "Invert Logic Input 3", "InvertLogicInput3");
	invertLogicInput4Parameter = BooleanParameter::make(false, "Invert Logic Input 4", "InvertLogicInput4");
	invertLogicInput5Parameter = BooleanParameter::make(false, "Invert Logic Input 5", "InvertLogicInput5");
	invertLogicInput6Parameter = BooleanParameter::make(false, "Invert Logic Input 6", "InvertLogicInput6");
	
	updateActuatorInterface();
	
	//———— configure pdo data ———
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	
	axis->processDataConfiguration.enableFrequencyMode();
	axis->processDataConfiguration.frequencyActualValue = false;
	axis->configureProcessData();
	
	txPdoAssignement.addEntry(0x6044, 0x0, 16, "VelocityActual", &velocityActual_rpm);
	txPdoAssignement.addEntry(0x2002, 0xC, 16, "MotorPower", &motorPower);
	txPdoAssignement.addEntry(0x2016, 0x3, 16, "LogicInputs", &logicInputs);
	txPdoAssignement.addEntry(0x207B, 0x17, 16, "STOstate", &stoState);
	txPdoAssignement.addEntry(0x2029, 0x16, 16, "LastFaultCode", &lastFaultCode);

}

//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool ATV320::startupConfiguration() {
	
	if(!axis->setOperatingMode(DS402Axis::OperatingMode::VELOCITY)) {
		return Logger::error("failed to set operating mode");
	}
	
	//[chcf] set control profile to "Not separate" (= 1) for pure CiA.402 Velocity Mode
	if(!writeSDO_U16(0x2036, 0x2, 1)) return false;
	
	//[cls] set disable limit switch input to bit 11 of control word
	b_canDisableLimitSwitches = writeSDO_U16(0x205F, 0x8, 219);
	b_disableLimitSwitches = false;
	
	//—————————————— ASSIGN PDOS —————————————————
	
	if(!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) return false;
	if(!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) return false;
	
	//——— cache parameter values
	
	b_reverseDirection = invertDirection->value;
	
	return true;
}

//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void ATV320::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
		
	if(b_waitingForEnable){
		static const long long maxEnableTime_nanoseconds = 500'000'000; //500ms
		//for some reason switched on is sometimes a valid 'enabled' power state with ATV320
		if(axis->getActualPowerState() == DS402Axis::PowerState::OPERATION_ENABLED || axis->getActualPowerState() == DS402Axis::PowerState::SWITCHED_ON) {
			Logger::debug("[{}] Drive Enabled", getName());
			b_waitingForEnable = false;
		}
		else if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - enableRequestTime_nanoseconds > maxEnableTime_nanoseconds){
			Logger::warn("[{}] Enable request timed out", getName());
			b_waitingForEnable = false;
			axis->disable();
		}
	}
	
	//update general state
	b_motorVoltagePresent = axis->hasVoltage();								//b4
	b_remoteControlEnabled = axis->isRemoteControlActive();					//b9
	b_referenceReached = axis->getOperatingModeSpecificStatusWordBit_10();	//b10
	b_referenceOutsideLimits = axis->isInternalLimitReached();				//b11
	b_stopKeyPressed = axis->getOperatingModeSpecificStatusWordBit_12();	//b12
	b_stoActive = stoState != 0;
	b_hasFault = axis->hasFault();
	
	auto actualPowerState = axis->getActualPowerState();
	if(isOffline())																actuator->state = DeviceState::OFFLINE;
	else if(!axis->isRemoteControlActive()) 									actuator->state = DeviceState::NOT_READY;
	else if(b_stoActive)														actuator->state = DeviceState::NOT_READY;
	else if(!axis->hasVoltage())												actuator->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402Axis::PowerState::NOT_READY_TO_SWITCH_ON)	actuator->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402Axis::PowerState::SWITCH_ON_DISABLED)		actuator->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402Axis::PowerState::SWITCHED_ON)				actuator->state = DeviceState::ENABLED;
	else if(actualPowerState == DS402Axis::PowerState::OPERATION_ENABLED)		actuator->state = DeviceState::ENABLED;
	else 																		actuator->state = DeviceState::READY;
	
	if(isOffline())						gpio->state = DeviceState::OFFLINE;
	else if(b_stoActive)				gpio->state = DeviceState::NOT_READY;
	else if(isStateSafeOperational()) 	gpio->state = DeviceState::READY;
	else if(isStateOperational())		gpio->state = DeviceState::ENABLED;
	else 								gpio->state = DeviceState::NOT_READY;
	
	
	if(!actuator->isEmergencyStopActive() && b_stoActive) Logger::warn("[{}] STO triggered", getName());
	else if(actuator->isEmergencyStopActive() && !b_stoActive) Logger::info("[{}] STO cleared", getName());
	
	actuator->actuatorProcessData.b_isEmergencyStopActive = b_stoActive;
	actuator->actuatorProcessData.effortActual = (double)motorPower / 100.0;
	actuator->feedbackProcessData.velocityActual = velocityActual_rpm / 60.0;
	actuator->actuatorConfig.b_supportsHoldingBrakeControl = false;
	
	//update output pin data
	*actualVelocity = actuator->getVelocity();
	*actualLoad = actuator->getEffort();
	*stoStatusValue = b_stoActive;
	*digitalInput1Signal = invertLogicInput1Parameter->value ? logicInputs & 0x1 : !(logicInputs & 0x1);
	*digitalInput2Signal = invertLogicInput2Parameter->value ? logicInputs & 0x2 : !(logicInputs & 0x2);
	*digitalInput3Signal = invertLogicInput3Parameter->value ? logicInputs & 0x4 : !(logicInputs & 0x4);
	*digitalInput4Signal = invertLogicInput4Parameter->value ? logicInputs & 0x8 : !(logicInputs & 0x8);
	*digitalInput5Signal = invertLogicInput5Parameter->value ? logicInputs & 0x10 : !(logicInputs & 0x10);
	*digitalInput6Signal = invertLogicInput6Parameter->value ? logicInputs & 0x20 : !(logicInputs & 0x20);
}

//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void ATV320::writeOutputs() {
	
	//handle enabling & disabling
	if(actuator->actuatorProcessData.b_disable){
		actuator->actuatorProcessData.b_disable = false;
		b_waitingForEnable = false;
		axis->disable();
	}
	else if(actuator->actuatorProcessData.b_enable){
		if(axis->hasFault()) axis->doFaultReset();
		else {
			actuator->actuatorProcessData.b_enable = false;
			b_waitingForEnable = true;
			enableRequestTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
			axis->enable();
		}
	}
	
	//fault handling
	if(axis->hasFault()){
		//auto clear fault during enable
		if(b_waitingForEnable) axis->doFaultReset();
		else axis->disable();
	}
	
	if(b_reverseDirection) axis->setFrequency(-actuator->actuatorProcessData.velocityTarget * 60.0);
	else axis->setFrequency(actuator->actuatorProcessData.velocityTarget * 60.0);
	
	axis->setManufacturerSpecificControlWordBit_11(b_disableLimitSwitches);
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}

//============================= SAVING AND LOADING DEVICE DATA ============================

bool ATV320::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	//————— Motor Parameters —————
	XMLElement* motorParametersXML = xml->InsertNewChildElement("MotorParameters");
	standartMotorFrequencyParameter->save(motorParametersXML);
	motorControlTypeParameter->save(motorParametersXML);
	ratedMotorPowerParameter->save(motorParametersXML);
	nominalMotorVoltageParameter->save(motorParametersXML);
	nominalMotorCurrentParameter->save(motorParametersXML);
	nominalMotorSpeedParameter->save(motorParametersXML);
	
	//————— Motion Control Settings —————
	XMLElement* motionControlSettingsXML = xml->InsertNewChildElement("MotionControlSettings");
	accelerationRampTime->save(motionControlSettingsXML);
	decelerationRampTime->save(motionControlSettingsXML);
	invertDirection->save(motionControlSettingsXML);
	lowControlFrequencyParameter->save(motionControlSettingsXML);
	highControlFrequencyParameter->save(motionControlSettingsXML);
	maxFrequencyParameter->save(motionControlSettingsXML);
	switchingFrequencyParameter->save(motionControlSettingsXML);
	boostTypeParameter->save(motionControlSettingsXML);
	boostAmountParameter->save(motionControlSettingsXML);
	boostThresholdParameter->save(motionControlSettingsXML);

	//————— Frequency References —————
	XMLElement* frequencyReferenceXML = xml->InsertNewChildElement("FrequencyReference");
	frequencyReference1_Parameter->save(frequencyReferenceXML);
	frequencyReference2_Parameter->save(frequencyReferenceXML);
	referenceSwitchingPin_Parameter->save(frequencyReferenceXML);
	
	//————— IO configuration —————
	XMLElement* ioConfigurationXML = xml->InsertNewChildElement("IOConfiguration");
	twoOrThreeWireControl_Parameter->save(ioConfigurationXML);
	forwardStopLimitAssignementParameter->save(ioConfigurationXML);
	reverseStopLimitAssignementParameter->save(ioConfigurationXML);
	stopLimitConfigurationParameter->save(ioConfigurationXML);
	faultResetPin_Parameter->save(ioConfigurationXML);
	externalFaultPin_Parameter->save(ioConfigurationXML);
	presetSlowdownSpeedPin_Parameter->save(ioConfigurationXML);

	
	//————— Logic input configuration —————
	XMLElement* logicInputXML = xml->InsertNewChildElement("LogicInputSettings");
	logicInput1OnDelayParameter->save(logicInputXML);
	logicInput2OnDelayParameter->save(logicInputXML);
	logicInput3OnDelayParameter->save(logicInputXML);
	logicInput4OnDelayParameter->save(logicInputXML);
	logicInput5OnDelayParameter->save(logicInputXML);
	logicInput6OnDelayParameter->save(logicInputXML);
	invertLogicInput1Parameter->save(logicInputXML);
	invertLogicInput2Parameter->save(logicInputXML);
	invertLogicInput3Parameter->save(logicInputXML);
	invertLogicInput4Parameter->save(logicInputXML);
	invertLogicInput5Parameter->save(logicInputXML);
	invertLogicInput6Parameter->save(logicInputXML);
	
	
	return true;
}

bool ATV320::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	//————— Motor Parameters —————
	if(XMLElement* motorParametersXML = xml->FirstChildElement("MotorParameters")){
		standartMotorFrequencyParameter->load(motorParametersXML);
		motorControlTypeParameter->load(motorParametersXML);
		ratedMotorPowerParameter->load(motorParametersXML);
		nominalMotorVoltageParameter->load(motorParametersXML);
		nominalMotorCurrentParameter->load(motorParametersXML);
		nominalMotorSpeedParameter->load(motorParametersXML);
	}
	
	//————— Motion Control Settings —————
	if(XMLElement* motionControlSettingsXML = xml->FirstChildElement("MotionControlSettings")){
		accelerationRampTime->load(motionControlSettingsXML);
		decelerationRampTime->load(motionControlSettingsXML);
		invertDirection->load(motionControlSettingsXML);
		lowControlFrequencyParameter->load(motionControlSettingsXML);
		highControlFrequencyParameter->load(motionControlSettingsXML);
		maxFrequencyParameter->load(motionControlSettingsXML);
		switchingFrequencyParameter->load(motionControlSettingsXML);
		boostTypeParameter->load(motionControlSettingsXML);
		boostAmountParameter->load(motionControlSettingsXML);
		boostThresholdParameter->load(motionControlSettingsXML);
	}
	
	//————— Frequency Reference —————
	if(XMLElement* frequencyReferenceXML = xml->FirstChildElement("FrequencyReference")){
		frequencyReference1_Parameter->load(frequencyReferenceXML);
		frequencyReference2_Parameter->load(frequencyReferenceXML);
		referenceSwitchingPin_Parameter->load(frequencyReferenceXML);
	}
	
	//————— IO configuration —————
	if(XMLElement* ioConfigurationXML = xml->FirstChildElement("IOConfiguration")){
		twoOrThreeWireControl_Parameter->load(ioConfigurationXML);
		forwardStopLimitAssignementParameter->load(ioConfigurationXML);
		reverseStopLimitAssignementParameter->load(ioConfigurationXML);
		stopLimitConfigurationParameter->load(ioConfigurationXML);
		faultResetPin_Parameter->load(ioConfigurationXML);
		externalFaultPin_Parameter->load(ioConfigurationXML);
		presetSlowdownSpeedPin_Parameter->load(ioConfigurationXML);
	}
	
	//————— Logic input configuration —————
	if(XMLElement* logicInputXML = xml->FirstChildElement("LogicInputSettings")){
		logicInput1OnDelayParameter->load(logicInputXML);
		logicInput2OnDelayParameter->load(logicInputXML);
		logicInput3OnDelayParameter->load(logicInputXML);
		logicInput4OnDelayParameter->load(logicInputXML);
		logicInput5OnDelayParameter->load(logicInputXML);
		logicInput6OnDelayParameter->load(logicInputXML);
		invertLogicInput1Parameter->load(logicInputXML);
		invertLogicInput2Parameter->load(logicInputXML);
		invertLogicInput3Parameter->load(logicInputXML);
		invertLogicInput4Parameter->load(logicInputXML);
		invertLogicInput5Parameter->load(logicInputXML);
		invertLogicInput6Parameter->load(logicInputXML);
	}
	
	updateActuatorInterface();
	return true;
}








std::string ATV320::getShortStatusString(){
	if(!isConnected()) return "Device Offline";
	else if(!b_motorVoltagePresent) return "No Motor Voltage";
	else if(!b_remoteControlEnabled) return "Remote Disabled";
	else if(b_stoActive) return "STO Active";
	else if(b_hasFault) return "Fault Code " + std::to_string(lastFaultCode);
	else if(axis->getActualPowerState() == DS402Axis::PowerState::NOT_READY_TO_SWITCH_ON) return "Restart Needed";
	else return axis->getPowerStateString(axis->getActualPowerState());
}


std::string ATV320::getStatusString(){
	if(!isConnected()) return "Device is Offline";
	else if(!b_motorVoltagePresent) return "No Motor Voltage : Check Power Connections";
	else if(!b_remoteControlEnabled) return "Network Control Disabled : Disable Local Controls";
	else if(b_stoActive) return "STO Active : Release Emergency Stop";
	else if(b_hasFault) return getFaultString();
	else if(axis->getActualPowerState() == DS402Axis::PowerState::NOT_READY_TO_SWITCH_ON) return "Drive needs a restart...";
	else return axis->getPowerStateString(axis->getActualPowerState());
}

glm::vec4 ATV320::getStatusColor(){
	if(!isConnected()) return Colors::blue;
	else if(!b_motorVoltagePresent) return Colors::red;
	else if(!b_remoteControlEnabled) return Colors::red;
	else if(b_stoActive) return Colors::red;
	else if(b_hasFault) return Colors::orange;
	else if(!actuator->isReady()) return Colors::red;
	else if(actuator->isEnabled()) return Colors::green;
	else return Colors::yellow;
}

std::string ATV320::getFaultString(){
	switch(lastFaultCode){
		case 0: return 		"0 : No fault";
		case 2: return 		"2 : EEprom control fault";
		case 3: return 		"3 : Incorrect configuration";
		case 4: return 		"4 : Invalid config parameters";
		case 5: return 		"5 : Modbus coms fault";
		case 6: return 		"6 : Com Internal link fault";
		case 7: return 		"7 : Network fault";
		case 8: return 		"8 : External fault logic input";
		case 9: return 		"9 : Overcurrent fault";
		case 10: return 	"10 : Precharge";
		case 11: return 	"11 : Speed feedback loss";
		case 12: return 	"12 : Output speed <> ref";
		case 16: return 	"16 : Drive overheating fault";
		case 17: return 	"17 : Motor overload fault";
		case 18: return 	"18 : DC bus overvoltage fault";
		case 19: return 	"19 : Supply overvoltage fault";
		case 20: return 	"20 : 1 motor phase loss fault";
		case 21: return 	"21 : Supply phase loss fault";
		case 22: return 	"22 : Supply undervolt fault";
		case 23: return 	"23 : Motor short circuit";
		case 24: return 	"24 : Motor overspeed fault";
		case 25: return 	"25 : Auto-tuning fault";
		case 26: return 	"26 : Rating error";
		case 27: return 	"27 : Incompatible control card";
		case 28: return 	"28 : Internal coms link fault";
		case 29: return 	"29 : Internal manu zone fault";
		case 30: return 	"30 : EEprom power fault";
		case 32: return 	"32 : Ground short circuit";
		case 33: return 	"33 : 3 motor phase loss fault";
		case 34: return 	"34 : Comms fault CANopen";
		case 35: return 	"35 : Brake control fault";
		case 38: return 	"38 : External fault comms";
		case 41: return 	"41 : Brake feedback fault";
		case 42: return 	"42 : PC coms fault";
		case 44: return 	"44 : Torque/current limit fault";
		case 45: return 	"45 : HMI coms fault";
		case 49: return 	"49 : LI6=PTC failed";
		case 50: return 	"50 : LI6=PTC overheat fault";
		case 51: return 	"51 : Internal I measure fault";
		case 52: return 	"52 : Internal i/p volt circuit flt";
		case 53: return 	"53 : Internal temperature fault";
		case 54: return 	"54 : IGBT overheat fault";
		case 55: return 	"55 : IGBT short circuit fault";
		case 56: return 	"56 : motor short circuit";
		case 58: return 	"58 : Output cont close fault";
		case 59: return 	"59 : Output cont open fault";
		case 64: return 	"64 : input contactor";
		case 67: return 	"67 : IGBT desaturation";
		case 68: return 	"68 : Internal option fault";
		case 69: return 	"69 : internal- CPU";
		case 71: return 	"71 : AI3 4-20mA loss";
		case 73: return 	"73 : Cards pairing";
		case 76: return 	"76 : Dynamic load fault";
		case 77: return 	"77 : Interrupted config.";
		case 99: return 	"99 : Channel switching fault";
		case 100: return 	"100 : Process Underlaod Fault";
		case 101: return 	"101 : Process Overload Fault";
		case 105: return 	"105 : Angle error";
		case 107: return 	"107 : Safety fault";
		case 108: return 	"108 : FB fault";
		case 109: return 	"109 : FB stop fault";
		default: return 	std::to_string(lastFaultCode) + "Unknown Fault";
	}
}




bool ATV320::ConfigurationUploadTask::canStart(){
	if(atv320->isOffline()) return false;
	else if(atv320->isStateOperational()) return false;
	return true;
}

void ATV320::ConfigurationUploadTask::onExecution(){
	
	//[nsp] nominal motor speed (in rpm)
	//[bfr] standart motor frequency, 50Hz or 60Hz, setting this modifies [hsp] [frs] [tfr] and should be set first
	//[frs] rated motor frequency, identical to [bfr] but can go from 10 to 800Hz
	//[tfr] max frequency overspeed error threshold (should be 110% of [hsp]) 60Hz if [bfr] is 50Hz and 72Hz if [bfr] is 60Hz, 10 to 599Hz, should not exceed 10x [frs]
	//[hsp] max control frequency in Hz
	//[lsp] min control frequency in Hz
	uint16_t nominalMotorFrequency;
	uint16_t maxMotorFrequency;
	switch(atv320->standartMotorFrequencyParameter->value){
		case StandartMotorFrequency::HZ_50:
			nominalMotorFrequency = 50;
			maxMotorFrequency = 60;
			break;
		case StandartMotorFrequency::HZ_60:
			nominalMotorFrequency = 60;
			maxMotorFrequency = 72;
			break;
		default:
			nominalMotorFrequency = 0;
			maxMotorFrequency = 0;
			break;
	}
	
	std::vector<std::shared_ptr<SDOTask>> configUploadList = {
		
		//—————— Motor Configuration ———————
		//[bfr] Standart Motor Frequency (0 = 50Hz / 1 = 60Hz)
		SDOTask::prepareUpload(0x2000, 0x10, uint16_t(atv320->standartMotorFrequencyParameter->value), "Standart motor frequency [bfr]"),
		//[ctt] Motor Control Type
		SDOTask::prepareUpload(0x2042, 0x8, uint16_t(atv320->motorControlTypeParameter->value), "Motor control type [ctt]"),
		//[mpc] set motor parameter choice to motor power (= 0)
		SDOTask::prepareUpload(0x2042, 0xF, uint16_t(0), "Motor parameter choice [mpc]"),
		//[uns] rated motor voltage in 1V increments
		SDOTask::prepareUpload(0x2042, 0x2, uint16_t(atv320->nominalMotorVoltageParameter->value), "Nominal motor voltage [uns]"),
		//[npr] rated motor power in 0.1Kw increments
		SDOTask::prepareUpload(0x2042, 0xE, uint16_t(atv320->ratedMotorPowerParameter->value * 100), "Rated motor power [uns]"),
		//[ncr] nominal motor current in 0.1A increments
		SDOTask::prepareUpload(0x2042, 0x4, uint16_t(atv320->nominalMotorCurrentParameter->value * 10), "Nominal Motor current [ncr]"),
		//[nsp] nominal motor speed (in rpm)
		SDOTask::prepareUpload(0x2042, 0x5, uint16_t(atv320->nominalMotorSpeedParameter->value), "Nominal motor speed [nsp]"),
		//[frs] nominal motor frequency (in .1Hz increments) (should be identical to [bfr])
		SDOTask::prepareUpload(0x2042, 0x3, uint16_t(nominalMotorFrequency * 10), "Nominal motor frequency [frs]"),
		
		//————————— Motion Control Settings —————————————
		//[lsp] set minimum control speed in hertz to 0Hz (in .1Hz increments)
		SDOTask::prepareUpload(0x2001, 0x6, uint16_t(atv320->lowControlFrequencyParameter->value * 10), "Low speed [lsp]"),
		//[tfr] max output frequency overspeed error threshold (in 0.1Hz increments), needs to be set before [hsp]
		SDOTask::prepareUpload(0x2001, 0x4, uint16_t(atv320->maxFrequencyParameter->value * 10), "Max output frequency"),
		//[hsp] set maximum control speed to in 0.1Hz increments, needs to be set after [tfr]
		SDOTask::prepareUpload(0x2001, 0x5, uint16_t(atv320->highControlFrequencyParameter->value * 10), "High Speed [hsp]"),
		//[sfr] switching frequency (in 0.1KHz increments)
		SDOTask::prepareUpload(0x2001, 0x3, uint16_t(atv320->switchingFrequencyParameter->value * 10), "Switching Frequency [sfr]"),
		//[boa] Boost Type
		SDOTask::prepareUpload(0x206D, 0xB, uint16_t(atv320->boostTypeParameter->value), "Boost Activation [boa]"),
		//[boo] boost amount (in 1% increments)
		SDOTask::prepareUpload(0x206D, 0xD, int16_t(atv320->boostAmountParameter->value), "Boost [boo]"),
		//[fab] boost activation threshold (in 0.1Hz increments)
		SDOTask::prepareUpload(0x206D, 0xC, uint16_t(atv320->boostThresholdParameter->value * 10), "Action Boost [fab]"),

		//———————— Frequency Reference —————————
		//[fr1] Frequency Reference 1
		SDOTask::prepareUpload(0x2036, 0xE, uint16_t(atv320->frequencyReference1_Parameter->value), "Frequency Reference 1 [fr1]"),
		//[fr2] Frequency Reference 2
		SDOTask::prepareUpload(0x2036, 0xF, uint16_t(atv320->frequencyReference2_Parameter->value), "Frequency Reference 2 [fr2]"),
		//[rfc] Frequency Reference Switch
		SDOTask::prepareUpload(0x2036, 0xC, uint16_t(atv320->referenceSwitchingPin_Parameter->value), "Frequency Reference Selector [rfc]"),
		
		//———————— Ramp Settings ———————————
		//[inr] set ramp unit increment to hundreds of seconds (.01s = 0  / .1s = 1  /  1.s = 2)
		SDOTask::prepareUpload(0x203C, 0x15, uint16_t(0), "Ramp unit increment"),
		//[acc] set acceleration time
		SDOTask::prepareUpload(0x203C, 0x2, uint16_t(atv320->accelerationRampTime->value * 100.0), "Acceleration ramp time [acc]"),
		//[dec] set deceleration time
		SDOTask::prepareUpload(0x203C, 0x3, uint16_t(atv320->decelerationRampTime->value * 100.0), "Deceleration ramp time [dec]"),
		
		//—————————————— IO Configuration ———————————————
		
		//[tcc] two or three wire control (0 = 2-Wire / 1 = 3-Wire)
		SDOTask::prepareUpload(0x2051, 0x2, uint16_t(atv320->twoOrThreeWireControl_Parameter->value), "Two or Three Wire Control [tcc]"),
		//[tct] Type of 2-Wire Control
		SDOTask::prepareUpload(0x2051, 0x3, uint16_t(1), "Type of Two Wire Control [tct]"),
		//[saf] forward limit stop
		SDOTask::prepareUpload(0x205F, 0x2, uint16_t(atv320->forwardStopLimitAssignementParameter->value), "Forward stop limit assignement [saf]"),
		//[sar] reverse limit stop
		SDOTask::prepareUpload(0x205F, 0x3, uint16_t(atv320->reverseStopLimitAssignementParameter->value), "Reverse stop limit assignement [sar]"),
		//[sal] //stop signal active high or low
		SDOTask::prepareUpload(0x205F, 0x9, uint16_t(atv320->stopLimitConfigurationParameter->value), "Stop limit configuration [sal]"),
		//[rsf] fault reset input assignement
		SDOTask::prepareUpload(0x2029, 0x19, uint16_t(atv320->faultResetPin_Parameter->value), "Fault Reset Input Assignement [rsf]"),
		//[etf] external fault input assignemnt
		SDOTask::prepareUpload(0x2029, 0x20, uint16_t(atv320->externalFaultPin_Parameter->value), "External Fault Input Assignement [etf]"),
		//[ps2] Preset 2 frequency input assignement
		SDOTask::prepareUpload(0x2054, 0x2, uint16_t(atv320->presetSlowdownSpeedPin_Parameter->value), "Preset 2 Frequency Input Assignement [ps2]"),
		//[sp2] Preset Speed 2
		SDOTask::prepareUpload(0x2054, 0xB, uint16_t(atv320->presetSlowdownSpeed_Parameter->value), "Preset Speed 2 [sp2]"),
	
		//—————————— MemoStop ———————————

		//[mstp] disable the memostop function
		//else the drive will remember limit switches
		//this caused many weird issues where the motor just did not want to move after limit switch configuration changed
		//or even after the signals were disconnected and reconnected
		SDOTask::prepareUpload(0x205F, 0x18, uint16_t(0), "Disable Memo Stop [mstp]"),
		
		//—————————— Digital IO Configuration ———————————
		//[l1d] -> [l6d] logic input on delay parameters
		SDOTask::prepareUpload(0x200A, 0x2, uint16_t(atv320->logicInput1OnDelayParameter->value), "Logic input 1 on-time"),
		SDOTask::prepareUpload(0x200A, 0x3, uint16_t(atv320->logicInput2OnDelayParameter->value), "Logic input 2 on-time"),
		SDOTask::prepareUpload(0x200A, 0x4, uint16_t(atv320->logicInput3OnDelayParameter->value), "Logic input 3 on-time"),
		SDOTask::prepareUpload(0x200A, 0x5, uint16_t(atv320->logicInput4OnDelayParameter->value), "Logic input 4 on-time"),
		SDOTask::prepareUpload(0x200A, 0x6, uint16_t(atv320->logicInput5OnDelayParameter->value), "Logic input 5 on-time"),
		SDOTask::prepareUpload(0x200A, 0x7, uint16_t(atv320->logicInput6OnDelayParameter->value), "Logic input 6 on-time"),
		
		//————————— Save Parameters —————————
		//[scs] save parameters to congiration #0 (= 1)
		SDOTask::prepareUpload(0x2032, 0x2, uint16_t(1), "Save Parameters to Configuration 1")
		
	};
	

	setStatusString("Uploading configuration");
	if(!atv320->executeSDOTasks(configUploadList)){
		setStatusString("Configuration upload failed");
		Logger::warn("[{}] Failed to upload configuration", atv320->getName());
		return;
	}
	
	setStatusString("Saving to EEPROM");
	Timing::Timer saveTimer;
	saveTimer.setExpirationSeconds(1.0);
	
	while(true){
		uint16_t savestate;
		if(atv320->readSDO_U16(0x2032, 0x2, savestate)) {
			if(savestate == 0x0){
				setStatusString("Upload Suceeded");
				Logger::info("[{}] Configuration saved on device", atv320->getName());
				return;
			}
		}
		if(saveTimer.isExpired()) {
			setStatusString("Configuration Upload Failed, could not save to EEPROM");
			Logger::warn("[{}] Configuration save timed out (took more than 5 seconds)", atv320->getName());
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}


bool ATV320::StandstillTuningTask::canStart(){
	if(atv320->isOffline()) return false;
	//else if(atv320->actuator->isEnabled()) return false;
	return true;
}

void ATV320::StandstillTuningTask::onExecution(){

	//———— Standstill motor tune after motor parameter assignement
	
	setStatusString("Autotuning started");
	
	std::vector<std::shared_ptr<SDOTask>> autotuningCommands = {
		SDOTask::prepareUpload(0x2042, 0x9, uint16_t(2), "Tuning: Erase Tune [tun]"),
		//SDOTask::prepareUpload(0x2042, 0x9, uint16_t(0), "Tuning: No Action [tun]"),
		SDOTask::prepareUpload(0x2042, 0x9, uint16_t(1), "Tuning: Do tune [tun]")
	};
	
	if(!atv320->executeSDOTasks(autotuningCommands)){
		setStatusString("Autotuning request failed");
		return;
	}
	
	Timing::Timer autotuningTimer;
	autotuningTimer.setExpirationSeconds(3.0);
	while(true){
		//[tus] auto tuning status (0 = not done, 1 = pending, 2 = in progress, 3 = failed, 4 = done)
		uint16_t autotuningStatus;
		if(atv320->readSDO_U16(0x2042, 0xA, autotuningStatus)){
			Logger::warn("tuning state = {}", autotuningStatus);
			if(autotuningStatus == 3){
				setStatusString("Autotuning Started but Failed");
				return;
			}
			else if(autotuningStatus == 4) break;
		}
		
		if(autotuningTimer.isExpired()) {
			setStatusString("Autotuning timed out");
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	setStatusString("Saving to EEPROM");
			
	if(!atv320->writeSDO_U16(0x2032, 0x2, 1)){
		setStatusString("Could not save to EEPROM");
		return;
	}
	
	Timing::Timer saveTimer;
	saveTimer.setExpirationSeconds(1.0);
	
	while(true){
		uint16_t savestate;
		if(atv320->readSDO_U16(0x2032, 0x2, savestate)) {
			if(savestate == 0x0){
				setStatusString("Tuning Suceeded");
				Logger::info("[{}] Configuration saved on device", atv320->getName());
				return;
			}
		}
		if(saveTimer.isExpired()) {
			setStatusString("Tuning succeeded but could not to save to EEPROM");
			Logger::warn("[{}] Configuration save timed out (took more than 5 seconds)", atv320->getName());
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
}
