#include <pch.h>

#include "Layout.h"





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

void Layout::edit(){
	LayoutManager::edit(shared_from_this());
}

void Layout::remove(){
	LayoutManager::remove(shared_from_this());
}





namespace LayoutManager{

	std::vector<std::shared_ptr<Layout>> layouts;
	std::shared_ptr<Layout> currentLayout = nullptr;
	std::shared_ptr<Layout> defaultLayout = nullptr;
	std::shared_ptr<Layout> editedLayout = nullptr;
	bool b_editRequested = false;

	std::vector<std::shared_ptr<Layout>>& getLayouts(){ return layouts; }
	std::shared_ptr<Layout> getCurrentLayout(){ return currentLayout; }
	std::shared_ptr<Layout> getDefaultLayout(){ return defaultLayout; }
	std::shared_ptr<Layout> getEditedLayout(){ return editedLayout; }
	bool isEditRequested(){ return b_editRequested; }

	void edit(std::shared_ptr<Layout> editedLayout_){
		editedLayout = editedLayout_;
		b_editRequested = true;
	}

	void endEdit(){
		editedLayout = nullptr;
	}

	void editor(){
		if(b_editRequested) {
			ImGui::OpenPopup("Edit Layout");
			b_editRequested = false;
		}
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Edit Layout", nullptr, popupFlags)) {
			editedLayout->nameEditField();
			if (ImGui::Button("Close") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void makeActive(std::shared_ptr<Layout> layout){
		currentLayout = layout;
		ImGui::LoadIniSettingsFromMemory(layout->layoutString.c_str());
	}

	void makeDefault(std::shared_ptr<Layout> layout){
		defaultLayout = layout;
	}

	void setDefault(){
		if(defaultLayout) makeActive(defaultLayout);
	}

	void addCurrent(){
		auto newLayout = std::make_shared<Layout>();
		newLayout->layoutString = ImGui::SaveIniSettingsToMemory();
		sprintf(newLayout->name, "Layout %i", int(layouts.size()));
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
			auto newLayout = std::make_shared<Layout>();
			if(!newLayout->load(layoutXML)) return Logger::warn("Could not load layout");
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


}
