#include <pch.h>

#include "Gui_Private.h"
#include "Window.h"

namespace NewGui{

std::vector<std::shared_ptr<NewWindow>> openWindows;
std::vector<std::shared_ptr<NewWindow>> openingWindows;
std::vector<std::shared_ptr<NewWindow>> closingWindows;

void openWindow(std::shared_ptr<NewWindow> window){
	openWindows.push_back(window);
}

void closeWindow(std::shared_ptr<NewWindow> window){
	closingWindows.push_back(window);
}

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
	
	for(auto openingWindow : openingWindows){
		openingWindow->b_isOpen = true;
		openingWindow->onOpen();
		openWindows.push_back(openingWindow);
	}
	
	for(auto openWindow : openWindows){
		openWindow->draw();
	}
	
}

};
