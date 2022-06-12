#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui.h"

#include "ApplicationWindow.h"

#include "Gui/Project/ProjectGui.h"
#include "Environnement/Environnement.h"
#include "Gui/StageView/StageView.h"

#include "Gui/Plot/SequencerGui.h"

#include "Gui/Environnement/EnvironnementGui.h"
#include "Gui/Plot/PlotGui.h"

#include "Gui/Assets/Images.h"

#include "Tests/C_Curves.h"
#include "Tests/CommandZ.h"
#include "Gui/Utilities/ReorderableList.h"
#include "Gui/Environnement/Dashboard/Managers.h"


#include "Layout.h"

namespace Gui {

std::vector<std::shared_ptr<Window>> windowDictionnary;
std::vector<std::shared_ptr<Window>>& getWindowDictionnary(){ return windowDictionnary; }
void addWindowToDictionnary(std::shared_ptr<Window> window){ windowDictionnary.push_back(window); }
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
void closeWindow(std::shared_ptr<Window> window){
	for(int i = 0; i < openWindows.size(); i++){
		if(openWindows[i] == window){
			openWindows[i]->b_open = false;
			openWindows[i]->onClose();
			openWindows.erase(openWindows.begin() + i);
			break;
		}
	}
}

void closeAllWindows(){
	for(auto& window : openWindows){
		window->b_open = false;
		window->onClose();
	}
	openWindows.clear();
}


std::vector<std::shared_ptr<Popup>> popupList;
std::vector<std::shared_ptr<Popup>>& getPopups(){ return popupList; }
void openPopup(std::shared_ptr<Popup> popup){
	popup->b_open = true;
	popup->onOpen();
	popupList.push_back(popup);
}
void closePopup(std::shared_ptr<Popup> popup){
	for(int i = 0; i < popupList.size(); i++){
		if(popupList[i] == popup){
			popupList[i]->b_open = false;
			popupList[i]->onClose();
			popupList.erase(popupList.begin() + i);
			break;
		}
	}
}


bool b_initialized = false;
ImGuiID dockspaceID;

void initialize(){
	auto environnementEditorWindow = Environnement::Gui::EnvironnementEditorWindow::get();
	environnementEditorWindow->addToDictionnary();
	environnementEditorWindow->open();
	
	auto setupWindow = Environnement::Gui::SetupWindow::get();
	setupWindow->addToDictionnary();
	setupWindow->open();
	
	auto manoeuvreListWindow = PlotGui::ManoeuvreListWindow::get();
	manoeuvreListWindow->addToDictionnary();
	manoeuvreListWindow->open();
	
	auto trackSheetEditorWindow = PlotGui::TrackSheetEditorWindow::get();
	trackSheetEditorWindow->addToDictionnary();
	trackSheetEditorWindow->open();
	
	auto curveEditorWindow = PlotGui::CurveEditorWindow::get();
	curveEditorWindow->addToDictionnary();
	curveEditorWindow->open();
	
	auto SpatialEditorWindow = PlotGui::SpatialEditorWindow::get();
	SpatialEditorWindow->addToDictionnary();
	SpatialEditorWindow->open();
	
	auto sequencerWindow = Sequencer::Gui::SequencerWindow::get();
	sequencerWindow->addToDictionnary();
	sequencerWindow->open();
	
	auto dashboardWindow = DashboardWindow::get();
	dashboardWindow->addToDictionnary();
	dashboardWindow->open();
	
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TabActive] = ImVec4(.6f, .4f, 0.f, 1.f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.f, 0.f, 0.f, 1.f);
	style.FrameRounding = 5.0;
	
	dockspaceID = ImGui::GetID("MainDockspace");
	if(LayoutManager::getDefaultLayout()) LayoutManager::getDefaultLayout()->makeActive();
	else resetToFactoryLayout();
	
	b_initialized = true;
};

void draw(){
	
	//one time initialization on start
	if(!b_initialized) initialize();
	
	//get coordinates for main window and toolbar
	glm::vec2 mainWindowPosition = ImGui::GetMainViewport()->WorkPos;
	glm::vec2 mainWindowSize = ImGui::GetMainViewport()->WorkSize;
	float toolbarHeight = ImGui::GetTextLineHeight() * 4.0;
	mainWindowSize.y -= toolbarHeight;
	
	//draw main window with menu bar and dockspace
	ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_NoMove |
											ImGuiWindowFlags_MenuBar |					//main window has menu bar
											ImGuiWindowFlags_NoBringToFrontOnFocus |	//can't hide other windows behind the main window
											ImGuiWindowFlags_NoDocking |				//can't dock windows into the main window (but can into the contained dockspace)
											ImGuiWindowFlags_NoCollapse |
											ImGuiWindowFlags_NoResize |
											ImGuiWindowFlags_NoTitleBar |
											ImGuiWindowFlags_NoScrollbar |
											ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::SetNextWindowPos(mainWindowPosition);
	ImGui::SetNextWindowSize(mainWindowSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("MainWindow", nullptr, dockspaceWindowFlags);
	ImGui::PopStyleVar();
	menuBar();
	ImGui::DockSpace(dockspaceID, ImGui::GetContentRegionAvail(), ImGuiDockNodeFlags_NoWindowMenuButton);
	ImGui::End();
	
	//draw all windows
	for(auto& window : openWindows) window->draw();
	
	//draw toolbar
	ImGui::SetNextWindowPos(mainWindowPosition + glm::vec2(0, mainWindowSize.y));
	ImGui::SetNextWindowSize(glm::vec2(mainWindowSize.x, toolbarHeight));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.25));
	ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoTitleBar |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoCollapse |
									ImGuiWindowFlags_NoDocking |
									ImGuiWindowFlags_NoScrollWithMouse |
									ImGuiWindowFlags_NoScrollbar |
									ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin("Toolbar", nullptr, toolbarFlags);
	toolbar(toolbarHeight);
	ImGui::End();
	ImGui::PopStyleVar();
		
	//draw all popups
	for(auto& popup : popupList) popup->draw();
}

void resetToFactoryLayout(){
	ImGui::DockBuilderRemoveNodeDockedWindows(dockspaceID);
	ImGui::DockBuilderRemoveNodeChildNodes(dockspaceID);
	for(auto& window : windowDictionnary) ImGui::DockBuilderDockWindow(window->name.c_str(), dockspaceID);
	ImGui::DockBuilderFinish(dockspaceID);
}

}
