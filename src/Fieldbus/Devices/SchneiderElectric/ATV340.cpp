#include <pch.h>
#include "ATV340.h"

#include "Fieldbus/EtherCatFieldbus.h"

ATV340::ParameterOptions ATV340::options;

void ATV340::onConnection() {}

void ATV340::onDisconnection() {}

void ATV340::onConstruction() {
	
	EtherCatDevice::onConstruction();
	
	setName("ATV340");
	
	//create submodules
	axis = DS402Axis::make(std::static_pointer_cast<EtherCatDevice>(shared_from_this()));
	motor = std::make_shared<ATV340_Motor>(std::static_pointer_cast<ATV340>(shared_from_this()));
	gpio = std::make_shared<ATV340_GPIO>(std::static_pointer_cast<ATV340>(shared_from_this()));
	
	
	motor_pin = NodePin::createInstance(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Actuator", "Actuator");
	gpio_pin = NodePin::createInstance(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Gpio", "Gpio");
	
	velocity_Pin = NodePin::createInstance(velocity_Value, NodePin::Direction::NODE_OUTPUT, "Velocity", "Velocity", NodePin::Flags::DisableDataField);
	load_Pin = NodePin::createInstance(load_Value, NodePin::Direction::NODE_OUTPUT, "Effort", "Effort", NodePin::Flags::DisableDataField);
	

	digitalInput1_Pin = NodePin::createInstance(digitalInput1_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 1", "Digital Input 1",
												NodePin::Flags::DisableDataField);
	digitalInput2_Pin = NodePin::createInstance(digitalInput2_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 2", "Digital Input 2",
												NodePin::Flags::DisableDataField);
	digitalInput3_Pin = NodePin::createInstance(digitalInput3_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 3", "Digital Input 3",
												NodePin::Flags::DisableDataField);
	digitalInput4_Pin = NodePin::createInstance(digitalInput4_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 4", "Digital Input 4",
												NodePin::Flags::DisableDataField);
	digitalInput5_Pin = NodePin::createInstance(digitalInput5_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 5", "Digital Input 5",
												NodePin::Flags::DisableDataField);
	analogInput1_pin = NodePin::createInstance(analogInput1_value, NodePin::Direction::NODE_OUTPUT, "Analog Input 1", "Analog Input 1",
											   NodePin::Flags::DisableDataField);
	analogInput2_pin = NodePin::createInstance(analogInput2_value, NodePin::Direction::NODE_OUTPUT, "Analog Input 2", "Analog Input 2",
											   NodePin::Flags::DisableDataField);
	digitalOutput1_Pin = NodePin::createInstance(digitalOutput1_Signal, NodePin::Direction::NODE_INPUT, "Digital Output 1", "Digital Output 1");
	digitalOutput2_Pin = NodePin::createInstance(digitalOutput2_Signal, NodePin::Direction::NODE_INPUT, "Digital Output 2", "Digital Output 2");
	relaisOutput1_Pin = NodePin::createInstance(relaisOutput1_Signal, NodePin::Direction::NODE_INPUT, "Relais Output 1", "Relais Output 1");
	relaisOutput2_Pin = NodePin::createInstance(relaisOutput2_Signal, NodePin::Direction::NODE_INPUT, "Relais Output 2", "Relais Output 2");
	
	motor_pin->assignData(std::static_pointer_cast<ActuatorInterface>(motor));
	gpio_pin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	//add node pins
	addNodePin(motor_pin);
	addNodePin(gpio_pin);
	addNodePin(velocity_Pin);
	addNodePin(load_Pin);
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
	
	
	
	//———— Process Data
	
	pdo_digitalIn = Legato::BooleanParameter::createInstance(false, "Read Digital Inputs", "ConfigDigitalInputs");
	//configure parameter callbacks
	pdo_digitalIn->addEditCallback([this](){
		digitalInput1_Pin->setVisible(pdo_digitalIn->getValue());
		digitalInput2_Pin->setVisible(pdo_digitalIn->getValue());
		digitalInput3_Pin->setVisible(pdo_digitalIn->getValue());
		digitalInput4_Pin->setVisible(pdo_digitalIn->getValue());
		digitalInput5_Pin->setVisible(pdo_digitalIn->getValue());
		if(!pdo_digitalIn->getValue()){
			digitalInput1_Pin->disconnectAllLinks();
			digitalInput2_Pin->disconnectAllLinks();
			digitalInput3_Pin->disconnectAllLinks();
			digitalInput4_Pin->disconnectAllLinks();
			digitalInput5_Pin->disconnectAllLinks();
		}
	});
	pdo_digitalOut = Legato::BooleanParameter::createInstance(false, "Write Digital Outputs", "ConfigDigitalOutputs");
	pdo_digitalOut->addEditCallback([this](){
		digitalOutput1_Pin->setVisible(pdo_digitalOut->getValue());
		digitalOutput2_Pin->setVisible(pdo_digitalOut->getValue());
		relaisOutput1_Pin->setVisible(pdo_digitalOut->getValue());
		relaisOutput2_Pin->setVisible(pdo_digitalOut->getValue());
		if(!pdo_digitalOut->getValue()){
			digitalOutput1_Pin->disconnectAllLinks();
			digitalOutput2_Pin->disconnectAllLinks();
			relaisOutput1_Pin->disconnectAllLinks();
			relaisOutput2_Pin->disconnectAllLinks();
		}
	});
	pdo_readAnalogIn1 = Legato::BooleanParameter::createInstance(false, "Read Analog Input 1", "ConfigAnalogInput1");
	pdo_readAnalogIn1->addEditCallback([this](){
		analogInput1_pin->setVisible(pdo_readAnalogIn1->getValue());
		if(!pdo_readAnalogIn1->getValue()) analogInput1_pin->disconnectAllLinks();
	});
	pdo_readAnalogIn2 = Legato::BooleanParameter::createInstance(false, "Read Analog Input 2", "ConfigAnalogInput2");
	pdo_readAnalogIn2->addEditCallback([this](){
		analogInput2_pin->setVisible(pdo_readAnalogIn2->getValue());
		if(!pdo_readAnalogIn2->getValue()) analogInput2_pin->disconnectAllLinks();
	});
	pdo_motorVelocity = Legato::BooleanParameter::createInstance(false, "Read Output Velocity", "ConfigMotorOutputVelocity");
	pdo_motorEffort = Legato::BooleanParameter::createInstance(false, "Read Motor Effort", "ConfigMotorEffort");
	pdoConfigParameters = Legato::ParameterGroup::createInstance("PDOconfig",{
		pdo_digitalIn,
		pdo_digitalOut,
		pdo_readAnalogIn1,
		pdo_readAnalogIn2,
		pdo_motorVelocity,
		pdo_motorEffort
	});
	for(auto parameter : pdoConfigParameters->getParameters()){
		parameter->addEditCallback([this](){ configureProcessData(); });
		parameter->onEdit();
	}
	
	invertDigitalInput1_Param = Legato::BooleanParameter::createInstance(false, "Invert Digital Input 1", "InvertDigitalInput1");
	invertDigitalInput2_Param = Legato::BooleanParameter::createInstance(false, "Invert Digital Input 2", "InvertDigitalInput2");
	invertDigitalInput3_Param = Legato::BooleanParameter::createInstance(false, "Invert Digital Input 3", "InvertDigitalInput3");
	invertDigitalInput4_Param = Legato::BooleanParameter::createInstance(false, "Invert Digital Input 4", "InvertDigitalInput4");
	invertDigitalInput5_Param = Legato::BooleanParameter::createInstance(false, "Invert Digital Input 5", "InvertDigitalInput5");
	invertDigitalOutput1_Param = Legato::BooleanParameter::createInstance(false, "Invert Digital Output 1", "InvertDigitalOutput1");
	invertDigitalOutput2_Param = Legato::BooleanParameter::createInstance(false, "Invert Digital Output 1", "InvertDigitalOutput1");
	invertRelay1_Param = Legato::BooleanParameter::createInstance(false, "Invert Relay 1", "InvertRelay1");
	invertRelay2_Param = Legato::BooleanParameter::createInstance(false, "Invert Relay 2", "InvertRelay1");
	digitalSignalInversion = Legato::ParameterGroup::createInstance("Digital Signal Inversion", {
		invertDigitalInput1_Param,
		invertDigitalInput2_Param,
		invertDigitalInput3_Param,
		invertDigitalInput4_Param,
		invertDigitalInput5_Param,
		invertDigitalOutput1_Param,
		invertDigitalOutput2_Param,
		invertRelay1_Param,
		invertRelay2_Param
	});
	
	
	//———— Motor Nameplate
	
	//[bfr] {Async} motor standard frequency
	motorStandartFrequency_Param = Legato::OptionParameter::createInstance(options.Hz50, options.motorStandardFrequency_Options, "Motor Standard Frequency", "MotorStandardFrequency");
	
	//[mpc] motor parameter choice (0=NominalPower,1=NominalCosinusPhi)
	motorParameterChoice_Param = Legato::OptionParameter::createInstance(options.NominalPower, options.motorParameterChoice_Options, "Motor Parameter Choice", "MotorParameterChoice");
	motorParameterChoice_Param->addEditCallback([this](){
		if(motorParameterChoice_Param->getValue() == options.NominalPower.getInt()){
			cosinusPhi_Param->setDisabled(true);
			nominalMotorPower_Param->setDisabled(false);
		}else if(motorParameterChoice_Param->getValue() == options.CosinusPhi.getInt()){
			cosinusPhi_Param->setDisabled(false);
			nominalMotorPower_Param->setDisabled(true);
		}else{
			cosinusPhi_Param->setDisabled(false);
			nominalMotorPower_Param->setDisabled(false);
		}
	});
	motorParameterChoice_Param->onEdit();
	
	//[cos] {Async} motor 1 cosinus phi (0.01 increments)
	cosinusPhi_Param = Legato::NumberParameter<double>::createInstance(0.0, "Cosinus Phi", "CosinusPhi");
	cosinusPhi_Param->allowNegatives(false);
	
	//[npr] {Async} nominal motor power (0.01 Watt increments)
	nominalMotorPower_Param = Legato::NumberParameter<double>::createInstance(0.0, "Nominal Motor Power", "NominalMotorPower");
	nominalMotorPower_Param->setUnit(Units::Power::KiloWatt);
	nominalMotorPower_Param->allowNegatives(false);
	
	//[uns] {Async} nominal motor voltage (1v increments)
	nominalMotorVoltage_Param = Legato::NumberParameter<int>::createInstance(0, "Nominal Motor Voltage", "NominalMotorVoltage");
	nominalMotorVoltage_Param->setUnit(Units::Voltage::Volt);
	nominalMotorVoltage_Param->allowNegatives(false);
	
	//[ncr] {Async} nominal motor current (0.01 Ampere increments)
	nominalMotorCurrent_Param = Legato::NumberParameter<double>::createInstance(0.0, "Nominal Motor Current", "NominalMotorCurrent");
	nominalMotorCurrent_Param->setUnit(Units::Current::Ampere);
	nominalMotorCurrent_Param->allowNegatives(false);
	
	//[frs] {Async} nominal motor frequency (0.1Hz increments)
	nominalMotorFrequency_Param = Legato::NumberParameter<double>::createInstance(0.0, "Nominal Motor Frequency", "NominalMotorFrequency");
	nominalMotorFrequency_Param->setUnit(Units::Frequency::Hertz);
	nominalMotorFrequency_Param->allowNegatives(false);
	
	//[nsp] {Async} nominal motor speed (rpm)
	nominalMotorSpeed_Param = Legato::NumberParameter<int>::createInstance(0, "Nominal Motor Speed", "NominalMotorSpeed");
	nominalMotorSpeed_Param->setSuffix("rpm");
	nominalMotorSpeed_Param->allowNegatives(false);
	
	//[ith] Motor Thermal Current (0.01 Ampere increments)
	motorThermalCurrent_Param = Legato::NumberParameter<double>::createInstance(0.0, "Motor Thermal Current", "MotorThermalCurrent");
	motorThermalCurrent_Param->setUnit(Units::Current::Ampere);
	motorThermalCurrent_Param->allowNegatives(false);
	
	//[tfr] Motor Maximum Frequency (0.1 Hz increments)
	motorMaximumFrequency_Param = Legato::NumberParameter<double>::createInstance(0.0, "Motor Maximum Frequency", "MotorMaximumFrequency");
	motorMaximumFrequency_Param->setUnit(Units::Frequency::Hertz);
	motorMaximumFrequency_Param->allowNegatives(false);
	
	motorNameplateParameters = Legato::ParameterGroup::createInstance("MotorNameplate", {
		motorStandartFrequency_Param,
		motorParameterChoice_Param,
		cosinusPhi_Param,
		nominalMotorPower_Param,
		nominalMotorVoltage_Param,
		nominalMotorCurrent_Param,
		nominalMotorFrequency_Param,
		nominalMotorSpeed_Param,
		motorThermalCurrent_Param,
		motorMaximumFrequency_Param
	});
	
	//———— Brake Logic
	
	//[blc] brake assignement
	brakeOutputAssignement_Param = Legato::OptionParameter::createInstance(options.NoDigitalOutput, options.digitalOutput_Options, "Brake Output Assignement", "BrakeOutputAssignement");
	brakeOutputAssignement_Param->addEditCallback([this](){
		bool brakeOutputUnassigned = brakeOutputAssignement_Param->getValue() == options.NoDigitalOutput.getInt();
		brakeMovementType_Param->setDisabled(brakeOutputUnassigned);
		brakeReleaseTime_Param->setDisabled(brakeOutputUnassigned);
		brakeEngageTime_Param->setDisabled(brakeOutputUnassigned);
		brakeReleaseCurrent_Param->setDisabled(brakeOutputUnassigned);
		brakeReleaseFrequency_Param->setDisabled(brakeOutputUnassigned);
		brakeEngageFrequency_Param->setDisabled(brakeOutputUnassigned);
	});
	brakeOutputAssignement_Param->onEdit();
	
	//[bst] movement type (0= Horizontal Movement, 1=Hoisting)
	brakeMovementType_Param = Legato::OptionParameter::createInstance(options.HorizontalMovement, options.brakeMovementType_Options, "Brake Movement Type", "BrakeMovementType");
	
	//[brt] brake release time
	brakeReleaseTime_Param = NumberParameter<double>::createInstance(0.1, "Brake Release Time", "BrakeReleaseTime");
	brakeReleaseTime_Param->setUnit(Units::Time::Second);
	brakeReleaseTime_Param->allowNegatives(false);
	
	//[bet] brake engage time
	brakeEngageTime_Param = NumberParameter<double>::createInstance(0.5, "Brake Engage Time", "BrakeEngageTime");
	brakeEngageTime_Param->setUnit(Units::Time::Second);
	brakeEngageTime_Param->allowNegatives(false);
	
	//[ibr] brake release current (default is nominal motor current)
	brakeReleaseCurrent_Param = NumberParameter<double>::createInstance(1.0, "Brake Release Current", "BrakeReleaseCurrent");
	brakeReleaseCurrent_Param->setUnit(Units::Current::Ampere);
	brakeReleaseCurrent_Param->allowNegatives(false);
	
	//[bir] brake release frequency (default is 0Hz)
	brakeReleaseFrequency_Param = NumberParameter<double>::createInstance(0.0, "Brake Release Frequency", "BrakeReleaseFrequency");
	brakeReleaseFrequency_Param->setUnit(Units::Frequency::Hertz);
	brakeReleaseFrequency_Param->allowNegatives(false);
	
	//[ben] brake engage frequency (default is 0Hz)
	brakeEngageFrequency_Param = NumberParameter<double>::createInstance(0.0, "Brake Engage Frequency", "BrakeEngageFrequency");
	brakeEngageFrequency_Param->setUnit(Units::Frequency::Hertz);
	brakeEngageFrequency_Param->allowNegatives(false);
	
	brakeLogicParameters = Legato::ParameterGroup::createInstance("BrakeLogic", {
		brakeOutputAssignement_Param,
		brakeMovementType_Param,
		brakeReleaseTime_Param,
		brakeEngageTime_Param,
		brakeReleaseCurrent_Param,
		brakeReleaseFrequency_Param,
		brakeEngageFrequency_Param
	});
	
	//———— Embedded Encoder
	
	//[eecp] embedded encoder etype (0=None, 1=AB, 2=SinCos)
	embeddedEncoderType_Param = Legato::OptionParameter::createInstance(options.EmbeddedEncoderTypeNone, options.embeddedEncoderType_Options, "Embedded Encoder Type", "EmbeddedEncoderType");
	embeddedEncoderType_Param->addEditCallback([this](){
		if(embeddedEncoderType_Param->getValue() == options.EmbeddedEncoderTypeAB.getInt()){
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
	
	//[eecv] embedded encoder supply voltage (5=5V, 12=12V, 24=24V)
	embeddedEncoderVoltage_Param = Legato::OptionParameter::createInstance(options.EmbeddedEncoder24V, options.embeddedEncoderVoltage_Options, "Embedded Encoder Voltage", "EmbeddedEncoderVoltage");
	
	//[epg] pulses per encoder revolution
	embeddedEncoderPulsesPerRevolution_Param = NumberParameter<int>::createInstance(0, "Embedded Encoder Pulses Per revolution", "EmbeddedEncoderPulsesPerRevolution");
	embeddedEncoderPulsesPerRevolution_Param->allowNegatives(false);
	
	//[eeri] embedded encoder revolution inversion (0=No, 1=Yes)
	embeddedEncoderInvertDirection_Param = Legato::BooleanParameter::createInstance(false, "Invert Embedded Encoder Direction", "InvertEmbeddedEncoderDirection");
	
	//[eenu] embedded encoder usage (0=None, 1=SpeedMonitoring, 2=SpeedRegulation, 3=SpeedReference)
	embeddedEncoderUsage_Param = Legato::OptionParameter::createInstance(options.EmbeddedEncoderNoUsage, options.embeddedEncoderUsage_Options, "Embedded Encoder Usage", "EmbeddedEncoderUsage");
	
	embeddedEncoderParameters = Legato::ParameterGroup::createInstance("EmbeddedEncoder", {
		embeddedEncoderType_Param,
		embeddedEncoderVoltage_Param,
		embeddedEncoderPulsesPerRevolution_Param,
		embeddedEncoderInvertDirection_Param,
		embeddedEncoderUsage_Param
	});
	
	//———— Motor Control
	
	//[ctt] Motor Control Type
	motorControlType_Param = Legato::OptionParameter::createInstance(options.SensorlessFullFlux, options.motorControlType_Options, "Motor Control Type", "MotorControlType");
	
	velocityLimitRPM_Param = Legato::NumberParameter<int>::createInstance(1000, "Velocity Limit", "VelocityLimit");
	velocityLimitRPM_Param->setSuffix("rpm");
	velocityLimitRPM_Param->allowNegatives(false);
	
	accelerationRampTime_Param = Legato::NumberParameter<double>::createInstance(3.0, "Acceleration Ramp Time", "AccelerationRampTime");
	accelerationRampTime_Param->setUnit(Units::Time::Second);
	accelerationRampTime_Param->allowNegatives(false);
	
	decelerationRampTime_Param = Legato::NumberParameter<double>::createInstance(3.0, "Deceleration Ramp Time", "DecelerationRampTime");
	decelerationRampTime_Param->setUnit(Units::Time::Second);
	decelerationRampTime_Param->allowNegatives(false);
	
	switchingFrequency_Param = Legato::NumberParameter<double>::createInstance(16, "Switching Frequency", "SwitchingFrequeny");
	switchingFrequency_Param->setUnit(Units::Frequency::Kilohertz);
	switchingFrequency_Param->allowNegatives(false);
	
	motorControlParameters = Legato::ParameterGroup::createInstance("MotorControl", {
		motorControlType_Param,
		velocityLimitRPM_Param,
		accelerationRampTime_Param,
		decelerationRampTime_Param,
		switchingFrequency_Param
	});
	
	//———— Analog IO Configuration
	
	//[AI1T]
	analogInput1Type_Param = Legato::OptionParameter::createInstance(options.AnalogInputTypeVoltage, options.analogInput1Type_Options, "Analog Input 1 Type", "AnalogInput1Type");
	
	//[AI2T]
	analogInput2Type_Param = Legato::OptionParameter::createInstance(options.AnalogInputTypeVoltage, options.analogInput2Type_Options, "Analog Input 2 Type", "AnalogInput2Type");
	
	//[CrL1]
	analogInputMinCurrent_Param = NumberParameter<double>::createInstance(4.0, "Analog Input Minimum Current", "AnalogInputMinimumCurrentValue");
	analogInputMinCurrent_Param->setUnit(Units::Current::Milliampere);
	analogInputMinCurrent_Param->allowNegatives(false);
	
	//[CrH1]
	analogInputMaxCurrent_Param = NumberParameter<double>::createInstance(20.0, "Analog Input Maximum Current", "AnalogInputMaximumCurrentValue");
	analogInputMaxCurrent_Param->setUnit(Units::Current::Milliampere);
	analogInputMaxCurrent_Param->allowNegatives(false);
	
	//[UIL1] = [UIL2] if voltage is selected
	analogInputMinVoltage_Param = NumberParameter<double>::createInstance(0.0, "Analog Input Minimum Voltage", "AnalogInputMinimumVoltageValue");
	analogInputMinVoltage_Param->setUnit(Units::Voltage::Volt);
	analogInputMinVoltage_Param->allowNegatives(false);
	
	//[UIH1] = [UIH2] if voltage is selected
	analogInputMaxVoltage_Param = NumberParameter<double>::createInstance(10.0, "Analog Input Maximum Voltage", "AnalogInputMaximumVoltageValue");
	analogInputMaxVoltage_Param->setUnit(Units::Voltage::Volt);
	analogInputMaxVoltage_Param->allowNegatives(false);
	
	analogIoConfigParameters = Legato::ParameterGroup::createInstance("AnalogIOConfig", {
		analogInput1Type_Param,
		analogInput2Type_Param,
		analogInputMinCurrent_Param,
		analogInputMaxCurrent_Param,
		analogInputMinVoltage_Param,
		analogInputMaxVoltage_Param
	});
	
	//———— Digital IO Configuration
	
	//[LAF] Stop forward limit assignement
	forwardLimitSignal_Param = Legato::OptionParameter::createInstance(options.NoDigitalInput, options.digitalInput_Options, "Forward Limit Signal", "ForwardLimitSignal");
	
	//[LAR] Stop Reverse limit assignement
	reverseLimitSignal_Param = Legato::OptionParameter::createInstance(options.NoDigitalInput, options.digitalInput_Options, "Reverse Limit Signal", "ReverseLimitSignal");
	
	digitalIoConfigParameters = Legato::ParameterGroup::createInstance("DigitalIOConfig", {
		forwardLimitSignal_Param,
		reverseLimitSignal_Param
	});
	
	
	

	

	
	

}

void ATV340::configureProcessData(){
	
	if(EtherCatFieldbus::isRunning()) return;
	
	rxPdoAssignement.clear();
	txPdoAssignement.clear();
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	axis->processDataConfiguration.enableFrequencyMode();
	axis->processDataConfiguration.frequencyActualValue = false;
	
	if(pdo_digitalIn->getValue()) 		txPdoAssignement.addEntry(0x2016, 0x2, 16, "Logic Inputs Physical Image", &logicInputs);
	if(pdo_digitalOut->getValue()) 		rxPdoAssignement.addEntry(0x2016, 0xD, 16, "Logic Outputs States", &logicOutputs);
	if(pdo_motorEffort->getValue()) 		txPdoAssignement.addEntry(0x2002, 0xC, 16, "MotorPower", &motorEffort);
	if(pdo_motorVelocity->getValue())	axis->processDataConfiguration.frequencyActualValue = true;
	if(pdo_readAnalogIn1->getValue())	txPdoAssignement.addEntry(0x2016, 0x21, 16, "AI1 standardized value", &analogInput1);
	if(pdo_readAnalogIn2->getValue()) 	txPdoAssignement.addEntry(0x2016, 0x22, 16, "AI2 standardized value", &analogInput2);
	
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
	if(!writeSDO_U16(0x2036, 0xE, ref1configuration, "Ref 1 Configuration")) {
		return Logger::error("failed to set velocity reference");
	}

	//[rfc] Frequency Switching assignement
	uint16_t frequencySwitchingAssigment = 96; //96 = Fixed Reference Frequency Channel 1
	if(!writeSDO_U16(0x2036, 0xC, frequencySwitchingAssigment, "Frequency Switching Assignement")) {
		return Logger::error("failed to set reference switching assignement");
	}
	
	//[chcf] Control Mode
	uint16_t controlMode = 1; //1=combined mode, ethercat has complete control of the drive
	if(!writeSDO_U16(0x2036, 0x2, controlMode, "Control Mode")) {
		return Logger::error("failed to set control mode");
	}
	
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
	motor->actuatorProcessData.b_isEmergencyStopActive = b_estop;
	if(isStateNone()) motor->state = DeviceState::OFFLINE;
	else if(!axis->hasVoltage()) motor->state = DeviceState::NOT_READY;
	else if(b_isEnabled) motor->state = DeviceState::ENABLED;
	else if(b_isReady) motor->state = DeviceState::READY;
	else motor->state = DeviceState::NOT_READY;
	
	//effort is reported in 1% increments
	double effort = double(motorEffort) / 100.0;
	motor->actuatorProcessData.effortActual = effort;
	*load_Value = effort;
	
	//actual control velocity
	*velocity_Value = double(axis->getActualFrequency()) / 60.0;
		
	bool DI1 = logicInputs & (0x1 << 0);
	bool DI2 = logicInputs & (0x1 << 1);
	bool DI3 = logicInputs & (0x1 << 2);
	bool DI4 = logicInputs & (0x1 << 3);
	bool DI5 = logicInputs & (0x1 << 4);
	
	*digitalInput1_Signal = invertDigitalInput1_Param->getValue() ? !DI1 : DI1;
	*digitalInput2_Signal = invertDigitalInput2_Param->getValue() ? !DI2 : DI2;
	*digitalInput3_Signal = invertDigitalInput3_Param->getValue() ? !DI3 : DI3;
	*digitalInput4_Signal = invertDigitalInput4_Param->getValue() ? !DI4 : DI4;
	*digitalInput5_Signal = invertDigitalInput5_Param->getValue() ? !DI5 : DI5;
	
	//for standardized values, the value seems to be in 0.01% increments
	//to normalize the value to a 0-1 range we divide by 10000
	*analogInput1_value = double(analogInput1) / 10000.0;
	*analogInput2_value = double(analogInput2) / 10000.0;
}

//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void ATV340::writeOutputs() {
	
	long long now_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	
	//handle enabling & disabling
	if(motor->actuatorProcessData.b_disable){
		motor->actuatorProcessData.b_disable = false;
		b_waitingForEnable = false;
		axis->disable();
	}
	else if(motor->actuatorProcessData.b_enable){
		if(axis->hasFault()) axis->doFaultReset();
		else {
			motor->actuatorProcessData.b_enable = false;
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
	
	if(relaisOutput1_Pin->isConnected()) relaisOutput1_Pin->copyConnectedPinValue();
	if(relaisOutput2_Pin->isConnected()) relaisOutput2_Pin->copyConnectedPinValue();
	if(digitalOutput1_Pin->isConnected()) digitalOutput1_Pin->copyConnectedPinValue();
	if(digitalOutput2_Pin->isConnected()) digitalOutput2_Pin->copyConnectedPinValue();
	
	logicOutputs = 0x0;
	if(*relaisOutput1_Signal != invertRelay1_Param->getValue())			logicOutputs |= 0x1 << 0;
	if(*relaisOutput2_Signal != invertRelay2_Param->getValue())			logicOutputs |= 0x1 << 1;
	if(*digitalOutput1_Signal != invertDigitalOutput1_Param->getValue())	logicOutputs |= 0x1 << 8;
	if(*digitalOutput2_Signal != invertDigitalOutput2_Param->getValue())	logicOutputs |= 0x1 << 9;
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}

//============================= SAVING AND LOADING DEVICE DATA ============================

bool ATV340::onSerialization(){
	assert(false && "Cannot save or load this yet");
	/*
	using namespace tinyxml2;
	
	if(!pdoConfigParameters.save(xml)) return false;
	if(!motorNameplateParameters.save(xml)) return false;
	if(!brakeLogicParameters.save(xml)) return false;
	if(!embeddedEncoderParameters.save(xml)) return false;
	if(!motorControlParameters.save(xml)) return false;
	if(!analogIoConfigParameters.save(xml)) return false;
	if(!digitalIoConfigParameters.save(xml)) return false;
	*/
	return true;
}

bool ATV340::onDeserialization(){
	assert(false && "Cannot save or load this yet");
	/*
	using namespace tinyxml2;
	
	if(!pdoConfigParameters.load(xml)) return false;
	if(!motorNameplateParameters.load(xml)) return false;
	if(!brakeLogicParameters.load(xml)) return false;
	if(!embeddedEncoderParameters.load(xml)) return false;
	if(!motorControlParameters.load(xml)) return false;
	//if(!analogIoConfigParameters.load(xml)) return false;
	//if(!digitalIoConfigParameters.load(xml)) return false;
	
	for(auto parameter : pdoConfigParameters.get()) parameter->onEdit();
	for(auto parameter : motorNameplateParameters.get()) parameter->onEdit();
	for(auto parameter : brakeLogicParameters.get()) parameter->onEdit();
	for(auto parameter : embeddedEncoderParameters.get()) parameter->onEdit();
	for(auto parameter : motorControlParameters.get()) parameter->onEdit();
	for(auto parameter : digitalIoConfigParameters.get()) parameter->onEdit();
	for(auto parameter : analogIoConfigParameters.get()) parameter->onEdit();
	*/
	return true;
}
























void ATV340::configureDrive(){
	
	if(isOffline()) {
		Logger::warn("Can't upload configuration when device is offline");
		return;
	}
	else if(isStateOperational()) {
		Logger::warn("Can't upload configuration while device is operational and network running");
		return;
	}
	else Logger::info("Starting device configuration upload");
	
	std::thread driveConfigurationHandler = std::thread([this](){
		
		//———— Unit Scaling Retrieval
		uint16_t powerScalingCode;
		if(!readSDO_U16(0x2044, 0x3, powerScalingCode, "Power Scaling")) return;
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
		if(!readSDO_U16(0x2044, 0x2, currentScalingCode, "Current Scaling")) return;
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
		uint16_t motorStandard = motorStandartFrequency_Param->getValue();
		if(!writeSDO_U16(0x2000, 0x10, motorStandard, "Motor Standard Frequency")) return;
		
		
		//———— Motor Nameplate
		
		//[mpc] motor parameter choice (0=NominalPower,1=NominalCosinusPhi)
		uint16_t motorParameterChoice = motorParameterChoice_Param->getValue();
		if(!writeSDO_U16(0x2042, 0xF, motorParameterChoice, "Motor Parameter Choice")) return;
		
		if(motorParameterChoice == options.NominalPower.getInt()){
			//[npr] {Async} nominal motor power
			uint16_t nominalMotorPower = nominalMotorPower_Param->value / powerScaling;
			if(!writeSDO_U16(0x2042, 0xE, nominalMotorPower, "Nominal Motor Power")) return;
		}else if(motorParameterChoice == options.CosinusPhi.getInt()){
			//[cos] {Async} motor 1 cosinus phi (0.01 increments)
			uint16_t motor1CosinusPhi = cosinusPhi_Param->value * 100.0;
			if(!writeSDO_U16(0x2042, 0x7, motor1CosinusPhi, "Motor Cosinus Phi")) return;
		}
		
		//[uns] {Async} nominal motor voltage (1v increments)
		uint16_t nominalMotorVoltage = nominalMotorVoltage_Param->value;
		if(!writeSDO_U16(0x2042, 0x2, nominalMotorVoltage, "Motor Nominal Voltage")) return;
		
		//[ncr] {Async} nominal motor current
		uint16_t nominalMotorCurrent = nominalMotorCurrent_Param->value / currentScaling;
		if(!writeSDO_U16(0x2042, 0x4, nominalMotorCurrent, "Motor Nominal Current")) return;
		
		//[frs] {Async} nominal motor frequency (0.1Hz increments)
		uint16_t nominalMotorFrequency = nominalMotorFrequency_Param->value * 10.0;
		if(!writeSDO_S16(0x2042, 0x3, nominalMotorFrequency, "Motor Nominal Frequency")) return;
		
		//[nsp] {Async} nominal motor speed (rpm)
		uint16_t nominalMotorSpeed = nominalMotorSpeed_Param->value;
		if(!writeSDO_U16(0x2042, 0x5, nominalMotorSpeed, "Nominal Motor Speed")) return;
		
		//[ith] Motor Thermal Current
		uint16_t motorThermalCurrent = motorThermalCurrent_Param->value / currentScaling;
		if(!writeSDO_U16(0x2042, 0x17, motorThermalCurrent, "Motor Thermal Current")) return;
		
		
		//———— Maximum Motor Frequency
		
		//[tfr] Motor Maximum Frequency (0.1 Hz increments)
		uint16_t motorMaximumFrequency = motorMaximumFrequency_Param->value * 10.0;
		if(!writeSDO_U16(0x2001, 0x4, motorMaximumFrequency, "Motor Maximum Frequency")) return;

		//———— Brake Logic Control
		
		//[blc] brake assignement
		uint16_t brakeAssignement = brakeOutputAssignement_Param->getValue();
		if(!writeSDO_U16(0x2046, 0x2, brakeAssignement, "Brake Output Assignement")) return;
		
		//[bst] movement type
		uint16_t brakeMovementType = brakeMovementType_Param->getValue();
		if(!writeSDO_U16(0x2046, 0x9, brakeMovementType, "Brake Movement Type")) return;
		
		//[brt] brake release time (int 0.01s increments)
		uint16_t brakeReleaseTime = brakeReleaseTime_Param->value * 100.0;
		if(!writeSDO_U16(0x2046, 0x5, brakeReleaseTime, "Brake Release Time")) return;

		//[bet] brake engage time
		uint16_t brakeEngageTime = brakeEngageTime_Param->value * 100.0;
		if(!writeSDO_U16(0x2046, 0x6, brakeEngageTime, "Brake Engage Time")) return;
		
		//[ibr] brake release current (default is nominal motor current)
		uint16_t brakeReleaseCurrent = brakeReleaseCurrent_Param->value / currentScaling;
		if(!writeSDO_U16(0x2046, 0x7, brakeReleaseCurrent, "Brake Release Current")) return;
		
		//[bir] brake release frequency (default is 0Hz, in 0.1Hz increments)
		uint16_t brakeReleaseFrequency = brakeReleaseFrequency_Param->value * 10.0;
		if(!writeSDO_U16(0x2046, 0xD, brakeReleaseFrequency, "Brake Release Frequency")) return;
		
		//[ben] brake engage frequency (default is 0Hz, in 0.1Hz increments)
		uint16_t brakeEngageFrequency = brakeEngageFrequency_Param->value * 10.0;
		if(!writeSDO_U16(0x2046, 0x4, brakeEngageFrequency, "Brake Release Engage Frequency")) return;
		
		
		//TODO: encoder verification
		
		
		//———— Embedded Encoder
		
		//[eecp] embedded encoder etype (0=None, 1=AB, 2=SinCos)
		uint16_t encoderType = embeddedEncoderType_Param->getValue();
		if(!writeSDO_U16(0x201A, 0x47, encoderType, "Embedded Encoder Type")) return;
		
		if(encoderType == options.EmbeddedEncoderTypeAB.getInt()){
			//[eecv] embedded encoder supply voltage (5=5V, 12=12V, 24=24V)
			uint16_t encoderSupplyVoltage = embeddedEncoderVoltage_Param->getValue();
			if(!writeSDO_U16(0x201A, 0x50, encoderSupplyVoltage, "Encoder Supply Voltage")) return;
			
			//[epg] pulses per encoder revolution
			uint16_t pulsesPerEncoderRevolution = embeddedEncoderPulsesPerRevolution_Param->value;
			if(!writeSDO_U16(0x201A, 0x48, pulsesPerEncoderRevolution, "Pulses Per Encoder Revolution")) return;
			
			//[eeri] emebedded encoder revolution inversion (0=No, 1=Yes)
			uint16_t encoderinvertion = embeddedEncoderInvertDirection_Param->getValue() ? 1 : 0;
			if(!writeSDO_U16(0x201A, 0x4F, encoderinvertion, "Embedded Encoder Invert Direction")) return;
			
			//[eenu] embedded encoder usage (0=None, 1=SpeedMonitoring, 2=SpeedRegulation, 3=SpeedReference)
			uint16_t embeddedEncoderUsage = embeddedEncoderUsage_Param->getValue();
			if(!writeSDO_U16(0x201A, 0x4E, embeddedEncoderUsage, "Embedded Encoder Usage")) return;
		}
			
		//———— Motor Control
		
		//[ctt] Motor Control Type
		uint16_t motorControlType = motorControlType_Param->getValue();
		//0 = Sensorless flux vector V (no encoder feedback, multiple identical motors supported)
		//2 = Full flux Vector (encoder feedback necessary)
		if(!writeSDO_U16(0x2042, 0x8, motorControlType, "Motor Control Type")) return;
		
		//[lsp] low speed (0.1Hz Increments)
		uint16_t lowSpeed = 0;
		if(!writeSDO_U16(0x2001, 0x6, lowSpeed, "Low Speed")) return;
		
		//[hsp] high speed (0.1Hz Increments)
		uint16_t highSpeed = 500;
		if(!writeSDO_U16(0x2001, 0x5, highSpeed, "High Speed")) return;
		
		//[inr] Ramp Time Increment: 0.01 second increments
		uint16_t rampIncrement = 0; //(0=0.01, 1=0.1, 2=1.0)
		if(!writeSDO_U16(0x203C, 0x15, rampIncrement, "Ramp Time Increment")) return;
		
		//[acc] acceleration ramp time (0.01 second increments)
		uint16_t accelerationRampTime = accelerationRampTime_Param->value * 100;
		if(!writeSDO_U16(0x203C, 0x2, accelerationRampTime, "Acceleration Ramp Time")) return;
		
		//[dec] deceleration ramp time (0.01 second increments)
		uint16_t decelerationRampTime = decelerationRampTime_Param->value * 100;
		if(!writeSDO_U16(0x203C, 0x3, decelerationRampTime, "Deceleration Ramp Time")) return;
		
		//[sfr] switching frequency (0.1KHz increments)
		uint16_t switchingFrequency = switchingFrequency_Param->value * 10.0;
		if(!writeSDO_U16(0x2001, 0x3, switchingFrequency, "Switching Frequency")) return;
		
		
		//———— IO Config
		
		//[AI1T]
		uint16_t analogInput1Type = analogInput1Type_Param->getValue();
		if(!writeSDO_S16(0x200E, 0x3, analogInput1Type, "Analog Input 1 Type")) return;
		
		//[AI2T]
		uint16_t analogInput2Type = analogInput2Type_Param->getValue();
		if(!writeSDO_S16(0x200E, 0x4, analogInput2Type, "Analog Input 2 Type")) return;
		
		if(analogInput1Type == options.AnalogInputTypeCurrent.getInt()){
			uint16_t analogMinCurrent = analogInputMinCurrent_Param->value * 10.0;
			uint16_t analogMaxCurrent = analogInputMaxCurrent_Param->value * 10.0;
			//[CrL1]
			if(!writeSDO_U16(0x200E, 0x21, analogMinCurrent, "Analog Input 1 Min Current")) return;
			//[CrH1]
			if(!writeSDO_U16(0x200E, 0x2B, analogMaxCurrent, "Analog Input 1 Max Current")) return;
		}
		
		if(analogInput1Type == options.AnalogInputTypeVoltage.getInt() || analogInput2Type == options.AnalogInputTypeVoltage.getInt()){
			uint16_t analog2MinVoltage = analogInputMinVoltage_Param->value * 10.0;
			uint16_t analog2MaxVoltage = analogInputMaxVoltage_Param->value * 10.0;
			//[UIL1]
			if(!writeSDO_U16(0x200E, 0xD, analog2MinVoltage, "Analog Input 1 Min Voltage")) return;
			//[UIH1]
			if(!writeSDO_U16(0x200E, 0x17, analog2MaxVoltage, "Analog Input 1 Max Voltage")) return;
		}
		
		//[LAF] Stop forward limit assignement
		uint16_t forwardLimitSignal = forwardLimitSignal_Param->getValue();
		if(!writeSDO_U16(0x2056, 0x2, forwardLimitSignal, "Forward Limit Signal")) return;
		
		//[LAR] Stop Reverse limit assignement
		uint16_t reverseLimitSignal = reverseLimitSignal_Param->getValue();
		if(!writeSDO_U16(0x2056, 0x3, reverseLimitSignal, "Reverse Limit Signal")) return;

		if(!saveToEEPROM()) Logger::error("Failed to save configuration to EEPROM");
		else Logger::info("Configuration uploaded and saved to EEPROM");
			
	});
	driveConfigurationHandler.detach();
}

void ATV340::startStandardTuning(){
	
	if(isOffline()) Logger::warn("Can't start motor Tuning while drive is offline");
	else if(!isStateOperational()) return Logger::warn("Can't start motor Tuning while the drive is not operational and the network running");
	else if(!motor->isEnabled()) return Logger::warn("Can't start motor Tuning while the drive is not enabled");
	else Logger::info("Starting Motor Tuning");
	
	std::thread motorTuningHandler = std::thread([this](){
		
		//———— Standstill motor tune after motor parameter assignement
		
		//[tunt] tuning type
		uint16_t tuningType = 2; //2 = Standart; 3 = Rotation
		if(!writeSDO_U16(0x2042, 0x1B, tuningType, "Tuning Type")) return;
		
		//[tun] autotuning (0=NoAction; 1=ApplyAutotuning; 2=EraseAutotuning)
		uint16_t autotuning = 1;
		if(!writeSDO_U16(0x2042, 0x9, autotuning, "Autotuning Control")) return;
		
		Logger::info("Started Autotuning");
		
		Timing::Timer autotuneTimer;
		autotuneTimer.setExpirationSeconds(5.0);
		
		while(true){
			//[tus] autotuning status (0=NotDone; 1=Pending; 2=InProgress; 3=Fail; 4=Done)
			uint16_t autotuningStatus;
			if(readSDO_U16(0x2042, 0xA, autotuningStatus)){
				switch(autotuningStatus){
					case 0: Logger::info("Autotuning Not Done"); return;
					case 1: Logger::info("Autotuning Pending"); break;
					case 2: break;
					case 3: Logger::error("Autotuning Failed"); return;
					case 4: Logger::info("Autotuning Succeeded !"); return;
				}
			}
			if(autotuneTimer.isExpired()) {
				Logger::warn("auto tune timed out (took more than 5 seconds)");
				return;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	
	});
	motorTuningHandler.detach();
}

void ATV340::startRotationTuning(){
	
	if(isOffline()) return Logger::warn("Can't start motor Tuning while drive is offline");
	else if(!isStateOperational()) return Logger::warn("Can't start motor Tuning while the drive is not operational and the network running");
	else if(!motor->isEnabled()) return Logger::warn("Can't start motor Tuning while the drive is not enabled");
	else Logger::info("Starting Motor Tuning");
	
	std::thread motorTuningHandler = std::thread([this](){
		
		//———— Standstill motor tune after motor parameter assignement
		
		//[LAC] access level
		uint16_t accessLevel = 3; //0 = basic; 1 = Standard; //3 = Expert
		if(!writeSDO_U16(0x2000, 0x7, accessLevel, "Access Level")) return;
		
		//[tunt] tuning type
		uint16_t tuningType = 3; //2 = Standart; 3 = Rotation
		if(!writeSDO_U16(0x2042, 0x1B, tuningType, "Tuning Type")) return;
		
		//[tun] autotuning (0=NoAction; 1=ApplyAutotuning; 2=EraseAutotuning)
		
		//erase autotunins
		uint16_t autotuning = 2;
		if(!writeSDO_U16(0x2042, 0x9, autotuning, "Autotuning Control")) return;
		
		//apply autotuning
		autotuning = 1;
		if(!writeSDO_U16(0x2042, 0x9, autotuning, "Autotuning Control")) return;
		
		Logger::info("Started Autotuning");
		
		Timing::Timer autotuneTimer;
		autotuneTimer.setExpirationSeconds(5.0);
		
		while(true){
			//[tus] autotuning status (0=NotDone; 1=Pending; 2=InProgress; 3=Fail; 4=Done)
			uint16_t autotuningStatus;
			if(readSDO_U16(0x2042, 0xA, autotuningStatus)){
				switch(autotuningStatus){
					case 0: Logger::info("Autotuning Not Done"); return;
					case 1: Logger::info("Autotuning Pending"); break;
					case 2: break;
					case 3: Logger::error("Autotuning Failed"); return;
					case 4: Logger::info("Autotuning Succeeded !"); return;
				}
			}
			if(autotuneTimer.isExpired()) {
				Logger::warn("auto tune timed out (took more than 5 seconds)");
				return;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	
	});
	motorTuningHandler.detach();
}

bool ATV340::saveToEEPROM(){
	
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
}

void ATV340::resetFactorySettings(){
	if(isOffline()) return;
	else Logger::info("Resetting drive to factory settings");
	
	std::thread factoryResetHandler([this](){
		uint16_t extendedCommandWord = 0x1;
		bool success = writeSDO_U16(0x2037, 0x5, extendedCommandWord);
	});
	factoryResetHandler.detach();
}
