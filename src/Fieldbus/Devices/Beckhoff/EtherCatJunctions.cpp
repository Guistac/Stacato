#include <pch.h>

#include "EtherCatJunctions.h"

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

void EL2008::onDisconnection() {}
void EL2008::onConnection() {}
void EL2008::initialize() {
	rxPdoAssignement.addNewModule(0x1600);
	for(int i = 0; i < 8; i++){
		char name[32];
		snprintf(name, 32, "Channel %i", i + 1);
		rxPdoAssignement.addEntry(0x1600 + i, 0x1, 1, name, &outputs[i]);
	}
}
bool EL2008::startupConfiguration() { return true; }
void EL2008::readInputs() {}
void EL2008::writeOutputs(){
	rxPdoAssignement.pushDataTo(identity->outputs);
}
bool EL2008::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EL2008::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EL5001::onDisconnection() {}
void EL5001::onConnection() {}
void EL5001::initialize() {
	
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
	});
	singleturnResolution = NumberParameter<int>::make(12, "Singleturn resolution", "SingleturnResolution");
	singleturnResolution->addEditCallback([this](){
		if(singleturnResolution->value < 0) singleturnResolution->overwrite(0);
		else if(singleturnResolution->value > 32) singleturnResolution->overwrite(32);
		updateSSIFrameFormat();
	});
	ssiCoding_parameter = OptionParameter::make2(ssiCoding_gray, ssiCodingOptions, "SSI Coding", "SSICoding");
	ssiBaudrate_parameter = OptionParameter::make2(ssiBaudrate_500Khz, ssiBaudrateOptions, "SSI Baudrate", "SSIBaudrate");
	
	updateSSIFrameFormat();
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x3101, 0x1, 8, "Status", &status);
	txPdoAssignement.addEntry(0x3101, 0x2, 32, "Value", &ssiValue);
}

bool EL5001::startupConfiguration() {
	
	bool success = true;
	
	success &= writeSDO_U8(0x4061, 0x1, 0x1, "Disable Frame Error");
	success &= writeSDO_U8(0x4061, 0x2, 0x0, "Enable Power Failure Bit");
	success &= writeSDO_U8(0x4061, 0x3, 0x1, "Enable Inhibit Time");
	success &= writeSDO_U16(0x4066, 0x0, ssiCoding_parameter->value, "SSI Coding");
	success &= writeSDO_U16(0x4067, 0x0, ssiBaudrate_parameter->value, "SSI baud rate");
	success &= writeSDO_U16(0x4068, 0x0, 2, "SSI frame type");
	success &= writeSDO_U16(0x4069, 0x0, ssiFrameSize->value, "SSI Frame size");
	success &= writeSDO_U16(0x406A, 0x0, multiturnResolution->value + singleturnResolution->value, "SSI Data length");
	
	return true;
}

void EL5001::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	
	b_dataError = status & 0x1;		//b0
	b_frameError = status & 0x2;	//b1
	b_powerFailure = status & 0x4;	//b2
	b_dataMismatch = status & 0x8;	//b3
	b_syncError = status & 0x20;	//b5
	b_txPdoState = status & 0x40;	//b6
	b_txPdoToggle = status & 0x80;	//b7
	
	Logger::warn("{} {:b}", getName(), ssiValue);

}
void EL5001::writeOutputs(){}
bool EL5001::saveDeviceData(tinyxml2::XMLElement* xml) {
	ssiFrameSize->save(xml);
	multiturnResolution->save(xml);
	singleturnResolution->save(xml);
	ssiCoding_parameter->save(xml);
	ssiBaudrate_parameter->save(xml);
	return true;
}
bool EL5001::loadDeviceData(tinyxml2::XMLElement* xml) {
	ssiFrameSize->load(xml);
	multiturnResolution->load(xml);
	singleturnResolution->load(xml);
	ssiCoding_parameter->load(xml);
	ssiBaudrate_parameter->load(xml);
	updateSSIFrameFormat();
	return true;
}

void EL5001::updateSSIFrameFormat(){
	frameFormatString = "";
	
	for(int i = 0; i < multiturnResolution->value; i++) frameFormatString += "M";
	for(int i = 0; i < singleturnResolution->value; i++) frameFormatString += "S";
	int remainingBits = ssiFrameSize->value - multiturnResolution->value - singleturnResolution->value;
	if(remainingBits > 0) for(int i = 0; i < remainingBits; i++) frameFormatString += "0";
	
}
