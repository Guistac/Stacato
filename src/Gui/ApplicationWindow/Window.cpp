#include "Window.h"

#include "Gui.h"

#include <imgui.h>
#include <imgui_internal.h>

void Window::addToDictionnary(){ Gui::addWindowToDictionnary(shared_from_this()); }
void Window::removeFromDictionnary(){ Gui::removeWindowFromDictionnary(shared_from_this()); }

void Window::open(){
	if(isOpen()) return;
	Gui::openWindow(shared_from_this());
}

void Window::draw(){
	if(!b_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
	if(ImGui::Begin(name.c_str(), &b_open)){
		if(!b_padding) ImGui::PopStyleVar();
		drawContent();
	}else if(!b_padding) ImGui::PopStyleVar();
	ImGui::End();
}



void Popup::open(){ Gui::openPopup(shared_from_this()); }

void Popup::close(){ Gui::closePopup(shared_from_this()); }

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

