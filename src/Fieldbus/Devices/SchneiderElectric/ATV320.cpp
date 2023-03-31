#include "ATV320.h"

#include "Fieldbus/EtherCatFieldbus.h"

void ATV320::onConnection() {
}

void ATV320::onDisconnection() {
}

void ATV320::updateActuatorInterface(){
	actuator->actuatorConfig.velocityLimit = maxVelocityRPM->value / 60.0;
	actuator->actuatorConfig.accelerationLimit = actuator->getVelocityLimit() / accelerationRampTime->value;
	actuator->actuatorConfig.decelerationLimit = actuator->getVelocityLimit() / decelerationRampTime->value;
}

void ATV320::initialize() {
	
	std::shared_ptr<ATV320> thisDrive = std::static_pointer_cast<ATV320>(shared_from_this());
	actuator = std::make_shared<ATV_Motor>(thisDrive);
	gpio = std::make_shared<ATV_GPIO>(thisDrive);
	
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

	maxVelocityRPM->addEditCallback([this](){ updateActuatorInterface(); });
	accelerationRampTime->addEditCallback([this](){ updateActuatorInterface(); });
	decelerationRampTime->addEditCallback([this](){ updateActuatorInterface(); });
	
	
	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(0x6040, 0x0, 16, "ControlWorld", &ds402Control.controlWord);
	rxPdoAssignement.addEntry(0x6042, 0x0, 16, "VelocityTarget", &velocityTarget_rpm);
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6041, 0x0, 16, "StatusWord", &ds402Status.statusWord);
	txPdoAssignement.addEntry(0x6044, 0x0, 16, "VelocityActual", &velocityActual_rpm);
	txPdoAssignement.addEntry(0x2002, 0xC, 16, "MotorPower", &motorPower);
	txPdoAssignement.addEntry(0x2016, 0x3, 16, "LogicInputs", &logicInputs);
	txPdoAssignement.addEntry(0x207B, 0x17, 16, "STOstate", &stoState);
	txPdoAssignement.addEntry(0x2029, 0x16, 16, "LastFaultCode", &lastFaultCode);
	
	ratedMotorCurrentParameter = NumberParameter<double>::make(0.0, "Rated Motor Curent", "RatedMotorCurrent");
	ratedMotorCurrentParameter->setUnit(Units::Current::Ampere);
	ratedMotorPowerParameter = NumberParameter<double>::make(0.0, "Rated Motor Power", "RatedMotorPower");
	ratedMotorPowerParameter->setUnit(Units::Power::KiloWatt);
	
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
}

//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool ATV320::startupConfiguration() {
	
	//———————————— MOTOR SETTINGS ———————————
	
	//set motor frequency to 50Hz (0 = 50Hz / 1 = 60Hz)
	uint16_t standartMotorFrequency = 0;
	if(!writeSDO_U16(0x2000, 0x10, standartMotorFrequency)) return false;
	
	//rated motor voltage in 1V increments
	uint16_t ratedMotorVoltage = 400;
	if(!writeSDO_U16(0x2042, 0x2, ratedMotorVoltage)) return false;
	
	//rated motor power in 0.1Kw increments
	if(!writeSDO_U16(0x2042, 0xE, ratedMotorPowerParameter->value * 10)) return false;
	
	//rate motor current in 0.1A increments
	if(!writeSDO_U16(0x2042, 0x4, ratedMotorCurrentParameter->value * 10)) return false;
	
	//nominal rate motor speed in rpm
	uint16_t nominalMotorSpeed = 1400;
	if(!writeSDO_U16(0x2042, 0x5, nominalMotorSpeed)) return false;

	//TODO: this must be configured when the fieldbus is not active
	//set motor control type to standart (Standart aynchronous motor control = 3)
	uint16_t motorControlType = 3;
	if(!writeSDO_U16(0x2042, 0x8, motorControlType)) return false;
	
	//————————— CONTROL SETTINGS —————————————
	
	//set max output frequency to 50Hz (in 0.1Hz increments)
	uint16_t maxOutputFrequency = 500;
	if(!writeSDO_U16(0x2001, 0x4, maxOutputFrequency)) return false;
	
	//set minimum control speed in hertz to 0Hz (in .1Hz increments)
    //uint16_t lowSpeed = lowSpeedHertz->value * 10.0;
	uint16_t lowSpeed = 0.0;
	if(!writeSDO_U16(0x2001, 0x6, lowSpeed)) return false;
	
	//set maximum control speed to in 0.1Hz increments
	uint16_t highSpeed = 500;
	if(!writeSDO_U16(0x2001, 0x5, highSpeed)) return false;
	
	//set drive switching frequency to 16 Khz (max)
	uint16_t switchingFrequency = 160;
	if(!writeSDO_U16(0x2001, 0x3, switchingFrequency)) return false;
	
	
	//—————————————— IO SETTINGS ———————————————
	
	//set terminal block control to Two Wire (0 = 2-Wire / 1 = 3-Wire)
	//uint16_t twoOrThreeWireControl = 0;
	//if(!writeSDO_U16(0x2051, 0x2, twoOrThreeWireControl)) return false;
	
	//2-Wire Control level detection is -Signal Transition-
	//uint16_t twoWireControlType = 1;
	//if(!writeSDO_U16(0x2051, 0x3, twoWireControlType)) return false;
	
	//set Forward input to Logic Input 1 (LI1)
	//uint16_t forwardInputAssignement = 129;
	//if(!writeSDO_U16(0x2051, 0x5, forwardInputAssignement)) return false;
	
	//set reverse input to Logic Input 2 (LI2)
	//uint16_t reverseInputAssignement = 130;
	//if(!writeSDO_U16(0x2051, 0x6, reverseInputAssignement)) return false;
	
	//set fault reset input assignement to pin LI5
	//uint16_t faultResetInputAssignement = 133;
	//if(!writeSDO_U16(0x2029, 0x19, faultResetInputAssignement)) return false;
	
	//set method of switching between frequency references (LI3 = 131)
	//uint16_t referenceSwitchingInputAssignement = 131;
	//if(!writeSDO_U16(0x2036, 0xC, referenceSwitchingInputAssignement)) return false;
	
	//set external fault pin to Logic Input 6 (LI6 = 134)
	//uint16_t externalFaultInputAssignement = 134;
	//if(!writeSDO_U16(0x2029, 0x20, externalFaultInputAssignement)) return false;
	
	//set preset slowdown speed input assignement to pin LI4 = 132
	//uint16_t presetSpeed2InputAssignement = 132;
	//if(!writeSDO_U16(0x2054, 0x2, presetSpeed2InputAssignement)) return false;
	   
	//set preset slowdown speed to 5Hz (in .1Hz Increments)
	//uint16_t presetSpeed2Frequency = slowdownVelocityHertz->value * 10.0;
	//if(!writeSDO_U16(0x2054, 0xB, presetSpeed2Frequency)) return false;
	
	
	//—————————————— RAMP SETTINGS ——————————————
	
	//set ramp unit increment to hundreds of seconds (.01s = 0  / .1s = 1  /  1.s = 2)
	uint16_t rampIncrement = 0;
	if(!writeSDO_U16(0x203C, 0x15, rampIncrement)) return false;
	
	//set acceleration time
	uint16_t accelerationTime = accelerationRampTime->value * 100.0;
	if(!writeSDO_U16(0x203C, 0x2, accelerationTime)) return false;
	
	//set deceleration time
	uint16_t decelerationTime = decelerationRampTime->value * 100.0;
	if(!writeSDO_U16(0x203C, 0x3, decelerationTime)) return false;
	
	
	//——————————————— Network Control Settings ——————————
	
	//set frequency reference 1 to Communication card
	uint16_t communicationCardReference = 169;
	if(!writeSDO_U16(0x2036, 0xE, communicationCardReference)) return false;
	if(!writeSDO_U16(0x2036, 0xF, communicationCardReference)) return false;
	
	//set frequency reference 1 to Analog Input 1 (AI1)
	//uint16_t reference1 = 1;
	//if(!writeSDO_U16(0x2036, 0xE, reference1)) return false;
	
	//set frequency reference 2 to communication card (EtherCAT Card)
	//uint16_t reference2 = 169;
	//if(!writeSDO_U16(0x2036, 0xF, reference2)) return false;
	
	//set control profile to "Not separate" for pure CiA.402 Velocity Mode
	uint16_t controlProfile = 1;
	if(!writeSDO_U16(0x2036, 0x2, controlProfile)) return false;
	
	
	//—————————————— INPUT ASSIGNEMENTS ———————————————
	
	//[saf]
	uint16_t forwardLimitAssignement = forwardStopLimitAssignementParameter->value;
	if(!writeSDO_U16(0x205F, 0x2, forwardLimitAssignement)) return false;
	
	//[sar]
	uint16_t reverseLimitAssignement = reverseStopLimitAssignementParameter->value;
	if(!writeSDO_U16(0x205F, 0x3, reverseLimitAssignement)) return false;
	
	//[sal]
	uint16_t stopLimitConfiguration = stopLimitConfigurationParameter->value;
	if(!writeSDO_U16(0x205F, 0x9, stopLimitConfiguration)) return false;
	
	//[L1D] -> [L6D]
	if(!writeSDO_U16(0x200A, 0x2, logicInput1OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x3, logicInput2OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x4, logicInput3OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x5, logicInput4OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x6, logicInput5OnDelayParameter->value)) return false;
	if(!writeSDO_U16(0x200A, 0x7, logicInput6OnDelayParameter->value)) return false;
	
	//—————————————— ASSIGN PDOS —————————————————
	
	if(!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) return false;
	if(!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) return false;
	
	b_reverseDirection = invertDirection->value;
	
	return true;
}

//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void ATV320::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	
	//read power state and react to change
	auto newPowerState = ds402Status.getPowerState();
	if(newPowerState != actualPowerState){
		std::string message = "Power State changed to " + std::string(Enumerator::getDisplayString(newPowerState));
		pushEvent(message.c_str(), false /*!DS402::isNominal(newPowerState)*/);
	}
	actualPowerState = newPowerState;
	
	//enable request timeout
	if(requestedPowerState == DS402::PowerState::OPERATION_ENABLED && actualPowerState != DS402::PowerState::OPERATION_ENABLED){
		if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - enableRequestTime_nanoseconds > enableRequestTimeout_nanoseconds){
			Logger::warn("{} : Enable Request Timeout", getName());
			requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
		}
	}
	
	//update general state
	b_remoteControlEnabled = ds402Status.isRemoteControlled();
	b_stoActive = stoState != 0;
	b_hasFault = ds402Status.hasFault();
	b_motorVoltagePresent = ds402Status.statusWord & 0x10;
	
	//update output pin data
	*actualVelocity = velocityActual_rpm / 60;
	*actualLoad = motorPower / 100.0;
	*digitalInput1Signal = logicInputs & 0x1;
	*digitalInput2Signal = logicInputs & 0x2;
	*digitalInput3Signal = logicInputs & 0x4;
	*digitalInput4Signal = logicInputs & 0x8;
	*digitalInput5Signal = logicInputs & 0x10;
	*digitalInput6Signal = logicInputs & 0x20;
	
	//update subdevices
	if(!isConnected()) 														actuator->state = DeviceState::OFFLINE;
	if(!b_remoteControlEnabled)												actuator->state = DeviceState::NOT_READY;
	else if(b_stoActive) 													actuator->state = DeviceState::NOT_READY;
	else if(!b_motorVoltagePresent) 										actuator->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402::PowerState::NOT_READY_TO_SWITCH_ON) 	actuator->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402::PowerState::SWITCH_ON_DISABLED)		actuator->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402::PowerState::OPERATION_ENABLED) 		actuator->state = DeviceState::ENABLED;
	else 																	actuator->state = DeviceState::READY;

	if(!isConnected()) 					gpio->state = DeviceState::OFFLINE;
	else if(b_stoActive)				gpio->state = DeviceState::NOT_READY;
	else if(isStateSafeOperational()) 	gpio->state = DeviceState::READY;
	else if(isStateOperational()) 		gpio->state = DeviceState::ENABLED;
	else 								gpio->state = DeviceState::NOT_READY;
	
	actuator->actuatorProcessData.b_isEmergencyStopActive = b_stoActive;
	actuator->actuatorProcessData.effortActual = (double)motorPower / 100.0;
	//actuator->actualVelocity = velocityActual_rpm;
	actuator->actuatorConfig.b_supportsHoldingBrakeControl = false;
}

//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void ATV320::writeOutputs() {
	
	//state change commands
	if(actuator->actuatorProcessData.b_enable){
		actuator->actuatorProcessData.b_enable = false;
		requestedPowerState = DS402::PowerState::OPERATION_ENABLED;
		enableRequestTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	}
	if(actuator->actuatorProcessData.b_disable){
		actuator->actuatorProcessData.b_disable = false;
		requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	}
	if(actuator->actuatorProcessData.b_quickstop){
		actuator->actuatorProcessData.b_quickstop = false;
		requestedPowerState = DS402::PowerState::QUICKSTOP_ACTIVE;
	}
	ds402Control.setPowerState(requestedPowerState, actualPowerState);
	
	//clear errors when we enable the power stage
	if(b_hasFault && !b_isResettingFault && requestedPowerState == DS402::PowerState::OPERATION_ENABLED){
		b_isResettingFault = true;
		ds402Control.performFaultReset();
	}else b_isResettingFault = false;
	ds402Control.updateControlWord();
	
	//set target velocity
	if(b_reverseDirection) velocityTarget_rpm = -actuator->actuatorProcessData.velocityTarget * 60.0;
	else velocityTarget_rpm = actuator->actuatorProcessData.velocityTarget * 60.0;
	
	
	//Logger::warn("{}", velocityTarget_rpm);
	
	rxPdoAssignement.pushDataTo(identity->outputs);
}

//============================= SAVING AND LOADING DEVICE DATA ============================

bool ATV320::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* kinematicsXML = xml->InsertNewChildElement("KinematicLimits");
	accelerationRampTime->save(kinematicsXML);
	decelerationRampTime->save(kinematicsXML);
	maxVelocityRPM->save(kinematicsXML);
	invertDirection->save(kinematicsXML);
	slowdownVelocityHertz->save(kinematicsXML);
    lowSpeedHertz->save(kinematicsXML);
	
	XMLElement* motorParametersXML = xml->InsertNewChildElement("MotorParameters");
	ratedMotorCurrentParameter->save(motorParametersXML);
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
	if(!maxVelocityRPM->load(kinematicsXML)) return false;
	if(!accelerationRampTime->load(kinematicsXML)) return false;
	if(!decelerationRampTime->load(kinematicsXML)) return false;
	maxVelocityRPM->onEdit();
	accelerationRampTime->onEdit();
	decelerationRampTime->onEdit();
	if(!invertDirection->load(kinematicsXML)) return false;
	if(!slowdownVelocityHertz->load(kinematicsXML)) return false;
    if(!lowSpeedHertz->load(kinematicsXML)) return false;
	
	XMLElement* motorParametersXML = xml->FirstChildElement("MotorParameters");
	if(!ratedMotorCurrentParameter->load(motorParametersXML)) return false;
	if(!ratedMotorPowerParameter->load(motorParametersXML)) return false;
	
	XMLElement* inputAssignementXML = xml->FirstChildElement("InputAssignements");
	if(kinematicsXML == nullptr) return Logger::warn("Could not find input assignement attribute");
	if(!forwardStopLimitAssignementParameter->load(inputAssignementXML)) return false;
	if(!reverseStopLimitAssignementParameter->load(inputAssignementXML)) return false;
	if(!stopLimitConfigurationParameter->load(inputAssignementXML)) return false;
	
	return true;
}








std::string ATV320::getShortStatusString(){
	if(!isConnected()) return "Device Offline";
	else if(!b_motorVoltagePresent) return "No Motor Voltage";
	else if(!b_remoteControlEnabled) return "Remote Disabled";
	else if(b_stoActive) return "STO Active";
	else if(b_hasFault) return "Fault Code " + std::to_string(lastFaultCode);
	else if(actualPowerState == DS402::PowerState::NOT_READY_TO_SWITCH_ON) return "Restart Needed";
	else return Enumerator::getDisplayString(actualPowerState);
}


std::string ATV320::getStatusString(){
	if(!isConnected()) return "Device is Offline";
	else if(!b_motorVoltagePresent) return "No Motor Voltage : Check Power Connections";
	else if(!b_remoteControlEnabled) return "Network Control Disabled : Disable Local Controls";
	else if(b_stoActive) return "STO Active : Release Emergency Stop";
	else if(b_hasFault) return getFaultString();
	else if(actualPowerState == DS402::PowerState::NOT_READY_TO_SWITCH_ON) return "Drive needs a restart...";
	else return Enumerator::getDisplayString(actualPowerState);
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
