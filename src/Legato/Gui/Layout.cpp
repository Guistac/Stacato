#include <pch.h>

#include "Gui_Private.h"
#include "Layout.h"
#include "Legato/Gui/Window.h"

#include <tinyxml2.h>
#include <imgui.h>


bool Layout::onSerialization(){
	Component::onSerialization();
	
	using namespace tinyxml2;
	
	XMLElement* openWindowsXML = xmlElement->InsertNewChildElement("OpenWindows");
	for(auto& openWindow : openWindowIds){
		XMLElement* windowXML = openWindowsXML->InsertNewChildElement("Window");
		windowXML->SetAttribute("Name", openWindow.c_str());
	}
	
	XMLElement* iniStringXML = xmlElement->InsertNewChildElement("ImGuiIniString");
	iniStringXML->InsertNewText("\n\n");
	iniStringXML->InsertNewText(layoutString.c_str());
	
	return true;
}

bool Layout::onDeserialization(){
	Component::onDeserialization();
	
	using namespace tinyxml2;
			
	XMLElement* openWindowsXML = xmlElement->FirstChildElement("OpenWindows");
	if(openWindowsXML == nullptr){
		Logger::warn("[Layout] Failed to load : Could not find layout open windows attribute");
		return false;
	}
	
	XMLElement* openWindowXML = openWindowsXML->FirstChildElement("Window");
	while(openWindowXML){
		const char* openWindowName;
		if(openWindowXML->QueryStringAttribute("Name", &openWindowName) != XML_SUCCESS){
			Logger::warn("[Layout] Failed to load : Could not find open window name attribute");
			return false;
		}
		openWindowIds.push_back(openWindowName);
		openWindowXML = openWindowXML->NextSiblingElement("Window");
	}
	
	XMLElement* iniStringXML = xmlElement->FirstChildElement("ImGuiIniString");
	if(iniStringXML == nullptr){
		Logger::warn("[Layout] Failed to load : Could not find Layout ImGuiIniString attribute");
		return false;
	}
	
	layoutString = iniStringXML->GetText();
	int i = 0;
	while(layoutString[i] == '\n') i++;
	layoutString = layoutString.substr(i);
	 
	return true;
}

void Layout::onConstruction() {
	Component::onConstruction();
	addChild(name);
}

void Layout::copyFrom(std::shared_ptr<Legato::Component> source) {
	Component::copyFrom(source);
	//copy copy copy
}

void Layout::overwrite(){
	layoutString = ImGui::SaveIniSettingsToMemory();
	openWindowIds.clear();
	for(auto openWindow : Legato::Gui::WindowManager::getOpenWindows()){
		openWindowIds.push_back(openWindow->getName());
	}
}


void RenameLayoutPopup::onDraw(){
	ImGui::Text("Layout Name");
	
	ImGui::InputText("##layoutame", nameBuffer, 128);
	
	if(ImGui::Button("Cancel")){
		close();
	}
	ImGui::SameLine();
	if(ImGui::Button("Ok")){
		//renamedLayout->setName(nameBuffer);
		close();
	}
}



