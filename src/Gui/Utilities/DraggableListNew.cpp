#include <pch.h>

#include <imgui.h>

#include "DraggableListNew.h"

#include <GLFW/glfw3.h>

namespace DraggableListNew{

	int currentItemIndex = -1;
	int pressedItemIndex = -1;
	int draggedItemIndex = -1;
	
	int dragSourceIndex = -1;
	int dragDestinationIndex = -1;

	ImVec2 pressedItemPressRelativePosition(0,0);
	ImVec2 draggedItemSize(0,0);
	ImVec2 draggedItemCenterAbsolutePosition(0,0);

	struct ListItem{
		ImVec2 absolutePosition;
		ImVec2 size;
		int index;
		bool b_isAboveDraggedItem;
	};
	std::vector<ListItem> items;

	bool begin(const char* ID, ImVec2 size_arg){
				
		if(size_arg.x <= 0.0 || size_arg.y <= 0.0) size_arg = ImGui::GetContentRegionAvail();
				 
		bool b_open = ImGui::BeginChild(ID, size_arg, true);
		
		items.clear();
		currentItemIndex = -1;
		dragSourceIndex = -1;
		dragDestinationIndex = -1;
		
		if(pressedItemIndex >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
			draggedItemIndex = pressedItemIndex;
			ImVec2 mouse = ImGui::GetMousePos();
			
			ImVec2 draggedItemAbsolutePosition(ImGui::GetWindowPos().x + ImGui::GetCursorPosX(), mouse.y - pressedItemPressRelativePosition.y);
			
			draggedItemCenterAbsolutePosition.x = draggedItemAbsolutePosition.x + draggedItemSize.x / 2.0;
			draggedItemCenterAbsolutePosition.y = draggedItemAbsolutePosition.y + draggedItemSize.y / 2.0;
			
			if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)){
				pressedItemIndex = -1;
				draggedItemIndex = -1;
			}
		}
		
		
		if(!b_open) end(false);
		return b_open;
	}

	void end(bool b_wasOpen){
						
		if(draggedItemIndex >= 0){
			
			for(ListItem& item : items){
				if(item.b_isAboveDraggedItem) {
					dragDestinationIndex = item.index;
					break;
				}
			}
			if(dragDestinationIndex == -1) dragDestinationIndex = items.back().index + 1;
			if(dragDestinationIndex > draggedItemIndex) dragDestinationIndex--;
			
			if(dragDestinationIndex != draggedItemIndex){
				float dropGraphicPositionY;
				if(dragDestinationIndex >= items.size()){
					dropGraphicPositionY = items.back().absolutePosition.y + items.back().size.y;
				}else{
					dropGraphicPositionY = items[dragDestinationIndex].absolutePosition.y - ImGui::GetStyle().ItemSpacing.y;
				}
				ImVec2 min(ImGui::GetWindowPos().x, dropGraphicPositionY);
				ImVec2 max(min.x + ImGui::GetContentRegionAvail().x + 2.0 * ImGui::GetStyle().WindowPadding.x, dropGraphicPositionY + ImGui::GetStyle().ItemSpacing.y);
				ImGui::GetForegroundDrawList()->AddRectFilled(min, max, ImColor(1.0f, 1.0f, 1.0f, .5f));
				
			}
		}
		
		
		if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
			if(draggedItemIndex >= 0){
				dragSourceIndex = draggedItemIndex;
				draggedItemIndex = -1;
			}
			pressedItemIndex = -1;
		}
		
		ImGui::EndChild();
		
	}

	ImVec2 currentItemSize;
	ImVec2 currentItemCursorPos;

	bool beginItem(const char* ID, float height){
		assert(height > 0.0);
		
		currentItemIndex++;
		currentItemCursorPos = ImGui::GetCursorPos();
		currentItemSize = ImVec2(ImGui::GetContentRegionAvail().x, height);
		
		if(currentItemIndex == draggedItemIndex){
			
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
			ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(.0f, .0f, .0f, .0f));
			
			ImVec2 tooltipPosition(ImGui::GetWindowPos().x + ImGui::GetCursorPosX(), ImGui::GetMousePos().y - pressedItemPressRelativePosition.y);
			ImGui::SetNextWindowPos(tooltipPosition, ImGuiCond_Always);
			ImGui::SetNextWindowSize(currentItemSize);
			
			ImGuiWindowFlags tooltipFlags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar;
			ImGui::Begin(ID, nullptr, tooltipFlags);
			
			ImGui::InvisibleButton(ID, currentItemSize);
			ImGui::SetCursorPos(ImVec2(0,0));
			
		}else {
			
			ImGui::InvisibleButton(ID, currentItemSize);
			
			if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()){
				pressedItemIndex = currentItemIndex;
				ImVec2 mouse = ImGui::GetMousePos();
				ImVec2 item = ImGui::GetItemRectMin();
				pressedItemPressRelativePosition.x = mouse.x - item.x;
				pressedItemPressRelativePosition.y = mouse.y - item.y;
				draggedItemSize = currentItemSize;
			}
			
			if(draggedItemIndex >= 0){
				items.push_back(ListItem{
					.absolutePosition = ImGui::GetItemRectMin(),
					.size = currentItemSize,
					.index = currentItemIndex
				});
				ImVec2 currentItemCenterAbsolutePosition(ImGui::GetItemRectMin().x + currentItemSize.x / 2.0, ImGui::GetItemRectMin().y + currentItemSize.y / 2.0);
				items.back().b_isAboveDraggedItem = draggedItemCenterAbsolutePosition.y < currentItemCenterAbsolutePosition.y;
			}
			
			ImGui::PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true);
			ImGui::SetCursorPos(currentItemCursorPos);
		}
		return true;
	}

	void endItem(bool b_wasOpen){
		
		if(currentItemIndex == draggedItemIndex){
			ImGui::End(); //end tooltip window
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor();
		}else{
			ImGui::PopClipRect();
		}
			
		ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);
		ImGui::SetCursorPosY(currentItemCursorPos.y + currentItemSize.y + ImGui::GetStyle().ItemSpacing.y);
	}




	bool wasReordered(int& fromIndex, int& toIndex){
		fromIndex = dragSourceIndex;
		toIndex = dragDestinationIndex;
		return dragSourceIndex != -1 && dragDestinationIndex != -1 && dragSourceIndex != dragDestinationIndex;
	}

};





















struct TestItem{
	std::string name;
	float height;
	ImVec4 color;
};

void dragListTest(){
	static std::vector<TestItem> items = {
		{.name = "Zero", 	.height = 30.0, .color = ImVec4(0.0f, 0.0f, 0.0f, 1.f)},
		{.name = "One", 	.height = 30.0, .color = ImVec4(0.0f, 0.0f, 0.7f, 1.f)},
		{.name = "Two", 	.height = 40.0, .color = ImVec4(0.0f, 0.7f, 0.0f, 1.f)},
		{.name = "Three", 	.height = 50.0, .color = ImVec4(0.0f, 0.7f, 0.7f, 1.f)},
		{.name = "Four", 	.height = 35.0, .color = ImVec4(0.7f, 0.0f, 0.0f, 1.f)},
		{.name = "Five", 	.height = 45.0, .color = ImVec4(0.7f, 0.0f, 0.7f, 1.f)},
		{.name = "Six", 	.height = 55.0, .color = ImVec4(0.7f, 0.7f, 0.0f, 1.f)},
		{.name = "Seven", 	.height = 20.0, .color = ImVec4(0.0f, 0.0f, 0.0f, .5f)},
		{.name = "Eight", 	.height = 60.0, .color = ImVec4(0.0f, 0.0f, 0.7f, .5f)},
		{.name = "Nine", 	.height = 99.0, .color = ImVec4(0.0f, 0.7f, 0.0f, .5f)},
		{.name = "Ten", 	.height = 35.0, .color = ImVec4(0.0f, 0.7f, 0.7f, .5f)}
		
		
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
