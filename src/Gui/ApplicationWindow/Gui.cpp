#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui.h"

#include "ApplicationWindow.h"

#include "Gui/Project/ProjectGui.h"
#include "Environnement/Environnement.h"
#include "Gui/StageView/StageView.h"

#include "Gui/Environnement/EnvironnementGui.h"
#include "Gui/Plot/PlotGui.h"

#include "Gui/Assets/Images.h"

namespace Gui {

	void draw() {		
		
		//=== Push Main Style Variables ===
		ImGui::PushStyleColor(ImGuiCol_TabActive, glm::vec4(0.6, 0.4, 0.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0, 0.0, 0.0, 1.0));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0);
		
		//=== Define Bottom Toolbar and main window height
		float toolbarHeight = ImGui::GetTextLineHeight() * 3.2;
		glm::vec2 mainWindowPosition = ImGui::GetMainViewport()->WorkPos;
		glm::vec2 mainWindowSize = ImGui::GetMainViewport()->WorkSize;
		mainWindowSize.y -= toolbarHeight;
		
		//=== Draw Main Window with MenuBar & DockSpace
		ImGui::SetNextWindowPos(mainWindowPosition);
		ImGui::SetNextWindowSize(mainWindowSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0, 3.0));
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
		ImGui::DockSpace(dockspaceID,
						 ImGui::GetContentRegionAvail(),
						 ImGuiDockNodeFlags_NoWindowMenuButton);
		ImGui::End();
	
		//=== Reset Default Layout if Requested ===
		if(shouldResetDefaultLayout()){
			ImGui::DockBuilderRemoveNodeDockedWindows(dockspaceID);
			ImGui::DockBuilderRemoveNodeChildNodes(dockspaceID);
			ImGui::DockBuilderDockWindow("Environnement Editor", dockspaceID);
			ImGui::DockBuilderDockWindow("Stage View", dockspaceID);
			ImGui::DockBuilderDockWindow("Machine", dockspaceID);
			ImGui::DockBuilderDockWindow("Setup", dockspaceID);
			ImGui::DockBuilderDockWindow("Plot", dockspaceID);
			ImGui::DockBuilderFinish(dockspaceID);
		}
		
		//=== Submit Application Windows as movable dock nodes ===
		
		if(!Environnement::isEditorLocked()){
			ImGui::Begin("Environnement Editor");
			Environnement::Gui::gui();
			ImGui::End();
		}

		ImGui::Begin("Setup");
		Environnement::Gui::homingAndSetup();
		ImGui::End();
		
		ImGui::Begin("Machine");
		Environnement::Gui::machineList();
		ImGui::End();
		
		ImGui::Begin("Stage View");
		StageView::draw();
		ImGui::End();
		 
		ImGui::Begin("Plot");
		PlotGui::editor();
		ImGui::End();
		
		//=== Finish Resetting Default Layout ===
		
		if(shouldResetDefaultLayout()){
			ImGui::SetWindowFocus("Environnement Editor");
			finishResetDefaultLayout();
		}
		
		
		//=== Draw Bottom Toolbar ===
		ImGui::SetNextWindowPos(mainWindowPosition + glm::vec2(0, mainWindowSize.y));
		ImGui::SetNextWindowSize(glm::vec2(mainWindowSize.x, toolbarHeight));
		ImGui::Begin("##Toolbar", nullptr,
					 ImGuiWindowFlags_NoTitleBar |
					 ImGuiWindowFlags_NoResize |
					 ImGuiWindowFlags_NoCollapse |
					 ImGuiWindowFlags_NoDocking);
		toolbar(toolbarHeight);
		ImGui::End();
		
		//=== Draw Popups (if any are open) ===
		popups();
		
		
		//=== Pop Main Style Variables ===
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();

	}

	bool b_resetDefaultLayout = true;
	void resetDefaultLayout(){ b_resetDefaultLayout = true; }
	bool shouldResetDefaultLayout(){ return b_resetDefaultLayout; }
	void finishResetDefaultLayout(){ b_resetDefaultLayout = false; }

}
