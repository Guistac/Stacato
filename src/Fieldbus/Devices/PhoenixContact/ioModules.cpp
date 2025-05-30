#include <pch.h>
#include "ioModules.h"

#include "Fieldbus/EtherCatFieldbus.h"

namespace PhoenixContact{


namespace ModuleFactory{
	std::vector<EtherCAT::ModularDeviceProfile::DeviceModule*>& getModules(){
		static std::vector<EtherCAT::ModularDeviceProfile::DeviceModule*> moduleList = {
			new IB_IL_24_DI_4(),
			new IB_IL_24_DO_4(),
			new IB_IL_24_48_DOR_2(),
			new IB_IL_SSI_IN()
		};
		return moduleList;
	}
}



//=================================================================
//======================= 4x Digital Input ========================
//=================================================================

void IB_IL_24_DI_4::onConstruction(){
	for(int i = 0; i < 4; i++){
		static char pinName[64];
		int inputNumber = i + 1;
		snprintf(pinName, 64, "Digital Input %i", inputNumber);
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, pinName);
		std::shared_ptr<bool> pinValue = std::make_shared<bool>(false);
		pin->assignData(pinValue);
		outputPinValues.push_back(pinValue);
		outputPins.push_back(pin);
	}
}
void IB_IL_24_DI_4::onSetIndex(int i){
	for(int i = 0; i < 4; i++){
		int inputNumber = i + 1;
		sprintf((char*)outputPins[i]->getDisplayString(), "Module %i Digital Input %i", moduleIndex, inputNumber);
		sprintf((char*)outputPins[i]->getSaveString(), "Module%iDigitalInput%i", moduleIndex, inputNumber);
	}
}
void IB_IL_24_DI_4::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	txPdoAssignement.addNewModule(0x1A00 + moduleIndex);
	txPdoAssignement.addEntry(0x6000 + moduleIndex * 0x10, 0x1, 8, "Digital Inputs", &inputByte);
}
void IB_IL_24_DI_4::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	//module has no rxPdo
}
bool IB_IL_24_DI_4::configureParameters(){ return true; }
void IB_IL_24_DI_4::readInputs(){
	for(int i = 0; i < 4; i++){
		bool value = (inputByte >> i) & 0x1;
		*outputPinValues[i] = invertInputs[i] ? !value : value;
	}
}
void IB_IL_24_DI_4::writeOutputs(){}
void IB_IL_24_DI_4::moduleGui(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Signal Inversion");
	ImGui::PopFont();
	for(int i = 0; i < 4; i++){
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
bool IB_IL_24_DI_4::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->InsertNewChildElement("SignalInversion");
	char attributeName[64];
	for(int i = 0; i < 4; i++){
		int inputNumber = i + 1;
		sprintf(attributeName, "InvertInput%i", inputNumber);
		inversionXML->SetAttribute(attributeName, invertInputs[i]);
	}
	return true;
}
bool IB_IL_24_DI_4::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->FirstChildElement("SignalInversion");
	if(inversionXML == nullptr) {
		return Logger::warn("could not find signal inversion attribute");
	}
	char attributeName[32];
	for(int i = 0; i < 4; i++){
		int inputNumber = i + 1;
		sprintf(attributeName, "InvertInput%i", inputNumber);
		if(inversionXML->QueryBoolAttribute(attributeName, &invertInputs[i]) != XML_SUCCESS) {
			return Logger::warn("could not find input %i inversion attribute", inputNumber);
		}
	}
	return true;
}

//=================================================================
//======================= 4x Digital Output =======================
//=================================================================

void IB_IL_24_DO_4::onConstruction(){
	for(int i = 0; i < 4; i++){
		static char pinName[64];
		int outputNumber = i + 1;
		sprintf(pinName, "Digital Output %i", outputNumber);
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, pinName);
		std::shared_ptr<bool> pinValue = std::make_shared<bool>(false);
		pin->assignData(pinValue);
		inputPinValues.push_back(pinValue);
		inputPins.push_back(pin);
	}
}
void IB_IL_24_DO_4::onSetIndex(int i){
	for(int i = 0; i < 4; i++){
		int outputNumber = i + 1;
		sprintf((char*)inputPins[i]->getDisplayString(), "Module %i Digital Output %i", moduleIndex, outputNumber);
		sprintf((char*)inputPins[i]->getSaveString(), "Module%iDigitalOutput%i", moduleIndex, outputNumber);
	}
}
void IB_IL_24_DO_4::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	//module has no txPdo
}
void IB_IL_24_DO_4::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	rxPdoAssignement.addNewModule(0x1600 + moduleIndex);
	rxPdoAssignement.addEntry(0x7000 + moduleIndex * 0x10, 0x1, 8, "Digital Outputs", &outputByte);
}
bool IB_IL_24_DO_4::configureParameters(){ return true; }
void IB_IL_24_DO_4::readInputs(){}
void IB_IL_24_DO_4::writeOutputs(){
	outputByte = 0x0;
	for(int i = 0; i < 4; i++){
		if(inputPins[i]->isConnected()) inputPins[i]->copyConnectedPinValue();
		bool inputPinValue = *inputPinValues[i];
		if(invertOutputs[i]) inputPinValue = !inputPinValue;
		if(inputPinValue) outputByte |= (0x1 << i);
	}
}
void IB_IL_24_DO_4::moduleGui(){
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("All outputs return to low when fieldbus is stopped.");
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
bool IB_IL_24_DO_4::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->InsertNewChildElement("SignalInversion");
	char attributeName[64];
	for(int i = 0; i < 4; i++){
		int outputNumber = i + 1;
		sprintf(attributeName, "InvertOutput%i", outputNumber);
		inversionXML->SetAttribute(attributeName, invertOutputs[i]);
	}
	return true;
}
bool IB_IL_24_DO_4::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->FirstChildElement("SignalInversion");
	if(inversionXML == nullptr) {
		return Logger::warn("could not find signal inversion attribute");
	}
	char attributeName[32];
	for(int i = 0; i < 4; i++){
		int outputNumber = i + 1;
		sprintf(attributeName, "InvertOutput%i", outputNumber);
		if(inversionXML->QueryBoolAttribute(attributeName, &invertOutputs[i]) != XML_SUCCESS){
			return Logger::warn("could not find output %i inversion attribute", outputNumber);
		}
	}
	return true;
}


//=======================================================
//================== Relais Output x2 ===================
//=======================================================



void IB_IL_24_48_DOR_2::onConstruction(){
	for(int i = 0; i < 2; i++){
		static char pinName[64];
		int outputNumber = i + 1;
		sprintf(pinName, "Relais Output %i", outputNumber);
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, pinName);
		std::shared_ptr<bool> pinValue = std::make_shared<bool>(false);
		pin->assignData(pinValue);
		inputPinValues.push_back(pinValue);
		inputPins.push_back(pin);
	}
}

void IB_IL_24_48_DOR_2::onSetIndex(int i){
	for(int i = 0; i < 2; i++){
		int outputNumber = i + 1;
		sprintf((char*)inputPins[i]->getDisplayString(), "Module %i Relais Output %i", moduleIndex, outputNumber);
		sprintf((char*)inputPins[i]->getSaveString(), "Module%iRelaisOutput%i", moduleIndex, outputNumber);
	}
}
void IB_IL_24_48_DOR_2::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	//module has no txPdo
}
void IB_IL_24_48_DOR_2::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	rxPdoAssignement.addNewModule(0x1600 + moduleIndex);
	rxPdoAssignement.addEntry(0x7000 + moduleIndex * 0x10, 0x1, 8, "Relais Outputs", &outputByte);
}
bool IB_IL_24_48_DOR_2::configureParameters(){ return true; }
void IB_IL_24_48_DOR_2::readInputs(){}
void IB_IL_24_48_DOR_2::writeOutputs(){
	outputByte = 0x0;
	for(int i = 0; i < 2; i++){
		if(inputPins[i]->isConnected()) inputPins[i]->copyConnectedPinValue();
		bool inputPinValue = *inputPinValues[i];
		if(invertOutputs[i]) inputPinValue = !inputPinValue;
		if(inputPinValue) outputByte |= (0x1 << i);
	}
}
void IB_IL_24_48_DOR_2::moduleGui(){
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("All outputs return to open when fieldbus is stopped.");
	ImGui::PopStyleColor();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Signal Inversion");
	ImGui::PopFont();
	for(int i = 0; i < 2; i++){
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
bool IB_IL_24_48_DOR_2::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->InsertNewChildElement("SignalInversion");
	char attributeName[64];
	for(int i = 0; i < 2; i++){
		int outputNumber = i + 1;
		sprintf(attributeName, "InvertOutput%i", outputNumber);
		inversionXML->SetAttribute(attributeName, invertOutputs[i]);
	}
	return true;
}
bool IB_IL_24_48_DOR_2::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* inversionXML = xml->FirstChildElement("SignalInversion");
	if(inversionXML == nullptr) {
		return Logger::warn("could not find signal inversion attribute");
	}
	char attributeName[32];
	for(int i = 0; i < 2; i++){
		int outputNumber = i + 1;
		sprintf(attributeName, "InvertOutput%i", outputNumber);
		if(inversionXML->QueryBoolAttribute(attributeName, &invertOutputs[i]) != XML_SUCCESS){
			return Logger::warn("could not find output %i inversion attribute", outputNumber);
		}
	}
	return true;
}




//=================================================================
//============== 050-1BS00 Single SSI Encoder Input ===============
//=================================================================

void IB_IL_SSI_IN::onConstruction(){
	
	auto thisEncoderModule = std::static_pointer_cast<IB_IL_SSI_IN>(shared_from_this());
	encoder = std::make_shared<IB_IL_SSI_IN::SsiEncoder>(thisEncoderModule);
	encoderPin->assignData(std::static_pointer_cast<MotionFeedbackInterface>(encoder));
	
	encoder->feedbackConfig.b_supportsVelocityFeedback = true;
	encoder->feedbackConfig.b_supportsPositionFeedback = true;
	
	outputPins.push_back(encoderPin);
	outputPins.push_back(resetPin);
	
	resolutionParameter->setEditCallback([this](std::shared_ptr<Parameter> parameter){
		resolutionParameter->validateRange(8, 25, true, true);
		singleturnResolutionParameter->onEdit();
	});
	singleturnResolutionParameter->setEditCallback([this](std::shared_ptr<Parameter> parameter){
		singleturnResolutionParameter->validateRange(0, resolutionParameter->value, true, true);
		updateEncoderWorkingRange();
	});
	centerWorkingRangeOnZeroParameter->setEditCallback([this](std::shared_ptr<Parameter> parameter){
		updateEncoderWorkingRange();
	});
	hasResetSignalParameter->setEditCallback([this](std::shared_ptr<Parameter> parameter){
		resetPin->setVisible(hasResetSignalParameter->value);
	});
	
	resetPin->setVisible(hasResetSignalParameter->value);
}
void IB_IL_SSI_IN::onSetIndex(int i){
	sprintf((char*)encoderPin->getDisplayString(), "Module %i SSI Encoder", i);
	sprintf((char*)encoderPin->getSaveString(), "Module%iSSIEncoder", i);
	sprintf((char*)resetPin->getDisplayString(), "Module %i SSI Encoder Reset", i);
	sprintf((char*)resetPin->getSaveString(), "Module%iSSIEncoderReset", i);
}
void IB_IL_SSI_IN::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	txPdoAssignement.addNewModule(0x1A00 + moduleIndex);
	txPdoAssignement.addEntry(0x6000 + moduleIndex * 0x10, 0x1, 32, "SSI Encoder Data", &encoderData);
}
void IB_IL_SSI_IN::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	rxPdoAssignement.addNewModule(0x1600 + moduleIndex);
	rxPdoAssignement.addEntry(0x7000 + moduleIndex * 0x10, 0x1, 32, "SSI Control Data", &controlData);
}
bool IB_IL_SSI_IN::configureParameters(){
	
	
	uint8_t controlCode = SSI::ControlCode::READ_POSITION;
	
	uint8_t parity;
	switch(parityParameter->value){
		case SSI::Parity::NONE:	parity = 0x0; break;
		case SSI::Parity::ODD:	parity = 0x1; break;
		case SSI::Parity::EVEN:	parity = 0x2; break;
	}
	
	uint8_t rev = 0x0;
	if(invertDirectionParameter->value) rev = 0x1;
	
	uint8_t resolution = std::clamp(resolutionParameter->value, 8, 25) - 7;
	
	uint8_t speed;
	switch(baudrateParameter->value){
		case SSI::Baudrate::KHz_100: 	speed = 0x1; break;
		case SSI::Baudrate::KHz_200: 	speed = 0x2; break;
		case SSI::Baudrate::KHz_400: 	speed = 0x3; break;
		case SSI::Baudrate::KHz_800: 	speed = 0x4; break;
		case SSI::Baudrate::MHz_1:		speed = 0x5; break;
	}
	
	uint8_t code;
	switch(codeParameter->value){
		case SSI::Code::BINARY: code = 0x0; break;
		case SSI::Code::GRAY:	code = 0x1; break;
	}
	
	uint8_t byte0 = (controlCode & 0x7F) << 1;
	uint8_t byte1 = ((parity & 0x3) << 4) | (rev & 0x1);
	uint8_t byte2 = resolution & 0x1F;
	uint8_t byte3 = ((speed & 0x7) << 4) | (code & 0x1);
	
	controlData = byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
	
	*resetPinValue = false;
	
	updateEncoderWorkingRange();
	
	return true;
}
void IB_IL_SSI_IN::readInputs(){
	
	uint8_t byte0 = encoderData & 0xFF;
	uint8_t byte1 = (encoderData >> 8) & 0xFF;
	uint8_t byte2 = (encoderData >> 16) & 0xFF;
	uint8_t byte3 = (encoderData >> 24) & 0xFF;
	
	uint8_t status = (byte0 >> 1) & 0x7F;
	rawPositionData = byte3 | (byte2 << 8) | (byte1 << 16) | ((byte0 & 0x1) << 24);
	int32_t positionSigned = rawPositionData;
	 
	if(status == 0x0) statusCode = SSI::StatusCode::OFFLINE;
	else if(status == 0x1) statusCode = SSI::StatusCode::OPERATION;
	else if(status & 0x8000) statusCode = SSI::StatusCode::ACKNOWLEDGE_FAULT;
	else if(status == 0x42) statusCode = SSI::StatusCode::FAULT_ENCODER_SUPPLY_NOT_PRESENT_OR_SHORT_CIRCUIT;
	else if(status == 0x44) statusCode = SSI::StatusCode::FAULT_PARITY_ERROR;
	else if(status == 0x48) statusCode = SSI::StatusCode::FAULT_INVALID_CONFIGURATION_DATA;
	else if(status == 0x50) statusCode = SSI::StatusCode::FAULT_INVALID_CONTROL_CODE;
	else statusCode = SSI::StatusCode::UNKNOWN;
	
	uint32_t incrementsPerRevolution = 0x1 << singleturnResolutionParameter->value;
	uint32_t incrementsTotal = 0x1 << resolutionParameter->value;
	
	if(centerWorkingRangeOnZeroParameter->value && rawPositionData >= incrementsTotal / 2) positionSigned -= incrementsTotal;
	
	positionBeforeOffset = double(positionSigned) / double(incrementsPerRevolution);
	uint64_t readingTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	
	double deltaT_seconds = double(readingTime_nanoseconds - previousReadingTime_nanoseconds) / 1000000000.0;
	double deltaP_revolutions = positionBeforeOffset - previousPosition_revolutions;
	
	previousReadingTime_nanoseconds = readingTime_nanoseconds;
	previousPosition_revolutions = positionBeforeOffset;
	
	encoder->feedbackProcessData.positionActual = positionBeforeOffset + positionOffset;
	encoder->feedbackProcessData.velocityActual = deltaP_revolutions / deltaT_seconds;
	
	if(!parentDevice->isStateOperational()) encoder->state = DeviceState::NOT_READY;
	else if(parentDevice->isStateInit()) encoder->state = DeviceState::OFFLINE;
	else if(statusCode != SSI::StatusCode::OPERATION) encoder->state = DeviceState::NOT_READY;
	else encoder->state = DeviceState::ENABLED;
}
void IB_IL_SSI_IN::writeOutputs(){

	//change control code here (if needed)
	controlData &= 0xFFFFFF01;
	uint8_t controlCode;
	switch(statusCode){
		case SSI::StatusCode::UNKNOWN:
		case SSI::StatusCode::OFFLINE:
		case SSI::StatusCode::OPERATION:
		case SSI::StatusCode::ACKNOWLEDGE_FAULT:
			controlCode = SSI::ControlCode::READ_POSITION;
			break;
		case SSI::StatusCode::FAULT_ENCODER_SUPPLY_NOT_PRESENT_OR_SHORT_CIRCUIT:
		case SSI::StatusCode::FAULT_PARITY_ERROR:
		case SSI::StatusCode::FAULT_INVALID_CONFIGURATION_DATA:
		case SSI::StatusCode::FAULT_INVALID_CONTROL_CODE:
			controlCode = SSI::ControlCode::ACKNOWLEDGE_FAULT;
			break;
	}
	controlData |= (controlCode << 1);
	
	
	
	if(encoder->feedbackProcessData.b_overridePosition){
		encoder->feedbackProcessData.b_overridePosition = false;
		encoder->feedbackProcessData.b_positionOverrideBusy = true;
		
		if(hasResetSignalParameter->value){
			*resetPinValue = true;
			resetStartTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
			Logger::info("[{}] resetting SSI encoder", encoder->getName());
		}else{
			positionOffset = encoder->feedbackProcessData.positionOverride - positionBeforeOffset;
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
		*resetPinValue = EtherCatFieldbus::getCycleProgramTime_nanoseconds() - resetStartTime_nanoseconds < resetSignalTimeParameter->value * 1000000;
		
		if(rawPositionData == 0x0){
			encoder->feedbackProcessData.b_positionOverrideBusy = false;
			encoder->feedbackProcessData.b_positionOverrideSucceeded = true;
			positionOffset = encoder->feedbackProcessData.positionOverride;
			encoder->feedbackProcessData.positionActual = encoder->feedbackProcessData.positionOverride;
			updateEncoderWorkingRange();
			Logger::info("[IB_IL_SSI_IN] Successfully reset SSI encoder position");
		}
		else if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - resetStartTime_nanoseconds > resetSignalTimeParameter->value * 1000000 * 2){
			encoder->feedbackProcessData.b_positionOverrideBusy = false;
			encoder->feedbackProcessData.b_positionOverrideSucceeded = false;
			*resetPinValue = false;
			Logger::error("[IB_IL_SSI_IN] Failed to reset SSI encoder position");
		}
	}

}

void IB_IL_SSI_IN::updateEncoderWorkingRange(){
	uint32_t incrementsPerRevolution = 0x1 << singleturnResolutionParameter->value;
	uint32_t incrementsTotal = 0x1 << resolutionParameter->value;
	
	double workingRangeDelta = incrementsTotal / incrementsPerRevolution;
	double workingRangeMin = 0.0;
	double workingRangeMax = workingRangeDelta;
	if(centerWorkingRangeOnZeroParameter->value){
		workingRangeMin -= workingRangeDelta / 2.0;
		workingRangeMax -= workingRangeDelta / 2.0;
	}
	
	encoder->feedbackConfig.positionLowerWorkingRangeBound = workingRangeMin + positionOffset;
	encoder->feedbackConfig.positionUpperWorkingRangeBound = workingRangeMax + positionOffset;
}



void IB_IL_SSI_IN::moduleGui(){
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("The bus coupler needs to be power cycled after applying the settings to make them take effect.");
	ImGui::PopStyleColor();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Overall Resolution");
	ImGui::PopFont();
	resolutionParameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Singleturn Resolution");
	ImGui::PopFont();
	singleturnResolutionParameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Coding");
	ImGui::PopFont();
	codeParameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Transmission Rate");
	ImGui::PopFont();
	baudrateParameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Parity");
	ImGui::PopFont();
	parityParameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Reset Signal");
	ImGui::PopFont();
	hasResetSignalParameter->gui();
	ImGui::SameLine();
	ImGui::Text("%s reset signal", hasResetSignalParameter->value ? "Has" : "Does not have");
	
	if(hasResetSignalParameter->value){
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Reset Time");
		ImGui::PopFont();
		resetSignalTimeParameter->gui();
	}
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Invert Direction");
	ImGui::PopFont();
	invertDirectionParameter->gui();
	ImGui::SameLine();
	ImGui::Text("Direction is %sinverted", invertDirectionParameter->value ? "" : "not ");
	
	float minRange = encoder->feedbackConfig.positionLowerWorkingRangeBound;
	float maxRange = encoder->feedbackConfig.positionUpperWorkingRangeBound;
	float pos = encoder->feedbackProcessData.positionActual;
	float vel = encoder->feedbackProcessData.velocityActual;
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Working range adjustement");
	ImGui::PopFont();
	centerWorkingRangeOnZeroParameter->gui();
	ImGui::SameLine();
	ImGui::Text("Working range is %scentered on zero", centerWorkingRangeOnZeroParameter->value ? "" : "not ");
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::Text("(%.f to %.f revolutions)", minRange, maxRange);
	ImGui::PopStyleColor();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Realtime encoder data");
	ImGui::PopFont();
	

	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("%s", encoder->getStatusString().c_str());
	ImGui::PopStyleColor();
	
	float positionInWorkingRange = (pos - minRange) / (maxRange - minRange);
	float velocityNormalized = vel / 10.0;
	
	ImVec2 progressSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
	
	static char statusString[64];
	sprintf(statusString, "%.3f rev", pos);
	ImGui::ProgressBar(positionInWorkingRange, progressSize, statusString);
	
	sprintf(statusString, "%.2f rev/s", vel);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, velocityNormalized > 0.0 ? Colors::green : Colors::red);
	ImGui::ProgressBar(std::abs(velocityNormalized), progressSize, statusString);
	ImGui::PopStyleColor();
	
	if(ImGui::Button("Hard Reset")) encoder->overridePosition(0.0);
	
	ImGui::Text("Raw SSI Position : %i", rawPositionData);
	
}

bool IB_IL_SSI_IN::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	resolutionParameter->save(xml);
	singleturnResolutionParameter->save(xml);
	parityParameter->save(xml);
	invertDirectionParameter->save(xml);
	baudrateParameter->save(xml);
	codeParameter->save(xml);
	centerWorkingRangeOnZeroParameter->save(xml);
	hasResetSignalParameter->save(xml);
	resetSignalTimeParameter->save(xml);
	xml->SetAttribute("PositionOffset", positionOffset);
	return true;
}
bool IB_IL_SSI_IN::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	if(!resolutionParameter->load(xml)) return false;
	if(!singleturnResolutionParameter->load(xml)) return false;
	if(!parityParameter->load(xml)) return false;
	if(!invertDirectionParameter->load(xml)) return false;
	if(!baudrateParameter->load(xml)) return false;
	if(!codeParameter->load(xml)) return false;
	if(!centerWorkingRangeOnZeroParameter->load(xml)) return false;
	if(!hasResetSignalParameter->load(xml)) return false;
	if(!resetSignalTimeParameter->load(xml)) return false;
	if(xml->QueryDoubleAttribute("PositionOffset", &positionOffset) != XML_SUCCESS) return false;
	
	updateEncoderWorkingRange();
	resetPin->setVisible(hasResetSignalParameter->value);
	return true;
}





}
