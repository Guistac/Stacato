#include "Window.h"

#include "Gui.h"

#include <imgui.h>
#include <imgui_internal.h>


void Window::addToDictionnary(){ WindowManager::addWindowToDictionnary(shared_from_this()); }
void Window::removeFromDictionnary(){ WindowManager::removeWindowFromDictionnary(shared_from_this()); }

void Window::open(){
	if(isOpen()) return;
	WindowManager::openWindow(shared_from_this());
}

void Window::close(){ WindowManager::closeWindow(shared_from_this()); }

void Window::focus(){ WindowManager::focusWindow(shared_from_this()); }

void Window::draw(){
	if(!b_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
	bool b_manualOpen = true;
	if(ImGui::Begin(name.c_str(), &b_manualOpen)){
		imguiWindow = ImGui::GetCurrentContext()->CurrentWindow;
		if(!b_padding) ImGui::PopStyleVar();
		drawContent();
	}else if(!b_padding) ImGui::PopStyleVar();
	ImGui::End();
	if(!b_manualOpen) close();
}

bool Window::isFocused(){
	ImGuiWindow* focusedWindow = ImGui::GetCurrentContext()->NavWindow;
	if(focusedWindow == imguiWindow) return true;
	else if(ImGui::IsWindowChildOf(focusedWindow, imguiWindow, false, false)) return true;
	return false;
}






void Popup::open(){ WindowManager::openPopup(shared_from_this()); }

void Popup::close(){ WindowManager::closePopup(shared_from_this()); }

void Popup::draw(){
	if(!ImGui::IsPopupOpen(name.c_str())) ImGui::OpenPopup(name.c_str());
	
	glm::vec2 size = getSize();
	if(size != glm::vec2(.0f, .0f)) ImGui::SetNextWindowSize(size);
	
	if(b_modal){
		glm::vec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		
		bool b_manualOpen = true;
		bool* b_openPointer = b_canClose ? &b_manualOpen : nullptr;
		if(ImGui::BeginPopupModal(name.c_str(), b_openPointer, popupFlags)){
			drawContent();
			ImGui::EndPopup();
		}
		if(b_canClose && !b_manualOpen) close();
	}else{
		if(ImGui::BeginPopup(name.c_str())){
			drawContent();
			ImGui::EndPopup();
		}
	}
}







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
	if(isInDictionnary(window)) return;
	windowDictionnary.push_back(window);
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
	window->onOpen();
	openWindows.push_back(window);
}



std::vector<std::shared_ptr<Window>> windowsToClose;
void closeWindow(std::shared_ptr<Window> window){ windowsToClose.push_back(window); }

void closeAllWindows(){
	for(auto& window : openWindows) {
		window->b_open = false;
		window->onClose();
	}
	openWindows.clear();
}

void closeWindows(){
	for(auto& windowToClose : windowsToClose){
		windowToClose->b_open = false;
		windowToClose->onClose();
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
	popup->onOpen();
	popup->b_open = true;
	popupList.push_back(popup);
}

std::vector<std::shared_ptr<Popup>> popupsToClose;
void closePopup(std::shared_ptr<Popup> popup){ popupsToClose.push_back(popup); }
void closePopups(){
	for(auto& popupToClose : popupsToClose){
		popupToClose->b_open = false;
		popupToClose->onClose();
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
