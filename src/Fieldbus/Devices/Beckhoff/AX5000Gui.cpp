#include "AX5000.h"
#include <tinyxml2.h>
#include <iostream>
#include <fstream>
#include "Gui/Utilities/FileDialog.h"

void AX5103::deviceSpecificGui(){
	ax5000.gui();
}
void AX5203::deviceSpecificGui(){
	ax5000.gui();
}



void AX5000::gui() {
	if(ImGui::BeginTabItem("AX5000")){
		if(ImGui::BeginTabBar("Drive")){
			if(ImGui::BeginTabItem("Drive")){
				
				
				if(ImGui::Button("Read Invalid IDNs for PreOp->SafeOp")) getInvalidIDNsForSafeOp();
				if(ImGui::Button("Read Shutdown Errors")) getShutdownErrorList();
				if(ImGui::Button("Read Error History")) getErrorHistory();
				if(ImGui::Button("Read Diagnostics Message")) getDiagnosticsMessage();
				 
				auto axisControlGui = [&](std::shared_ptr<Actuator> servo, uint8_t axisNumber){
					ImGui::PushID(axisNumber);
					ImGui::PushFont(Fonts::sansBold20);
					ImGui::Text("Axis %i", axisNumber);
					ImGui::PopFont();
					if(servo->isEnabled()) {
						if(ImGui::Button("Disable")) servo->disable();
					}
					else {
						ImGui::BeginDisabled(!servo->isReady());
						if(ImGui::Button("Enable")) servo->enable();
						ImGui::EndDisabled();
					}
					ImGui::SameLine();
					ImGui::Text("%s", servo->getStatusString().c_str());
					if(ImGui::SliderFloat("Velocity", &servo->guiVelocitySliderValue, -10.0f, 10.0f)) servo->setVelocityTarget(servo->guiVelocitySliderValue);
					if(ImGui::IsItemDeactivatedAfterEdit()) {
						servo->setVelocityTarget(0.0);
						servo->guiVelocitySliderValue = 0.0;
					}
					
					ImGui::InputDouble("##offsetTarget", &servo->gui_offsetTarget);
					ImGui::SameLine();
					if(ImGui::Button("Override Position")){
						servo->overridePosition(servo->gui_offsetTarget);
					}
					
					ImGui::Text("Velocity: %.3frev/s", servo->feedbackProcessData.velocityActual);
					ImGui::Text("Position: %.3frev", servo->feedbackProcessData.positionActual);
					//ImGui::Text("PositionOffset: %ib", servo->positionOffset);
					//ImGui::Text("PositionRaw: %ib (%.2frev)", servo->positionRaw, double(servo->positionRaw) / servo->unitsPerRev);
					ImGui::Text("Effort: %.1f%%", servo->actuatorProcessData.effortActual * 100.0);
					ImGui::PopID();
				};
				
				for(int i = 0; i < actuators.size(); i++){
					axisControlGui(actuators[i], i);
				}
				 
				ImGui::EndTabItem();
				
			}
			if(ImGui::BeginTabItem("General")){
				invertSTO_param->gui(Fonts::sansBold15);
				invertDigitalIn0_param->gui(Fonts::sansBold15);
				invertDigitalIn1_param->gui(Fonts::sansBold15);
				invertDigitalIn2_param->gui(Fonts::sansBold15);
				invertDigitalIn3_param->gui(Fonts::sansBold15);
				invertDigitalIn4_param->gui(Fonts::sansBold15);
				invertDigitalIn5_param->gui(Fonts::sansBold15);
				invertDigitalIn6_param->gui(Fonts::sansBold15);
				invertDigitalOut7_param->gui(Fonts::sansBold15);
				ImGui::EndTabItem();
			}
			for(int i = 0; i < actuators.size(); i++){
				if(ImGui::BeginTabItem(actuators[i]->getName().c_str())){
					actuators[i]->settingsGui();
					ImGui::EndTabItem();
				}
			}
			if(ImGui::BeginTabItem("Utilities")){
				if (ImGui::Button("Read TwinCAT StartupList XML")) {
					FileDialog::FileTypeFilter filter("TwinCAT StartupList", "xml");
					FileDialog::FilePath filePath;
					char defaultFileName[128] = "startupList.hex";
					if (FileDialog::load(filePath, filter)) {
						readTwinCatStartupListXML(filePath.path);
					}
				}
				ImGui::EndTabItem();
			}
			
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}

void AX5000::Actuator::settingsGui(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis %i", channel);
	ImGui::PopFont();
	motorType->gui(Fonts::sansBold15);
	velocityLimit_revps->gui(Fonts::sansBold15);
	accelerationLimit_revps2->gui(Fonts::sansBold15);
	positionFollowingErrorLimit_rev->gui(Fonts::sansBold15);
	currentLimit_amps->gui(Fonts::sansBold15);
	currentPeak_amps->gui(Fonts::sansBold15);
	invertDirection_param->gui(Fonts::sansBold15);
}


void AX5000::readTwinCatStartupListXML(std::string filePath){
	using namespace tinyxml2;
	XMLDocument file;
	if(XML_SUCCESS != file.LoadFile(filePath.c_str())) return;
	
	XMLElement* mbxXML = file.FirstChildElement("EtherCATMailbox");
	if(mbxXML == nullptr) return;
	
	XMLElement* soeXML = mbxXML->FirstChildElement("SoE");
	if(soeXML == nullptr) return;
	
	XMLElement* initCmdsXML = soeXML->FirstChildElement("InitCmds");
	if(initCmdsXML == nullptr) return;
	
	std::string transcription =
	"struct StartupItem{\n"
		"\tstd::string comment;\n"
		"\tint driveNo;\n"
	   "\tint idn;\n"
	   "\tstd::vector<uint8_t> data;\n"
	"};\n"
	"\n"
	"std::vector<StartupItem> startupList = {\n";
	
	XMLElement* initCmdXML = initCmdsXML->FirstChildElement("InitCmd");
	while(initCmdXML){
		
		//Extract data from XML
		XMLElement* commentXML = initCmdXML->FirstChildElement("Comment");
		if(commentXML == nullptr) return;
		XMLElement* driveNoXML = initCmdXML->FirstChildElement("DriveNo");
		if(driveNoXML == nullptr) return;
		XMLElement* IDNXML = initCmdXML->FirstChildElement("IDN");
		if(IDNXML == nullptr) return;
		XMLElement* dataXML = initCmdXML->FirstChildElement("Data");
		if(dataXML == nullptr) return;
		std::string comment = commentXML->GetText();
		int driveNo = std::stoi(driveNoXML->GetText());
		int idn = std::stoi(IDNXML->GetText());
		std::string stringData = dataXML->GetText();
		if(stringData.length() % 2 != 0) return;
		std::vector<uint8_t> byteData(stringData.length() / 2, 0x0);
		   for (size_t i = 0; i < stringData.length(); i += 2) {
			   std::string byteString = stringData.substr(i, 2);
			   // Convert the string to a byte (base 16)
			   uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
			   byteData[i/2] = byte;
		   }
		
		std::string itemString =
			"\tStartupItem{\n"
			"\t\t.comment = \"";
		itemString += comment + "\",\n";
		itemString += "\t\t.driveNo = " + std::to_string(driveNo) + ",\n";
		itemString += "\t\t.idn = " + std::to_string(idn) + ",\n";
		itemString += "\t\t.data = {";
		if(byteData.size() > 8) itemString += "\n\t\t\t";
		for(int i = 0; i < byteData.size(); i++){
			char buffer[5];
			snprintf(buffer, 5, "0x%02X", byteData[i]);
			itemString += std::string(buffer);
			if(i != byteData.size() - 1) itemString += ", ";
			if(byteData.size() > 8 && i % 8 == 7 && i != byteData.size() - 1) itemString += "\n\t\t\t";
		}
		if(byteData.size() > 8) itemString += "\n\t\t}\n";
		else itemString += "}\n";
		itemString += "\t}";
		
		transcription += itemString;
		
		initCmdXML = initCmdXML->NextSiblingElement("InitCmd");
		if(initCmdXML != nullptr) transcription += ",\n";
		else transcription += "\n";
	}
	
	transcription += "};";
	
	std::string outFileName =
		std::filesystem::path(filePath).remove_filename().string() +
		std::filesystem::path(filePath).stem().string() + ".h";
	
	std::ofstream outFile(outFileName);
	outFile << transcription;
	outFile.close();
	
	Logger::info("Wrote file to {}", outFileName);
}
