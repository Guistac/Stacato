#include <pch.h>

#include "EnvironnementGui.h"
#include "Dashboard.h"
#include "Gui/ApplicationWindow/ApplicationWindow.h"

#include "Gui/Assets/Colors.h"

#include <imgui.h>
#include <imgui_internal.h>

ImVec2 operator*(const ImVec2& other, const float& multiplier){ return ImVec2(other.x * multiplier, other.y * multiplier); }

class TestWidget : public Dashboard::Widget{
public:
	
	TestWidget(glm::vec2 pos, glm::vec2 size_) : Widget(){
		position = pos;
		size = size_;
	}
	
	virtual void gui() override {
		ImGui::Text("Widget");
		if(ImGui::Button("Test")) Logger::warn("Testing...");
		ImGui::Text("Position: x: %.1f y:%.1f", position.x, position.y);
	}
	
};

namespace Dashboard{

	std::vector<std::shared_ptr<Widget>> widgets = {
		std::make_shared<TestWidget>(glm::vec2(0,0),glm::vec2(200, 300)),
		std::make_shared<TestWidget>(glm::vec2(250,100),glm::vec2(200, 300)),
		std::make_shared<TestWidget>(glm::vec2(450,300),glm::vec2(200, 300)),
		std::make_shared<TestWidget>(glm::vec2(-100,-200),glm::vec2(200, 300))
	};
	std::vector<std::shared_ptr<Widget>>& getWidgets(){ return widgets; }

	void addWidget(std::shared_ptr<Widget> widget){ widgets.push_back(widget); }
	void removeWidget(std::shared_ptr<Widget> widget){
		for(int i = 0; i < widgets.size(); i++){
			if(widgets[i] == widget){
				widgets.erase(widgets.begin() + i);
				break;
			}
		}
	}
	void moveWidgetToTop(std::shared_ptr<Widget> widget){
		removeWidget(widget);
		addWidget(widget);
	}
	void moveWidget(glm::vec2 newPosition){}
	void resizeWidget(glm::vec2 newSize);

	float scale = 1.0;
	float minScale = 0.1;
	float maxScale = 10.0;
	glm::vec2 offset(0.0, 0.0);
	glm::vec2 dashboardPosition;
	glm::vec2 dashboardSize;
	glm::vec2 dashboardMax;
	std::shared_ptr<Widget> draggedWidget = nullptr;

	glm::vec2 screenToCanvas(glm::vec2 screen){ return ((screen - dashboardPosition) / scale) - offset; };
	glm::vec2 canvasToScreen(glm::vec2 canvas){ return ((canvas + offset) * scale) + dashboardPosition; };
	glm::vec2 canvasToCursor(glm::vec2 canvas){ return (canvas + offset) * scale; };

	void zoom(glm::vec2 screenZoomPosition, float delta){
		glm::vec2 canvasZoomPosition = screenToCanvas(screenZoomPosition);
		scale *= 1.0 + delta;
		scale = std::clamp(scale, minScale, maxScale);
		glm::vec2 newCanvasZoomPosition = screenToCanvas(screenZoomPosition);
		glm::vec2 offsetSlip = newCanvasZoomPosition - canvasZoomPosition;
		offset += offsetSlip;
	};
	void pan(glm::vec2 mouseDelta){ offset += mouseDelta / scale; };

	void draw(glm::vec2 size){
				
		//begin dashboard window and set main coordinates
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
		ImGui::BeginChild("dashboard", size, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleVar();
		dashboardPosition = ImGui::GetWindowPos();
		dashboardSize = ImGui::GetWindowSize();
		dashboardMax = dashboardPosition + dashboardSize;
		
		//main interaction catcher
		ImGui::SetCursorPos(ImVec2(0,0));
		ImGui::InvisibleButton("Dashboard", dashboardSize);
		ImGui::SetItemAllowOverlap();
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		
		//panning & zooming
		if(ImRect(dashboardPosition, dashboardMax).Contains(ImGui::GetMousePos())){
			double zoomDelta = ApplicationWindow::getMacOsTrackpadZoom();
			if(zoomDelta != 0.0) zoom(ImGui::GetMousePos(), zoomDelta);
			ImGuiIO& io = ImGui::GetIO();
			glm::vec2 scrollDelta = glm::vec2(io.MouseWheelH, io.MouseWheel);
			if(scrollDelta != glm::vec2(0.0f, 0.0f)) pan(scrollDelta * 10.0);
		}
		if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
			pan(ImGui::GetIO().MouseDelta);
		}
		
		//set style scaling
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
		
		//draw grid
		glm::vec2 minCanvas = screenToCanvas(ImGui::GetItemRectMin());
		glm::vec2 maxCanvas = screenToCanvas(ImGui::GetItemRectMax());
		float tickSpacing = 50.0;
		float startX = minCanvas.x - fmod(minCanvas.x, tickSpacing);
		float startY = minCanvas.y - fmod(minCanvas.y, tickSpacing);
		for(float x = startX; x < maxCanvas.x; x += tickSpacing){
			glm::vec2 lineMin = canvasToScreen(glm::vec2(x, minCanvas.y));
			glm::vec2 lineMax = canvasToScreen(glm::vec2(x, maxCanvas.y));
			drawing->AddLine(lineMin, lineMax, ImColor(1.0f, 1.0f, 1.0f, .1f));
		}
		for(float y = startY; y < maxCanvas.y; y += tickSpacing){
			glm::vec2 lineMin = canvasToScreen(glm::vec2(minCanvas.x, y));
			glm::vec2 lineMax = canvasToScreen(glm::vec2(maxCanvas.x, y));
			drawing->AddLine(lineMin, lineMax, ImColor(1.0f, 1.0f, 1.0f, .1f));
		}
		
		//draw widgets
		ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::almostBlack);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().FrameRounding);
		std::shared_ptr<Widget> clickedWidget = nullptr;
		glm::vec2 padding = ImGui::GetStyle().WindowPadding;
		for(int i = 0; i < widgets.size(); i++){
			ImGui::PushID(i);
			auto widget = widgets[i];
			glm::vec2 widgetSize = widget->size * scale;
			glm::vec2 cursor = canvasToCursor(widget->position);
			ImGui::SetCursorPos(cursor);
			if(ImGui::BeginChild("Widget", widgetSize, true, ImGuiWindowFlags_NoMove)){
				ImGui::SetCursorPos(padding);
				widget->gui();
				if(ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()){
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
					if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
						clickedWidget = widget;
						draggedWidget = widget;
					}
				}
			}
			ImGui::EndChild();
			ImGui::PopID();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		if(clickedWidget) moveWidgetToTop(clickedWidget);
		
		
		//widget interactino / dragging
		if(draggedWidget && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
			glm::vec2 delta = ImGui::GetIO().MouseDelta;
			draggedWidget->position += delta / scale;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
		}
		if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) draggedWidget = nullptr;
		
		//reset default style
		ImGui::GetStyle() = defaultStyleCopy;
		ImGui::SetWindowFontScale(1.0);
				
		//end dashboard
		ImGui::EndChild();
	}

	void fitView(){
		//get content coordinates
		glm::vec2 contentMin(FLT_MAX);
		glm::vec2 contentMax(FLT_MIN);
		for(auto& widget : getWidgets()){
			glm::vec2 widgetMin = widget->position;
			glm::vec2 widgetMax = widget->position + widget->size;
			contentMin.x = std::min(contentMin.x, widgetMin.x);
			contentMin.y = std::min(contentMin.y, widgetMin.y);
			contentMax.x = std::max(contentMax.x, widgetMax.x);
			contentMax.y = std::max(contentMax.y, widgetMax.y);
		}
		glm::vec2 contentSize(contentMax.x - contentMin.x, contentMax.y - contentMin.y);
		//adjust scale to fit content
		glm::vec2 scalingRatio = dashboardSize / contentSize;
		scale = std::min(scalingRatio.x, scalingRatio.y);
		
		//adjust offset to fit content
		glm::vec2 viewSize = dashboardSize / scale;
		if(dashboardSize.x / dashboardSize.y > contentSize.x / contentSize.y){
			//Y is limiting
			offset.y = -contentMin.y;
			offset.x = -contentMin.x + (viewSize.x - contentSize.x) / 2.0;
		}else{
			//X is limiting
			offset.x = -contentMin.x;
			offset.y = -contentMin.y + (viewSize.y - contentSize.y) / 2.0;
		}
	}

};






namespace Environnement::Gui{
	void dashboard(){
		if(ImGui::Button("Fit View")) Dashboard::fitView();
		Dashboard::draw(ImGui::GetContentRegionAvail());
	}
};

