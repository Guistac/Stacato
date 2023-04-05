#include "ATV320.h"

#include "Fieldbus/EtherCatFieldbus.h"

void ATV320::onConnection() {
}

void ATV320::onDisconnection() {
}

void ATV320::updateActuatorInterface(){
	actuator->actuatorConfig.velocityLimit = nominalMotorSpeedParameter->value / 60.0;
	actuator->actuatorConfig.accelerationLimit = actuator->getVelocityLimit() / accelerationRampTime->value;
	actuator->actuatorConfig.decelerationLimit = actuator->getVelocityLimit() / decelerationRampTime->value;
}

void ATV320::initialize() {
	
	axis = DS402Axis::make(std::static_pointer_cast<EtherCatDevice>(shared_from_this()));
	actuator = std::make_shared<ATV_Motor>(std::static_pointer_cast<ATV320>(shared_from_this()));
	gpio = std::make_shared<ATV_GPIO>(std::static_pointer_cast<ATV320>(shared_from_this()));
	
	actuatorPin->assignData(std::static_pointer_cast<ActuatorInterface>(actuator));
	gpioPin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	addNodePin(actuatorPin);
	addNodePin(gpioPin);
	
	addNodePin(digitalInput1Pin);
	addNodePin(digitalInput2Pin);
	addNodePin(digitalInput3Pin);
	addNodePin(digitalInput4Pin);
	addNodePin(digitalInput5Pin);
	addNodePin(digitalInput6Pin);
	addNodePin(actualVelocityPin);
	addNodePin(actualLoadPin);
	
	//—————— Motion Control Configuration ———————
	
	accelerationRampTime = NumberParameter<double>::make(3.0, "Acceleration Ramp", "AccelerationRamp");
	accelerationRampTime->setUnit(Units::Time::Second);
	accelerationRampTime->addEditCallback([this](){ updateActuatorInterface(); });
	
	decelerationRampTime = NumberParameter<double>::make(1.0, "Deceleration Ramp", "DecelerationRamp");
	decelerationRampTime->setUnit(Units::Time::Second);
	decelerationRampTime->addEditCallback([this](){ updateActuatorInterface(); });
	
	slowdownVelocityHertz = NumberParameter<double>::make(5.0, "Slowdown Velocity", "SlowdownVelocity");
	slowdownVelocityHertz->setUnit(Units::Frequency::Hertz);
	
	invertDirection = BooleanParameter::make(false, "Invert Motion Direction", "InvertMotionDirection");
	
	lowSpeedHertz = NumberParameter<double>::make(0.0, "Low Speed", "LowSpeed");
	lowSpeedHertz->setUnit(Units::Frequency::Hertz);

	//————— Motor Configuration —————

	std::vector<OptionParameter::Option*> motorFrequencyOptions = {
		&option_frequency50Hz,
		&option_frequency60Hz
	};
	
	std::vector<OptionParameter::Option*> motorControlOptions = {
		&option_motorControlType_sensorlessFluxVector,
		&option_motorControlType_standardMotorLaw,
		&option_motorControlType_5pointVoltageFrequency,
		&option_motorControlType_synchronousMotor,
		&option_motorControlType_VFQuadratic,
		&option_motorControlType_energySaving
	};
	
	//[bfr]
	standartMotorFrequencyParameter = OptionParameter::make2(option_frequency50Hz, motorFrequencyOptions, "Standart Motor Frequency [bfr]", "StandartMotorFrequency");
	//[ctt]
	motorControlTypeParameter = OptionParameter::make2(option_motorControlType_standardMotorLaw, motorControlOptions, "Motor Control Type [ctt]", "MotorControlType");
	//[npr]
	ratedMotorPowerParameter = NumberParameter<double>::make(0.0, "Rated Motor Power [npr]", "RatedMotorPower");
	ratedMotorPowerParameter->setUnit(Units::Power::KiloWatt);
	//set MPC to power and not cosinus phi
	//[uns]
	nominalMotorVoltageParameter = NumberParameter<double>::make(0.0, "Rated Motor Voltage [uns]", "RatedMotorVoltage");
	nominalMotorVoltageParameter->setUnit(Units::Voltage::Volt);
	//[ncr]
	nominalMotorCurrentParameter = NumberParameter<double>::make(0.0, "Rated Motor Current [ncr]", "RatedMotorCurrent");
	nominalMotorCurrentParameter->setUnit(Units::Current::Ampere);
	//[frs]
	nominalMotorFrequencyParameter = NumberParameter<double>::make(0.0, "Motor Rated Frequency [frs]", "MotorRatedFrequency");
	nominalMotorFrequencyParameter->setUnit(Units::Frequency::Hertz);
	//[nps]
	nominalMotorSpeedParameter = NumberParameter<double>::make(0.0, "Motor Rated Speed [nps]", "MotorRatedSpeed");
	nominalMotorSpeedParameter->setUnit(Units::AngularDistance::Revolution);
	nominalMotorSpeedParameter->setSuffix("/min");
	nominalMotorSpeedParameter->addEditCallback([this](){ updateActuatorInterface(); });
	 
	
	//[nsp] rated motor speed
	//[frs] rated motor frequency
	//[tfr] max frequency
	//[bfr] standart motor frequency
	
	//————— IO Configuration
	
	std::vector<OptionParameter::Option*> logicInputOptions = {
		&option_logicInput_none,
		&option_logicInput_LI1,
		&option_logicInput_LI2,
		&option_logicInput_LI3,
		&option_logicInput_LI4,
		&option_logicInput_LI5,
		&option_logicInput_LI6
	};
	forwardStopLimitAssignementParameter = OptionParameter::make2(option_logicInput_none, logicInputOptions, "Forward limit input", "ForwardStopSignal");
	reverseStopLimitAssignementParameter = OptionParameter::make2(option_logicInput_none, logicInputOptions, "Reverse limit input", "ReverseStopSignal");
	
	std::vector<OptionParameter::Option*> activeLowHighOptions = {
		&option_activeLow,
		&option_activeHigh
	};
	stopLimitConfigurationParameter = OptionParameter::make2(option_activeHigh, activeLowHighOptions, "Stop limit configuration", "StopLimitConfiguration");
	
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
	
	updateActuatorInterface();
	
	//———— configure pdo data ———
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	
	axis->processDataConfiguration.enableFrequencyMode();
	axis->configureProcessData();
	
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
	
	//———————————— MOTOR SETTINGS ———————————
	
	//[bfr] Standart Motor Frequency (0 = 50Hz / 1 = 60Hz)
	if(!writeSDO_U16(0x2000, 0x10, standartMotorFrequencyParameter->value)) return false;

	//[ctt] Motor Control Type
	if(!writeSDO_U16(0x2042, 0x8, motorControlTypeParameter->value)) return false;
	
	//[uns] rated motor voltage in 1V increments
	if(!writeSDO_U16(0x2042, 0x2, ratedMotorPowerParameter->value)) return false;
	
	//[npr] rated motor power in 0.1Kw increments
	if(!writeSDO_U16(0x2042, 0xE, ratedMotorPowerParameter->value * 100)) return false;
	
	//[ncr] nominal motor current in 0.1A increments
	if(!writeSDO_U16(0x2042, 0x4, nominalMotorCurrentParameter->value * 10)) return false;
	
	//[nsp] nominal motor speed (in rpm)
	if(!writeSDO_U16(0x2042, 0x5, nominalMotorSpeedParameter->value)) return false;

	//[frs] nominal motor frequency (in .1Hz increments)
	if(!writeSDO_U16(0x242, 0x3, nominalMotorFrequencyParameter->value * 10)) return false;
	
	//————————— CONTROL SETTINGS —————————————
	
	//set minimum control speed in hertz to 0Hz (in .1Hz increments)
	if(!writeSDO_U16(0x2001, 0x6, 0)) return false;
	
	//set maximum control speed to in 0.1Hz increments
	uint16_t highSpeed = 500;
	if(!writeSDO_U16(0x2001, 0x5, highSpeed)) return false;
	
	//set max output frequency to 50Hz (in 0.1Hz increments)
	uint16_t maxOutputFrequency = 500;
	if(!writeSDO_U16(0x2001, 0x4, maxOutputFrequency)) return false;
	
	//set drive switching frequency to 16 Khz (max)
	if(!writeSDO_U16(0x2001, 0x3, 160)) return false;
	
	//[inr] set ramp unit increment to hundreds of seconds (.01s = 0  / .1s = 1  /  1.s = 2)
	if(!writeSDO_U16(0x203C, 0x15, 0)) return false;
	
	//[acc] set acceleration time
	if(!writeSDO_U16(0x203C, 0x2, accelerationRampTime->value * 100.0)) return false;
	
	//[dec] set deceleration time
	if(!writeSDO_U16(0x203C, 0x3, decelerationRampTime->value * 100.0)) return false;
	
	//[bfr]
	//[frs]
	//[nsp]
	//[tfr] 2001/4
	//[hsp] 2001/5
	
	
	//——————————————— Control Reference Settings ——————————
	
	//[fr1] set frequency reference 1 to Communication card = 169
	if(!writeSDO_U16(0x2036, 0xE, 169)) return false;
	
	//[rfc] set reference switching to reference 1 fixed = 96
	if(!writeSDO_U16(0x2036, 0xC, 96)) return false;
	
	//[chcf] set control profile to "Not separate" (= 1) for pure CiA.402 Velocity Mode
	if(!writeSDO_U16(0x2036, 0x2, 1)) return false;
	
	//—————————————— IO Configuration ———————————————
	
	//[saf] forward limit stop
	if(!writeSDO_U16(0x205F, 0x2, forwardStopLimitAssignementParameter->value)) return false;
	
	//[sar] reverse limit stop
	if(!writeSDO_U16(0x205F, 0x3, reverseStopLimitAssignementParameter->value)) return false;
	
	//[sal] //stop signal active high or low
	if(!writeSDO_U16(0x205F, 0x9, stopLimitConfigurationParameter->value)) return false;
	
	//[l1d] -> [l6d] logic input on delay parameters
	if(!writeSDO_U16(0x200A, 0x2, logicInput1OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x3, logicInput2OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x4, logicInput3OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x5, logicInput4OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x6, logicInput5OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x7, logicInput6OnDelayParameter->value)) return false;
	
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
	
/*
	//read power state and react to change
	auto newPowerState = ds402Status.getPowerState();
	if(newPowerState != actualPowerState){
		std::string message = "Power State changed to " + std::string(Enumerator::getDisplayString(newPowerState));
		pushEvent(message.c_str(), false);
	}
	actualPowerState = newPowerState;
	
	//enable request timeout
	//ATV320 seems to consider 'Switched On' as a valid 'Enabled' power state
	if(requestedPowerState == DS402::PowerState::OPERATION_ENABLED && !(actualPowerState == DS402::PowerState::OPERATION_ENABLED
																		|| actualPowerState == DS402::PowerState::SWITCHED_ON)){
		if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - enableRequestTime_nanoseconds > enableRequestTimeout_nanoseconds){
			Logger::warn("{} : Enable Request Timeout", getName());
			requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
		}
	}
*/
	
	//update general state
	b_remoteControlEnabled = axis->isRemoteControlActive();
	b_stoActive = stoState != 0;
	b_hasFault = axis->hasFault();
	b_motorVoltagePresent = axis->hasVoltage();
	
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
	
	actuator->actuatorProcessData.b_isEmergencyStopActive = b_stoActive;
	actuator->actuatorProcessData.effortActual = (double)motorPower / 100.0;
	actuator->feedbackProcessData.velocityActual = axis->getActualVelocity() / 60.0;
	actuator->actuatorConfig.b_supportsHoldingBrakeControl = false;
	
	//update output pin data
	*actualVelocity = actuator->getVelocity();
	*actualLoad = actuator->getEffort();
	*digitalInput1Signal = logicInputs & 0x1;
	*digitalInput2Signal = logicInputs & 0x2;
	*digitalInput3Signal = logicInputs & 0x4;
	*digitalInput4Signal = logicInputs & 0x8;
	*digitalInput5Signal = logicInputs & 0x10;
	*digitalInput6Signal = logicInputs & 0x20;
}

//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void ATV320::writeOutputs() {
	
	long long now_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	
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
			enableRequestTime_nanoseconds = now_nanoseconds;
			axis->enable();
		}
	}
	else if(b_waitingForEnable){
		static const long long maxEnableTime_nanoseconds = 500'000'000; //500ms
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
	
	if(b_reverseDirection) axis->setFrequency(-actuator->actuatorProcessData.velocityTarget * 60.0);
	else axis->setFrequency(actuator->actuatorProcessData.velocityTarget * 60.0);
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}

//============================= SAVING AND LOADING DEVICE DATA ============================

bool ATV320::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* kinematicsXML = xml->InsertNewChildElement("KinematicLimits");
	accelerationRampTime->save(kinematicsXML);
	decelerationRampTime->save(kinematicsXML);
	
	invertDirection->save(kinematicsXML);
	slowdownVelocityHertz->save(kinematicsXML);
    lowSpeedHertz->save(kinematicsXML);
	
	XMLElement* motorParametersXML = xml->InsertNewChildElement("MotorParameters");
	nominalMotorCurrentParameter->save(motorParametersXML);
	ratedMotorPowerParameter->save(motorParametersXML);
	
	XMLElement* inputAssignementXML = xml->InsertNewChildElement("InputAssignements");
	forwardStopLimitAssignementParameter->save(inputAssignementXML);
	reverseStopLimitAssignementParameter->save(inputAssignementXML);
	stopLimitConfigurationParameter->save(inputAssignementXML);
	
	return true;
}

bool ATV320::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* kinematicsXML = xml->FirstChildElement("KinematicLimits");
	if(kinematicsXML == nullptr) return Logger::warn("Could not find kinematic limits attribute");
	if(!accelerationRampTime->load(kinematicsXML)) return false;
	if(!decelerationRampTime->load(kinematicsXML)) return false;
	accelerationRampTime->onEdit();
	decelerationRampTime->onEdit();
	if(!invertDirection->load(kinematicsXML)) return false;
	if(!slowdownVelocityHertz->load(kinematicsXML)) return false;
    if(!lowSpeedHertz->load(kinematicsXML)) return false;
	
	XMLElement* motorParametersXML = xml->FirstChildElement("MotorParameters");
	if(!nominalMotorCurrentParameter->load(motorParametersXML)) return false;
	if(!ratedMotorPowerParameter->load(motorParametersXML)) return false;
	
	XMLElement* inputAssignementXML = xml->FirstChildElement("InputAssignements");
	if(kinematicsXML == nullptr) return Logger::warn("Could not find input assignement attribute");
	if(!forwardStopLimitAssignementParameter->load(inputAssignementXML)) return false;
	if(!reverseStopLimitAssignementParameter->load(inputAssignementXML)) return false;
	if(!stopLimitConfigurationParameter->load(inputAssignementXML)) return false;
	
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
