#include <pch.h>

#include "VipaModule.h"

#include <imgui.h>
#include <tinyxml2.h>

#include "Fieldbus/EtherCatFieldbus.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

namespace VipaModuleFactory{

	std::vector<VipaModule*> modules = {
		new VIPA_022_1HD10(),
		new VIPA_021_1BF00(),
		new VIPA_022_1BF00(),
		new VIPA_050_1BS00(),
		new VIPA_032_1BD70(),
		new VIPA_050_1BB40()
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
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, pinName);
		std::shared_ptr<bool> pinValue = std::make_shared<bool>(false);
		pin->assignData(pinValue);
		inputPinValues.push_back(pinValue);
		inputPins.push_back(pin);
	}
	/*
	inputBitCount = 0;
	inputByteCount = 0;
	outputBitCount = 8;
	outputByteCount = 1;
	 */
}
void VIPA_022_1HD10::onSetIndex(int i){
	for(int i = 0; i < 4; i++){
		sprintf((char*)inputPins[i]->getDisplayString(), "Module %i Relais Output %i", moduleIndex, i);
		sprintf((char*)inputPins[i]->getSaveString(), "Module%iRelaisOutput%i", moduleIndex, i);
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
		if(inputPins[i]->isConnected()) inputPins[i]->copyConnectedPinValue();
		bool inputPinValue = inputPins[i]->read<bool>();
		outputs[i] = invertOutputs[i] ? !inputPinValue : inputPinValue;
	}
}

void VIPA_022_1HD10::moduleParameterGui(){
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("All relais return to open when fieldbus is stopped.");
	ImGui::PopStyleColor();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Signal Inversion");
	ImGui::PopFont();
	
	for(int i = 0; i < 4; i++){
		ImGui::PushID(i);
		ImGui::Checkbox("##invert", &invertOutputs[i]);
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", inputPins[i]->getDisplayString());
		ImGui::PopFont();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0.0));
		ImGui::SameLine();
		ImGui::PopStyleVar();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		if(invertOutputs[i]) ImGui::Text(" is inverted");
		else ImGui::Text(" is not inverted");
		ImGui::PopStyleColor();
		ImGui::PopID();
	}
}

bool VIPA_022_1HD10::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->InsertNewChildElement("SignalInversion");
	char attributeName[64];
	for(int i = 0; i < 4; i++){
		sprintf(attributeName, "InvertRelais%i", i);
		inversionXML->SetAttribute(attributeName, invertOutputs[i]);
	}
	return true;
}

bool VIPA_022_1HD10::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->FirstChildElement("SignalInversion");
	if(inversionXML == nullptr) Logger::warn("could not find signal inversion attribute");
	char attributeName[32];
	for(int i = 0; i < 4; i++){
		sprintf(attributeName, "InvertRelais%i", i);
		if(inversionXML->QueryBoolAttribute(attributeName, &invertOutputs[i]) != XML_SUCCESS) Logger::warn("could not find relais %i inversion attribute", i);
	}
	return true;
}

//=================================================================
//================== 021-1BF00 8x Digital Input ===================
//=================================================================

void VIPA_021_1BF00::onConstruction(){
	for(int i = 0; i < 8; i++){
		static char pinName[64];
		sprintf(pinName, "Digital Input %i", i);
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, pinName);
		std::shared_ptr<bool> pinValue = std::make_shared<bool>(false);
		pin->assignData(pinValue);
		outputPinValues.push_back(pinValue);
		outputPins.push_back(pin);
	}
	/*
	inputBitCount = 8;
	inputByteCount = 1;
	outputBitCount = 0;
	outputByteCount = 0;
	 */
}
void VIPA_021_1BF00::onSetIndex(int i){
	for(int i = 0; i < 8; i++){
		sprintf((char*)outputPins[i]->getDisplayString(), "Module %i Digital Input %i", moduleIndex, i);
		sprintf((char*)outputPins[i]->getSaveString(), "Module%iDigitalInput%i", moduleIndex, i);
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
		*outputPinValues[i] = invertInputs[i] ? !value : value;
	}
}
void VIPA_021_1BF00::writeOutputs(){
	//no output writing for this SLIO module
}

void VIPA_021_1BF00::moduleParameterGui(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Signal Inversion");
	ImGui::PopFont();
	for(int i = 0; i < 8; i++){
		ImGui::PushID(i);
		ImGui::Checkbox("##invert", &invertInputs[i]);
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", outputPins[i]->getDisplayString());
		ImGui::PopFont();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0.0));
		ImGui::SameLine();
		ImGui::PopStyleVar();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		if(invertInputs[i]) ImGui::Text(" is inverted");
		else ImGui::Text(" is not inverted");
		ImGui::PopStyleColor();
		ImGui::PopID();
	}
}

bool VIPA_021_1BF00::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->InsertNewChildElement("SignalInversion");
	char attributeName[64];
	for(int i = 0; i < 8; i++){
		sprintf(attributeName, "InvertInput%i", i);
		inversionXML->SetAttribute(attributeName, invertInputs[i]);
	}
	return true;
}

bool VIPA_021_1BF00::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->FirstChildElement("SignalInversion");
	if(inversionXML == nullptr) Logger::warn("could not find signal inversion attribute");
	char attributeName[32];
	for(int i = 0; i < 8; i++){
		sprintf(attributeName, "InvertInput%i", i);
		if(inversionXML->QueryBoolAttribute(attributeName, &invertInputs[i]) != XML_SUCCESS) Logger::warn("could not find input %i inversion attribute", i);
	}
	return true;
}

//=================================================================
//================== 022-1BF00 8x Digital Output ===================
//=================================================================

void VIPA_022_1BF00::onConstruction(){
	for(int i = 0; i < 8; i++){
		static char pinName[64];
		sprintf(pinName, "Digital Output %i", i);
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, pinName);
		std::shared_ptr<bool> pinValue = std::make_shared<bool>(false);
		pin->assignData(pinValue);
		inputPinValues.push_back(pinValue);
		inputPins.push_back(pin);
	}
	/*
	inputBitCount = 0;
	inputByteCount = 0;
	outputBitCount = 8;
	outputByteCount = 1;
	 */
}

void VIPA_022_1BF00::onSetIndex(int i){
	for(int i = 0; i < 8; i++){
		sprintf((char*)inputPins[i]->getDisplayString(), "Module %i Digital Output %i", moduleIndex, i);
		sprintf((char*)inputPins[i]->getSaveString(), "Module%iDigitalOutput%i", moduleIndex, i);
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
		if(inputPins[i]->isConnected()) inputPins[i]->copyConnectedPinValue();
		bool inputPinValue = *inputPinValues[i];
		outputs[i] = invertOutputs[i] ? !inputPinValue : inputPinValue;
	}
}

void VIPA_022_1BF00::moduleParameterGui(){
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("All outputs return to low when fieldbus is stopped.");
	ImGui::PopStyleColor();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Signal Inversion");
	ImGui::PopFont();
	for(int i = 0; i < 8; i++){
		ImGui::PushID(i);
		ImGui::Checkbox("##invert", &invertOutputs[i]);
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", inputPins[i]->getDisplayString());
		ImGui::PopFont();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0.0));
		ImGui::SameLine();
		ImGui::PopStyleVar();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		if(invertOutputs[i]) ImGui::Text(" is inverted");
		else ImGui::Text(" is not inverted");
		ImGui::PopStyleColor();
		ImGui::PopID();
	}
}

bool VIPA_022_1BF00::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->InsertNewChildElement("SignalInversion");
	char attributeName[64];
	for(int i = 0; i < 8; i++){
		sprintf(attributeName, "InvertOutput%i", i);
		inversionXML->SetAttribute(attributeName, invertOutputs[i]);
	}
	return true;
}

bool VIPA_022_1BF00::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->FirstChildElement("SignalInversion");
	if(inversionXML == nullptr) Logger::warn("could not find signal inversion attribute");
	char attributeName[32];
	for(int i = 0; i < 8; i++){
		sprintf(attributeName, "InvertOutput%i", i);
		if(inversionXML->QueryBoolAttribute(attributeName, &invertOutputs[i]) != XML_SUCCESS) Logger::warn("could not find output %i inversion attribute", i);
	}
	return true;
}

//=================================================================
//============== 050-1BS00 Single SSI Encoder Input ===============
//=================================================================

void VIPA_050_1BS00::onConstruction(){
	auto thisEncoderModule = std::static_pointer_cast<VIPA_050_1BS00>(shared_from_this());
	encoder = std::make_shared<VIPA_050_1BS00::SsiEncoder>(thisEncoderModule);
	
	encoder->feedbackConfig.b_supportsForceFeedback = false;
	encoder->feedbackConfig.b_supportsPositionFeedback = true;
	encoder->feedbackConfig.b_supportsVelocityFeedback = true;
	
	encoderPin->assignData(std::static_pointer_cast<MotionFeedbackInterface>(encoder));
	outputPins.push_back(encoderPin);
	resetPin->assignData(resetPinValue);
	outputPins.push_back(resetPin);
	updateResetPinVisibility();
}

void VIPA_050_1BS00::onSetParentBusCoupler(std::shared_ptr<VipaBusCoupler_053_1EC01> busCoupler){
	//encoder->setParentDevice(busCoupler);
}

void VIPA_050_1BS00::onSetIndex(int i){
	sprintf((char*)encoderPin->getDisplayString(), "Module %i SSI Encoder", moduleIndex);
	sprintf((char*)encoderPin->getSaveString(), "Module%iSSIEncoder", moduleIndex);
	sprintf((char*)resetPin->getDisplayString(), "Module %i SSI Encoder Reset", moduleIndex);
	sprintf((char*)resetPin->getSaveString(), "Module%iEncoderReset", moduleIndex);
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
	
	uint16_t idleTimeSetting = getMeasurementPauseTimeValue(pausetime);
	if(!parentBusCoupler->writeSDO_U16(settingsObjectIndex, 0x2, idleTimeSetting)) return false;
	
	uint16_t transmissionFrequencySetting = getTransmissionRateValue(transmissionFrequency);
	if(!parentBusCoupler->writeSDO_U16(settingsObjectIndex, 0x3, transmissionFrequencySetting)) return false;
	
	uint8_t normalisationBitCountSetting = normalisationBitCountToCanValue(normalisationBitCount);
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0x5, normalisationBitCountSetting)) return false;
	
	uint8_t encoderBitCountSetting = encoderBitCountToCanValue(encoderBitCount);
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0x6, encoderBitCountSetting)) return false;
	
	uint8_t ssiModeSetting = 0x0;
	ssiModeSetting |= 0b10; //set ssi master mode
	if(getBitDirectionValue(bitshiftDirection)) ssiModeSetting |= 0b100;
	if(getClockEdgeValue(clockEdge)) ssiModeSetting |= 0b1000;
	if(getEncodingValue(encodingFormat)) ssiModeSetting |= 0b10000;
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
	
	int totalIncrements = 0x1 << (multiturnBitCount + singleTurnBitCount);
	int positionIncrements = encoderValue;
	if(b_invertDirection) positionIncrements = totalIncrements - encoderValue;
	
	encoderPosition_revolutions = (double)positionIncrements / (double)incrementsPerRevolution;
	if(b_centerRangeOnZero && encoderPosition_revolutions >= maxRevolutions / 2) encoderPosition_revolutions -= maxRevolutions;
	
	uint16_t deltaT_microseconds = time_microseconds - previousReadingTime_microseconds;
	double readingDeltaT_seconds = double(deltaT_microseconds) / 1000000.0;
	double positionDelta_revolutions = encoderPosition_revolutions - previousEncoderPosition_revolutions;
	encoderVelocity_revolutionsPerSecond = positionDelta_revolutions / readingDeltaT_seconds;
	
	previousReadingTime_microseconds = time_microseconds;
	previousEncoderPosition_revolutions = encoderPosition_revolutions;
	
	encoder->feedbackProcessData.positionActual = encoderPosition_revolutions + positionOffset;
	encoder->feedbackProcessData.velocityActual = encoderVelocity_revolutionsPerSecond;
	
	if(parentBusCoupler->isStateOperational()) encoder->state = DeviceState::ENABLED;
	else encoder->state = DeviceState::OFFLINE;
	
	
}

void VIPA_050_1BS00::writeOutputs(){
	
	if(encoder->feedbackProcessData.b_overridePosition){
		encoder->feedbackProcessData.b_overridePosition = false;
		encoder->feedbackProcessData.b_positionOverrideBusy = true;
		
		if(b_hasResetSignal){
			*resetPinValue = true;
			resetStartTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
			Logger::info("[{}] resetting SSI encoder", encoder->getName());
		}else{
			positionOffset = encoder->feedbackProcessData.positionOverride - encoderPosition_revolutions;
			encoder->feedbackProcessData.b_positionOverrideBusy = false;
			encoder->feedbackProcessData.b_positionOverrideSucceeded = true;
			Logger::info("[{}] Encoder position reset", encoder->getName());
			updateEncoderWorkingRange();
		}
		
	
	}
	
	if(encoder->feedbackProcessData.b_positionOverrideBusy){
		encoder->feedbackProcessData.velocityActual = 0.0;
		if(encoderValue == 0x0){
			encoder->feedbackProcessData.b_positionOverrideBusy = false;
			encoder->feedbackProcessData.b_positionOverrideSucceeded = true;
			positionOffset = encoder->feedbackProcessData.positionOverride;
			encoder->feedbackProcessData.positionActual = encoder->feedbackProcessData.positionOverride;
			*resetPinValue = false;
			updateEncoderWorkingRange();
			Logger::info("[{}] Successfully reset encoder position", encoder->getName());
		}
		else if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - resetStartTime_nanoseconds > resetTime_milliseconds * 1'000'000){
			encoder->feedbackProcessData.b_positionOverrideBusy = false;
			encoder->feedbackProcessData.b_positionOverrideSucceeded = false;
			*resetPinValue = false;
			Logger::error("[{}] Failed to reset encoder position", encoder->getName());
		}
	}
	
}

void VIPA_050_1BS00::onDisconnection(){
	encoder->state = DeviceState::OFFLINE;
}

void VIPA_050_1BS00::moduleParameterGui(){
	
	ImGui::Text("Encoding");
	if(ImGui::BeginCombo("##EncodingSelector", Enumerator::getDisplayString(encodingFormat))){
		for(auto& type : Enumerator::getTypes<Encoding>()){
			if(ImGui::Selectable(type.displayString, encodingFormat == type.enumerator)) encodingFormat = type.enumerator;
		}
		ImGui::EndCombo();
	}
	
	ImGui::Text("SSI Transmission Frequency");
	if(ImGui::BeginCombo("##TransmissionFrequencySelector", Enumerator::getDisplayString(transmissionFrequency))){
		for(auto& type : Enumerator::getTypes<TransmissionRate>()){
			if(ImGui::Selectable(type.displayString, transmissionFrequency == type.enumerator)) transmissionFrequency = type.enumerator;
		}
		ImGui::EndCombo();
	}
	
	ImGui::Text("SSI Measurement Pause Time");
	if(ImGui::BeginCombo("##PausetimeSelector", Enumerator::getDisplayString(pausetime))){
		for(auto& type : Enumerator::getTypes<MeasurementPauseTime>()){
			if(ImGui::Selectable(type.displayString, pausetime == type.enumerator)) pausetime = type.enumerator;
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
	
	if(ImGui::Checkbox("Invert Direction", &b_invertDirection)) updateEncoderWorkingRange();
	
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
		if(ImGui::BeginCombo("##ShiftDirectionSelector", Enumerator::getDisplayString(bitshiftDirection))){
			for(auto& type : Enumerator::getTypes<BitDirection>()){
				if(ImGui::Selectable(type.displayString, bitshiftDirection == type.enumerator)) bitshiftDirection = type.enumerator;
			}
			ImGui::EndCombo();
		}
		
		ImGui::Text("Data Clock Edge");
		if(ImGui::BeginCombo("##ClockEdgeSelector", Enumerator::getDisplayString(clockEdge))){
			for(auto& type : Enumerator::getTypes<ClockEdge>()){
				if(ImGui::Selectable(type.displayString, clockEdge == type.enumerator)) clockEdge = type.enumerator;
			}
			ImGui::EndCombo();
		}
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	
	double pos = encoder->getPosition();
	double min = encoder->getPositionLowerWorkingRangeBound();
	double max = encoder->getPositionUpperWorkingRangeBound();
	ImGui::Text("Encoder position in working range (%.3f to %.3f revolutions)", min, max);
	
	glm::vec2 progressBarSize(widgetWidth, ImGui::GetFrameHeight());
	
	static char encoderRangeProgressString[64];
	sprintf(encoderRangeProgressString, "%.3f revolutions", pos);
	ImGui::ProgressBar((float)encoder->getPositionNormalizedToWorkingRange(), progressBarSize, encoderRangeProgressString);
	   
	static char encoderVelocityString[64];
	sprintf(encoderVelocityString, "%.3f rev/s", encoderVelocity_revolutionsPerSecond);
	float velocityProgress = std::abs(encoderVelocity_revolutionsPerSecond) / 10.0;
	ImGui::ProgressBar(velocityProgress, progressBarSize, encoderVelocityString);
	
	bool disableResetButton = !b_hasResetSignal;
	//ImGui::BeginDisabled(disableResetButton);
	if(ImGui::Button("Reset Encoder")) encoder->overridePosition(0.0);
	ImGui::SameLine();
	if(ImGui::Button("Reset Offset")) positionOffset = 0.0;
	//ImGui::EndDisabled();
	ImGui::Text("Raw SSI Value: %i", encoderValue);
	ImGui::Text("Encoder Position: %.3f", encoderPosition_revolutions);
	ImGui::Text("Position Offset: %.3f", positionOffset);
	ImGui::Text("Output Position: %.3f", encoder->getPosition());
}

void VIPA_050_1BS00::updateEncoderWorkingRange(){
	int multiturnBitCount = encoderBitCount - singleTurnBitCount;
	int maxRevolutions = 0x1 << multiturnBitCount;
	
	double workingRangeMin = 0.0;
	double workingRangeMax = maxRevolutions;
	if(b_centerRangeOnZero){
		workingRangeMin -= maxRevolutions / 2.0;
		workingRangeMax -= maxRevolutions / 2.0;
	}
	
	encoder->feedbackConfig.positionLowerWorkingRangeBound = workingRangeMin + positionOffset;
	encoder->feedbackConfig.positionUpperWorkingRangeBound = workingRangeMax + positionOffset;
}

void VIPA_050_1BS00::updateResetPinVisibility(){
	resetPin->setVisible(b_hasResetSignal);
}

bool VIPA_050_1BS00::save(tinyxml2::XMLElement* xml){
	xml->SetAttribute("Encoding", Enumerator::getSaveString(encodingFormat));
	xml->SetAttribute("TransmissionRate", Enumerator::getSaveString(transmissionFrequency));
	xml->SetAttribute("IdleTime", Enumerator::getSaveString(pausetime));
	xml->SetAttribute("CenterRangeOnZero", b_centerRangeOnZero);
	xml->SetAttribute("InvertDirection", b_invertDirection);
	xml->SetAttribute("HasResetSignal", b_hasResetSignal);
	if(b_hasResetSignal) xml->SetAttribute("ResetTime", resetTime_milliseconds);
	xml->SetAttribute("TotalBitCount", encoderBitCount);
	xml->SetAttribute("SingleTurnBitCount", singleTurnBitCount);
	xml->SetAttribute("IgnoredBitCount", normalisationBitCount);
	xml->SetAttribute("BitShiftDirection", Enumerator::getSaveString(bitshiftDirection));
	xml->SetAttribute("ClockEdge", Enumerator::getSaveString(clockEdge));
	xml->SetAttribute("PositionOffset", positionOffset);
	return true;
}

bool VIPA_050_1BS00::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	const char* encodingString;
	if(xml->QueryStringAttribute("Encoding", &encodingString) != XML_SUCCESS) return Logger::warn("Could not find Encoding attribute");
	if(!Enumerator::isValidSaveName<Encoding>(encodingString)) return Logger::warn("Could not identify Encoding attribute");
	encodingFormat = Enumerator::getEnumeratorFromSaveString<Encoding>(encodingString);
	const char* frequencyString;
	if(xml->QueryStringAttribute("TransmissionRate", &frequencyString) != XML_SUCCESS) return Logger::warn("Could not find Frequency attribute");
	if(!Enumerator::isValidSaveName<TransmissionRate>(frequencyString)) return Logger::warn("Could not identify Transmission Rate attribute");
	transmissionFrequency = Enumerator::getEnumeratorFromSaveString<TransmissionRate>(frequencyString);
	const char* idleTimeString;
	if(xml->QueryStringAttribute("IdleTime", &idleTimeString) != XML_SUCCESS) return Logger::warn("Could not find Idle Time attribute");
	if(!Enumerator::isValidSaveName<MeasurementPauseTime>(idleTimeString)) return Logger::warn("Could not identify idle time attribute");
	pausetime = Enumerator::getEnumeratorFromSaveString<MeasurementPauseTime>(idleTimeString);
	if(xml->QueryBoolAttribute("InvertDirection", &b_invertDirection) != XML_SUCCESS) return Logger::warn("Could not identify Invert Direction attribute");
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
	if(!Enumerator::isValidSaveName<BitDirection>(bitShiftDirectionString)) return Logger::warn("Could not identify Bit shift attribute");
	bitshiftDirection = Enumerator::getEnumeratorFromSaveString<BitDirection>(bitShiftDirectionString);
	const char* clockEdgeString;
	if(xml->QueryStringAttribute("ClockEdge", &clockEdgeString) != XML_SUCCESS) return Logger::warn("Could not find Clock Edge attribute");
	if(!Enumerator::isValidSaveName<ClockEdge>(clockEdgeString)) return Logger::warn("Could not identify Clock Edge Attribute");
	clockEdge = Enumerator::getEnumeratorFromSaveString<ClockEdge>(clockEdgeString);
	if(xml->QueryDoubleAttribute("PositionOffset", &positionOffset) != XML_SUCCESS) return Logger::warn("Could not find position Offset attribute");
	updateEncoderWorkingRange();
	return true;
}






//=================================================================
//================= 032-1BD70 4x 12bit Analog Output =================
//=================================================================

void VIPA_032_1BD70::onConstruction(){
	for(int i = 0; i < 4; i++){
		static char pinName[64];
		sprintf(pinName, "Analog Output %i", i);
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, pinName);
		std::shared_ptr<double> pinValue = std::make_shared<double>(0.0);
		pin->assignData(pinValue);
		inputPinValues.push_back(pinValue);
		inputPins.push_back(pin);
	}
	/*
	inputBitCount = 0;
	inputByteCount = 0;
	outputBitCount = 64;
	outputByteCount = 8;
	 */
}

void VIPA_032_1BD70::onSetIndex(int i){
	for(int i = 0; i < 4; i++){
		sprintf((char*)inputPins[i]->getDisplayString(), "Module %i Analog Output %i", moduleIndex, i);
		sprintf((char*)inputPins[i]->getSaveString(), "Module%iAnalogOutput%i", moduleIndex, i);
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
		uint8_t voltageRangeSettingValue = getVoltageRangeValue(voltageRangeSettings[i]);
		if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, voltageRangeSettingSubindex, voltageRangeSettingValue)) return false;
	}
	return true;
}

void VIPA_032_1BD70::readInputs(){
	//no input reading for this SLIO Module
}

void VIPA_032_1BD70::writeOutputs(){
	for(int i = 0; i < 4; i++){
		double realValue;
		if(inputPins[i]->isConnected()) inputPins[i]->copyConnectedPinValue();
		realValue = *inputPinValues[i];
		int16_t outputValue;
		switch(voltageRangeSettings[i]){
			case VoltageRange::ZERO_TO_10V:
				realValue = std::min(realValue, 10.0);
				realValue = std::max(realValue, 0.0); 	
				outputValue = 16384.0 * realValue / 10.0;
				break;
			case VoltageRange::NEGATIVE_TO_POSITIVE_10V:
				realValue = std::min(realValue, 10.0);
				realValue = std::max(realValue, -10.0);
				outputValue = 16384.0 * realValue / 10.0;
				break;
		}
		outputs[i] = outputValue;
	}
}

void VIPA_032_1BD70::moduleParameterGui(){
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::Text("All analog outputs return to 0 Volts when fieldbus is stopped.");
	ImGui::PopStyleColor();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Output Voltage Ranges");
	ImGui::PopFont();
	
	for(int i = 0; i < 4; i++){
		ImGui::PushID(i);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", inputPins[i]->getDisplayString());
		ImGui::PopFont();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		if(ImGui::BeginCombo("##inputRangeSelector", Enumerator::getDisplayString(voltageRangeSettings[i]))){
			for(auto& type : Enumerator::getTypes<VoltageRange>()){
				if(ImGui::Selectable(type.displayString, voltageRangeSettings[i] == type.enumerator)) voltageRangeSettings[i] = type.enumerator;
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Short Circuit Detection", &shortCircuitDetectionSettings[i]);
		ImGui::PopID();
	}
}

bool VIPA_032_1BD70::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	for(int i = 0; i < 4; i++){
		XMLElement* channelXML = xml->InsertNewChildElement("Channel");
		channelXML->SetAttribute("Range", Enumerator::getSaveString(voltageRangeSettings[i]));
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
		if(!Enumerator::isValidSaveName<VoltageRange>(voltageRangeString)) return Logger::warn("Could not read voltage range attribute");
		voltageRangeSettings[i] = Enumerator::getEnumeratorFromSaveString<VoltageRange>(voltageRangeString);
		
		if(channelXML->QueryBoolAttribute("ShortCircuitDetection", &shortCircuitDetectionSettings[i]) != XML_SUCCESS) Logger::warn("Could not find short circuit detection attribute");
	}
	
	return true;
}










//=================================================================
//============ 050-1BB40 Two Channel Frequency Counter ============
//=================================================================

void VIPA_050_1BB40::onConstruction(){
	outputPins.push_back(frequency0Pin);
	outputPins.push_back(frequency1Pin);
	/*
	inputBitCount = 160;
	inputByteCount = 20;
	outputBitCount = 96;
	outputByteCount = 12;
	 */
	channel0MeasurementPeriod = NumberParameter<double>::make(0.1,
															  "Channel 0 Measurement Period",
															  "Ch0MeasurementPeriod",
															  "%.4f",
															  Units::Time::Second,
															  false);
	channel1MeasurementPeriod = NumberParameter<double>::make(0.1,
															  "Channel 1 Measurement Period",
															  "Ch1MeasurementPeriod",
															  "%.4f",
															  Units::Time::Second,
															  false);
}

void VIPA_050_1BB40::onSetIndex(int i){
	sprintf((char*)frequency0Pin->getDisplayString(), "Module %i Frequency Counter 1", moduleIndex);
	sprintf((char*)frequency1Pin->getDisplayString(), "Module %i Frequency Counter 2", moduleIndex);
}

void VIPA_050_1BB40::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	//this module has no tx pdo module
	txPdoAssignement.addNewModule(0x1A00 + moduleIndex);
	uint16_t dataObjectIndex = 0x6000 + moduleIndex;
	txPdoAssignement.addEntry(dataObjectIndex, 1, 32, "FM_PERIOD_CH0", &fm_period_ch0);
	txPdoAssignement.addEntry(dataObjectIndex, 2, 32, "FM_RISING_EDGES_CH0", &fm_rising_edges_ch0);
	txPdoAssignement.addEntry(dataObjectIndex, 3, 32, "FM_PERIOD_CH1", &fm_period_ch1);
	txPdoAssignement.addEntry(dataObjectIndex, 4, 32, "FM_RISING_EDGES_CH1", &fm_rising_edges_ch1);
	txPdoAssignement.addEntry(dataObjectIndex, 5, 16, "FM_STATUS_CH0", &fm_status_ch0);
	txPdoAssignement.addEntry(dataObjectIndex, 6, 16, "FM_STATUS_CH1", &fm_status_ch1);
}

void VIPA_050_1BB40::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	rxPdoAssignement.addNewModule(0x1600 + moduleIndex);
	uint16_t dataObjectIndex = 0x7000 + moduleIndex;
	rxPdoAssignement.addEntry(dataObjectIndex, 1, 32, "FM_PRESET_PERIOD_CH0", &fm_preset_period_ch0);
	rxPdoAssignement.addEntry(dataObjectIndex, 2, 32, "FM_PRESET_PERIOD_CH1", &fm_preset_period_ch1);
	rxPdoAssignement.addEntry(dataObjectIndex, 3, 16, "FM_CONTROL_CH0", &fm_control_ch0);
	rxPdoAssignement.addEntry(dataObjectIndex, 4, 16, "FM_CONTROL_CH1", &fm_control_ch1);
}

bool VIPA_050_1BB40::configureParameters(){
	uint16_t settingsObjectIndex = 0x3100 + moduleIndex;
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0x2, getInputFilterCode(channel0InputFilter->value))) return false;
	if(!parentBusCoupler->writeSDO_U8(settingsObjectIndex, 0x4, getInputFilterCode(channel1InputFilter->value))) return false;
	return true;
}

void VIPA_050_1BB40::readInputs(){
	double ch0_period = fm_period_ch0 / 8000000.0; //measurement period in seconds
	double ch1_period = fm_period_ch1 / 8000000.0;
	bool ch0_active = fm_status_ch0 & 0x100;
	bool ch1_active = fm_status_ch1 & 0x100;
	
	if(ch0_period > 0.0) *frequency0Value = fm_rising_edges_ch0 / ch0_period;
	if(ch1_period > 0.0) *frequency1Value = fm_rising_edges_ch1 / ch1_period;
}

void VIPA_050_1BB40::writeOutputs(){
	fm_control_ch0 = 0x100;
	fm_control_ch1 = 0x100;
	fm_preset_period_ch0 = channel0MeasurementPeriod->getValue() * 1000000.0;
	fm_preset_period_ch1 = channel1MeasurementPeriod->getValue() * 1000000.0;
}

void VIPA_050_1BB40::moduleParameterGui(){
	 
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Channel 1");
	ImGui::PopFont();
	
	ImGui::Text("Input Filter");
	channel0InputFilter->gui();
	
	ImGui::Text("Measurement Period");
	channel0MeasurementPeriod->gui();
	
	//ImGui::Checkbox("Enable##ch0", &ch0Control);
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Channel 2");
	ImGui::PopFont();
	
	ImGui::Text("Input Filter");
	channel1InputFilter->gui();
	
	ImGui::Text("Measurement Period");
	channel1MeasurementPeriod->gui();
	
	//ImGui::Checkbox("Enable##ch1", &ch1Control);
}

bool VIPA_050_1BB40::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	channel0InputFilter->save(xml);
	channel1InputFilter->save(xml);
	channel0MeasurementPeriod->save(xml);
	channel1MeasurementPeriod->save(xml);
	return true;
}

bool VIPA_050_1BB40::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	if(!channel0InputFilter->load(xml)){
		Logger::warn("could not load attribute channel 0 input filter of vipa module frequency counter");
		return false;
	}
	if(!channel1InputFilter->load(xml)){
		Logger::warn("could not load attribute channel 1 input filter of vipa module frequency counter");
		return false;
	}
	if(!channel0MeasurementPeriod->load(xml)){
		Logger::warn("could not load attribute channel 0 measurement time of vipa module frequency counter");
		return false;
	}
	if(!channel1MeasurementPeriod->load(xml)){
		Logger::warn("could not load attribute channel 1 measurement time of vipa module frequency counter");
		return false;
	}
	return true;
}
