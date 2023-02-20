#include <pch.h>

#include "Gui_Private.h"
#include "Window.h"
#include "Layout.h"

#include <imgui_internal.h>

namespace Legato::Gui::WindowManager{

std::vector<std::shared_ptr<Window>> openWindows;
std::vector<std::shared_ptr<Window>> openingWindows;
std::vector<std::shared_ptr<Window>> closingWindows;

void openWindow(std::shared_ptr<Window> window){ openingWindows.push_back(window); }
void closeWindow(std::shared_ptr<Window> window){ closingWindows.push_back(window); }
std::vector<std::shared_ptr<Window>>& getOpenWindows(){ return openWindows; }

std::vector<std::shared_ptr<Popup>> openPopups;
std::vector<std::shared_ptr<Popup>> openingPopups;
std::vector<std::shared_ptr<Popup>> closingPopups;

void openPopup(std::shared_ptr<Popup> popup){ openingPopups.push_back(popup); }
void closePopup(std::shared_ptr<Popup> popup){ closingPopups.push_back(popup); }



std::vector<std::shared_ptr<Window>> registeredWindows;

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
	newLayout->setName("New Layout");
	newLayout->overwrite();
	return newLayout;
}


std::shared_ptr<Layout> queuedLayout;

void applyLayout(std::shared_ptr<Layout> layout){
	queuedLayout = layout;
}

void update(){
	
	if(queuedLayout){
		//close all windows
		for(auto window : openWindows) window->b_isOpen = false;
		openWindows.clear();
		//open windows that match the layouts opened windows
		for(auto& openWindowName : queuedLayout->openWindowIds){
			for(auto registeredWindow : registeredWindows){
				if(registeredWindow->getName() == openWindowName){
					WindowManager::openWindow(registeredWindow);
					break;
				}
			}
		}
	}
	
	for(auto closingWindow : closingWindows){
		closingWindow->b_isOpen = false;
		closingWindow->onClose();
		for(size_t i = openWindows.size() - 1; i >= 0; i--){
			if(openWindows[i] == closingWindow){
				openWindows.erase(openWindows.begin() + i);
				break;
			}
		}
	}
	closingWindows.clear();
	
	for(auto openingWindow : openingWindows){
		openingWindow->b_isOpen = true;
		openingWindow->onOpen();
		openWindows.push_back(openingWindow);
	}
	openingWindows.clear();
	
	for(auto openWindow : openWindows) openWindow->draw();
	
	
	
	
	
	for(auto openWindow : openWindows){
		openWindow->imguiWindow->Viewport;
	}
	
	
	
	
	for(auto closingPopup : closingPopups){
		closingPopup->b_isOpen = false;
		closingPopup->onClose();
		for(size_t i = openPopups.size() - 1; i >= 0; i--){
			if(openPopups[i] == closingPopup){
				openPopups.erase(openPopups.begin() + i);
				break;
			}
		}
	}
	closingPopups.clear();
	
	for(auto openingPopup : openingPopups){
		openingPopup->b_isOpen = true;
		openingPopup->onOpen();
		openPopups.push_back(openingPopup);
	}
	openingPopups.clear();
	
	for(auto openPopup : openPopups) openPopup->draw();
	
	
	
	if(queuedLayout){
		ImGui::LoadIniSettingsFromMemory(queuedLayout->layoutString.c_str());
		queuedLayout = nullptr;
	}
}


bool b_areWindowsLocked = true;
bool areWindowsLocked(){ return b_areWindowsLocked; }
bool unlockWindows() { b_areWindowsLocked = false; }
bool lockWindows() { b_areWindowsLocked = true; }

};




/*
 std::vector<std::shared_ptr<Window>> windowsToFocus;
 void focusWindow(std::shared_ptr<Window> window){ windowsToFocus.push_back(window); }

 void focusWindows(){
	 if(windowsToFocus.empty()) return;
	 for(auto& window : windowsToFocus) ImGui::FocusWindow(window->imguiWindow);
	 windowsToFocus.clear();
 }
*/
