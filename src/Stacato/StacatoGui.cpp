#include <pch.h>

#include "StacatoGui.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Images.h"
#include "Visualizer/Visualizer.h"

#include "Legato/Gui/Gui.h"

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 1
#define OPENGL_VERSION_STRING "#version 410 core"


#include "Legato/Gui/Layout.h"

#include "Gui/ApplicationWindow/Gui.h"

#include <imgui_internal.h>


#include "Legato/Gui/Window.h"
#include <GLFW/glfw3.h>
#include "Gui/Project/ProjectGui.h"

#include "Project/StacatoProject.h"
#include "StacatoEditor.h"

#include "Gui/Utilities/FileDialog.h"

#include "Legato/Workspace.h"


namespace Stacato::Gui{

ImGuiID dockspaceID;

/*
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
 */

void initialize(){
	Fonts::load(Legato::Gui::getScale());
	Images::load();
	ImGui::StyleColorsDark();
	
	//Environnement::StageVisualizer::initialize(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR);
}

void terminate(){
	//Environnement::StageVisualizer::terminate();
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
		style.PopupRounding = 0.0;
		style.WindowRounding = 0.0;
		style.GrabRounding = rounding;
		dockspaceID = ImGui::GetID("MainDockspace");
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
	menuBar();
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
	toolbar(toolbarHeight);
	ImGui::End();
	ImGui::PopStyleVar();
	
}











	void load(){
		FileDialog::FilePath path;
		FileDialog::FileTypeFilter filter("Stacato Project File", "stacato");
		if(FileDialog::load(path, filter)) {
			std::filesystem::path fsPath = path.path;
			Workspace::openFile(fsPath);
		}
	}

	bool save(){
		if(Stacato::Editor::hasCurrentProject()){
			auto project = Stacato::Editor::getCurrentProject();
			if(project->hasFilePath()) return project->writeFile();
			else return saveAs();
		}
		return false;
	}

	bool saveAs(){
		if(!Stacato::Editor::hasCurrentProject()) return false;
		auto project = Stacato::Editor::getCurrentProject();
		FileDialog::FilePath path;
		FileDialog::FileTypeFilter filter("Stacato Project File", "stacato");
		if(FileDialog::save(path, filter, "project")) {
			std::filesystem::path fsPath = path.path;
			project->setFilePath(fsPath);
			return project->writeFile();
		}
		return false;
	}




	void QuitApplicationPopup::onDraw() {
		ImGui::Text("Do you really want to exit the application ?");
		ImGui::Text("Proceeding will stop motion and discard any unsaved changes");
		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) close();
		ImGui::SameLine();
		if (ImGui::Button("Quit without Saving")) {
			close();
			//ApplicationWindow::quit();
		}
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2, 0.5, 0.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2, 0.4, 0.1, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.6, 0.2, 1.0));
		if (ImGui::Button("Save and Quit") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
			//if(Project::Gui::save()) ApplicationWindow::quit();
			close();
		}
		ImGui::PopStyleColor(3);
	}


	
	void AboutPopup::onDraw() {
		float iconSize = ImGui::GetTextLineHeight() * 7.0;
		float textSize = Fonts::sansBold42->FontSize + Fonts::sansBold20->FontSize + ImGui::GetTextLineHeight() + 2.0 * ImGui::GetStyle().ItemSpacing.y;
		
		glm::vec2 cursorPos = ImGui::GetCursorPos();
		ImGui::Image(Images::StacatoIcon.getID(), glm::vec2(iconSize));
		ImGui::SameLine();
		
		ImGui::BeginGroup();
		ImGui::SetCursorPosY(cursorPos.y + iconSize - textSize);
		
		ImGui::PushFont(Fonts::sansBold42);
		ImGui::Text("Stacato");
		ImGui::PopFont();
		
		ImFont* bold = Fonts::sansBold20;
		ImFont* thin = Fonts::sansLight20;
		auto textFont = [](const char* text, ImFont* font, bool sameline = true){
			ImGui::PushFont(font);
			ImGui::Text("%s", text);
			ImGui::PopFont();
			if(sameline) ImGui::SameLine();
		};
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
		textFont("Sta", bold);
		textFont("ge ", thin);
		textFont("C", bold);
		textFont("ontrol ", thin);
		textFont("A", bold);
		textFont("utomation ", thin);
		textFont("To", bold);
		textFont("olbox", thin, false);
		ImGui::PopStyleVar();
		
		
		ImGui::Text("Leo Becker - L'Atelier Artefact - 2021");
		ImGui::EndGroup();
	}

	void CloseProjectPopup::onDraw(){
		ImGui::Text("Do you really want to close the current project ?");
		ImGui::Text("Proceeding will stop motion and discard any unsaved changes");
		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) close();
		ImGui::SameLine();
		if (ImGui::Button("Close without saving")) {
			//Project::confirmNewProjectRequest();
			close();
		}
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2, 0.5, 0.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2, 0.4, 0.1, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.6, 0.2, 1.0));
		if (ImGui::Button("Save and Close") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
			bool b_saved = Stacato::Editor::getCurrentProject()->writeFile();
			close();
		}
		ImGui::PopStyleColor(3);
	}




};




/*
 void setFloatingPointComma(){
	 char* l = setlocale(LC_NUMERIC, "fr_FR.UTF-8");
	 ImGui::GetCurrentContext()->PlatformLocaleDecimalPoint = *localeconv()->decimal_point;
 }
 void setFloatingPointPeriod(){
	 char* l = setlocale(LC_NUMERIC, "C");
	 ImGui::GetCurrentContext()->PlatformLocaleDecimalPoint = *localeconv()->decimal_point;
 }
 */
