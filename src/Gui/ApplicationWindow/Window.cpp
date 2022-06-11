#include "Window.h"

#include "Gui.h"

#include <imgui.h>
#include <imgui_internal.h>

void Window::addToDictionnary(){
	Gui::addWindowToDictionnary(shared_from_this());
}

void Window::removeFromDictionnary(){
	close();
	Gui::removeWindowFromDictionnary(shared_from_this());
}

void Window::open(){
	Gui::openWindow(shared_from_this());
	b_open = true;
	onOpen();
}

void Window::close(){
	Gui::closeWindow(shared_from_this());
	b_open = false;
	onClose();
}

void Window::draw(){
	if(!b_open) return;
	if(!b_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
	if(ImGui::Begin(name.c_str(), &b_open)){
		if(!b_padding) ImGui::PopStyleVar();
		drawContent();
	}else if(!b_padding) ImGui::PopStyleVar();
	ImGui::End();
}



void Popup::open(){
	ImGui::OpenPopup(name.c_str());
	b_open = true;
	Gui::openPopup(shared_from_this());
}

void Popup::close(){
	ImGui::CloseCurrentPopup();
	b_open = false;
	Gui::closePopup(shared_from_this());
}

void Popup::draw(){
	if(b_modal){
		glm::vec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if(ImGui::BeginPopupModal(name.c_str(), &b_open, popupFlags)){
			drawContent();
			ImGui::EndPopup();
		}
	}else{
		if(ImGui::BeginPopup(name.c_str())){
			drawContent();
			ImGui::EndPopup();
		}
	}
}

