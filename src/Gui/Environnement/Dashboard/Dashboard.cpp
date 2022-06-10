#include <pch.h>

#include "Dashboard.h"
#include "Widgets.h"

#include "Gui/Environnement/EnvironnementGui.h"
#include "Gui/ApplicationWindow/ApplicationWindow.h"

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
	availableWidgets.push_back(widget->widget);
	for(int i = 0; i < widgets.size(); i++){
		if(widgets[i] == widget){
			widgets.erase(widgets.begin() + i);
			break;
		}
	}
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
}

void Dashboard::removeAvailableWidget(std::shared_ptr<Widget> widget){
	for(int i = 0; i < availableWidgets.size(); i++){
		if(availableWidgets[i] == widget){
			availableWidgets.erase(availableWidgets.begin() + i);
			break;
		}
	}
	for(int i = 0; i < widgets.size(); i++){
		if(widgets[i]->widget->uniqueID == widget->uniqueID){
			removeWidget(widgets[i]);
		}
	}
}


glm::vec2 Dashboard::screenToCanvas(glm::vec2 screen){ return ((screen - dashboardPosition) / scale) - offset; };
glm::vec2 Dashboard::canvasToScreen(glm::vec2 canvas){ return ((canvas + offset) * scale) + dashboardPosition; };
glm::vec2 Dashboard::canvasToCursor(glm::vec2 canvas){ return (canvas + offset) * scale; };

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
	
	glm::vec2 size = ImGui::GetContentRegionAvail();
	
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
	if(!b_lockView){
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
	//draw widgets
	ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::almostBlack);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().FrameRounding);
	
	std::shared_ptr<WidgetInstance> clickedWidget = nullptr;
	std::shared_ptr<WidgetInstance> deletedWidget = nullptr;
	bool widgetHovered = false;
	
	
	for(int i = 0; i < widgets.size(); i++){
		ImGui::PushID(i);
		auto widget = widgets[i];
		
		glm::vec2 widgetSize;
		if(widget->isResizeable()) widgetSize = widget->size;
		else widgetSize = widget->getDefaultSize();
		widgetSize *= scale;
		
		glm::vec2 cursor = canvasToCursor(widget->position);
		ImGui::SetCursorPos(cursor);
		
		if(ImGui::BeginChild("Widget", widgetSize, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)){
			widget->gui();
			if(ImGui::IsWindowHovered()){
				widgetHovered = true;
				if(!ImGui::IsAnyItemHovered()){
					
					if(!b_lockWidgets){
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
							clickedWidget = widget;
							draggedWidget = widget;
							selectWidget(widget);
						}
					}
					
					if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SUPER) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SUPER)){
						ImGui::BeginTooltip();
						ImGui::Text("UID: %i", widget->widget->uniqueID);
						ImGui::EndTooltip();
					}
				}
			}
		}
		ImGui::EndChild();
		
		if(isWidgetSelected(widget)){
			float borderWidth = ImGui::GetTextLineHeight() * 0.2;
			glm::vec2 widgetMin = dashboardPosition + cursor;
			glm::vec2 widgetMax = widgetMin + widgetSize;
			glm::vec2 min = widgetMin - glm::vec2(borderWidth / 2.0);
			glm::vec2 max = widgetMax + glm::vec2(borderWidth / 2.0);
			float rounding = ImGui::GetStyle().FrameRounding + borderWidth / 2.0;
			ImGui::GetWindowDrawList()->AddRect(min, max, ImColor(Colors::white), rounding, ImDrawFlags_RoundCornersAll, borderWidth);
			if(ImGui::IsKeyPressed(GLFW_KEY_BACKSPACE) || ImGui::IsKeyPressed(GLFW_KEY_DELETE)) deletedWidget = widget;
		}
		
		ImGui::PopID();
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	
	if(clickedWidget) moveWidgetToTop(clickedWidget);
	if(deletedWidget) removeWidget(deletedWidget);
	if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !widgetHovered) deselectWidget();
	
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

void Dashboard::widgetAdder(){
	ImGui::Text("Available Widgets");
	ImGui::Separator();
	for(auto& widget : availableWidgets){
		ImGui::Selectable(widget->name.c_str());
		if(ImGui::BeginDragDropSource()){
			ImGui::SetDragDropPayload("Widget", &widget->uniqueID, sizeof(int));
			ImGui::Text("%s", widget->name.c_str());
			ImGui::EndDragDropSource();
		}
	}
}

void Dashboard::gui(){
	
	static float adderWidth = ImGui::GetTextLineHeight() * 10.0;
	static float minAdderWidth = ImGui::GetTextLineHeight() * 5.0;
	static float maxAdderWidth = ImGui::GetTextLineHeight() * 20.0;
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::darkGray);
	if(ImGui::BeginChild("WidgetDicionnary", ImVec2(adderWidth, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_AlwaysUseWindowPadding)){
		ImGui::Text("Available Widgets");
		ImGui::Separator();
		for(auto& widget : availableWidgets){
			ImGui::Selectable(widget->name.c_str());
			if(ImGui::BeginDragDropSource()){
				ImGui::SetDragDropPayload("Widget", &widget->uniqueID, sizeof(int));
				ImGui::Text("%s", widget->name.c_str());
				ImGui::EndDragDropSource();
			}
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
	
	adderWidth += verticalSeparator(ImGui::GetTextLineHeight() * 0.5);
	adderWidth = std::clamp(adderWidth, minAdderWidth, maxAdderWidth);
	
	if(ImGui::BeginChild("Dashboard")){
		
		if(ImGui::Button("Fit View")) fitView();
		ImGui::SameLine();
		ImGui::Checkbox("Show Grid", &b_drawGrid);
		ImGui::SameLine();
		ImGui::Checkbox("Lock View", &b_lockView);
		ImGui::SameLine();
		ImGui::Checkbox("Lock Widgets", &b_lockWidgets);
		
		canvas();
		
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
		
	}
	ImGui::EndChild();
}

void Dashboard::fitView(){
	if(getWidgets().empty()) return;
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

bool Dashboard::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	xml->SetAttribute("Scale", scale);
	xml->SetAttribute("OffsetX", offset.x);
	xml->SetAttribute("OffsetY", offset.y);
	for(auto& widget : widgets){
		XMLElement* widgetXML = xml->InsertNewChildElement("Widget");
		widgetXML->SetAttribute("UniqueID", widget->widget->uniqueID);
		widgetXML->SetAttribute("PositionX", widget->position.x);
		widgetXML->SetAttribute("PositionY", widget->position.y);
		widgetXML->SetAttribute("SizeX", widget->size.x);
		widgetXML->SetAttribute("SizeY", widget->size.y);
	}
}

bool Dashboard::load(tinyxml2::XMLElement* xml){
	
}






















namespace DashboardManager{

std::vector<std::shared_ptr<Dashboard>> dashboards = {
	std::make_shared<Dashboard>()
};
std::vector<std::shared_ptr<Dashboard>>& getDashboards(){ return dashboards; }

};





namespace Environnement::Gui{
void dashboards(){
	
	auto dashboard = DashboardManager::getDashboards().front();
	dashboard->gui();
	
}
};
