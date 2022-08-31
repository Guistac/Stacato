#pragma once

#include <tinyxml2.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include "Gui/ApplicationWindow/Window.h"

class Layout : public std::enable_shared_from_this<Layout>{
public:
	
	static std::shared_ptr<Layout> load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);
	
	void makeActive();
	bool isActive();
	
	void makeDefault();
	bool isDefault();
	
	void edit();
	void remove();
	
	void overwrite();
	
	std::string layoutString;
	std::vector<std::string> openWindowIds;
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

	bool save(const char* filePath);
	bool load(const char* filePath);

	void clearAll();

	void capture();
	void edit(std::shared_ptr<Layout> editedLayout);
	void makeActive(std::shared_ptr<Layout> layout);
	void makeDefault(std::shared_ptr<Layout> layout);
	void setDefault();
	void remove(std::shared_ptr<Layout> removedLayout);

	class LayoutEditorPopup : public Popup{
	public:
		LayoutEditorPopup() : Popup("Edit Layout", true, true){}
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(LayoutEditorPopup);
	};

	void manage();

};

