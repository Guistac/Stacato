#include <pch.h>
#include "Window.h"

#include "Gui_Private.h"

void Window::open(){
	NewGui::openWindow(shared_from_this());
}

void Window::close(){
	NewGui::closeWindow(shared_from_this());
}

void Window::focus(){
	
}

void Window::draw(){
	//open window
	onDraw();
	//close window
}

bool Window::isOpen(){
	return b_isOpen;
}

bool Window::isFocused(){
	
}




/*
void Window::addToDictionnary(){
	auto thisWindow = shared_from_this();
	WindowManager::addWindowToDictionnary(thisWindow);
}
void Window::removeFromDictionnary(){
	auto thisWindow = shared_from_this();
	WindowManager::removeWindowFromDictionnary(thisWindow);
}

void Window::open(){
	if(isOpen()) return;
	auto thisWindow = shared_from_this();
	WindowManager::openWindow(thisWindow);
}

void Window::close(){
	auto thisWindow = shared_from_this();
	WindowManager::closeWindow(thisWindow);
}

void Window::focus(){
	auto thisWindow = shared_from_this();
	WindowManager::focusWindow(thisWindow);
}

void Window::draw(){
	if(!b_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
	bool b_manualOpen = true;
	
	ImGuiWindowFlags windowFlags;
	if(LayoutManager::b_lockLayout) windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	else windowFlags = ImGuiWindowFlags_None;
	
	
	if(ImGui::Begin(name.c_str(), &b_manualOpen, windowFlags)){
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
	if(!ImGui::IsPopupOpen(name.c_str())) {
		ImGui::OpenPopup(name.c_str());
		onPopupOpen();
	}
	
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

*/
