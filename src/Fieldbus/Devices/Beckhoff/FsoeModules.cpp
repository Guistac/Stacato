#include "FsoeModules.h"


void EL2912::onDisconnection() {}
void EL2912::onConnection() {}
void EL2912::initialize() {
	std::shared_ptr<NodePin> out1pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Safe Output 1", "SafeOutput1");
	std::shared_ptr<NodePin> out2pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Safe Output 2", "SafeOutput2");
	out1pin->assignData(out1);
	out2pin->assignData(out2);
	addNodePin(out1pin);
	addNodePin(out2pin);
}
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
	fsoeConfig.watchdogTimeout_ms = 500;
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
	processData.safeOutput1Fault = safe_inputs & 0x1;
	processData.safeOutput2Fault = safe_inputs & 0x2;
}
void EL2912::writeOutputs(){
	for(auto inputPin : getInputPins()) if(inputPin->isConnected()) inputPin->copyConnectedPinValue();
	
	processData.safeOutput1 = *out1;
	processData.safeOutput1ErrAck = false;
	processData.safeOutput2 = *out2;
	processData.safeOutput2ErrAck = false;

	safe_outputs = 0x0;
	if(processData.safeOutput1) safe_outputs |= 0x1;
	if(processData.safeOutput1ErrAck) safe_outputs |= 0x2;
	if(processData.safeOutput2) safe_outputs |= 0x4;
	if(processData.safeOutput2ErrAck) safe_outputs |= 0x8;
	
	fsoeConnection.b_sendFailsafeData = false;
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
void EL1904::initialize() {
	std::shared_ptr<NodePin> in1Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Safe Input 1", "SafeInput1");
	std::shared_ptr<NodePin> in2Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Safe Input 2", "SafeInput2");
	std::shared_ptr<NodePin> in3Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Safe Input 3", "SafeInput3");
	std::shared_ptr<NodePin> in4Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Safe Input 4", "SafeInput4");
	in1Pin->assignData(in1);
	in2Pin->assignData(in2);
	in3Pin->assignData(in3);
	in4Pin->assignData(in4);
	addNodePin(in1Pin);
	addNodePin(in2Pin);
	addNodePin(in3Pin);
	addNodePin(in4Pin);
}
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
	fsoeConfig.watchdogTimeout_ms = 500;
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
	fsoeConnection.receiveFrame(identity->inputs, 6, &processData.safe_inputs, 1);
	*in1 = processData.safe_inputs & 0x1;
	*in2 = processData.safe_inputs & 0x2;
	*in3 = processData.safe_inputs & 0x4;
	*in4 = processData.safe_inputs & 0x8;
}
void EL1904::writeOutputs(){
	fsoeConnection.b_sendFailsafeData = false;
	fsoeConnection.sendFrame(identity->outputs, 6, &processData.safe_outputs, 1);
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
