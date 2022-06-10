#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui.h"

#include "ApplicationWindow.h"

#include "Gui/Project/ProjectGui.h"
#include "Environnement/Environnement.h"
#include "Gui/StageView/StageView.h"

#include "Gui/Plot/Sequencer.h"

#include "Gui/Environnement/EnvironnementGui.h"
#include "Gui/Plot/PlotGui.h"

#include "Gui/Assets/Images.h"

#include "Tests/C_Curves.h"
#include "Tests/CommandZ.h"
#include "Gui/Utilities/ReorderableList.h"


#include "Layout.h"

namespace Gui {

	void draw() {		
		
		//=== Push Main Style Variables ===
		ImGui::PushStyleColor(ImGuiCol_TabActive, glm::vec4(0.6, 0.4, 0.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0, 0.0, 0.0, 1.0));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0);
		
		//=== Define Bottom Toolbar and main window height
		float toolbarHeight = ImGui::GetTextLineHeight() * 4.0;
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
			
			//ImGuiID leftID;
			//ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.15, &leftID, &dockspaceID);
			
			ImGui::DockBuilderDockWindow("Environnement", dockspaceID);
			//ImGui::DockBuilderDockWindow("Stage", dockspaceID);
			ImGui::DockBuilderDockWindow("Machines", dockspaceID);
			ImGui::DockBuilderDockWindow("Setup", dockspaceID);
			ImGui::DockBuilderDockWindow("Manoeuvre List", dockspaceID);
			ImGui::DockBuilderDockWindow("Manoeuvre Sheet", dockspaceID);
			ImGui::DockBuilderDockWindow("Manoeuvre Curves", dockspaceID);
			//ImGui::DockBuilderDockWindow("Sequencer", dockspaceID);
			//ImGui::DockBuilderDockWindow("cCurvesTest", dockspaceID);
			//ImGui::DockBuilderDockWindow("CommandZ", dockspaceID);
			ImGui::DockBuilderFinish(dockspaceID);
		}
		
		//=== Submit Application Windows as movable dock nodes ===
		
		if(!Environnement::isEditorLocked()){
			ImGui::Begin("Environnement");
			Environnement::Gui::gui();
			ImGui::End();
		}

		if(ImGui::Begin("Setup")) Environnement::Gui::homingAndSetup();
		ImGui::End();
		
		if(ImGui::Begin("Machines"))
			//Environnement::Gui::machineList();
			Environnement::Gui::dashboards();
		ImGui::End();
		 
		if(ImGui::Begin("Manoeuvre List")) PlotGui::manoeuvreList();
		ImGui::End();
		
		if(ImGui::Begin("Manoeuvre Sheet")) PlotGui::trackSheetEditor();
		ImGui::End();
		
		if(ImGui::Begin("Manoeuvre Curves")) PlotGui::curveEditor();
		ImGui::End();
		
		/*
		if(ImGui::Begin("cCurvesTest")) cCurvesTest();
		ImGui::End();
		
		if(ImGui::Begin("CommandZ")) testUndoHistory();
		ImGui::End();
		*/
		 
		/*
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0,0));
		if(ImGui::Begin("Sequencer")) Sequencer::Gui::editor();
		ImGui::End();
		ImGui::PopStyleVar();
		*/
		
		
		 
		/*
		if(ImGui::Begin("Stage")) StageView::draw();
		ImGui::End();
		*/
		 
		//=== Finish Resetting Default Layout ===
		
		if(shouldResetDefaultLayout()){
			ImGui::SetWindowFocus("Environnement");
			//ImGui::SetWindowFocus("Sequencer");
			//ImGui::SetWindowFocus("Track Sheet Editor");
			//ImGui::SetWindowFocus("cCurvesTest");
			//ImGui::SetWindowFocus("CommandZ");
			finishResetDefaultLayout();
			
			LayoutManager::setDefault();
		}
		
		
		//=== Draw Bottom Toolbar ===
		ImGui::SetNextWindowPos(mainWindowPosition + glm::vec2(0, mainWindowSize.y));
		ImGui::SetNextWindowSize(glm::vec2(mainWindowSize.x, toolbarHeight));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.25));
		ImGui::Begin("##Toolbar", nullptr,
					 ImGuiWindowFlags_NoTitleBar |
					 ImGuiWindowFlags_NoResize |
					 ImGuiWindowFlags_NoCollapse |
					 ImGuiWindowFlags_NoDocking |
					 ImGuiWindowFlags_NoScrollWithMouse |
					 ImGuiWindowFlags_NoScrollbar);
		toolbar(toolbarHeight);
		ImGui::End();
		ImGui::PopStyleVar();
		
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
