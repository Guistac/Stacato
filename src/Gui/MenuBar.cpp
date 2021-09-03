#include <pch.h>

#include "Gui.h"
#include "Framework/GuiWindow.h"

bool imguiDemoWindowOpen = false;
bool imguiMetricsWindowOpen = false;
bool implotDemoWindowOpen = false;

void mainMenuBar(bool closeWindowRequest) {

	bool openImguiDemoWindow = false;
	bool openImguiMetricsWindow = false;
	bool openImplotDemoWindow = false;
	bool openAboutPopup = false;

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Staccato")) {
		if (ImGui::MenuItem("About")) openAboutPopup = true;
		ImGui::Separator();
		if (ImGui::MenuItem("Quit")) closeWindowRequest = true;
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("File")) {
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit")) {
		ImGui::EndMenu();
	}
	if (ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT) && ImGui::IsKeyDown(GLFW_KEY_LEFT_SUPER)) {
		if (ImGui::BeginMenu("Utilities")) {
			if (ImGui::MenuItem("ImGui Demo Window", nullptr, &imguiDemoWindowOpen))		openImguiDemoWindow = true;
			if (ImGui::MenuItem("ImGui Metrics Window", nullptr, &imguiMetricsWindowOpen))	openImguiMetricsWindow = true;
			if (ImGui::MenuItem("ImPlot Demo Window", nullptr, &implotDemoWindowOpen))		openImplotDemoWindow = true;
			ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();

	//modals
	quitApplicationModal(closeWindowRequest);
	aboutModal(openAboutPopup);


	//utility windows
	if (openImguiDemoWindow) {
		ImGui::SetNextWindowFocus();
		imguiDemoWindowOpen = true;
	}
	if(imguiDemoWindowOpen) ImGui::ShowDemoWindow(&imguiDemoWindowOpen);
	
	if (openImguiMetricsWindow) {
		ImGui::SetNextWindowFocus();
		imguiMetricsWindowOpen = true;
	}
	if(imguiMetricsWindowOpen) ImGui::ShowMetricsWindow(&imguiMetricsWindowOpen);

	if (openImplotDemoWindow) {
		ImGui::SetNextWindowFocus();
		implotDemoWindowOpen = true;
	}
	if(implotDemoWindowOpen) ImPlot::ShowDemoWindow(&implotDemoWindowOpen);

}

void quitApplicationModal(bool quitRequest) {
	glm::vec2 center = ImGui::GetMainViewport()->GetCenter();
	static bool b_quitApplicationPopupOpen = false;
	bool quitApplication = false;
	ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	if (quitRequest) {
		ImGui::OpenPopup("Quitting Application");
		b_quitApplicationPopupOpen = true;
	}
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Quitting Application", &b_quitApplicationPopupOpen, popupFlags)) {
		ImGui::Text("Do you really want to exit the application ?");
		ImGui::Text("Proceeding will stop motion and discard any unsaved changes");
		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) ImGui::CloseCurrentPopup();
		ImGui::SameLine();
		if (ImGui::Button("Quit without Saving")) quitApplication = true;
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2, 0.5, 0.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2, 0.4, 0.1, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.6, 0.2, 1.0));
		if (ImGui::Button("Save and Quit") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
			quitApplication = true;
		}
		ImGui::PopStyleColor(3);
		ImGui::EndPopup();
	}
	if (quitApplication) {
		GuiWindow::setShouldClose();
	}
}

void aboutModal(bool openModal) {
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	static bool b_AboutPopupOpen = false;
	if (openModal) {
		ImGui::OpenPopup("About");
		b_AboutPopupOpen = true;
	}
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("About", &b_AboutPopupOpen, popupFlags)) {
		ImGui::PushFont(Fonts::robotoBold42);
		ImGui::Text("Staccato");
		ImGui::PopFont();
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Stage Control Automation Technology Software");
		ImGui::PopFont();
		ImGui::Text("Leo Becker - L'Atelier Artefact - 2021");
		ImGui::Separator();
		if (ImGui::Button("Close") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}