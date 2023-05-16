#include "KincoFD.h"

#include "Fieldbus/EtherCatFieldbus.h"

void KincoFD::onDisconnection() {}
void KincoFD::onConnection() {}

void KincoFD::onDetection(){	
	int16_t CMD_q_Limit;
	readSDO_S16(0x60F6, 0xF, CMD_q_Limit);
	driveCurrentLimit = float(CMD_q_Limit) / float(incrementsPerAmpere);
}

void KincoFD::initialize() {
	auto thisDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	axis = DS402Axis::make(thisDevice);
	
	actuator = std::make_shared<ActuatorInterface>();
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
	maxAcceleration_parameter = NumberParameter<double>::make(0.0, "Max Acceleration", "MaxAcceleration");
	maxAcceleration_parameter->setUnit(Units::AngularDistance::Revolution);
	maxAcceleration_parameter->setSuffix("/s\xc2\xb2");
	
	maxCurrent_parameter = NumberParameter<double>::make(0.0, "Max Current", "MaxCurrent");
	maxCurrent_parameter->setUnit(Units::Current::Ampere);
	maxCurrent_parameter->setFormat("%.2f");
	velocityFeedforward_parameter = NumberParameter<double>::make(0.0, "Velocity Feed Forward", "VelocityFeedForward");
	velocityFeedforward_parameter->setUnit(Units::Fraction::Percent);
	velocityFeedforward_parameter->setFormat("%.1f");
	maxFollowingError_parameter = NumberParameter<double>::make(0.0, "Max Position Following Error", "MaxFollowingError");
	maxFollowingError_parameter->setUnit(Units::AngularDistance::Revolution);
	followingErrorTimeout_parameter = NumberParameter<int>::make(0.0, "Position Following Error Time Out", "FollowingErrorTimeout");
	followingErrorTimeout_parameter->setUnit(Units::Time::Millisecond);
	
	brakingResistorResistance_parameter = NumberParameter<int>::make(0.0, "Braking Resistor Resistance", "BrakingResistorResistance");
	brakingResistorResistance_parameter->setUnit(Units::Resistance::Ohm);
	brakingResistorPower_parameter = NumberParameter<int>::make(0.0, "Braking Resistor Rated Power", "BrakingResistorRatedPower");
	brakingResistorPower_parameter->setUnit(Units::Power::Watt);
	
	DIN1Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN1 Function", "Din1Function"),
	DIN2Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN2 Function", "Din2Function"),
	DIN3Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN3 Function", "Din3Function"),
	DIN4Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN4 Function", "Din4Function"),
	DIN5Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN5 Function", "Din5Function"),
	DIN6Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN6 Function", "Din6Function"),
	DIN7Function_parameter = OptionParameter::make2(inputFunction_none, inputFunctionOptions, "DIN7 Function", "Din7Function"),
	
	
	
	
	axis->processDataConfiguration.enableCyclicSynchronousPositionMode();
	axis->processDataConfiguration.enableCyclicSynchronousVelocityMode();
	axis->processDataConfiguration.enableHomingMode();
	axis->processDataConfiguration.positionFollowingErrorActualValue = true;
	axis->processDataConfiguration.errorCode = true;
	axis->processDataConfiguration.currentActualValue = true;
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	axis->configureProcessData();
}

bool KincoFD::startupConfiguration() {
	//———— PDO Assignement

	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex())) return false;
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex())) return false;
	
	axis->setOperatingMode(DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY);
	
	//when writing these on startup, the txpdo fields are blank for some reason
	//maybe they should only be affected on first drive configuration
	//the drive only sends valid txpdos after a restart
	//enable clock sync
	//if(!writeSDO_U8(0x3011, 0x2, 1, "Enable Clock Sync")) return false;
	//set clock cycle time (0=1ms, 1=2ms, 2=4ms??, 3=8ms??)
	//if(!writeSDO_U8(0x3011, 0x1, 1, "Set Clock Time")) return false;
	
	//set command type to communication control (EtherCAT)
	if(!writeSDO_U8(0x3041, 0x2, 8, "Command Type")) return false;
	//enable error on ethercat disconnection
	if(!writeSDO_S16(0x6007, 0x0, 1, "Abort Connection Mode")) return false;
	int driveTimeoutDelay_milliseconds = EtherCatFieldbus::processInterval_milliseconds * 3;
	//error happens if no process data is received for a period equivalent to 3 cycles (in milliseconds)
	if(!writeSDO_U16(0x100C, 0x0, driveTimeoutDelay_milliseconds, "Guard_Time")) return false;
	//set time multiplier to 1
	if(!writeSDO_U8(0x100D, 0x0, 1, "Life_Time_Factor")) return false;
	
	//———— Synchronisation
	
	uint32_t cycleTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1'000'000;
	uint32_t shiftTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 500'000;
	ec_dcsync0(getSlaveIndex(), true, cycleTime_nanoseconds, shiftTime_nanoseconds);
	
	return true;
}
void KincoFD::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
	
	posActual = float(axis->getActualPosition()) / incrementsPerRevolution;
	velActual = float(axis->getActualVelocity()) / incrementsPerRevolutionPerSecond;
	
}
void KincoFD::writeOutputs(){
	
	if(b_enable){
		b_enable = false;
		if(axis->hasFault()) axis->doFaultReset();
		else axis->enable();
	}
	
	if(b_disable){
		b_disable = false;
		axis->disable();
	}
	
	if(b_modeSelection) {
		int32_t vel = velTarget * incrementsPerRevolutionPerSecond;
		axis->setVelocity(vel);
	}
	else {
		int32_t pos = posTarget * incrementsPerRevolution;
		axis->setPosition(pos);
	}
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}


bool KincoFD::saveDeviceData(tinyxml2::XMLElement* xml) {
	
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
	
	return true;
}
bool KincoFD::loadDeviceData(tinyxml2::XMLElement* xml) {

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
	
	return true;
}


void KincoFD::uploadConfiguration(){
	
	//enable clock sync
	if(!writeSDO_U8(0x3011, 0x2, 0, "Enable Clock Sync")) return false;
	//set clock cycle time (0=1ms, 1=2ms, 2=4ms??, 3=8ms??)
	if(!writeSDO_U8(0x3011, 0x1, 0, "Set Clock Time")) return false;
	
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
	
	//store drive data
	if(!writeSDO_U8(0x2FF0, 0x1, 1, "Store Data")) return false;
	//store motor data
	if(!writeSDO_U8(0x2FF0, 0x3, 1, "Store Motor Data")) return false;
}

void KincoFD::updateActuatorInterface(){
	
	auto& actuatorConfig = actuator->actuatorConfig;
	actuatorConfig.accelerationLimit = maxAcceleration_parameter->value;
	actuatorConfig.decelerationLimit = maxAcceleration_parameter->value;
	actuatorConfig.followingErrorLimit = maxFollowingError_parameter->value;
	actuatorConfig.velocityLimit = maxVelocity_parameter->value;
}
