#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "DraggableListNew.h"

#include <GLFW/glfw3.h>

namespace DraggableListNew{

	//immediate state
	int currentItemIndex = -1;
	int draggedItemIndex = -1;

	ImVec2 draggedItemAbsolutePosition;
	ImVec2 draggedItemCenterAbsolutePosition;

	struct ListItem{
		ImVec2 absolutePosition;
		ImVec2 size;
		int index;
		bool b_isAboveDraggedItem;
	};
	std::vector<ListItem> items;

	//retained state
	int pressedItemIndex = -1;
	ImVec2 pressedItemPressRelativePosition;
	ImVec2 draggedItemSize;

	//settings
	float scrollSensitivity = 0.1;

	//output data
	int dragSourceIndex = -1;
	int dragDestinationIndex = -1;



	bool begin(const char* ID, ImVec2 size_arg){
				
		//get auto size if none specified
		if(size_arg.x <= 0.0 || size_arg.y <= 0.0) size_arg = ImGui::GetContentRegionAvail();
				 
		//begin list child window, early out if possible
		if(!ImGui::BeginChild(ID, size_arg, false)){
			end(false);
			return false;
		}
		
		//reset list state
		items.clear();
		currentItemIndex = -1;
		dragSourceIndex = -1;
		dragDestinationIndex = -1;
		
		//if item is dragged
		if(pressedItemIndex >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
			
			//store index of dragged item in immediate state
			draggedItemIndex = pressedItemIndex;
			
			//set and limit position of dragged item, get its center position for drop target finding
			draggedItemAbsolutePosition.x = ImGui::GetWindowPos().x + ImGui::GetCursorPosX();
			draggedItemAbsolutePosition.y = ImGui::GetMousePos().y - pressedItemPressRelativePosition.y;
			
			draggedItemCenterAbsolutePosition.x = draggedItemAbsolutePosition.x + draggedItemSize.x / 2.0;
			draggedItemCenterAbsolutePosition.y = draggedItemAbsolutePosition.y + draggedItemSize.y / 2.0;
			
			draggedItemAbsolutePosition.y = std::min(draggedItemAbsolutePosition.y, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - draggedItemSize.y);
			draggedItemAbsolutePosition.y = std::max(draggedItemAbsolutePosition.y, ImGui::GetWindowPos().y - ImGui::GetStyle().ItemSpacing.y);
			
			draggedItemCenterAbsolutePosition.y = std::min(draggedItemCenterAbsolutePosition.y, ImGui::GetWindowPos().y + size_arg.y);
			draggedItemCenterAbsolutePosition.y = std::max(draggedItemCenterAbsolutePosition.y, ImGui::GetWindowPos().y);
						
			//allow cancelling of drag action by escape key
			if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)){
				pressedItemIndex = -1;
				draggedItemIndex = -1;
			}
		}
		
		return true;
	}


	void end(bool b_wasOpen){
		
		//early out of list
		if(!b_wasOpen){
			ImGui::EndChild();
			return;
		}
						
		//if an item is dragged
		if(draggedItemIndex >= 0){
			
			//get drop index
			for(ListItem& item : items){
				if(item.b_isAboveDraggedItem) {
					dragDestinationIndex = item.index;
					break;
				}
			}
			if(dragDestinationIndex == -1) dragDestinationIndex = items.back().index + 1;
			if(dragDestinationIndex > draggedItemIndex) dragDestinationIndex--;
			
			//draw drop target
			if(dragDestinationIndex != draggedItemIndex){
				float dropGraphicPositionY;
				if(dragDestinationIndex >= items.size()) dropGraphicPositionY = items.back().absolutePosition.y + items.back().size.y;
				else dropGraphicPositionY = items[dragDestinationIndex].absolutePosition.y - ImGui::GetStyle().ItemSpacing.y;
				ImVec2 min(ImGui::GetWindowPos().x, dropGraphicPositionY);
				ImVec2 max(min.x + ImGui::GetContentRegionAvail().x, dropGraphicPositionY + ImGui::GetStyle().ItemSpacing.y);
				float drawMin = ImGui::GetWindowPos().y - ImGui::GetStyle().ItemSpacing.y;
				float drawMax = drawMin + ImGui::GetWindowHeight() + ImGui::GetStyle().ItemSpacing.y;
				min.y = std::max(min.y, drawMin);
				min.y = std::min(min.y, drawMax);
				max.y = std::max(max.y, drawMin);
				max.y = std::min(max.y, drawMax);
				ImGui::GetForegroundDrawList()->AddRectFilled(min, max, ImColor(1.0f, 1.0f, 1.0f, .5f));
			}
			
			//scroll the list while dragging
			if(draggedItemAbsolutePosition.y <= ImGui::GetWindowPos().y){
				float dragDistanceFromBounds = ImGui::GetMousePos().y - pressedItemPressRelativePosition.y - ImGui::GetWindowPos().y;
				ImGui::SetScrollY(ImGui::GetScrollY() + dragDistanceFromBounds * scrollSensitivity);
			}else if(draggedItemAbsolutePosition.y >= ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - draggedItemSize.y){
				float dragDistanceFromBounds = ImGui::GetMousePos().y - pressedItemPressRelativePosition.y + draggedItemSize.y - ImGui::GetWindowPos().y - ImGui::GetWindowHeight();
				ImGui::SetScrollY(1.0 + ImGui::GetScrollY() + dragDistanceFromBounds * scrollSensitivity);
			}
		}
		
		//clear drag & press state
		if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
			//if we were dragging, set the drag source index for user query
			if(draggedItemIndex >= 0) dragSourceIndex = draggedItemIndex;
			draggedItemIndex = -1;
			pressedItemIndex = -1;
		}
		
		ImGui::EndChild();
		
	}





	//immediate state for current item
	ImVec2 currentItemSize;
	ImVec2 currentItemCursorPos;

	bool beginItem(const char* ID, float height){
		assert(height > 0.0);
		
		//store current item info in immediate state
		currentItemIndex++;
		currentItemCursorPos = ImGui::GetCursorPos();
		currentItemSize = ImVec2(ImGui::GetContentRegionAvail().x, height);
		
		//push the items unique index as an extra imgui ID for safety
		ImGui::PushID(currentItemIndex);
		
		//if the item is dragged
		if(currentItemIndex == draggedItemIndex){
			
			//start a floating window following the mouse
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
			ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(.0f, .0f, .0f, .0f));
			ImGui::SetNextWindowPos(draggedItemAbsolutePosition, ImGuiCond_Always);
			ImGui::SetNextWindowSize(currentItemSize);
			static ImGuiWindowFlags tooltipFlags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar;
			ImGui::Begin(ID, nullptr, tooltipFlags);
			ImGui::InvisibleButton(ID, currentItemSize);
			ImGui::SetCursorPos(ImVec2(0,0));
			return true;
			
		}
		
		//if the item is not dragged
		else {
			
			//main gui interaction catcher
			ImGui::InvisibleButton(ID, currentItemSize);
			
			//if an item is being dragged
			if(draggedItemIndex >= 0){
				//store all items in a list for later computing of the drop target
				items.push_back(ListItem{
					.absolutePosition = ImGui::GetItemRectMin(),
					.size = currentItemSize,
					.index = currentItemIndex
				});
				//remember the current item position relative to the dragged item position
				ImVec2 currentItemCenterAbsolutePosition(ImGui::GetItemRectMin().x + currentItemSize.x / 2.0, ImGui::GetItemRectMin().y + currentItemSize.y / 2.0);
				items.back().b_isAboveDraggedItem = draggedItemCenterAbsolutePosition.y < currentItemCenterAbsolutePosition.y;
			}
			
			//if the item is fullt clipped, early out of drawing its content
			if(ImGui::IsClippedEx(ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), ImGui::GetItemID())){
				endItem(false);
				return false;
			}
			
			//if the item is pressed, store its information in the retained state
			if(ImGui::IsItemClicked()){
				pressedItemIndex = currentItemIndex;
				ImVec2 mouse = ImGui::GetMousePos();
				ImVec2 item = ImGui::GetItemRectMin();
				pressedItemPressRelativePosition.x = mouse.x - item.x;
				pressedItemPressRelativePosition.y = mouse.y - item.y;
				draggedItemSize = currentItemSize;
			}
			
			//prepare for user submitted widgets and draw calls
			ImGui::PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true);
			ImGui::SetCursorPos(currentItemCursorPos);
			
			return true;
		}
		
	}


	void endItem(bool b_wasOpen){
		
		//if we did not early out of the item
		if(b_wasOpen){
			if(currentItemIndex == draggedItemIndex){
				//if the item was dragged, terminate the floating window
				ImGui::End();
				ImGui::PopStyleVar(2);
				ImGui::PopStyleColor();
			}else{
				//else just terminate the item clipping rectangle
				ImGui::PopClipRect();
			}
		}
		
		//always pop the index ID
		ImGui::PopID();
			
		//set cursor position to draw next item in list
		ImGui::SetCursorPosX(0.0);
		ImGui::SetCursorPosY(currentItemCursorPos.y + currentItemSize.y + ImGui::GetStyle().ItemSpacing.y);
	}



	//query reordering action
	bool wasReordered(int& fromIndex, int& toIndex){
		if(dragSourceIndex == -1 || dragDestinationIndex == -1 || dragSourceIndex == dragDestinationIndex) return false;
		fromIndex = dragSourceIndex;
		toIndex = dragDestinationIndex;
		return true;
	}

};





















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
	
	if(DraggableListNew::begin("ListTest")){
		
		for(auto& item : items){
			if(DraggableListNew::beginItem(item.name.c_str(), /*item.height*/ 40.0)){
				
				ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(),
														  ImGui::GetItemRectMax(),
														  ImColor(item.color),
														  5.0,
														  ImDrawFlags_RoundCornersAll);
				
				ImGui::Text("%s", item.name.c_str());
				DraggableListNew::endItem();
			}
		}
		
		DraggableListNew::end();
	}
	
	int fromIndex, toIndex;
	if(DraggableListNew::wasReordered(fromIndex, toIndex)){
		Logger::warn("moved item {} to {}", fromIndex, toIndex);
		
		TestItem temp = items[fromIndex];
		items.erase(items.begin() + fromIndex);
		items.insert(items.begin() + toIndex, temp);
		
	}
	
	
	
	
	
}
