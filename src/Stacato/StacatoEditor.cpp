#include <pch.h>

#include "StacatoEditor.h"

#include "Gui/Project/ProjectGui.h"
#include "Environnement/Environnement.h"
#include "Gui/Plot/SequencerGui.h"
#include "Gui/Environnement/EnvironnementGui.h"
#include "Gui/Plot/PlotGui.h"
#include "Gui/Environnement/Dashboard/Managers.h"
#include "Gui/Plot/PlaybackGui.h"
#include "Gui/Fieldbus/EtherCatGui.h"

#include "Legato/Gui/Gui.h"

class Window;

namespace Stacato::Editor{

std::vector<std::shared_ptr<Window>> userWindows;
std::vector<std::shared_ptr<Window>> administratorWindows;
std::vector<std::shared_ptr<Window>>& getUserWindows(){ return userWindows; }
std::vector<std::shared_ptr<Window>>& getAdministratorWindows(){ return administratorWindows; }

bool b_isLocked = false;

bool isLocked(){ return b_isLocked; }
void unlock(){
	b_isLocked = false;
	for(auto adminWindow : administratorWindows){
		Legato::Gui::WindowManager::registerWindow(adminWindow);
	}
}
void lock(){
	b_isLocked = true;
	for(auto adminWindow : administratorWindows){
		Legato::Gui::WindowManager::unregisterWindow(adminWindow);
		adminWindow->close();
	}
}
bool checkEditorPassword(std::string& password){ return password == "StacatoCompact"; }

void initialize(){
	//admin windows
	administratorWindows.push_back(Environnement::Gui::NodeEditorWindow::get());
	administratorWindows.push_back(Environnement::Gui::NodeManagerWindow::get());
	administratorWindows.push_back(Environnement::Gui::VisualizerScriptWindow::get());
	administratorWindows.push_back(Environnement::Gui::EtherCATWindow::get());
	administratorWindows.push_back(Environnement::Gui::ScriptEditorWindow::get());
	
	//user windows
	userWindows.push_back(Environnement::Gui::SetupWindow::get());
	userWindows.push_back(Environnement::Gui::VisualizerWindow::get());
	userWindows.push_back(PlotGui::ManoeuvreListWindow::get());
	userWindows.push_back(PlotGui::TrackSheetEditorWindow::get());
	userWindows.push_back(PlotGui::CurveEditorWindow::get());
	userWindows.push_back(DashboardWindow::get());
	userWindows.push_back(Playback::Gui::PlaybackManagerWindow::get());
	userWindows.push_back(EtherCatNetworkWindow::get());
	userWindows.push_back(EtherCatDevicesWindow::get());
	userWindows.push_back(Environnement::Gui::LogWindow::get());
	
	for(auto userWindow : userWindows){
		Legato::Gui::WindowManager::registerWindow(userWindow);
	}
	unlock();
}

void terminate(){
	
}


};
