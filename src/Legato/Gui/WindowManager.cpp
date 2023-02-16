#include <pch.h>

#include "Gui_Private.h"
#include "Window.h"

namespace Legato::Gui{

std::vector<std::shared_ptr<Window>> openWindows;
std::vector<std::shared_ptr<Window>> openingWindows;
std::vector<std::shared_ptr<Window>> closingWindows;
void openWindow(std::shared_ptr<Window> window){ openingWindows.push_back(window); }
void closeWindow(std::shared_ptr<Window> window){ closingWindows.push_back(window); }

void drawWindows(){
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
}

std::vector<std::shared_ptr<Popup>> openPopups;
std::vector<std::shared_ptr<Popup>> openingPopups;
std::vector<std::shared_ptr<Popup>> closingPopups;

void openPopup(std::shared_ptr<Popup> popup){ openingPopups.push_back(popup); }
void closePopup(std::shared_ptr<Popup> popup){ closingPopups.push_back(popup); }

void drawPopups(){
	for(auto closingPopup : closingPopups){
		closingPopup->b_isOpen = false;
		closingPopup->onClose();
		for(size_t i = openWindows.size() - 1; i >= 0; i--){
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
}



};




/*
namespace WindowManager{


std::vector<std::shared_ptr<Window>> windowDictionnary;
std::vector<std::shared_ptr<Window>>& getWindowDictionnary(){ return windowDictionnary; }
bool isInDictionnary(std::shared_ptr<Window> window){
	for(auto& dictionnaryWindow : windowDictionnary){
		if(dictionnaryWindow == window) return true;
	}
	return false;
}
void addWindowToDictionnary(std::shared_ptr<Window> window){
	if(!isInDictionnary(window)) {
		windowDictionnary.push_back(window);
	}
}
void removeWindowFromDictionnary(std::shared_ptr<Window> window){
	window->close();
	for(int i = 0; i < windowDictionnary.size(); i++){
		if(windowDictionnary[i] == window){
			windowDictionnary.erase(windowDictionnary.begin() + i);
			break;
		}
	}
}





std::vector<std::shared_ptr<Window>> openWindows;
std::vector<std::shared_ptr<Window>>& getOpenWindows(){ return openWindows; }
void openWindow(std::shared_ptr<Window> window){
	window->b_open = true;
	openWindows.push_back(window);
}



std::vector<std::shared_ptr<Window>> windowsToClose;
void closeWindow(std::shared_ptr<Window> window){ windowsToClose.push_back(window); }

 

void closeAllWindows(){
	for(auto& window : openWindows) {
		window->b_open = false;
	}
	openWindows.clear();
}

void closeWindows(){
	for(auto& windowToClose : windowsToClose){
		windowToClose->b_open = false;
		for(int i = 0; i < openWindows.size(); i++){
			if(openWindows[i] == windowToClose){
				openWindows.erase(openWindows.begin() + i);
			}
		}
	}
	windowsToClose.clear();
}




std::vector<std::shared_ptr<Window>> windowsToFocus;
void focusWindow(std::shared_ptr<Window> window){ windowsToFocus.push_back(window); }

void focusWindows(){
	if(windowsToFocus.empty()) return;
	for(auto& window : windowsToFocus) ImGui::FocusWindow(window->imguiWindow);
	windowsToFocus.clear();
}




std::vector<std::shared_ptr<Popup>> popupList;
std::vector<std::shared_ptr<Popup>>& getPopups(){ return popupList; }
void openPopup(std::shared_ptr<Popup> popup){
	if(popup->b_open) return;
	popup->b_open = true;
	popupList.push_back(popup);
}

std::vector<std::shared_ptr<Popup>> popupsToClose;
void closePopup(std::shared_ptr<Popup> popup){ popupsToClose.push_back(popup); }
void closePopups(){
	for(auto& popupToClose : popupsToClose){
		popupToClose->b_open = false;
		for(int i = 0; i < popupList.size(); i++){
			if(popupList[i] == popupToClose){
				popupList.erase(popupList.begin() + i);
				break;
			}
		}
	}
	popupsToClose.clear();
}



void manage(){
	for(auto& window : openWindows) window->draw();
	closeWindows();
	focusWindows();
	for(auto& popup : popupList) popup->draw();
	closePopups();
}
 

};
*/
