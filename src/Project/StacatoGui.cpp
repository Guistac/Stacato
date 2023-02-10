#include <pch.h>

#include "Stacato.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Images.h"
#include "Visualizer/Visualizer.h"

#include "Workspace/Gui.h"

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 1
#define OPENGL_VERSION_STRING "#version 410 core"


#include "Gui/ApplicationWindow/Layout.h"

#include "Gui/Project/ProjectGui.h"
#include "Environnement/Environnement.h"
#include "Gui/Plot/SequencerGui.h"
#include "Gui/Environnement/EnvironnementGui.h"
#include "Gui/Plot/PlotGui.h"
#include "Gui/Environnement/Dashboard/Managers.h"
#include "Gui/Plot/PlaybackGui.h"
#include "Gui/Fieldbus/EtherCatGui.h"

#include "Gui/ApplicationWindow/Gui.h"

#include <imgui_internal.h>


namespace Stacato::Gui{

ImGuiID dockspaceID;

void setDefaultLayout(){
	WindowManager::closeAllWindows();
	for(auto& window : WindowManager::getWindowDictionnary()) window->open();
	ImGui::DockBuilderRemoveNodeDockedWindows(dockspaceID);
	ImGui::DockBuilderRemoveNodeChildNodes(dockspaceID);
	for(auto& window : WindowManager::getOpenWindows()) ImGui::DockBuilderDockWindow(window->name.c_str(), dockspaceID);
	ImGui::DockBuilderFinish(dockspaceID);
	
#ifdef STACATO_DEBUG
	Environnement::Gui::NodeEditorWindow::get()->focus();
#else
	DashboardWindow::get()->focus();
#endif
}

void initialize(){
	Fonts::load(NewGui::getScale());
	Images::load();
	ImGui::StyleColorsDark();
	
	Environnement::StageVisualizer::initialize(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR);
	
	
	//editor windows
	Environnement::Gui::NodeEditorWindow::get()->addToDictionnary();
	Environnement::Gui::NodeManagerWindow::get()->addToDictionnary();
	Environnement::Gui::VisualizerScriptWindow::get()->addToDictionnary();
	Environnement::Gui::EtherCATWindow::get()->addToDictionnary();
	Environnement::Gui::ScriptEditorWindow::get()->addToDictionnary();
	
	//performance windows
	Environnement::Gui::SetupWindow::get()->addToDictionnary();
	Environnement::Gui::VisualizerWindow::get()->addToDictionnary();
	PlotGui::ManoeuvreListWindow::get()->addToDictionnary();
	PlotGui::TrackSheetEditorWindow::get()->addToDictionnary();
	PlotGui::CurveEditorWindow::get()->addToDictionnary();
	//Sequencer::Gui::SequencerWindow::get()->addToDictionnary();
	DashboardWindow::get()->addToDictionnary();
	Playback::Gui::PlaybackManagerWindow::get()->addToDictionnary();
	EtherCatNetworkWindow::get()->addToDictionnary();
	EtherCatDevicesWindow::get()->addToDictionnary();
	Environnement::Gui::LogWindow::get()->addToDictionnary();
	
	#if defined(STACATO_DEBUG)
	Environnement::unlockEditor();
	#else
	Environnement::lockEditor();
	#endif
		
}

void terminate(){
	Environnement::StageVisualizer::terminate();
}




void gui(){
	
	//one time initialization on start
	static bool b_initialized = false;
	if(!b_initialized) {
		
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TabActive] = ImVec4(.6f, .4f, 0.f, 1.f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.f, 0.f, 0.f, 1.f);
		float rounding = ImGui::GetTextLineHeight() * 0.25;
		style.FrameRounding = rounding;
		style.PopupRounding = rounding;
		style.WindowRounding = rounding;
		style.GrabRounding = rounding;
		
		dockspaceID = ImGui::GetID("MainDockspace");
		if(auto defaultLayout = LayoutManager::getDefaultLayout()) defaultLayout->makeActive();
		else setDefaultLayout();
		
		b_initialized = true;
	}

	
	//get coordinates for main window and toolbar
	glm::vec2 mainWindowPosition = ImGui::GetMainViewport()->WorkPos;
	float toolbarHeight = ImGui::GetTextLineHeight() * 4.0;
	glm::vec2 mainWindowSize = ImGui::GetMainViewport()->WorkSize;
	mainWindowSize.y -= toolbarHeight;
	glm::vec2 toolbarPosition(mainWindowPosition.x, mainWindowPosition.y + mainWindowSize.y);
	glm::vec2 toolbarSize(mainWindowSize.x, toolbarHeight);
	
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
	::Gui::menuBar();
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoCloseButton;
	ImGui::DockSpace(dockspaceID, ImGui::GetContentRegionAvail(), dockspaceFlags);
	ImGui::End();
	
	//draw toolbar
	ImGui::SetNextWindowPos(toolbarPosition);
	ImGui::SetNextWindowSize(toolbarSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.25));
	ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoTitleBar |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoCollapse |
									ImGuiWindowFlags_NoDocking |
									ImGuiWindowFlags_NoScrollWithMouse |
									ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Toolbar", nullptr, toolbarFlags);
	::Gui::toolbar(toolbarHeight);
	ImGui::End();
	ImGui::PopStyleVar();
	
	WindowManager::manage();
	LayoutManager::manage();
	
}

};
