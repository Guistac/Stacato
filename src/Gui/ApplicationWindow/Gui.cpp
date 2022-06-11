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


#include "Layout.h"

namespace Gui {

std::vector<std::shared_ptr<Window>> windowDictionnary;
std::vector<std::shared_ptr<Window>>& getWindowDictionnary(){ return windowDictionnary; }
void addWindowToDictionnary(std::shared_ptr<Window> window){ windowDictionnary.push_back(window); }
void removeWindowFromDictionnary(std::shared_ptr<Window> window){
	for(int i = 0; i < windowDictionnary.size(); i++){
		if(windowDictionnary[i] == window){
			windowDictionnary.erase(windowDictionnary.begin() + i);
			break;
		}
	}
}


std::vector<std::shared_ptr<Window>> openWindows;
std::vector<std::shared_ptr<Window>>& getOpenWindows(){ return openWindows; }
void openWindow(std::shared_ptr<Window> window){ openWindows.push_back(window); }
void closeWindow(std::shared_ptr<Window> window){
	for(int i = 0; i < openWindows.size(); i++){
		if(openWindows[i] == window){
			openWindows.erase(openWindows.begin() + i);
			break;
		}
	}
}


std::vector<std::shared_ptr<Popup>> popupList;
std::vector<std::shared_ptr<Popup>>& getPopups(){ return popupList; }
void openPopup(std::shared_ptr<Popup> popup){ popupList.push_back(popup); }
void closePopup(std::shared_ptr<Popup> popup){
	for(int i = 0; i < popupList.size(); i++){
		if(popupList[i] == popup){
			popupList.erase(popupList.begin() + i);
			break;
		}
	}
}


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
	
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TabActive] = ImVec4(.6f, .4f, 0.f, 1.f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.f, 0.f, 0.f, 1.f);
	style.FrameRounding = 5.0;
};


void draw(){
	//get coordinates for main window and toolbar
	glm::vec2 mainWindowPosition = ImGui::GetMainViewport()->WorkPos;
	glm::vec2 mainWindowSize = ImGui::GetMainViewport()->WorkSize;
	float toolbarHeight = ImGui::GetTextLineHeight() * 4.0;
	mainWindowSize.y -= toolbarHeight;
	
	//draw main window with menu bar and dockspace
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("MainWindow", nullptr,
				 ImGuiWindowFlags_MenuBar |					//main window has menu bar
				 ImGuiWindowFlags_NoBringToFrontOnFocus |	//can't hide other windows behind the main window
				 ImGuiWindowFlags_NoDocking |				//can't dock windows into the main window (but can into the contained dockspace)
				 ImGuiWindowFlags_NoCollapse |
				 ImGuiWindowFlags_NoResize |
				 ImGuiWindowFlags_NoTitleBar |
				 ImGuiWindowFlags_NoScrollbar |
				 ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PopStyleVar();
	menuBar();
	ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
	ImGui::DockSpace(dockspaceID, ImGui::GetContentRegionAvail(), ImGuiDockNodeFlags_NoWindowMenuButton);
	ImGui::End();
	
	//draw all windows
	for(auto& window : openWindows) window->draw();
	
	//draw toolbar
	ImGui::SetNextWindowPos(mainWindowPosition + glm::vec2(0, mainWindowSize.y));
	ImGui::SetNextWindowSize(glm::vec2(mainWindowSize.x, toolbarHeight));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.25));
	ImGui::Begin("Toolbar", nullptr,
				 ImGuiWindowFlags_NoTitleBar |
				 ImGuiWindowFlags_NoResize |
				 ImGuiWindowFlags_NoCollapse |
				 ImGuiWindowFlags_NoDocking |
				 ImGuiWindowFlags_NoScrollWithMouse |
				 ImGuiWindowFlags_NoScrollbar);
	toolbar(toolbarHeight);
	ImGui::End();
	ImGui::PopStyleVar();
	
	//draw all popups
	for(auto& popup : popupList) popup->draw();
}

}


/*
 
 //=== Reset Default Layout if Requested ===
 if(shouldResetDefaultLayout()){
	 ImGui::DockBuilderRemoveNodeDockedWindows(dockspaceID);
	 ImGui::DockBuilderRemoveNodeChildNodes(dockspaceID);
	 
	 //ImGuiID leftID;
	 //ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.15, &leftID, &dockspaceID);
	 
	 ImGui::DockBuilderDockWindow("Environnement", dockspaceID);
	 //ImGui::DockBuilderDockWindow("Stage", dockspaceID);
	 ImGui::DockBuilderDockWindow("Dashboard", dockspaceID);
	 ImGui::DockBuilderDockWindow("Setup", dockspaceID);
	 ImGui::DockBuilderDockWindow("Manoeuvre List", dockspaceID);
	 ImGui::DockBuilderDockWindow("Manoeuvre Sheet", dockspaceID);
	 ImGui::DockBuilderDockWindow("Manoeuvre Curves", dockspaceID);
	 //ImGui::DockBuilderDockWindow("Sequencer", dockspaceID);
	 //ImGui::DockBuilderDockWindow("cCurvesTest", dockspaceID);
	 //ImGui::DockBuilderDockWindow("CommandZ", dockspaceID);
	 ImGui::DockBuilderFinish(dockspaceID);
 }
 
 */
