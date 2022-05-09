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
	
	inline std::vector<std::shared_ptr<Layout>>& layouts(){
		static std::vector<std::shared_ptr<Layout>> layouts;
		return layouts;
	}

	inline std::shared_ptr<Layout>& current(){
		static std::shared_ptr<Layout> currentLayout = nullptr;
		return currentLayout;
	}

	inline std::shared_ptr<Layout>& defaultLayout(){
		static std::shared_ptr<Layout> defaultLayoutPtr = nullptr;
		return defaultLayoutPtr;
	}

	inline std::shared_ptr<Layout>& edited(){
		static std::shared_ptr<Layout> editedLayout;
		return editedLayout;
	}

	inline bool& editRequested(){
		static bool b_requested = false;
		return b_requested;
	}

	inline void edit(std::shared_ptr<Layout> editedLayout){
		edited() = editedLayout;
		editRequested() = true;
	}

	inline void endEdit(){
		edited() = nullptr;
	}

	inline void editor(){
		if(editRequested()) {
			ImGui::OpenPopup("Edit Layout");
			editRequested() = false;
		}
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Edit Layout", nullptr, popupFlags)) {
			edited()->nameEditField();
			if (ImGui::Button("Close") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	inline void setCurrent(std::shared_ptr<Layout> layout){
		current() = layout;
		ImGui::LoadIniSettingsFromMemory(layout->layoutString.c_str());
	}

	inline void makeDefault(std::shared_ptr<Layout> layout){
		defaultLayout() = layout;
	}

	inline void addCurrent(){
		auto newLayout = std::make_shared<Layout>();
		newLayout->layoutString = ImGui::SaveIniSettingsToMemory();
		sprintf(newLayout->name, "Layout %i", int(layouts().size()));
		layouts().push_back(newLayout);
		setCurrent(newLayout);
	}

	inline void remove(std::shared_ptr<Layout> removedLayout){
		for(int i = layouts().size() - 1; i >= 0; i--){
			if(layouts()[i] == removedLayout){
				layouts().erase(layouts().begin() + i);
				break;
			}
		}
	}

	inline bool save(const char* filePath){
		using namespace tinyxml2;
		XMLDocument layoutDocument;
		
		for(auto& layout : layouts()){
			XMLElement* layoutXML = layoutDocument.NewElement("Layout");
			layoutDocument.InsertEndChild(layoutXML);
			if(!layout->save(layoutXML)) return false;
		}
		
		return XML_SUCCESS == layoutDocument.SaveFile(filePath);
	}

	inline bool load(const char* filePath){
		using namespace tinyxml2;
		XMLDocument layoutDocument;
		XMLError loadResult = layoutDocument.LoadFile(filePath);
		if (loadResult != XML_SUCCESS) return Logger::warn("Could not Open Layout SaveFile (tinyxml2 error: {})", XMLDocument::ErrorIDToName(loadResult));
		
		tinyxml2::XMLElement* layoutXML = layoutDocument.FirstChildElement("Layout");
		while(layoutXML){
			auto newLayout = std::make_shared<Layout>();
			if(!newLayout->load(layoutXML)) return Logger::warn("Could not load layout");
			layouts().push_back(newLayout);
			layoutXML = layoutXML->NextSiblingElement("Layout");
		}
		return true;
	}

	inline void clearAll(){
		layouts().clear();
		current() = nullptr;
		edited() = nullptr;
		defaultLayout() = nullptr;
	}

};

inline void Layout::makeActive(){
	LayoutManager::setCurrent(shared_from_this());
}

inline bool Layout::isActive(){
	return LayoutManager::current().get() == this;
}

inline void Layout::makeDefault(){
	LayoutManager::makeDefault(shared_from_this());
}

inline bool Layout::isDefault(){
	return LayoutManager::defaultLayout().get() == this;
}

