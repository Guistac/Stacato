#include <pch.h>

#include "Layout.h"
#include "Gui.h"
#include "Window.h"

std::shared_ptr<Layout> Layout::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	auto newLayout = std::make_shared<Layout>();
	
	const char* layoutNameString;
	if(xml->QueryStringAttribute("Name", &layoutNameString) != XML_SUCCESS) {
		Logger::warn("Could not load Layout Name");
		return nullptr;
	}
	strcpy(newLayout->name, layoutNameString);
	
	bool b_default;
	if(xml->QueryBoolAttribute("IsDefault", &b_default) == XML_SUCCESS && b_default == true) newLayout->makeDefault();
	
	XMLElement* openWindowsXML = xml->FirstChildElement("OpenWindows");
	if(openWindowsXML == nullptr){
		Logger::warn("Could not find layout open windows attribute");
		return nullptr;
	}
	
	XMLElement* openWindowXML = openWindowsXML->FirstChildElement("Window");
	while(openWindowXML){
		const char* openWindowName;
		if(openWindowXML->QueryStringAttribute("Name", &openWindowName) != XML_SUCCESS){
			Logger::warn("Could not find open window name attribute");
			return nullptr;
		}
		newLayout->openWindowIds.push_back(openWindowName);
		openWindowXML = openWindowXML->NextSiblingElement("Window");
	}
	
	XMLElement* iniStringXML = xml->FirstChildElement("ImGuiIniString");
	if(iniStringXML == nullptr){
		Logger::warn("Could not find Layout ImGuiIniString attribute");
		return nullptr;
	}
	
	const char* iniString = iniStringXML->GetText();
	newLayout->layoutString = iniString;
	 
	return newLayout;
}



bool Layout::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	xml->SetAttribute("Name", name);
	if(isDefault()) xml->SetAttribute("IsDefault", true);
	
	XMLElement* openWindowsXML = xml->InsertNewChildElement("OpenWindows");
	for(auto& openWindow : openWindowIds){
		XMLElement* windowXML = openWindowsXML->InsertNewChildElement("Window");
		windowXML->SetAttribute("Name", openWindow.c_str());
	}
	
	XMLElement* iniStringXML = xml->InsertNewChildElement("ImGuiIniString");
	iniStringXML->InsertNewText("\n\n");
	iniStringXML->InsertNewText(layoutString.c_str());
	
	return true;
}



void Layout::overwrite(){
	layoutString = ImGui::SaveIniSettingsToMemory();
	openWindowIds.clear();
	for(auto& window : WindowManager::getOpenWindows()) {
		openWindowIds.push_back(window->name);
	}
}

void Layout::makeActive(){
	LayoutManager::makeActive(shared_from_this());
}

bool Layout::isActive(){
	return LayoutManager::getCurrentLayout() == shared_from_this();
}

void Layout::makeDefault(){
	LayoutManager::makeDefault(shared_from_this());
}

bool Layout::isDefault(){
	return LayoutManager::getDefaultLayout() == shared_from_this();
}

void Layout::edit(){ LayoutManager::edit(shared_from_this()); }

void Layout::remove(){
	LayoutManager::remove(shared_from_this());
}




namespace LayoutManager{

    bool b_lockLayout = true;

	std::vector<std::shared_ptr<Layout>> layouts;
	std::shared_ptr<Layout> currentLayout = nullptr;
	std::shared_ptr<Layout> defaultLayout = nullptr;
	std::shared_ptr<Layout> editedLayout = nullptr;

	std::vector<std::shared_ptr<Layout>>& getLayouts(){ return layouts; }
	std::shared_ptr<Layout> getCurrentLayout(){ return currentLayout; }
	std::shared_ptr<Layout> getDefaultLayout(){ return defaultLayout; }
	std::shared_ptr<Layout> getEditedLayout(){ return editedLayout; }

	std::shared_ptr<Layout> layoutToApply = nullptr;
	
	void edit(std::shared_ptr<Layout> editedLayout_){
		editedLayout = editedLayout_;
		LayoutEditorPopup::get()->open();
	}

	void makeActive(std::shared_ptr<Layout> layout){
		layoutToApply = layout;
	}

	void makeDefault(std::shared_ptr<Layout> layout){ defaultLayout = layout; }

	void setDefault(){ if(defaultLayout) makeActive(defaultLayout); }

	void capture(){
		auto newLayout = std::make_shared<Layout>();
		sprintf(newLayout->name, "Layout %i", int(layouts.size()));
		newLayout->overwrite();
		layouts.push_back(newLayout);
		makeActive(newLayout);
		edit(newLayout);
	}

	void remove(std::shared_ptr<Layout> removedLayout){
		for(int i = layouts.size() - 1; i >= 0; i--){
			if(layouts[i] == removedLayout){
				layouts.erase(layouts.begin() + i);
				break;
			}
		}
	}

	bool save(const char* filePath){
		using namespace tinyxml2;
		XMLDocument layoutDocument;
		XMLElement* layoutsXML = layoutDocument.NewElement("Layouts");
		layoutDocument.InsertEndChild(layoutsXML);
		
		for(auto& layout : layouts){
			XMLElement* layoutXML = layoutsXML->InsertNewChildElement("Layout");
			if(!layout->save(layoutXML)) return false;
		}
		
		return XML_SUCCESS == layoutDocument.SaveFile(filePath);
	}

	bool load(const char* filePath){
		using namespace tinyxml2;
		XMLDocument layoutDocument;
		XMLError loadResult = layoutDocument.LoadFile(filePath);
		if (loadResult != XML_SUCCESS) return Logger::warn("Could not Open Layout SaveFile (tinyxml2 error: {})", XMLDocument::ErrorIDToName(loadResult));
		
		XMLElement* layoutsXML = layoutDocument.FirstChildElement("Layouts");
		if(layoutsXML == nullptr) return Logger::warn("Could not find Layouts Attribute");
		
		XMLElement* layoutXML = layoutsXML->FirstChildElement("Layout");
		while(layoutXML){
			auto newLayout = Layout::load(layoutXML);
			if(newLayout == nullptr) {
				Logger::warn("Could not load layout");
				return false;
			}
			layouts.push_back(newLayout);
			layoutXML = layoutXML->NextSiblingElement("Layout");
		}
		
		return true;
	}

	void clearAll(){
		layouts.clear();
		currentLayout = nullptr;
		editedLayout = nullptr;
		defaultLayout = nullptr;
	}

	void LayoutEditorPopup::drawContent(){
		ImGui::Text("Layout Name :");
		editedLayout->nameEditField();
		if (ImGui::Button("Close") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) close();
	}

	void manage(){
		if(layoutToApply == nullptr) return;
		WindowManager::closeAllWindows();
		for(auto& openWindowName : layoutToApply->openWindowIds){
			for(auto& window : WindowManager::getWindowDictionnary()){
				if(openWindowName == window->name){
					window->open();
					break;
				}
			}
		}
		ImGui::LoadIniSettingsFromMemory(layoutToApply->layoutString.c_str());
		currentLayout = layoutToApply;
		layoutToApply = nullptr;
	}

}
