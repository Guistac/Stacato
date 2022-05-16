#pragma once

#include <tinyxml2.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

class Layout : public std::enable_shared_from_this<Layout>{
public:
	
	bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		const char* layoutNameString;
		if(xml->QueryStringAttribute("Name", &layoutNameString) != XML_SUCCESS) return Logger::warn("Could not load Layout Name");
		strcpy(name, layoutNameString);
		bool b_default;
		if(xml->QueryBoolAttribute("IsDefault", &b_default) == XML_SUCCESS && b_default == true) makeDefault();
		const char* iniString = xml->GetText();
		layoutString = iniString;
		return true;
	}
	
	bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute("Name", name);
		if(isDefault()) xml->SetAttribute("IsDefault", true);
		xml->InsertNewText("\n");
		xml->InsertNewText(layoutString.c_str());
		return true;
	}
	
	void makeActive();
	bool isActive();
	
	void makeDefault();
	bool isDefault();
	
	void edit();
	void remove();
	
	void overwriteCurrent(){
		layoutString = ImGui::SaveIniSettingsToMemory();
	}
	
	std::string layoutString;
	char name[256];
	
	void nameEditField(){
		ImGui::InputText("##LayoutName", name, 256);
	}
	
};



namespace LayoutManager{
	
	std::vector<std::shared_ptr<Layout>>& getLayouts();
	std::shared_ptr<Layout> getCurrentLayout();
	std::shared_ptr<Layout> getDefaultLayout();
	std::shared_ptr<Layout> getEditedLayout();
	bool isEditRequested();

	void endEdit();

	void editor();

	void addCurrent();

	bool save(const char* filePath);

	bool load(const char* filePath);

	void clearAll();

	void edit(std::shared_ptr<Layout> editedLayout);
	void makeActive(std::shared_ptr<Layout> layout);
	void makeDefault(std::shared_ptr<Layout> layout);
	void setDefault();
	void remove(std::shared_ptr<Layout> removedLayout);

};

