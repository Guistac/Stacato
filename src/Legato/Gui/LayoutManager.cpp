#include <pch.h>

#include "Layout.h"
#include "Gui_Private.h"

namespace Legato::Gui::LayoutManager{

	std::vector<std::shared_ptr<Window>> registeredWindows;
	std::shared_ptr<Layout> queuedLayout;

	void registerWindow(std::shared_ptr<Window> window){
		for(auto registeredWindow : registeredWindows){
			if(registeredWindow == window) return;
		}
		registeredWindows.push_back(window);
	}

	void unregisterWindow(std::shared_ptr<Window> window){
		for(int i = 0; i < registeredWindows.size(); i++){
			if(registeredWindows[i] == window){
				registeredWindows.erase(registeredWindows.begin() + i);
				return;
			}
		}
	}


	std::shared_ptr<Layout> captureCurentLayout(){
		auto newLayout = Layout::createInstance();
		sprintf(newLayout->name, "New Layout");
		newLayout->overwrite();
		return newLayout;
	}

	bool b_isLayoutLocked = true;
	bool isLayoutLocked(){ return b_isLayoutLocked; }
	bool unlockLayout() { b_isLayoutLocked = false; }
	bool lockLayout() { b_isLayoutLocked = true; }


	void applyLayout(std::shared_ptr<Layout> layout){
		queuedLayout = layout;
	}

	void update(){
		if(queuedLayout){
			
			//apply layout here
			
			queuedLayout = nullptr;
		}
	}


/*

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

	void LayoutEditorPopup::onDraw(){
		ImGui::Text("Layout Name :");
		editedLayout->nameEditField();
		if (ImGui::Button("Close") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) close();
	}

	void manage(){
		assert("UNIMPLEMENTED");
		
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
 
 */

}
