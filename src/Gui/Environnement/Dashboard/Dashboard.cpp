#include <pch.h>

#include "Dashboard.h"
#include "Widget.h"
#include "Managers.h"

#include "Gui/Environnement/EnvironnementGui.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <GLFW/glfw3.h>
#include <tinyxml2.h>

ImVec2 operator*(const ImVec2& other, const float& multiplier){ return ImVec2(other.x * multiplier, other.y * multiplier); }




void Dashboard::addWidget(std::shared_ptr<Widget> widget, glm::vec2 position){
	auto instance = WidgetInstance::make(widget);
	widgets.push_back(instance);
	instance->position = position;
	for(int i = 0; i < availableWidgets.size(); i++){
		if(availableWidgets[i] == widget){
			availableWidgets.erase(availableWidgets.begin() + i);
			break;
		}
	}
}

void Dashboard::removeWidget(std::shared_ptr<WidgetInstance> widget){
	if(widget->widget) availableWidgets.push_back(widget->widget);
	for(int i = 0; i < widgets.size(); i++){
		if(widgets[i] == widget){
			widgets.erase(widgets.begin() + i);
			break;
		}
	}
	if(isWidgetSelected(widget)) deselectWidget();
}

void Dashboard::moveWidgetToTop(std::shared_ptr<WidgetInstance> widget){
	auto tmp = widget;
	for(int i = 0; i < widgets.size(); i++){
		if(widgets[i] == widget){
			widgets.erase(widgets.begin() + i);
			widgets.push_back(tmp);
			return;
		}
	}
}

void Dashboard::moveWidget(std::shared_ptr<WidgetInstance> widget, glm::vec2 newPosition){}

void Dashboard::resizeWidget(std::shared_ptr<WidgetInstance> widget, glm::vec2 newSize){}






void Dashboard::addAvailableWidget(std::shared_ptr<Widget> widget){
	availableWidgets.push_back(widget);
	for(auto& widgetInstance : widgets){
		if(widgetInstance->widget == nullptr && widgetInstance->uniqueID == widget->uniqueID){
			widgetInstance->widget = widget;
			for(int i = 0; i < availableWidgets.size(); i++){
				if(availableWidgets[i] == widget){
					availableWidgets.erase(availableWidgets.begin() + i);
					break;
				}
			}
		}
	}
}

void Dashboard::removeAvailableWidget(std::shared_ptr<Widget> widget){
	for(int i = 0; i < availableWidgets.size(); i++){
		if(availableWidgets[i] == widget){
			availableWidgets.erase(availableWidgets.begin() + i);
			break;
		}
	}
	for(int i = 0; i < widgets.size(); i++){
		if(widgets[i]->uniqueID == widget->uniqueID){
			widgets.erase(widgets.begin() + i);
		}
	}
}

void Dashboard::zoom(glm::vec2 screenZoomPosition, float delta){
	glm::vec2 canvasZoomPosition = screenToCanvas(screenZoomPosition);
	scale *= 1.0 + delta;
	scale = std::clamp(scale, minScale, maxScale);
	glm::vec2 newCanvasZoomPosition = screenToCanvas(screenZoomPosition);
	glm::vec2 offsetSlip = newCanvasZoomPosition - canvasZoomPosition;
	offset += offsetSlip;
};

void Dashboard::pan(glm::vec2 mouseDelta){ offset += mouseDelta / scale; };

void Dashboard::canvas(){
	
	//get dashboard size and auto fit view if necessary
	glm::vec2 newSize = ImGui::GetContentRegionAvail();
	bool b_dashboardSizeChanged = dashboardSize != newSize;
	dashboardSize = newSize;
    if(b_dashboardSizeChanged || b_autofitBusy) {
        if(b_dashboardSizeChanged) {
            b_autofitBusy = true;
        }else if(b_autofitBusy){
            b_autofitBusy = false;
        }
        fitView();
    }
	
	//begin dashboard window and set main coordinates
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
	ImGui::BeginChild("dashboard", dashboardSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PopStyleVar();
	dashboardPosition = ImGui::GetWindowPos();
	dashboardMax = dashboardPosition + dashboardSize;
	
	//main interaction catcher
	ImGui::SetCursorPos(ImVec2(0,0));
	ImGui::InvisibleButton("Dashboard", dashboardSize);
	ImGui::SetItemAllowOverlap();
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	//panning & zooming
	if(!b_autoFit){
		if(ImGui::IsWindowHovered()){
			double zoomDelta = 0.0;// ApplicationWindow::getMacOsTrackpadZoom();
			if(zoomDelta != 0.0) zoom(ImGui::GetMousePos(), zoomDelta);
			ImGuiIO& io = ImGui::GetIO();
			glm::vec2 scrollDelta = glm::vec2(io.MouseWheelH, io.MouseWheel);
			if(scrollDelta != glm::vec2(0.0f, 0.0f)) pan(scrollDelta * 10.0);
		}
		if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
			pan(ImGui::GetIO().MouseDelta);
		}
	}
		
	//style scaling
	//we cannot use ImGui::GetStyle().ScaleAllSizes(float scale_factor)
	//because it rounds values to integers
	//instead we buffer the main style structure and generate a new one
	ImGuiStyle mainStyleCopy = ImGui::GetStyle();
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
	if(b_drawGrid){
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
	}
	
	std::shared_ptr<WidgetInstance> hoveredWidget = nullptr;
	ImGuiID previousHoveredID = ImGui::GetCurrentContext()->HoveredIdPreviousFrame;
	
	glm::vec2 widgetPadding(ImGui::GetTextLineHeight() * 0.25);
	
	for(int i = 0; i < widgets.size(); i++){
		ImGui::PushID(i);
		auto widgetInstance = widgets[i];
				
		ImDrawListSplitter widgetDrawingLayers;
		widgetDrawingLayers.Split(drawing, 2);
		widgetDrawingLayers.SetCurrentChannel(drawing, 1);

		glm::vec2 cursor = canvasToCursor(widgetInstance->position);
		ImGui::SetCursorPos(cursor);
		ImGui::BeginGroup();
		ImGui::SetCursorPos(cursor + widgetPadding);
		ImGui::BeginGroup();
		ImGuiID hoveredIdBeforeWidget = ImGui::GetHoveredID();
		widgetInstance->gui();
		ImGuiID hoveredIdInsideWidget = ImGui::GetHoveredID();
		ImGui::EndGroup();
		glm::vec2 contentSize = ImGui::GetItemRectSize();
		glm::vec2 widgetSize = contentSize + widgetPadding * 2.0;
		ImGui::SetCursorPos(cursor + widgetSize);
		ImGui::EndGroup();
		
		ImGui::SetCursorPos(cursor);
		ImGui::InvisibleButton("Interaction", widgetSize);
		ImGui::SetItemAllowOverlap();
		bool b_hovered = ImGui::GetItemID() == previousHoveredID;
		if(b_hovered) hoveredWidget = widgetInstance;
			
		widgetDrawingLayers.SetCurrentChannel(drawing, 0);
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		
		widgetInstance->size = widgetSize / scale;
		
		drawing->AddRectFilled(min, max, ImColor(Colors::almostBlack), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		float borderThickness;
		ImColor borderColor;
		if(isWidgetSelected(widgetInstance)){
			borderThickness = ImGui::GetTextLineHeight() * 0.2;
			borderColor = ImColor(Colors::white);
		}else {
			borderThickness = ImGui::GetTextLineHeight() * 0.05;
			borderColor = ImColor(Colors::darkGray);
		}
		drawing->AddRect(min, max, borderColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll, borderThickness);
		widgetDrawingLayers.Merge(drawing);
		
		ImGui::PopID();
	}
	
	//widget interaction / dragging
	if(!b_lockEdit && hoveredWidget){
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			draggedWidget = hoveredWidget;
			selectWidget(hoveredWidget);
			moveWidgetToTop(hoveredWidget);
		}
	}else if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) fitView();
	else if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) deselectWidget();

	if(draggedWidget && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
		glm::vec2 delta = ImGui::GetIO().MouseDelta;
		draggedWidget->position += delta / scale;
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
	}
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if(draggedWidget){
            draggedWidget->position.x = std::round(draggedWidget->position.x);
            draggedWidget->position.y = std::round(draggedWidget->position.y);
        }
        draggedWidget = nullptr;
    }
	if(selectedWidget){
		if(ImGui::IsKeyPressed(GLFW_KEY_BACKSPACE) || ImGui::IsKeyPressed(GLFW_KEY_DELETE)){
			removeWidget(selectedWidget);
		}
	}
	
	//reset default style
	ImGui::GetStyle() = mainStyleCopy;
	ImGui::SetWindowFontScale(1.0);
	
	//show dashboard controls
	glm::vec2 max = dashboardPosition + glm::vec2(dashboardSize.x, ImGui::GetFrameHeight() + 2.0 * ImGui::GetStyle().WindowPadding.y);
	if(!b_lockEdit || (ImRect(dashboardPosition, max).Contains(ImGui::GetMousePos()) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))){
		glm::vec2 padding = ImGui::GetStyle().WindowPadding;
		ImGui::SetCursorPos(padding);
		ImGui::BeginGroup();
		if(ImGui::Button("Fit View")) fitView();
		ImGui::SameLine();
		if(ImGui::Checkbox("Auto Fit", &b_autoFit)) fitView();
		ImGui::SameLine();
		ImGui::Checkbox("Lock", &b_lockEdit);
		ImGui::SameLine();
		ImGui::Checkbox("Show Grid", &b_drawGrid);
		ImGui::EndGroup();
	}
	
	//end dashboard
	ImGui::EndChild();
}

void Dashboard::fitView(){
	auto& widgets = getWidgets();
	if(widgets.empty()) return;
	//get content coordinates
	glm::vec2 contentMin(FLT_MAX);
	glm::vec2 contentMax(-FLT_MAX);
	for(auto& widget : widgets){
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
	glm::vec2 zoomPosition = (dashboardPosition + dashboardMax) / 2.0;
	zoom(zoomPosition, -0.01);
}



void Dashboard::gui(){

	static float adderWidth = ImGui::GetTextLineHeight() * 15.0;
	static float minAdderWidth = ImGui::GetTextLineHeight() * 10.0;
	static float maxAdderWidth = ImGui::GetTextLineHeight() * 25.0;
	
	if(!b_lockEdit){
		
		if(ImGui::BeginChild("EditorSideBar", ImVec2(adderWidth, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_AlwaysUseWindowPadding)){
			float availableWidth = ImGui::GetContentRegionAvail().x;

			if(getSelectedWidget() && getSelectedWidget()->hasWidget()){
				
				ImGui::PushFont(Fonts::sansBold15);
				backgroundText(getSelectedWidget()->widget->getName().c_str(), ImVec2(availableWidth, ImGui::GetFrameHeight()), Colors::darkGray);
				ImGui::PopFont();
				/*
				ImGui::Text("Position :");
				ImGui::SetNextItemWidth(availableWidth);
				getSelectedWidget()->positionParameter->gui();
				*/
				ImGui::Separator();
			}
			
			ImGui::PushFont(Fonts::sansBold15);
			backgroundText("Available Widgets", ImVec2(availableWidth, ImGui::GetFrameHeight()), Colors::darkGray);
			ImGui::PopFont();
			
			if(availableWidgets.empty()) {
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::PushFont(Fonts::sansLight15);
				ImGui::Text("No Widgets Available.");
				ImGui::PopFont();
				ImGui::PopStyleColor();
			}
			
			for(auto& widget : availableWidgets){
				ImGui::Selectable(widget->getName().c_str());
				if(ImGui::BeginDragDropSource()){
					ImGui::SetDragDropPayload("Widget", &widget->uniqueID, sizeof(int));
					ImGui::Text("%s", widget->getName().c_str());
					ImGui::EndDragDropSource();
				}
			}
			
			
		}
		ImGui::EndChild();
		
		adderWidth += verticalSeparator(ImGui::GetTextLineHeight() * 0.5, false);
		adderWidth = std::clamp(adderWidth, minAdderWidth, maxAdderWidth);
		ImGui::SameLine();
	}
		 
	glm::vec2 canvasPosition = ImGui::GetCursorPos();
	canvas();
	
	if(!b_lockEdit){
		ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(.0f, .0f, .0f, .0f));
		if(ImGui::BeginDragDropTarget()){
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(1.f, 1.f, 1.f, .1f));
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Widget");
			if (payload != nullptr && payload->DataSize == sizeof(int)) {
				int widgetID = *(int*)payload->Data;
				auto widget = WidgetManager::getWidgetByUniqueID(widgetID);
				glm::vec2 canvasMousePosition = screenToCanvas(ImGui::GetMousePos());
				if(widget) addWidget(widget, canvasMousePosition);
				Logger::warn("added widget {}", widgetID);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor();
	}
		
}








bool Dashboard::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	xml->SetAttribute("Scale", scale);
	xml->SetAttribute("OffsetX", offset.x);
	xml->SetAttribute("OffsetY", offset.y);
	
	xml->SetAttribute("AutoFit", b_autoFit);
	xml->SetAttribute("DrawGrid", b_drawGrid);
	xml->SetAttribute("LockEdit", b_lockEdit);
	
	for(auto& widget : widgets){
		XMLElement* widgetXML = xml->InsertNewChildElement("Widget");
		widget->save(widgetXML);
	}
	return true;
}

std::shared_ptr<Dashboard> Dashboard::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	auto dashboard = std::make_shared<Dashboard>();
	if(xml->QueryFloatAttribute("Scale", &dashboard->scale) != XML_SUCCESS) {
		Logger::warn("Could not find Dashboard Scale Attribute");
		return nullptr;
	}
	if(xml->QueryFloatAttribute("OffsetX", &dashboard->offset.x) != XML_SUCCESS) {
		Logger::warn("Could not find Dashboard Scale Attribute");
		return nullptr;
	}
	if(xml->QueryFloatAttribute("OffsetY", &dashboard->offset.y) != XML_SUCCESS) {
		Logger::warn("Could not find Dashboard Scale Attribute");
		return nullptr;
	}
	if(xml->QueryBoolAttribute("AutoFit", &dashboard->b_autoFit) != XML_SUCCESS) {
		Logger::warn("Could not find Dashboard Auto Fit Attribute");
		return nullptr;
	}
	if(xml->QueryBoolAttribute("DrawGrid", &dashboard->b_drawGrid) != XML_SUCCESS) {
		Logger::warn("Could not find Dashboard Draw Grid Attribute");
		return nullptr;
	}
	if(xml->QueryBoolAttribute("LockEdit", &dashboard->b_lockEdit) != XML_SUCCESS) {
		Logger::warn("Could not find Dashboard Lock Widgets Attribute");
		return nullptr;
	}
	
	XMLElement* widgetInstanceXML = xml->FirstChildElement("Widget");
	while(widgetInstanceXML){
		auto widget = WidgetInstance::load(widgetInstanceXML);
		if(widget == nullptr) {
			Logger::warn("Error Loading Widget");
			return nullptr;
		}
		dashboard->widgets.push_back(widget);
		widgetInstanceXML = widgetInstanceXML->NextSiblingElement("Widget");
	}
	
	return dashboard;
}


























namespace Environnement::Gui{
void dashboards(){
	DashboardManager::getDashboard()->gui();
}
};
