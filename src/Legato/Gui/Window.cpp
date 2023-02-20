#include <pch.h>
#include "Window.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui_Private.h"

void Window::open(){
	Legato::Gui::WindowManager::openWindow(shared_from_this());
}

void Window::close(){
	Legato::Gui::WindowManager::closeWindow(shared_from_this());
}

void Window::focus(){
	//we've not found out how to do this yet
}

void Window::draw(){
	
	if(!b_hasPadding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
	bool b_manualOpen = true;
	
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
	/*
	if(LayoutManager::b_lockLayout) windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	else windowFlags = ImGuiWindowFlags_None;
	*/
	if(ImGui::Begin(name.c_str(), &b_manualOpen, windowFlags)){
		imguiWindow = ImGui::GetCurrentContext()->CurrentWindow;
		if(!b_hasPadding) ImGui::PopStyleVar();
		onDraw();
	}else if(!b_hasPadding) ImGui::PopStyleVar();
	ImGui::End();
	if(!b_manualOpen) close();
}

bool Window::isOpen(){
	return b_isOpen;
}

bool Window::isFocused(){
	ImGuiWindow* focusedWindow = ImGui::GetCurrentContext()->NavWindow;
	if(focusedWindow == imguiWindow) return true;
	else if(ImGui::IsWindowChildOf(focusedWindow, imguiWindow, false, false)) return true;
	return false;
}




void Popup::open(){
	Legato::Gui::WindowManager::openPopup(shared_from_this());
}

void Popup::close(){
	Legato::Gui::WindowManager::closePopup(shared_from_this());
}

void Popup::draw(){
	if(!ImGui::IsPopupOpen(name.c_str())) {
		ImGui::OpenPopup(name.c_str());
		onOpen();
	}

	glm::vec2 size = getSize();
	if(size != glm::vec2(.0f, .0f)) ImGui::SetNextWindowSize(size);
	
	if(b_isModal){
		glm::vec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		
		bool b_manualOpen = true;
		bool* b_openPointer = b_canClose ? &b_manualOpen : nullptr;
		if(ImGui::BeginPopupModal(name.c_str(), b_openPointer, popupFlags)){
			onDraw();
			ImGui::EndPopup();
		}
		if(b_canClose && !b_manualOpen) close();
	}else{
		if(ImGui::BeginPopup(name.c_str())){
			onDraw();
			ImGui::EndPopup();
		}
	}
}
