#include <pch.h>

#include "VipaModule.h"

#include "NodeGraph/NodePin.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include <tinyxml2.h>

#include "Fieldbus/EtherCatFieldbus.h"

namespace VipaModuleFactory{

	std::vector<VipaModule*> modules = {
		new VIPA_022_1HD10(),
		new VIPA_021_1BF00(),
		new VIPA_022_1BF00(),
		new VIPA_050_1BS00(),
		new VIPA_032_1BD70()
	};

	std::vector<VipaModule*>& getModules(){
		return modules;
	}

	std::shared_ptr<VipaModule> getModule(const char* saveName){
		for(auto& module : modules){
			if(strcmp(module->getSaveName(), saveName) == 0) return module->getInstance();
		}
		return nullptr;
	}
};


//output modules (rx) are 0x1600 + i and hold objects of index 0x7000 + i
//input modules (tx) are 0x1A00 + i and hold objects of index 0x6000 + i


//=================================================================
//================== 022-1HD10 4x Relais Output ===================
//=================================================================

void VIPA_022_1HD10::onConstruction(){
	for(int i = 0; i < 4; i++){
		static char pinName[64];
		sprintf(pinName, "Relais Output %i", i);
		inputPins.push_back(std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_INPUT, pinName));
	}
	inputBitCount = 0;
	inputByteCount = 0;
	outputBitCount = 8;
	outputByteCount = 1;
}
void VIPA_022_1HD10::onSetIndex(int i){
	for(int i = 0; i < 4; i++){
		sprintf((char*)inputPins[i]->getDisplayName(), "Module %i Relais Output %i", moduleIndex, i);
	}
}
void VIPA_022_1HD10::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	//module has no tx pdo module
}
void VIPA_022_1HD10::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	rxPdoAssignement.addNewModule(0x1600 + moduleIndex);
	uint16_t dataObjectIndex = 0x7000 + moduleIndex;
	for(int i = 0; i < 4; i++){
		uint8_t subindex = i + 1;
		static char pdoEntryNameString[64];
		sprintf(pdoEntryNameString, "Relais Output %i", i);
		bool& dataReference = outputs[i];
		rxPdoAssignement.addEntry(dataObjectIndex, subindex, 1, pdoEntryNameString, &dataReference);
	}
	rxPdoAssignement.addEntry(dataObjectIndex, 0x5, 4, "Reserved", &pdoSpacerBits);
}
bool VIPA_022_1HD10::configureParameters(){
	//no parameter configuratio for this SLIO module
	return true;
}
void VIPA_022_1HD10::readInputs(){
	//no input reading for this SLIO module
}
void VIPA_022_1HD10::writeOutputs(){
	for(int i = 0; i < 4; i++){
		if(inputPins[i]->isConnected()){
			outputs[i] = inputPins[i]->getConnectedPins().front()->getBoolean();
		}else{
			outputs[i] = inputPins[i]->getBoolean();
		}
	}
}

//=================================================================
//================== 021-1BF00 8x Digital Input ===================
//=================================================================

void VIPA_021_1BF00::onConstruction(){
	for(int i = 0; i < 8; i++){
		static char pinName[64];
		sprintf(pinName, "Digital Input %i", i);
		outputPins.push_back(std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, pinName));
	}
	inputBitCount = 8;
	inputByteCount = 1;
	outputBitCount = 0;
	outputByteCount = 0;
}
void VIPA_021_1BF00::onSetIndex(int i){
	for(int i = 0; i < 8; i++){
		sprintf((char*)outputPins[i]->getDisplayName(), "Module %i Digital Input %i", moduleIndex, i);
	}
}
void VIPA_021_1BF00::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	txPdoAssignement.addNewModule(0x1A00 + moduleIndex);
	uint16_t dataObjectIndex = 0x6000 + moduleIndex;
	for(int i = 0; i < 8; i++){
		uint8_t subindex = i+1;
		int bitSize = 1;
		static char pdoEntryNameString[64];
		sprintf(pdoEntryNameString, "Digital Input %i", i);
		bool& dataReference = inputs[i];
		txPdoAssignement.addEntry(dataObjectIndex, subindex, bitSize, pdoEntryNameString, &dataReference);
	}
}
void VIPA_021_1BF00::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	//module has no rx pdo module
}
bool VIPA_021_1BF00::configureParameters(){
	//no parameter configuration for this SLIO module
	return true;
}
void VIPA_021_1BF00::readInputs(){
	for(int i = 0; i < 8; i++){
		bool value = inputs[i];
		std::shared_ptr<NodePin> pin = outputPins[i];
		pin->set(value);
	}
}
void VIPA_021_1BF00::writeOutputs(){
	//no output writing for this SLIO module
}

//=================================================================
//================== 022-1BF00 8x Digital Output ===================
//=================================================================

void VIPA_022_1BF00::onConstruction(){
	for(int i = 0; i < 8; i++){
		static char pinName[64];
		sprintf(pinName, "Digital Output %i", i);
		inputPins.push_back(std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_INPUT, pinName));
	}
	inputBitCount = 0;
	inputByteCount = 0;
	outputBitCount = 8;
	outputByteCount = 1;
}

void VIPA_022_1BF00::onSetIndex(int i){
	for(int i = 0; i < 8; i++){
		sprintf((char*)inputPins[i]->getDisplayName(), "Module %i Digital Output %i", moduleIndex, i);
	}
}

void VIPA_022_1BF00::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	//module has no rx pdo module
}

void VIPA_022_1BF00::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	rxPdoAssignement.addNewModule(0x1600 + moduleIndex);
	uint16_t dataObjectIndex = 0x7000 + moduleIndex;
	for(int i = 0; i < 8; i++){
		uint8_t subindex = i+1;
		int bitSize = 1;
		static char pdoEntryNameString[64];
		sprintf(pdoEntryNameString, "Digital Input %i", i);
		bool& dataReference = outputs[i];
		rxPdoAssignement.addEntry(dataObjectIndex, subindex, bitSize, pdoEntryNameString, &dataReference);
	}
}
bool VIPA_022_1BF00::configureParameters(){
	//no parameter configuration for this SLIO module
	return true;
}

void VIPA_022_1BF00::readInputs(){
	//no input writing for this SLIO module
}

void VIPA_022_1BF00::writeOutputs(){
	for(int i = 0; i < 8; i++){
		if(inputPins[i]->isConnected()){
			outputs[i] = inputPins[i]->getConnectedPins().front()->getBoolean();
		}else{
			outputs[i] = inputPins[i]->getBoolean();
		}
	}
}
//=================================================================
//============== 050-1BS00 Single SSI Encoder Input ===============
//=================================================================

void VIPA_050_1BS00::onConstruction(){
	encoderPin->set(encoderDevice);
	outputPins.push_back(encoderPin);
	outputPins.push_back(resetPin);
	inputBitCount = 48;
	inputByteCount = 6;
	outputBitCount = 0;
	outputByteCount = 0;
	updateResetPinVisibility();
}

void VIPA_050_1BS00::onSetParentBusCoupler(std::shared_ptr<VipaBusCoupler_053_1EC01> busCoupler){
	encoderDevice->setParentDevice(busCoupler);
}

void VIPA_050_1BS00::onSetIndex(int i){
	sprintf((char*)encoderPin->getDisplayName(), "Module %i SSI Encoder", moduleIndex);
	sprintf((char*)resetPin->getDisplayName(), "Module %i Encoder Reset", moduleIndex);
}

void VIPA_050_1BS00::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	txPdoAssignement.addNewModule(0x1A00 + moduleIndex);
	uint16_t dataObjectIndex = 0x6000 + moduleIndex;
	txPdoAssignement.addEntry(dataObjectIndex, 0x1, 32, "SSI Encoder Data", &encoderValue);
	txPdoAssignement.addEntry(dataObjectIndex, 0x2, 16, "SSI Measurement Time", &time_microseconds);
}

void VIPA_050_1BS00::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){ /*No RxPDO*/ }

bool VIPA_050_1BS00::configureParameters(){
	
	uint16_t settingsObjectIndex = 0x3100 + moduleIndex;
	
	uint16_t idleTimeSetting = getPauseTime(pausetime)->valueSetting;
	if(!parentBusCoupler->writeSDO_U16(settingsObjectIndex, 0x2, idleTimeSetting)) return false;
	
	uint16_t transmissionFrequencySetting = getTransmissionRate(transmissionFrequency)->valueSetting;
	if(!parentBusCoupler->writeSDO_U16(settingsObjectIndex, 0x3, transmissionFrequencySetting)) return false;
	
	uint8_t normalisationBitCountSetting = normalisationBitCountToCanValue(normalisationBitCount);
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0x5, normalisationBitCountSetting)) return false;
	
	uint8_t encoderBitCountSetting = encoderBitCountToCanValue(encoderBitCount);
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0x6, encoderBitCountSetting)) return false;
	
	uint8_t ssiModeSetting = 0x0;
	ssiModeSetting |= 0b10; //set ssi master mode
	if(getBitShift(bitshiftDirection)->bitValue) {
		ssiModeSetting |= 0b100;
	}
	if(getClockEdge(clockEdge)->bitValue) {
		ssiModeSetting |= 0b1000;
	}
	if(getEncoding(encodingFormat)->bitValue) {
		ssiModeSetting |= 0b10000;
	}
	//last three bits are reserved to 0
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0x7, ssiModeSetting)) return false;
	
	uint8_t ssiEnableFunction = 0x1;
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0xB, ssiEnableFunction)) return false;

	updateEncoderWorkingRange();
	
	return true;
}

void VIPA_050_1BS00::readInputs(){
	int incrementsPerRevolution = 0x1 << singleTurnBitCount;
	int multiturnBitCount = encoderBitCount - singleTurnBitCount;
	int maxRevolutions = 0x1 << multiturnBitCount;
	
	encoderPosition_revolutions = (float)encoderValue / (float)incrementsPerRevolution;
	if(b_centerRangeOnZero && encoderPosition_revolutions >= maxRevolutions / 2) encoderPosition_revolutions -= maxRevolutions;
		
	uint16_t readingDeltaT_microseconds = time_microseconds - previousReadingTime_microseconds;
	double positionDelta_revolutions = encoderPosition_revolutions - previousEncoderPosition_revolutions;
	encoderVelocity_revolutionsPerSecond = 1000000.0 * positionDelta_revolutions / (double)readingDeltaT_microseconds;
	
	previousReadingTime_microseconds = time_microseconds;
	previousEncoderPosition_revolutions = encoderPosition_revolutions;
	
	encoderDevice->positionRaw_positionUnits = encoderPosition_revolutions;
	encoderDevice->velocity_positionUnitsPerSecond = encoderVelocity_revolutionsPerSecond;
	
	encoderDevice->b_canHardReset = b_hasResetSignal && resetPin->isConnected();
	encoderDevice->b_detected = true;
	encoderDevice->b_ready = parentBusCoupler->isStateOperational();
	
	if(encoderDevice->b_doHardReset){
		encoderDevice->b_doHardReset = false;
		b_isResetting = true;
		resetPin->set(true);
		resetStartTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	}
	
	if(b_isResetting) {
		encoderDevice->velocity_positionUnitsPerSecond = 0.0;
		if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() > resetStartTime_nanoseconds + resetTime_milliseconds * 1000000.0){
			b_isResetting = false;
			resetPin->set(false);
		}
	}
}

void VIPA_050_1BS00::writeOutputs(){ /*No Outputs*/ }

void VIPA_050_1BS00::onConnection(){
	encoderDevice->b_ready = true;
}
void VIPA_050_1BS00::onDisconnection(){
	encoderDevice->b_ready = false;
}

void VIPA_050_1BS00::moduleParameterGui(){
	
	ImGui::Text("Encoding");
	if(ImGui::BeginCombo("##EncodingSelector", getEncoding(encodingFormat)->displayName)){
		for(auto& encoding : encodings){
			if(ImGui::Selectable(encoding.displayName, encodingFormat == encoding.format)) encodingFormat = encoding.format;
		}
		ImGui::EndCombo();
	}
	
	ImGui::Text("SSI Transmission Frequency");
	if(ImGui::BeginCombo("##TransmissionFrequencySelector", getTransmissionRate(transmissionFrequency)->displayName)){
		for(auto& rate : transmissionRates){
			if(ImGui::Selectable(rate.displayName, transmissionFrequency == rate.frequency)) transmissionFrequency = rate.frequency;
		}
		ImGui::EndCombo();
	}
	
	ImGui::Text("SSI Measurement Pause Time");
	const char* pausePreviewString = getPauseTime(pausetime)->displayName;
	if(ImGui::BeginCombo("##PausetimeSelector", pausePreviewString)){
		for(auto& time : pauseTimes){
			if(ImGui::Selectable(time.displayName, pausetime == time.microseconds)) pausetime = time.microseconds;
		}
		ImGui::EndCombo();
	}
	
	float widgetWidth = ImGui::GetItemRectSize().x;
	
	ImGui::Text("Total Significant Bit Count");
	ImGui::InputInt("##EncoderBitCount", &encoderBitCount);
	encoderBitCount = std::min(maxEncoderBits, encoderBitCount);
	encoderBitCount = std::max(minEncoderBits, encoderBitCount);
	
	ImGui::Text("Single Turn Bit Count");
	ImGui::InputInt("##SingleTurnBitCount", &singleTurnBitCount);
	singleTurnBitCount = std::min(maxEncoderBits, singleTurnBitCount);
	singleTurnBitCount = std::max(minEncoderBits, singleTurnBitCount);
	
	if(ImGui::Checkbox("Has Reset Signal", &b_hasResetSignal)) updateResetPinVisibility();
	
	if(b_hasResetSignal){
		ImGui::Text("Reset Time");
		ImGui::InputFloat("##resetTime", &resetTime_milliseconds, 0.0, 0.0, "%.1f ms");
	}
	
	if(ImGui::Checkbox("Center Encoder Range on Zero", &b_centerRangeOnZero)) updateEncoderWorkingRange();
	
	int multiturnBitCount = encoderBitCount - singleTurnBitCount;
	int maxRevolutions = 0x1 << multiturnBitCount;
	
	//----------- ADVANCED SSI SETTINGS -------------
	
	if(ImGui::TreeNode("Advanced Settings")){
		
		ImGui::Text("Ignored Encoder Bit Count");
		ImGui::InputInt("##normalisationBits", &normalisationBitCount);
		normalisationBitCount = std::min(maxNormalisationBits, normalisationBitCount);
		normalisationBitCount = std::max(minNormalisationBits, normalisationBitCount);
		
		ImGui::Text("Data Direction");
		if(ImGui::BeginCombo("##ShiftDirectionSelector", getBitShift(bitshiftDirection)->displayName)){
			for(auto& shift : bitShifts){
				if(ImGui::Selectable(shift.displayName, bitshiftDirection == shift.direction)) bitshiftDirection = shift.direction;
			}
			ImGui::EndCombo();
		}
		
		ImGui::Text("Data Clock Edge");
		if(ImGui::BeginCombo("##ClockEdgeSelector", getClockEdge(clockEdge)->displayName)){
			for(auto& edge : clockEdges){
				if(ImGui::Selectable(edge.displayName, clockEdge == edge.edge)) clockEdge = edge.edge;
			}
			ImGui::EndCombo();
		}
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	
	ImGui::Text("Encoder position in working range (%.3f to %.3f revolutions)", encoderDevice->getMinPosition(), encoderDevice->getMaxPosition());
	
	glm::vec2 progressBarSize(widgetWidth, ImGui::GetFrameHeight());
	
	static char encoderRangeProgressString[64];
	sprintf(encoderRangeProgressString, "%.3f revolutions", encoderDevice->positionRaw_positionUnits);
	ImGui::ProgressBar((float)encoderDevice->getPositionInRange(), progressBarSize, encoderRangeProgressString);
	   
	static char encoderVelocityString[64];
	sprintf(encoderVelocityString, "%.3f rev/s", encoderVelocity_revolutionsPerSecond);
	float velocityProgress = std::abs(encoderVelocity_revolutionsPerSecond) / 10.0;
	ImGui::ProgressBar(velocityProgress, progressBarSize, encoderVelocityString);
	
	bool disableResetButton = !b_hasResetSignal;
	if(disableResetButton) BEGIN_DISABLE_IMGUI_ELEMENT
	if(ImGui::Button("Reset Encoder")) encoderDevice->hardReset();
	if(disableResetButton) END_DISABLE_IMGUI_ELEMENT
}

void VIPA_050_1BS00::updateEncoderWorkingRange(){
	int multiturnBitCount = encoderBitCount - singleTurnBitCount;
	int maxRevolutions = 0x1 << multiturnBitCount;
	if(b_centerRangeOnZero){
		encoderDevice->rangeMin_positionUnits = -maxRevolutions / 2;
		encoderDevice->rangeMax_positionUnits = maxRevolutions / 2;
	}else{
		encoderDevice->rangeMin_positionUnits = 0;
		encoderDevice->rangeMax_positionUnits = maxRevolutions;
	}
}

void VIPA_050_1BS00::updateResetPinVisibility(){
	resetPin->setVisible(b_hasResetSignal);
}

bool VIPA_050_1BS00::save(tinyxml2::XMLElement* xml){
	xml->SetAttribute("Encoding", getEncoding(encodingFormat)->saveName);
	xml->SetAttribute("TransmissionRate", getTransmissionRate(transmissionFrequency)->saveName);
	xml->SetAttribute("IdleTime", getPauseTime(pausetime)->saveName);
	xml->SetAttribute("CenterRangeOnZero", b_centerRangeOnZero);
	xml->SetAttribute("HasResetSignal", b_hasResetSignal);
	if(b_hasResetSignal) xml->SetAttribute("ResetTime", resetTime_milliseconds);
	xml->SetAttribute("TotalBitCount", encoderBitCount);
	xml->SetAttribute("SingleTurnBitCount", singleTurnBitCount);
	xml->SetAttribute("IgnoredBitCount", normalisationBitCount);
	xml->SetAttribute("BitShiftDirection", getBitShift(bitshiftDirection)->saveName);
	xml->SetAttribute("ClockEdge", getClockEdge(clockEdge)->saveName);
	xml->SetAttribute("Zero_revolutions", encoderDevice->positionOffset_positionUnits);
	return true;
}

bool VIPA_050_1BS00::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	const char* encodingString;
	if(xml->QueryStringAttribute("Encoding", &encodingString) != XML_SUCCESS) return Logger::warn("Could not find Encoding attribute");
	if(getEncoding(encodingString) == nullptr) return Logger::warn("Could not identify Encoding attribute");
	encodingFormat = getEncoding(encodingString)->format;
	const char* frequencyString;
	if(xml->QueryStringAttribute("TransmissionRate", &frequencyString) != XML_SUCCESS) return Logger::warn("Could not find Frequency attribute");
	if(getTransmissionRate(frequencyString) == nullptr) return Logger::warn("Could not identify Transmission Rate attribute");
	transmissionFrequency = getTransmissionRate(frequencyString)->frequency;
	const char* idleTimeString;
	if(xml->QueryStringAttribute("IdleTime", &idleTimeString) != XML_SUCCESS) return Logger::warn("Could not find Idle Time attribute");
	if(getPauseTime(idleTimeString) == nullptr) return Logger::warn("Could not identify idle time attribute");
	pausetime = getPauseTime(idleTimeString)->microseconds;
	if(xml->QueryBoolAttribute("CenterRangeOnZero", &b_centerRangeOnZero) != XML_SUCCESS) return Logger::warn("Could not find Center on zero attribute");
	if(xml->QueryBoolAttribute("HasResetSignal", &b_hasResetSignal) != XML_SUCCESS) return Logger::warn("Could not find Has Reset Signal attribute");
	if(b_hasResetSignal){
		if(xml->QueryFloatAttribute("ResetTime", &resetTime_milliseconds) != XML_SUCCESS) return Logger::warn("Could not find encoder reset time attribute");
	}
	if(xml->QueryIntAttribute("TotalBitCount", &encoderBitCount) != XML_SUCCESS) return Logger::warn("Could not find Total Bit Count attribute");
	if(xml->QueryIntAttribute("SingleTurnBitCount", &singleTurnBitCount) != XML_SUCCESS) return Logger::warn("Could not find Singleturn bit count attribute");
	if(xml->QueryIntAttribute("IgnoredBitCount", &normalisationBitCount) != XML_SUCCESS) return Logger::warn("Could not find ignored bit count attribute");
	const char* bitShiftDirectionString;
	if(xml->QueryStringAttribute("BitShiftDirection", &bitShiftDirectionString) != XML_SUCCESS) return Logger::warn("Could not find Bitshift Direction attribute");
	if(getBitShift(bitShiftDirectionString) == nullptr) return Logger::warn("Could not identify Bit shift attribute");
	bitshiftDirection = getBitShift(bitShiftDirectionString)->direction;
	const char* clockEdgeString;
	if(xml->QueryStringAttribute("ClockEdge", &clockEdgeString) != XML_SUCCESS) return Logger::warn("Could not find Clock Edge attribute");
	if(getClockEdge(clockEdgeString) == nullptr) return Logger::warn("Could not identify Clock Edge Attribute");
	clockEdge = getClockEdge(clockEdgeString)->edge;
	if(xml->QueryDoubleAttribute("Zero_revolutions", &encoderDevice->positionOffset_positionUnits) != XML_SUCCESS) return Logger::warn("Could not find encoder zero attribute");
	updateEncoderWorkingRange();
	return true;
}



std::vector<VIPA_050_1BS00::PauseTime> VIPA_050_1BS00::pauseTimes = {
	{VIPA_050_1BS00::PauseTime::Microseconds::_1_MICROSECONDS, 0x0030, "1", "1 µs"},
	{VIPA_050_1BS00::PauseTime::Microseconds::_2_MICROSECONDS, 0x0060, "2", "2 µs"},
	{VIPA_050_1BS00::PauseTime::Microseconds::_4_MICROSECONDS, 0x00C0, "4", "4 µs"},
	{VIPA_050_1BS00::PauseTime::Microseconds::_8_MICROSECONDS, 0x0180, "8", "8 µs"},
	{VIPA_050_1BS00::PauseTime::Microseconds::_16_MICROSECONDS, 0x0300, "16", "16 µs"},
	{VIPA_050_1BS00::PauseTime::Microseconds::_32_MICROSECONDS, 0x0600, "32", "32 µs"},
	{VIPA_050_1BS00::PauseTime::Microseconds::_48_MICROSECONDS, 0x0900, "48", "48 µs"},
	{VIPA_050_1BS00::PauseTime::Microseconds::_64_MICROSECONDS, 0x0C00, "64", "64 µs"}
};

VIPA_050_1BS00::PauseTime* VIPA_050_1BS00::getPauseTime(const char* saveName){
	for(auto& time : pauseTimes){
		if(strcmp(saveName, time.saveName) == 0) return &time;
	}
	return nullptr;
}

VIPA_050_1BS00::PauseTime* VIPA_050_1BS00::getPauseTime(PauseTime::Microseconds microseconds){
	for(auto& time : pauseTimes){
		if(microseconds == time.microseconds) return &time;
	}
	return nullptr;
}

VIPA_050_1BS00::PauseTime* VIPA_050_1BS00::getPauseTime(uint16_t valueSetting){
	for(auto& time : pauseTimes){
		if(valueSetting == time.valueSetting) return &time;
	}
	return nullptr;
}

std::vector<VIPA_050_1BS00::TransmissionRate> VIPA_050_1BS00::transmissionRates = {
	{VIPA_050_1BS00::TransmissionRate::Frequency::_2_MEGAHERTZ, 	0x0018, "2Mhz", 	"2.0 Mhz"},
	{VIPA_050_1BS00::TransmissionRate::Frequency::_1p5_MEGAHERTZ, 	0x0020, "1.5Mhz", 	"1.5 Mhz"},
	{VIPA_050_1BS00::TransmissionRate::Frequency::_1_MEGAHERTZ, 	0x0030, "1Mhz", 	"1.0 Mhz"},
	{VIPA_050_1BS00::TransmissionRate::Frequency::_500_KILOHERTZ, 	0x0060, "500Khz", 	"500.0 Khz"},
	{VIPA_050_1BS00::TransmissionRate::Frequency::_250_KILOHERTZ, 	0x00C0, "250Khz", 	"250.0 Khz"},
	{VIPA_050_1BS00::TransmissionRate::Frequency::_125_KILOHERTZ, 	0x0180, "125Khz", 	"125.0 Khz"}
};

VIPA_050_1BS00::TransmissionRate* VIPA_050_1BS00::getTransmissionRate(const char* saveName){
	for(auto& rate : transmissionRates){
		if(strcmp(rate.saveName, saveName) == 0) return &rate;
	}
	return nullptr;
}

VIPA_050_1BS00::TransmissionRate* VIPA_050_1BS00::getTransmissionRate(TransmissionRate::Frequency frequency){
	for(auto& rate : transmissionRates){
		if(rate.frequency == frequency) return &rate;
	}
	return nullptr;
}

VIPA_050_1BS00::TransmissionRate* VIPA_050_1BS00::getTransmissionRate(uint16_t valueSetting){
	for(auto& rate : transmissionRates){
		if(rate.valueSetting == valueSetting) return &rate;
	}
	return nullptr;
}

int VIPA_050_1BS00::maxNormalisationBits = 15;
int VIPA_050_1BS00::minNormalisationBits = 0;

uint8_t VIPA_050_1BS00::normalisationBitCountToCanValue(int bitCount){
	uint8_t output = std::max(minNormalisationBits, bitCount);
	output = std::min(maxNormalisationBits, bitCount);
	return output;
}

int VIPA_050_1BS00::canValuetoNormalisationBitCount(uint8_t canValue){
	return canValue;
}

int VIPA_050_1BS00::maxEncoderBits = 32;
int VIPA_050_1BS00::minEncoderBits = 8;

uint8_t VIPA_050_1BS00::encoderBitCountToCanValue(int bitCount){
	bitCount = std::max(minEncoderBits, bitCount);
	bitCount = std::min(maxEncoderBits, bitCount);
	uint8_t output = bitCount - 1;
	return output;
}

int VIPA_050_1BS00::canValuetoEncoderBitCount(uint8_t canValue){
	return canValue + 1;
}

std::vector<VIPA_050_1BS00::BitShift> VIPA_050_1BS00::bitShifts = {
	{VIPA_050_1BS00::BitShift::Direction::LSB_FIRST, false, "LSBFirst", "LSB First"},
	{VIPA_050_1BS00::BitShift::Direction::MSB_FIRST, true, "MSBFirst", "MSB First (default)"}
};

VIPA_050_1BS00::BitShift* VIPA_050_1BS00::getBitShift(const char* saveName){
	for(auto& bitShift : bitShifts){
		if(strcmp(bitShift.saveName, saveName) == 0) return &bitShift;
	}
	return nullptr;
}

VIPA_050_1BS00::BitShift* VIPA_050_1BS00::getBitShift(BitShift::Direction direction){
	for(auto& bitShift : bitShifts){
		if(direction == bitShift.direction) return &bitShift;
	}
	return nullptr;
}

VIPA_050_1BS00::BitShift* VIPA_050_1BS00::getBitShift(bool valueSetting){
	for(auto& bitShift : bitShifts){
		if(valueSetting == bitShift.bitValue) return &bitShift;
	}
	return nullptr;
}

std::vector<VIPA_050_1BS00::ClockEdge> VIPA_050_1BS00::clockEdges = {
	{VIPA_050_1BS00::ClockEdge::Edge::FALLING_EDGE, false, "FallingEdge", "Falling Edge"},
	{VIPA_050_1BS00::ClockEdge::Edge::RISING_EDGE, false, "RisingEdge", "Rising Edge (default)"},
};

VIPA_050_1BS00::ClockEdge* VIPA_050_1BS00::getClockEdge(const char* saveName){
	for(auto& edge : clockEdges){
		if(strcmp(edge.saveName, saveName) == 0) return &edge;
	}
	return nullptr;
}

VIPA_050_1BS00::ClockEdge* VIPA_050_1BS00::getClockEdge(ClockEdge::Edge clockEdge){
	for(auto& edge : clockEdges){
		if(clockEdge == edge.edge) return &edge;
	}
	return nullptr;
}

VIPA_050_1BS00::ClockEdge* VIPA_050_1BS00::getClockEdge(bool valueSetting){
	for(auto& edge : clockEdges){
		if(valueSetting == edge.bitValue) return &edge;
	}
	return nullptr;
}

struct Encoding{
	enum Format{
		BINARY,
		GRAY
	};
	Format format;
	bool bitValue;
	const char* saveName[64];
	const char* displayName[64];
};

std::vector<VIPA_050_1BS00::Encoding> VIPA_050_1BS00::encodings = {
	{VIPA_050_1BS00::Encoding::Format::BINARY, false, "Binary", "Binary"},
	{VIPA_050_1BS00::Encoding::Format::GRAY, true, "Gray", "Gray"}
};

VIPA_050_1BS00::Encoding* VIPA_050_1BS00::getEncoding(const char* saveName){
	for(auto& encoding : encodings){
		if(strcmp(encoding.saveName, saveName) == 0) return &encoding;
	}
	return nullptr;
}

VIPA_050_1BS00::Encoding* VIPA_050_1BS00::getEncoding(Encoding::Format format){
	for(auto& encoding : encodings){
		if(format == encoding.format) return &encoding;
	}
	return nullptr;
}

VIPA_050_1BS00::Encoding* VIPA_050_1BS00::getEncoding(bool valueSetting){
	for(auto& encoding : encodings){
		if(valueSetting == encoding.bitValue) return &encoding;
	}
	return nullptr;
}


//=================================================================
//================= 032-1BD70 4x 12bit Analog Output =================
//=================================================================

void VIPA_032_1BD70::onConstruction(){
	for(int i = 0; i < 4; i++){
		static char pinName[64];
		sprintf(pinName, "Analog Output %i", i);
		inputPins.push_back(std::make_shared<NodePin>(NodeData::Type::REAL_VALUE, DataDirection::NODE_INPUT, pinName));
	}
	inputBitCount = 0;
	inputByteCount = 0;
	outputBitCount = 64;
	outputByteCount = 8;
}

void VIPA_032_1BD70::onSetIndex(int i){
	for(int i = 0; i < 4; i++){
		sprintf((char*)inputPins[i]->getDisplayName(), "Module %i Analog Output %i", moduleIndex, i);
	}
}

void VIPA_032_1BD70::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	//this module has no tx pdo module
}

void VIPA_032_1BD70::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	rxPdoAssignement.addNewModule(0x1600 + moduleIndex);
	uint16_t dataObjectIndex = 0x7000 + moduleIndex;
	for(int i = 0; i < 4; i++){
		uint8_t subindex = i + 1;
		static char pdoEntryNameString[64];
		sprintf(pdoEntryNameString, "Analog Output %i", i);
		int16_t& dataReference = outputs[i];
		rxPdoAssignement.addEntry(dataObjectIndex, subindex, 16, pdoEntryNameString, &dataReference);
	}
}

bool VIPA_032_1BD70::configureParameters(){
	uint16_t settingsObjectIndex = 0x3100 + moduleIndex;
	uint8_t shortCircuitDetectionParameter = 0x0;
	for(int i = 0; i < 4; i++){
		if(shortCircuitDetectionSettings[i]) shortCircuitDetectionParameter |= 0x1 << i;
	}
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0x2, shortCircuitDetectionParameter)) return false;
	for(int i = 0; i < 4; i++){
		uint8_t voltageRangeSettingSubindex = i + 3;
		if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, voltageRangeSettingSubindex, getVoltageRange(voltageRangeSettings[i])->valueSetting)) return false;
	}
	return true;
}

void VIPA_032_1BD70::readInputs(){
	//no input reading for this SLIO Module
}

void VIPA_032_1BD70::writeOutputs(){
	for(int i = 0; i < 4; i++){
		double realValue;
		if(inputPins[i]->isConnected()){
			realValue = inputPins[i]->getConnectedPins().front()->getReal();
		}else{
			realValue = inputPins[i]->getReal();
		}
		int16_t outputValue;
		switch(voltageRangeSettings[i]){
			case VoltageRange::Range::ZERO_TO_10V:
				realValue = std::min(realValue, 10.0);
				realValue = std::max(realValue, 0.0); 	
				outputValue = 16384.0 * realValue / 10.0;
				break;
			case VoltageRange::Range::NEGATIVE_TO_POSITIVE_10V:
				realValue = std::min(realValue, 10.0);
				realValue = std::max(realValue, -10.0);
				outputValue = 16384.0 * realValue / 10.0;
				break;
		}
		outputs[i] = outputValue;
	}
}

std::vector<VIPA_032_1BD70::VoltageRange> VIPA_032_1BD70::voltageRanges = {
	{VIPA_032_1BD70::VoltageRange::Range::ZERO_TO_10V, 0x20, "ZeroToPositive10V", "0-10V"},
	{VIPA_032_1BD70::VoltageRange::Range::NEGATIVE_TO_POSITIVE_10V, 0x22, "NegativeToPositive10V", "\xc2\xb1 10V"}
};

VIPA_032_1BD70::VoltageRange* VIPA_032_1BD70::getVoltageRange(const char* saveName){
	for(auto& range : voltageRanges){
		if(strcmp(saveName, range.saveName) == 0) return &range;
	}
	return nullptr;
}

VIPA_032_1BD70::VoltageRange* VIPA_032_1BD70::getVoltageRange(VoltageRange::Range rng){
	for(auto& range : voltageRanges){
		if(rng == range.range) return &range;
	}
	return nullptr;
}

VIPA_032_1BD70::VoltageRange* VIPA_032_1BD70::getVoltageRange(uint8_t valueSetting){
	for(auto& range : voltageRanges){
		if(valueSetting == range.valueSetting) return &range;
	}
	return nullptr;
}

void VIPA_032_1BD70::moduleParameterGui(){
	for(int i = 0; i < 4; i++){
		ImGui::PushID(i);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Analog Channel %i", i);
		ImGui::PopFont();
		ImGui::Text("Voltage Range");
		if(ImGui::BeginCombo("##inputRangeSelector", getVoltageRange(voltageRangeSettings[i])->displayName)){
			for(auto& range : voltageRanges){
				if(ImGui::Selectable(range.displayName, voltageRangeSettings[i] == range.range)) voltageRangeSettings[i] = range.range;
			}
			ImGui::EndCombo();
		}
		ImGui::Checkbox("Enable Short Circuit Detection", &shortCircuitDetectionSettings[i]);
		ImGui::PopID();
	}
}

bool VIPA_032_1BD70::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	for(int i = 0; i < 4; i++){
		XMLElement* channelXML = xml->InsertNewChildElement("Channel");
		channelXML->SetAttribute("Range", getVoltageRange(voltageRangeSettings[i])->saveName);
		channelXML->SetAttribute("ShortCircuitDetection", shortCircuitDetectionSettings[i]);
	}
	return true;
}

bool VIPA_032_1BD70::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* channelXML = xml->FirstChildElement("Channel");
	
	for(int i = 0; i < 4; i++){
		
		if(i != 0) channelXML = channelXML->NextSiblingElement("Channel");
		if(channelXML == nullptr) return Logger::warn("Could not find channel attribute");
		
		const char* voltageRangeString;
		if(channelXML->QueryStringAttribute("Range", &voltageRangeString) != XML_SUCCESS) return Logger::warn("Could not find voltage range attribute");
		if(getVoltageRange(voltageRangeString) == nullptr) return Logger::warn("Could not read voltage range attribute");
		voltageRangeSettings[i] = getVoltageRange(voltageRangeString)->range;
		
		if(channelXML->QueryBoolAttribute("ShortCircuitDetection", &shortCircuitDetectionSettings[i]) != XML_SUCCESS) Logger::warn("Could not find short circuit detection attribute");
	}
	
	return true;
}
