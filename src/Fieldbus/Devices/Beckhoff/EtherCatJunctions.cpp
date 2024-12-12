#include <pch.h>

#include "EtherCatJunctions.h"
#include "Fieldbus/EtherCatFieldbus.h"

void CU1124::onDisconnection() {}
void CU1124::onConnection() {}
void CU1124::initialize() {}
bool CU1124::startupConfiguration() { return true; }
void CU1124::readInputs() {}
void CU1124::writeOutputs(){}
bool CU1124::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool CU1124::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void CU1128::onDisconnection() {}
void CU1128::onConnection() {}
void CU1128::initialize() {}
bool CU1128::startupConfiguration() { return true; }
void CU1128::readInputs() {}
void CU1128::writeOutputs(){}
bool CU1128::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool CU1128::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EK1100::onDisconnection() {}
void EK1100::onConnection() {}
void EK1100::initialize() {}
bool EK1100::startupConfiguration() { return true; }
void EK1100::readInputs() {}
void EK1100::writeOutputs(){}
bool EK1100::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EK1100::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EK1122::onDisconnection() {}
void EK1122::onConnection() {}
void EK1122::initialize() {}
bool EK1122::startupConfiguration() { return true; }
void EK1122::readInputs() {}
void EK1122::writeOutputs(){}
bool EK1122::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EK1122::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EK1521_0010::onDisconnection() {}
void EK1521_0010::onConnection() {}
void EK1521_0010::initialize() {}
bool EK1521_0010::startupConfiguration() { return true; }
void EK1521_0010::readInputs() {}
void EK1521_0010::writeOutputs(){}
bool EK1521_0010::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EK1521_0010::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void CU1521_0010::onDisconnection() {}
void CU1521_0010::onConnection() {}
void CU1521_0010::initialize() {}
bool CU1521_0010::startupConfiguration() { return true; }
void CU1521_0010::readInputs() {}
void CU1521_0010::writeOutputs(){}
bool CU1521_0010::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool CU1521_0010::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EL2008::onDisconnection() {}
void EL2008::onConnection() {}
void EL2008::initialize() {
	rxPdoAssignement.addNewModule(0x1600);
	for(int i = 0; i < 8; i++){
		char name[32];
		snprintf(name, 32, "Channel %i", i + 1);
		rxPdoAssignement.addEntry(0x1600 + i, 0x1, 1, name, &outputs[i]);
	}
	for(int i = 0; i < 8; i++){
		char pinName[16];
		int inputNumber = i + 1;
		snprintf(pinName, 16, "DO%i", inputNumber);
		auto pinValue = std::make_shared<bool>(false);
		auto pin = std::make_shared<NodePin>(pinValue, NodePin::Direction::NODE_INPUT, pinName);
		pinValues.push_back(pinValue);
		addNodePin(pin);
		char parameterName[64];
		snprintf(parameterName, 64, "Invert DO%i", inputNumber);
		char parameterSaveName[64];
		snprintf(parameterSaveName, 64, "InvertDO%i", inputNumber);
		auto inversionParam = BooleanParameter::make(false, parameterName, parameterSaveName);
		signalInversionParams.push_back(inversionParam);
	}
}
bool EL2008::startupConfiguration() { return true; }
void EL2008::readInputs() {}
void EL2008::writeOutputs(){
	for(int i = 0; i < 8; i++){
		if(getInputPins()[i]->isConnected()) getInputPins()[i]->copyConnectedPinValue();
		outputs[i] = *pinValues[i];
		if(signalInversionParams[i]->value) outputs[i] = !outputs[i];
	}
	rxPdoAssignement.pushDataTo(identity->outputs);
}
bool EL2008::saveDeviceData(tinyxml2::XMLElement* xml) {
	for(int i = 0; i < 8; i++) signalInversionParams[i]->save(xml);
	return true;
}
bool EL2008::loadDeviceData(tinyxml2::XMLElement* xml) {
	for(int i = 0; i < 8; i++) signalInversionParams[i]->load(xml);
	return true;
}


void EL1008::onDisconnection() {}
void EL1008::onConnection() {}
void EL1008::initialize() {
	txPdoAssignement.addNewModule(0x1600);
	for(int i = 0; i < 8; i++){
		char name[32];
		snprintf(name, 32, "Channel %i", i + 1);
		txPdoAssignement.addEntry(0x1A00 + i, 0x1, 1, name, &inputs[i]);
	}
	for(int i = 0; i < 8; i++){
		char pinName[16];
		int inputNumber = i + 1;
		snprintf(pinName, 16, "DI%i", inputNumber);
		auto pinValue = std::make_shared<bool>(false);
		auto pin = std::make_shared<NodePin>(pinValue, NodePin::Direction::NODE_OUTPUT, pinName);
		pinValues.push_back(pinValue);
		addNodePin(pin);
		char parameterName[64];
		snprintf(parameterName, 64, "Invert DI%i", inputNumber);
		char parameterSaveName[64];
		snprintf(parameterSaveName, 64, "InvertDI%i", inputNumber);
		auto inversionParam = BooleanParameter::make(false, parameterName, parameterSaveName);
		signalInversionParams.push_back(inversionParam);
	}
}
bool EL1008::startupConfiguration() { return true; }
void EL1008::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	for(int i = 0; i < 8; i++){
		*pinValues[i] = signalInversionParams[i]->value ? !inputs[i] : inputs[i];
	}
}
void EL1008::writeOutputs(){}
bool EL1008::saveDeviceData(tinyxml2::XMLElement* xml) {
	for(int i = 0; i < 8; i++) signalInversionParams[i]->save(xml);
	return true;
}
bool EL1008::loadDeviceData(tinyxml2::XMLElement* xml) {
	for(int i = 0; i < 8; i++) signalInversionParams[i]->load(xml);
	return true;
}



void EL2624::onDisconnection() {}
void EL2624::onConnection() {}
void EL2624::initialize() {
	rxPdoAssignement.addNewModule(0x1600);
	for(int i = 0; i < 4; i++){
		char name[32];
		snprintf(name, 32, "Channel %i", i + 1);
		rxPdoAssignement.addEntry(0x1600 + i, 0x1, 1, name, &outputs[i]);
	}
	for(int i = 0; i < 4; i++){
		char pinName[16];
		int inputNumber = i + 1;
		snprintf(pinName, 16, "R%i", inputNumber);
		auto pinValue = std::make_shared<bool>(false);
		auto pin = std::make_shared<NodePin>(pinValue, NodePin::Direction::NODE_INPUT, pinName);
		pinValues.push_back(pinValue);
		addNodePin(pin);
		char parameterName[64];
		snprintf(parameterName, 64, "Invert R%i", inputNumber);
		char parameterSaveName[64];
		snprintf(parameterSaveName, 64, "InvertR%i", inputNumber);
		auto inversionParam = BooleanParameter::make(false, parameterName, parameterSaveName);
		signalInversionParams.push_back(inversionParam);
	}
}
bool EL2624::startupConfiguration() { return true; }
void EL2624::readInputs() {}
void EL2624::writeOutputs(){
	for(int i = 0; i < 4; i++){
		if(getInputPins()[i]->isConnected()) getInputPins()[i]->copyConnectedPinValue();
		outputs[i] = *pinValues[i];
		if(signalInversionParams[i]->value) outputs[i] = !outputs[i];
	}
	rxPdoAssignement.pushDataTo(identity->outputs);
}
bool EL2624::saveDeviceData(tinyxml2::XMLElement* xml) {
	for(int i = 0; i < 4; i++) signalInversionParams[i]->save(xml);
	return true;
}
bool EL2624::loadDeviceData(tinyxml2::XMLElement* xml) {
	for(int i = 0; i < 4; i++) signalInversionParams[i]->load(xml);
	return true;
}



/*
class EL2624 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL2624, "EL2624 4x Relay Output", "EL1008", "Beckhoff", "I/O", 0x2, 0xa403052)
	//rxPdo
	bool outputs[4] = {0,0,0,0};
	std::vector<std::shared_ptr<bool>> pinValues;
	std::vector<std::shared_ptr<NodePin>> pins;
	std::vector<BoolParam> signalInversionParams;
};
*/

void EL5001::onDisconnection() {
	encoder->state = DeviceState::OFFLINE;
}
void EL5001::onConnection() {}
void EL5001::initialize() {
	
	encoder = std::make_shared<SsiEncoder>(std::dynamic_pointer_cast<EL5001>(shared_from_this()));
	encoderPin->assignData(std::static_pointer_cast<MotionFeedbackInterface>(encoder));
	encoder->feedbackConfig.b_supportsVelocityFeedback = true;
	encoder->feedbackConfig.b_supportsPositionFeedback = true;
	
	centerOnZero_Param = BooleanParameter::make(false, "Center On Zero", "CenterOnZero");
	invertDirection_Param = BooleanParameter::make(false, "Invert Direction", "InvertDirection");
	
	hasResetSignal_Param = BooleanParameter::make(true, "Has position reset signal", "HasResetSignal");
	resetSignalTime_Param = NumberParameter<double>::make(100.0, "Reset Time", "ResetTime", "%.1f", Units::Time::Millisecond, false);
	
	ssiFrameSize = NumberParameter<int>::make(24, "SSI Frame Size", "SSIFrameSize");
	ssiFrameSize->addEditCallback([this](){
		if(ssiFrameSize->value < 0) ssiFrameSize->overwrite(0);
		else if(ssiFrameSize->value > 32) ssiFrameSize->overwrite(32);
		updateSSIFrameFormat();
	});
	multiturnResolution = NumberParameter<int>::make(12, "Multiturn resolution", "MultiturnResolution");
	multiturnResolution->addEditCallback([this](){
		if(multiturnResolution->value < 0) multiturnResolution->overwrite(0);
		else if(multiturnResolution->value > 32) multiturnResolution->overwrite(32);
		updateSSIFrameFormat();
		updateEncoderWorkingRange();
	});
	singleturnResolution = NumberParameter<int>::make(12, "Singleturn resolution", "SingleturnResolution");
	singleturnResolution->addEditCallback([this](){
		if(singleturnResolution->value < 0) singleturnResolution->overwrite(0);
		else if(singleturnResolution->value > 32) singleturnResolution->overwrite(32);
		updateSSIFrameFormat();
		updateEncoderWorkingRange();
	});
	inhibitTime = NumberParameter<int>::make(0, "Inhibit Time", "InhibitTime");
	inhibitTime->addEditCallback([this](){
		if(inhibitTime->value < 0) inhibitTime->overwrite(0);
		else if(inhibitTime->value > 65535) inhibitTime->overwrite(65535);
	});
	inhibitTime->setUnit(Units::Time::Microsecond);
	ssiCoding_parameter = OptionParameter::make2(ssiCoding_gray, ssiCodingOptions, "SSI Coding", "SSICoding");
	ssiBaudrate_parameter = OptionParameter::make2(ssiBaudrate_500Khz, ssiBaudrateOptions, "SSI Baudrate", "SSIBaudrate");
	
	centerOnZero_Param->setEditCallback([this](std::shared_ptr<Parameter> parameter){ updateEncoderWorkingRange(); });
	hasResetSignal_Param->setEditCallback([this](std::shared_ptr<Parameter> parameter){ resetPin->setVisible(hasResetSignal_Param->value); });
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x3101, 0x1, 8, "Status", &status);
	txPdoAssignement.addEntry(0x3101, 0x2, 32, "Value", &ssiValue);
	
	addNodePin(encoderPin);
	addNodePin(resetPin);
	
	updateSSIFrameFormat();
	updateEncoderWorkingRange();
}

bool EL5001::startupConfiguration() {
	
	bool success = true;
	
	//TODO: these parameters are store on the device eeprom upon writing them we should not upload them on each startup
	
	success &= writeSDO_U8(0x8010, 0x1, 0x0, "Disable Frame Error");
	success &= writeSDO_U8(0x8010, 0x2, 0x0, "Enable Power Failure Bit");
	success &= writeSDO_U8(0x8010, 0x3, inhibitTime->value != 0 ? 0x1 : 0x0, "Enable Inhibit Time");
	
	success &= writeSDO_U8(0x8010, 0x6, ssiCoding_parameter->value, "SSI Coding");
	success &= writeSDO_U8(0x8010, 0x9, ssiBaudrate_parameter->value, "SSI baud rate");
	success &= writeSDO_U8(0x8010, 0xF, 2, "SSI frame type"); //variable frame size (frame length and data length are parametrized manually)
	success &= writeSDO_U16(0x8010, 0x11, ssiFrameSize->value, "SSI Frame size"); //size of frame including power failure bits
	success &= writeSDO_U16(0x8010, 0x12, multiturnResolution->value + singleturnResolution->value, "SSI Data length"); //actual data length (mt+st)
	success &= writeSDO_U16(0x8010, 0x13, inhibitTime->value); //minimum time between two ssi readings
	
	return success;
}

void EL5001::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	
	b_dataError = status & 0x1;		//b0	//true when disconnecting data- signal or encoder power
	b_frameError = status & 0x2;	//b1	//true when disconnecting both clock signals
	b_powerFailure = status & 0x4;	//b2
	b_dataMismatch = status & 0x8;	//b3
	b_syncError = status & 0x20;	//b5
	b_txPdoState = status & 0x40;	//b6	//true when disconnecting both clock signals, data- signal or encoder power
	b_txPdoToggle = status & 0x80;	//b7
	
	uint32_t incrementsPerRevolution = 0x1 << singleturnResolution->value;
	uint32_t incrementsTotal = 0x1 << (singleturnResolution->value + multiturnResolution->value);
	
	int positionSigned_inc = ssiValue;
	//first invert direction if option is selected
	if(invertDirection_Param->value) positionSigned_inc = incrementsTotal - positionSigned_inc;
	//if centering on zero, subtract the entire range if we are over half the range
	if(centerOnZero_Param->value && positionSigned_inc >= incrementsTotal / 2) positionSigned_inc -= incrementsTotal;
	
	
	
	positionBeforeOffset_rev = double(positionSigned_inc) / double(incrementsPerRevolution);
	double position_rev = positionBeforeOffset_rev + positionOffset_rev;
	
	//do velocity calculation
	uint64_t readingTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	double deltaT_seconds = double(readingTime_nanoseconds - previousReadingTime_nanoseconds) / 1000000000.0;
	double deltaP_revolutions = position_rev - previousPosition_rev;
	previousReadingTime_nanoseconds = readingTime_nanoseconds;
	previousPosition_rev = position_rev;
	
	//write data to interface object
	encoder->feedbackProcessData.positionActual = position_rev;
	encoder->feedbackProcessData.velocityActual = deltaP_revolutions / deltaT_seconds;
	
	encoder->state = DeviceState::ENABLED;
	
	if(b_txPdoState) encoder->state = DeviceState::NOT_READY;
	else encoder->state = DeviceState::ENABLED;

}

void EL5001::writeOutputs(){

	if(encoder->feedbackProcessData.b_overridePosition){
		encoder->feedbackProcessData.b_overridePosition = false;
		encoder->feedbackProcessData.b_positionOverrideBusy = true;
		
		if(hasResetSignal_Param->value){
			*resetPinValue = true;
			resetStartTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
			Logger::info("[{}] resetting SSI encoder", encoder->getName());
		}else{
			positionOffset_rev = encoder->feedbackProcessData.positionOverride - positionBeforeOffset_rev;
			encoder->feedbackProcessData.b_positionOverrideBusy = false;
			encoder->feedbackProcessData.b_positionOverrideSucceeded = true;
			Logger::info("[{}] Encoder position reset", encoder->getName());
			updateEncoderWorkingRange();
		}
	}
	
	
	//ENCODER RESET PROCEDURE
	//some encoders (for example posital encoders) only reset their position on the falling edge of the reset signal
	//so we have to wait for a little longer for the ssi signal to return to 0 before considering the procedure to have failed
	//we pulse for the length set by the parameter and and then wait for the same time period for the ssi value to get to zero
	if(encoder->feedbackProcessData.b_positionOverrideBusy){
		encoder->feedbackProcessData.velocityActual = 0.0;
		*resetPinValue = EtherCatFieldbus::getCycleProgramTime_nanoseconds() - resetStartTime_nanoseconds < resetSignalTime_Param->value * 1000000;
		if(ssiValue == 0x0){
			encoder->feedbackProcessData.b_positionOverrideBusy = false;
			encoder->feedbackProcessData.b_positionOverrideSucceeded = true;
			positionOffset_rev = encoder->feedbackProcessData.positionOverride;
			encoder->feedbackProcessData.positionActual = encoder->feedbackProcessData.positionOverride;
			updateEncoderWorkingRange();
			*resetPinValue = false;
			Logger::info("[EL5001] Successfully reset SSI encoder position");
		}
		else if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - resetStartTime_nanoseconds > resetSignalTime_Param->value * 1000000 * 2){
			encoder->feedbackProcessData.b_positionOverrideBusy = false;
			encoder->feedbackProcessData.b_positionOverrideSucceeded = false;
			*resetPinValue = false;
			Logger::error("[EL5001] Failed to reset SSI encoder position");
		}
	}
	
}

bool EL5001::saveDeviceData(tinyxml2::XMLElement* xml) {
	ssiFrameSize->save(xml);
	multiturnResolution->save(xml);
	singleturnResolution->save(xml);
	ssiCoding_parameter->save(xml);
	ssiBaudrate_parameter->save(xml);
	inhibitTime->save(xml);
	centerOnZero_Param->save(xml);
	invertDirection_Param->save(xml);
	hasResetSignal_Param->save(xml);
	resetSignalTime_Param->save(xml);
	return true;
}
bool EL5001::loadDeviceData(tinyxml2::XMLElement* xml) {
	ssiFrameSize->load(xml);
	multiturnResolution->load(xml);
	singleturnResolution->load(xml);
	ssiCoding_parameter->load(xml);
	ssiBaudrate_parameter->load(xml);
	inhibitTime->load(xml);
	centerOnZero_Param->load(xml);
	invertDirection_Param->load(xml);
	hasResetSignal_Param->load(xml);
	resetSignalTime_Param->load(xml);
	updateSSIFrameFormat();
	updateEncoderWorkingRange();
	return true;
}

void EL5001::updateSSIFrameFormat(){
	frameFormatString = "";
	for(int i = 0; i < multiturnResolution->value; i++) frameFormatString += "M";
	for(int i = 0; i < singleturnResolution->value; i++) frameFormatString += "S";
	int remainingBits = ssiFrameSize->value - multiturnResolution->value - singleturnResolution->value;
	if(remainingBits > 0) for(int i = 0; i < remainingBits; i++) frameFormatString += "0";
}

void EL5001::updateEncoderWorkingRange(){
	uint32_t incrementsPerRevolution = 0x1 << singleturnResolution->value;
	uint32_t incrementsTotal = 0x1 << (singleturnResolution->value + multiturnResolution->value);
	
	double workingRangeDelta = incrementsTotal / incrementsPerRevolution;
	double workingRangeMin = 0.0;
	double workingRangeMax = workingRangeDelta;
	if(centerOnZero_Param->value){
		workingRangeMin -= workingRangeDelta / 2.0;
		workingRangeMax -= workingRangeDelta / 2.0;
	}
	
	encoder->feedbackConfig.positionLowerWorkingRangeBound = workingRangeMin + positionOffset_rev;
	encoder->feedbackConfig.positionUpperWorkingRangeBound = workingRangeMax + positionOffset_rev;
	
	encoderPin->updateConnectedPins();
	
}


void EL2912::onDisconnection() {}
void EL2912::onConnection() {}
void EL2912::initialize() {}
bool EL2912::startupConfiguration() {

	//actual parameters
	uint8_t moduloDiagTestPulse_ch0 = 0;
	uint8_t multiplierDiagTestPulse_ch0 = 0;
	bool standardOutputsActive_ch0 = false;
	bool diagTestPulseActive_ch0 = false;
	bool moduleFaultLinkActive_ch0 = false;
	uint8_t moduloDiagTestPulse_ch1 = 0;
	uint8_t multiplierDiagTestPulse_ch1 = 0;
	bool standardOutputsActive_ch1 = false;
	bool diagTestPulseActive_ch1 = false;
	bool moduleFaultLinkActive_ch1 = false;

	FsoeConnection::Config fsoeConfig;
	fsoeConfig.fsoeAddress = fsoeAddress;
	fsoeConfig.connectionID = 10;
	fsoeConfig.watchdogTimeout_ms = 2;
	fsoeConfig.applicationParameters = {
		0x2, 0x0, 0x0, 0x0,				//vendorID [fixed: Beckhoff == 0x2]
		0x60, 0xB, 0x0, 0x0,			//Module Ident [fixed Module == 2912]
		moduloDiagTestPulse_ch0,		//moduloDiagTestPulse_ch0
		multiplierDiagTestPulse_ch0,	//multiplierDiagTestPulse_ch0
		0x0,							//standardOutputsActive_ch0, diagTestPulseActive_ch0, moduleFaultLinkActive_ch0
		0x0, 							//spacer
		moduloDiagTestPulse_ch1,		//moduloDiagTestPulse_ch1
		multiplierDiagTestPulse_ch1,	//multiplierDiagTestPulse_ch1
		0x0,							//standardOutputsActive_ch1, diagTestPulseActive_ch1, moduleFaultLinkActive_ch1
		0x0, 							//spacer
		0x0, 0x0,						//storeCode [fixed to 0x0]
		0x0, 0x0						//projectCRC [fixed to 0x0]
	};
	if(standardOutputsActive_ch0) 	fsoeConfig.applicationParameters[10] |= 0x1;
	if(diagTestPulseActive_ch0) 	fsoeConfig.applicationParameters[10] |= 0x2;
	if(moduleFaultLinkActive_ch0) 	fsoeConfig.applicationParameters[10] |= 0x10;
	if(standardOutputsActive_ch1) 	fsoeConfig.applicationParameters[14] |= 0x1;
	if(diagTestPulseActive_ch1) 	fsoeConfig.applicationParameters[14] |= 0x2;
	if(moduleFaultLinkActive_ch1) 	fsoeConfig.applicationParameters[14] |= 0x10;
	fsoeConfig.safeOutputsSize = 1;
	fsoeConfig.safeInputsSize = 1;
	
	if(!fsoeConnection.initialize(fsoeConfig)) return false;
	
	return true;
}
void EL2912::readInputs(){
	fsoeConnection.receiveFrame(identity->inputs, 6, &safe_inputs, 1);
	safeOutput1Fault = safe_inputs & 0x1;
	safeOutput2Fault = safe_inputs & 0x2;
}
void EL2912::writeOutputs(){
	safeOutput1 = Timing::getBlink(0.5);
	safeOutput1ErrAck = false;
	safeOutput2 = !Timing::getBlink(0.5);
	safeOutput2ErrAck = false;

	safe_outputs = 0x0;
	if(safeOutput1) safe_outputs |= 0x1;
	if(safeOutput1ErrAck) safe_outputs |= 0x2;
	if(safeOutput2) safe_outputs |= 0x4;
	if(safeOutput2ErrAck) safe_outputs |= 0x8;
	
	fsoeConnection.sendFrame(identity->outputs, 6, &safe_outputs, 1);
}
bool EL2912::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EL2912::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }
void EL2912::downladSafetyParameters(){

	uint32_t vendorID;
	readSDO_U32(0x1018, 0x1, vendorID);

	uint32_t moduleIdent;
	readSDO_U32(0xf050, 0x1, moduleIdent);

	Logger::info("Vendor ID: {}", vendorID);
	Logger::info("module ident: {}", moduleIdent);

	auto downloadChannel = [this](uint16_t channelIndex){

		int channel = channelIndex == 0x8000 ? 0 : 1;

		uint8_t moduloDiagTestPulse;
		readSDO_U8(channelIndex, 0x1, moduloDiagTestPulse);

		uint8_t multiplierDiagTestPulse;
		readSDO_U8(channelIndex, 0x2, multiplierDiagTestPulse);

		uint8_t standardOutputsActive;
		readSDO_U8(channelIndex, 0x3, standardOutputsActive);

		uint8_t diagTestPulseActive;
		readSDO_U8(channelIndex, 0x4, diagTestPulseActive);

		uint8_t moduleFaultLinkActive;
		readSDO_U8(channelIndex, 0x7, moduleFaultLinkActive);

		Logger::info("[Ch{}] modulo diag test pulse: {}", channel, moduloDiagTestPulse);
		Logger::info("[Ch{}] multiplier diag test pulse: {}", channel, multiplierDiagTestPulse);
		Logger::info("[Ch{}] standard outputs active: {}", channel, standardOutputsActive);
		Logger::info("[Ch{}] diag test pulse active: {}", channel, diagTestPulseActive);
		Logger::info("[Ch{}] module fault link active: {}", channel, moduleFaultLinkActive);


	};

	downloadChannel(0x8000);
	downloadChannel(0x8010);

	uint16_t storeCode;
	readSDO_U16(0x10e0, 0x1, storeCode);

	uint16_t projectCRC;
	readSDO_U16(0x10e0, 0x2, projectCRC);

	Logger::info("store code: {}", storeCode);
	Logger::info("project crc: {}", projectCRC);
}

void EL1904::onDisconnection() {}
void EL1904::onConnection() {}
void EL1904::initialize() {}
bool EL1904::startupConfiguration() {

	//actual parameters
	uint8_t opMode = 0x0;
	bool sensorTestChannel1Active = false;
	bool sensorTestChannel2Active = false;
	bool sensorTestChannel3Active = false;
	bool sensorTestChannel4Active = false;
	uint8_t logicChannel12 = 0x0;
	uint8_t logicChannel34 = 0x0;
	
	FsoeConnection::Config fsoeConfig;
	fsoeConfig.fsoeAddress = fsoeAddress;
	fsoeConfig.connectionID = 0xABCD;
	fsoeConfig.watchdogTimeout_ms = 2;
	fsoeConfig.applicationParameters = {
		opMode,		//Opmode
		0x0,		//Sensor Test Channel 1-4 Active
		0x0, 0x0,	//Logic Channel 1&2 3&4
		0x0, 0x0,	//Store Code
		0x0, 0x0	//Project CRC
	};
	if(sensorTestChannel1Active) fsoeConfig.applicationParameters[1] |= 0x1;
	if(sensorTestChannel2Active) fsoeConfig.applicationParameters[1] |= 0x2;
	if(sensorTestChannel3Active) fsoeConfig.applicationParameters[1] |= 0x4;
	if(sensorTestChannel4Active) fsoeConfig.applicationParameters[1] |= 0x8;
	fsoeConfig.applicationParameters[2] |= logicChannel12;
	fsoeConfig.applicationParameters[2] |= (logicChannel34 << 2);
	fsoeConfig.safeOutputsSize = 1;
	fsoeConfig.safeInputsSize = 1;
	
	if(!fsoeConnection.initialize(fsoeConfig)) return false;

	return true;
}
void EL1904::readInputs() {
	fsoeConnection.receiveFrame(identity->inputs, 6, &safe_inputs, 1);
	
	safeInput1 = safe_inputs & 0x1;
	safeInput2 = safe_inputs & 0x2;
	safeInput3 = safe_inputs & 0x4;
	safeInput4 = safe_inputs & 0x8;
}
void EL1904::writeOutputs(){
	fsoeConnection.sendFrame(identity->outputs, 6, &safe_outputs, 1);
}
bool EL1904::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EL1904::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EL1904::downladSafetyParameters(){

	uint16_t operatingMode;
	readSDO_U16(0x8000, 0x1, operatingMode);

	uint8_t sensorTestChannel1Active;
	uint8_t sensorTestChannel2Active;
	uint8_t sensorTestChannel3Active;
	uint8_t sensorTestChannel4Active;
	readSDO_U8(0x8001, 0x1, sensorTestChannel1Active);
	readSDO_U8(0x8001, 0x2, sensorTestChannel2Active);
	readSDO_U8(0x8001, 0x3, sensorTestChannel3Active);
	readSDO_U8(0x8001, 0x4, sensorTestChannel4Active);
	
	uint8_t logicChannel12;
	uint8_t logicChannel34;
	readSDO_U8(0x8002, 0x1, logicChannel12);
	readSDO_U8(0x8002, 0x3, logicChannel34);

	Logger::info("opmode {}", operatingMode);
	Logger::info("testChannel 1: {}  2: {}  3: {}  4: {}", sensorTestChannel1Active, sensorTestChannel2Active, sensorTestChannel3Active, sensorTestChannel4Active);
	Logger::info("logicChannel 1&2: {}   3&4: {}", logicChannel12, logicChannel34);
}
