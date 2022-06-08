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



ImVec2 operator*(const ImVec2& other, const float& multiplier){ return ImVec2(other.x * multiplier, other.y * multiplier); }

void scaleTest(){
	
	static float scale = 1.0;
	static glm::vec2 offset(0.0, 0.0);
	
	struct Widget{
		std::string name;
		glm::vec2 position;
		glm::vec2 size;
	};
	
	static Widget w1{.name = "Widget1", .position = glm::vec2(0,0),		.size = glm::vec2(150, 150)};
	static Widget w2{.name = "Widget2", .position = glm::vec2(100,100),	.size = glm::vec2(150, 150)};
	static Widget w3{.name = "Widget3", .position = glm::vec2(250,100),	.size = glm::vec2(150, 150)};
	
	ImVec2 dashboardSize = ImGui::GetContentRegionAvail();
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
	ImGui::BeginChild("dashboard", dashboardSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PopStyleVar();
	glm::vec2 windowPos = ImGui::GetWindowPos();
	glm::vec2 windowSize = ImGui::GetWindowSize();
	glm::vec2 windowMax = windowPos + windowSize;
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(.0f, .0f, .0f, 1.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().FrameRounding);
	
	ImGui::SetCursorPos(ImVec2(0,0));
	ImGui::InvisibleButton("Dashboard", dashboardSize);
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	auto screenToCanvas = [&](glm::vec2 screen)->glm::vec2{
		return (screen - windowPos - offset) / scale;
	};
	
	auto canvasToScreen = [&](glm::vec2 canvas)->glm::vec2{
		return (canvas * scale) + offset + windowPos;
	};
	
	auto canvasToCursor = [&](glm::vec2 canvas)->glm::vec2{
		return (canvas * scale) + offset;
	};
	
	auto zoom = [&](glm::vec2 screenZoomPosition, float delta){
		glm::vec2 canvasZoomPosition = screenToCanvas(screenZoomPosition);
		scale *= 1.0 + delta;
		glm::vec2 newScreenZoomPosition = canvasToScreen(canvasZoomPosition);
		glm::vec2 zoomOffsetSlip = newScreenZoomPosition - screenZoomPosition;
		offset -= zoomOffsetSlip;
	};
	
	auto pan = [&](glm::vec2 mouseDelta){
		offset += mouseDelta;
	};
	
	auto widget = [&](Widget& w){
		glm::vec2 cursor = canvasToCursor(w.position);
		ImGui::SetCursorPos(cursor);
		if(ImGui::BeginChild(w.name.c_str(), w.size * scale, true)){
			ImGui::Text("%s", w.name.c_str());
			float size = ImGui::GetTextLineHeight();
			if(ImGui::Button("custom size", ImVec2(size * 5.0,size * 2.0)));
			
			if(ImGui::IsWindowHovered()){
				if(ImGui::IsMouseDown(ImGuiMouseButton_Left)){
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
					glm::vec2 delta = ImGui::GetIO().MouseDelta;
					w.position += delta;
				}else ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			}
		}else Logger::warn("window clipped");
		ImGui::EndChild();
	};
	
	
	
	if(ImRect(windowPos, windowMax).Contains(ImGui::GetMousePos())){
		double zoomDelta = ApplicationWindow::getMacOsTrackpadZoom();
		if(zoomDelta != 0.0) zoom(ImGui::GetMousePos(), zoomDelta);
		ImGuiIO& io = ImGui::GetIO();
		glm::vec2 scrollDelta = glm::vec2(io.MouseWheelH, io.MouseWheel);
		if(scrollDelta != glm::vec2(0.0f, 0.0f)) pan(scrollDelta * 10.0);
	}
	if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
		pan(ImGui::GetIO().MouseDelta);
	}
	
	 
	ImGuiStyle defaultStyleCopy = ImGui::GetStyle();
	ImGuiStyle& dashboardStyle = ImGui::GetStyle();
	dashboardStyle.WindowPadding = dashboardStyle.WindowPadding * scale;
	dashboardStyle.WindowRounding = dashboardStyle.WindowRounding * scale;
	dashboardStyle.WindowMinSize = dashboardStyle.WindowMinSize * scale;
	dashboardStyle.ChildRounding = dashboardStyle.ChildRounding * scale;
	dashboardStyle.PopupRounding = dashboardStyle.PopupRounding * scale;
	dashboardStyle.FramePadding = dashboardStyle.FramePadding * scale;
	dashboardStyle.FrameRounding = dashboardStyle.FrameRounding * scale;
	dashboardStyle.ItemSpacing = dashboardStyle.ItemSpacing * scale;
	dashboardStyle.ItemInnerSpacing = dashboardStyle.ItemInnerSpacing * scale;
	dashboardStyle.CellPadding = dashboardStyle.CellPadding * scale;
	dashboardStyle.TouchExtraPadding = dashboardStyle.TouchExtraPadding * scale;
	dashboardStyle.IndentSpacing = dashboardStyle.IndentSpacing * scale;
	dashboardStyle.ColumnsMinSpacing = dashboardStyle.ColumnsMinSpacing * scale;
	dashboardStyle.ScrollbarSize = dashboardStyle.ScrollbarSize * scale;
	dashboardStyle.ScrollbarRounding = dashboardStyle.ScrollbarRounding * scale;
	dashboardStyle.GrabMinSize = dashboardStyle.GrabMinSize * scale;
	dashboardStyle.GrabRounding = dashboardStyle.GrabRounding * scale;
	dashboardStyle.LogSliderDeadzone = dashboardStyle.LogSliderDeadzone * scale;
	dashboardStyle.TabRounding = dashboardStyle.TabRounding * scale;
	dashboardStyle.TabMinWidthForCloseButton = dashboardStyle.TabMinWidthForCloseButton * scale;
	dashboardStyle.DisplayWindowPadding = dashboardStyle.DisplayWindowPadding * scale;
	dashboardStyle.DisplaySafeAreaPadding = dashboardStyle.DisplaySafeAreaPadding * scale;
	dashboardStyle.MouseCursorScale = dashboardStyle.MouseCursorScale * scale;
	ImGui::SetWindowFontScale(scale);
	
	glm::vec2 minCanvas = screenToCanvas(ImGui::GetItemRectMin());
	glm::vec2 maxCanvas = screenToCanvas(ImGui::GetItemRectMax());
	float tickSpacing = 50.0;
	float startX = minCanvas.x - fmod(minCanvas.x, tickSpacing);
	float startY = minCanvas.y - fmod(minCanvas.y, tickSpacing);
	for(float x = startX; x < maxCanvas.x; x += tickSpacing){
		glm::vec2 lineMin = canvasToScreen(glm::vec2(x, minCanvas.y));
		glm::vec2 lineMax = canvasToScreen(glm::vec2(x, maxCanvas.y));
		drawing->AddLine(lineMin, lineMax, ImColor(1.0f, 1.0f, 1.0f, .3f));
	}
	for(float y = startY; y < maxCanvas.y; y += tickSpacing){
		glm::vec2 lineMin = canvasToScreen(glm::vec2(minCanvas.x, y));
		glm::vec2 lineMax = canvasToScreen(glm::vec2(maxCanvas.x, y));
		drawing->AddLine(lineMin, lineMax, ImColor(1.0f, 1.0f, 1.0f, .3f));
	}
	
	widget(w1);
	widget(w2);
	widget(w3);
	
	//reset default style
	ImGui::GetStyle() = defaultStyleCopy;
	ImGui::SetWindowFontScale(1.0);
	
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	
	ImDrawList* foregroundDrawing = ImGui::GetForegroundDrawList();
	ImGui::PushClipRect(windowPos, windowMax, true);
	glm::vec2 mouse = ImGui::GetMousePos();
	glm::vec2 canvas = screenToCanvas(mouse);
	static char coordString[128];
	sprintf(coordString, "screen: x:%.2f y:%.2f", mouse.x, mouse.y);
	foregroundDrawing->AddText(mouse + glm::vec2(0, ImGui::GetTextLineHeight()), ImColor(1.0f, 1.0f, 1.0f, 1.f), coordString);
	sprintf(coordString, "canvas: x:%.2f y:%.2f", canvas.x, canvas.y);
	foregroundDrawing->AddText(mouse + glm::vec2(0, ImGui::GetTextLineHeight() * 2.0), ImColor(1.0f, 1.0f, 1.0f, 1.f), coordString);
	ImGui::PopClipRect();
	
	ImGui::EndChild();
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
		 
		/*
		ImGui::Begin("ScaleTest");
		scaleTest();
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
