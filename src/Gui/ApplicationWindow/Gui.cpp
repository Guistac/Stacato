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



struct TestItem{
	std::string name;
	float height;
	ImVec4 color;
};

void dragListTest(){
	static std::vector<TestItem> items = {
		{.name = "Zero", 	.height = 30.0, .color = ImVec4(0.2f, 0.2f, 0.2f, 1.f)},
		{.name = "One", 	.height = 30.0, .color = ImVec4(0.0f, 0.0f, 0.7f, 1.f)},
		{.name = "Two", 	.height = 40.0, .color = ImVec4(0.0f, 0.7f, 0.0f, 1.f)},
		{.name = "Three", 	.height = 50.0, .color = ImVec4(0.0f, 0.7f, 0.7f, 1.f)},
		{.name = "Four", 	.height = 35.0, .color = ImVec4(0.7f, 0.0f, 0.0f, 1.f)},
		{.name = "Five", 	.height = 45.0, .color = ImVec4(0.7f, 0.0f, 0.7f, 1.f)},
		{.name = "Six", 	.height = 55.0, .color = ImVec4(0.7f, 0.7f, 0.0f, 1.f)},
		{.name = "Seven", 	.height = 20.0, .color = ImVec4(0.2f, 0.2f, 0.2f, .5f)},
		{.name = "Eight", 	.height = 60.0, .color = ImVec4(0.0f, 0.0f, 0.7f, .5f)},
		{.name = "Nine", 	.height = 99.0, .color = ImVec4(0.0f, 0.7f, 0.0f, .5f)},
		{.name = "Ten", 	.height = 35.0, .color = ImVec4(0.0f, 0.7f, 0.7f, .5f)},
		{.name = "0", 	.height = 30.0, .color = ImVec4(0.2f, 0.2f, 0.2f, 1.f)},
		{.name = "1", 	.height = 30.0, .color = ImVec4(0.0f, 0.0f, 0.7f, 1.f)},
		{.name = "2", 	.height = 40.0, .color = ImVec4(0.0f, 0.7f, 0.0f, 1.f)},
		{.name = "3", 	.height = 50.0, .color = ImVec4(0.0f, 0.7f, 0.7f, 1.f)},
		{.name = "4", 	.height = 35.0, .color = ImVec4(0.7f, 0.0f, 0.0f, 1.f)},
		{.name = "5", 	.height = 45.0, .color = ImVec4(0.7f, 0.0f, 0.7f, 1.f)},
		{.name = "6", 	.height = 55.0, .color = ImVec4(0.7f, 0.7f, 0.0f, 1.f)},
		{.name = "7", 	.height = 20.0, .color = ImVec4(0.2f, 0.2f, 0.2f, .5f)},
		{.name = "8", 	.height = 60.0, .color = ImVec4(0.0f, 0.0f, 0.7f, .5f)},
		{.name = "9", 	.height = 99.0, .color = ImVec4(0.0f, 0.7f, 0.0f, .5f)},
		{.name = "10", 	.height = 35.0, .color = ImVec4(0.0f, 0.7f, 0.7f, .5f)}
	};
	
	if(ReorderableList::begin("ListTest")){
		
		for(auto& item : items){
			if(ReorderableList::beginItem(40.0)){
				
				ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(),
														  ImGui::GetItemRectMax(),
														  ImColor(item.color),
														  5.0,
														  ImDrawFlags_RoundCornersAll);
				
				ImGui::Text("%s", item.name.c_str());
				ReorderableList::endItem();
			}
		}
		
		ReorderableList::end();
	}
	
	int fromIndex, toIndex;
	if(ReorderableList::wasReordered(fromIndex, toIndex)){
		Logger::warn("moved item {} to {}", fromIndex, toIndex);
		
		TestItem temp = items[fromIndex];
		items.erase(items.begin() + fromIndex);
		items.insert(items.begin() + toIndex, temp);
		
	}
	
	
	
	
	
}



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
		
		if(ImGui::Begin("Machines")) Environnement::Gui::machineList();
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
		 
		//if(ImGui::Begin("DragList")) dragListTest();
		//ImGui::End();
		
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
