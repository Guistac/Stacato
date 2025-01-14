#include "AX5000.h"
#include <tinyxml2.h>
#include <iostream>
#include <fstream>

void AX5103::deviceSpecificGui(){
	ax5000.gui();
}
void AX5203::deviceSpecificGui(){
	ax5000.gui();
}


#include "Gui/Utilities/FileDialog.h"


void AX5000::gui(){
	if(ImGui::BeginTabItem("AX5000")){
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
