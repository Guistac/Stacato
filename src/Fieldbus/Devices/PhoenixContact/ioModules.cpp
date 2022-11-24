#include <pch.h>
#include "ioModules.h"

namespace PhoenixContact{


namespace ModuleFactory{
	std::vector<EtherCAT::DeviceModule*>& getModules(){
		static std::vector<EtherCAT::DeviceModule*> moduleList = {
			new IB_IL_24_DI_4(),
			new IB_IL_24_DO_4(),
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
		sprintf(pinName, "Digital Input %i", i);
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, pinName);
		std::shared_ptr<bool> pinValue = std::make_shared<bool>(false);
		pin->assignData(pinValue);
		outputPinValues.push_back(pinValue);
		outputPins.push_back(pin);
	}
}
void IB_IL_24_DI_4::onSetIndex(int i){
	for(int i = 0; i < 4; i++){
		sprintf((char*)outputPins[i]->getDisplayString(), "Module %i Digital Input %i", moduleIndex, i);
		sprintf((char*)outputPins[i]->getSaveString(), "Module%iDigitalInput%i", moduleIndex, i);
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
		sprintf(attributeName, "InvertInput%i", i);
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
		sprintf(attributeName, "InvertInput%i", i);
		if(inversionXML->QueryBoolAttribute(attributeName, &invertInputs[i]) != XML_SUCCESS) {
			return Logger::warn("could not find input %i inversion attribute", i);
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
		sprintf(pinName, "Digital Output %i", i);
		std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, pinName);
		std::shared_ptr<bool> pinValue = std::make_shared<bool>(false);
		pin->assignData(pinValue);
		inputPinValues.push_back(pinValue);
		inputPins.push_back(pin);
	}
}
void IB_IL_24_DO_4::onSetIndex(int i){
	for(int i = 0; i < 4; i++){
		sprintf((char*)inputPins[i]->getDisplayString(), "Module %i Digital Output %i", moduleIndex, i);
		sprintf((char*)inputPins[i]->getSaveString(), "Module%iDigitalOutput%i", moduleIndex, i);
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
		sprintf(attributeName, "InvertOutput%i", i);
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
		sprintf(attributeName, "InvertOutput%i", i);
		if(inversionXML->QueryBoolAttribute(attributeName, &invertOutputs[i]) != XML_SUCCESS){
			return Logger::warn("could not find output %i inversion attribute", i);
		}
	}
	return true;
}


//=================================================================
//============== 050-1BS00 Single SSI Encoder Input ===============
//=================================================================

void IB_IL_SSI_IN::onConstruction(){}
void IB_IL_SSI_IN::onSetIndex(int i){}
void IB_IL_SSI_IN::addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement){
	txPdoAssignement.addNewModule(0x1A00 + moduleIndex);
	txPdoAssignement.addEntry(0x6000 + moduleIndex * 0x10, 0x1, 32, "SSI Encoder Data", &encoderData);
}
void IB_IL_SSI_IN::addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement){
	rxPdoAssignement.addNewModule(0x1600 + moduleIndex);
	rxPdoAssignement.addEntry(0x7000 + moduleIndex * 0x10, 0x1, 32, "SSI Control Data", &controlData);
}
bool IB_IL_SSI_IN::configureParameters(){ return true; }
void IB_IL_SSI_IN::readInputs(){}
void IB_IL_SSI_IN::writeOutputs(){}
void IB_IL_SSI_IN::moduleGui(){}
bool IB_IL_SSI_IN::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	return true;
}
bool IB_IL_SSI_IN::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	return true;
}





}
