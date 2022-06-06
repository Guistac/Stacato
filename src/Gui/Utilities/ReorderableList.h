#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <GLFW/glfw3.h>

namespace ReorderableList{

	struct State{
		//immediate state
		int currentItemIndex = -1;
		int draggedItemIndex = -1;
		ImVec2 currentItemSize;
		ImVec2 currentItemCursorPos;
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
		ImVec2 pressedItemPressRelativePosition;
		ImVec2 draggedItemSize;

		//settings
		float scrollSensitivity = 0.1;

		//output data
		int originIndex;
		int destinationIndex;
	};

	inline State& getState(){
		static State state;
		return state;
	}


	inline void storePressedItemIndex(int index){
		ImGuiID id = ImGui::GetCurrentWindow()->ID + 1;
		ImGui::GetStateStorage()->SetInt(id, index);
	}

	inline int readPressedItemIndex(){
		ImGuiID id = ImGui::GetCurrentWindow()->ID + 1;
		return ImGui::GetStateStorage()->GetInt(id, -1);
	}

	inline void storeDraggedItemIndex(int index){
		ImGuiID id = ImGui::GetCurrentWindow()->ID + 2;
		ImGui::GetStateStorage()->SetInt(id, index);
	}

	inline int readDraggedItemIndex(){
		ImGuiID id = ImGui::GetCurrentWindow()->ID + 2;
		return ImGui::GetStateStorage()->GetInt(id, -1);
	}

	void end(bool b_wasOpen = true);

	inline bool begin(const char* ID, ImVec2 size_arg = ImVec2(0,0)){
		
		//get auto size if none specified
		if(size_arg.x <= 0.0 || size_arg.y <= 0.0) size_arg = ImGui::GetContentRegionAvail();
		
		//begin list child window, early out if possible
		if(!ImGui::BeginChild(ID, size_arg, false)){
			end(false);
			return false;
		}
		
		//reset list state
		State& state = getState();
		state.items.clear();
		state.currentItemIndex = -1;
		state.originIndex = -1;
		state.destinationIndex = -1;
		
		state.draggedItemIndex = readDraggedItemIndex();
		int pressedItemIndex = readPressedItemIndex();
		
		//if item is dragged
		if(pressedItemIndex >= 0){
			if(ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
				//store index of dragged item in immediate state
				storeDraggedItemIndex(pressedItemIndex);
				state.draggedItemIndex = pressedItemIndex;
				
				//set and limit position of dragged item, get its center position for drop target finding
				state.draggedItemAbsolutePosition.x = ImGui::GetWindowPos().x + ImGui::GetCursorPosX();
				state.draggedItemAbsolutePosition.y = ImGui::GetMousePos().y - state.pressedItemPressRelativePosition.y;
				
				state.draggedItemCenterAbsolutePosition.x = state.draggedItemAbsolutePosition.x + state.draggedItemSize.x / 2.0;
				state.draggedItemCenterAbsolutePosition.y = state.draggedItemAbsolutePosition.y + state.draggedItemSize.y / 2.0;
				
				state.draggedItemAbsolutePosition.y = std::min(state.draggedItemAbsolutePosition.y, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - state.draggedItemSize.y);
				state.draggedItemAbsolutePosition.y = std::max(state.draggedItemAbsolutePosition.y, ImGui::GetWindowPos().y - ImGui::GetStyle().ItemSpacing.y);
				
				state.draggedItemCenterAbsolutePosition.y = std::min(state.draggedItemCenterAbsolutePosition.y, ImGui::GetWindowPos().y + size_arg.y);
				state.draggedItemCenterAbsolutePosition.y = std::max(state.draggedItemCenterAbsolutePosition.y, ImGui::GetWindowPos().y);
							
				//allow cancelling of drag action by escape key
				if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)){
					state.draggedItemIndex = -1;
					storePressedItemIndex(-1);
					storeDraggedItemIndex(-1);
				}
			}
		}
		
		return true;
	}

	
	void endItem(bool b_wasOpen = true);

	inline bool beginItem(float height){
		assert(height > 0.0);
		
		State& state = getState();
		
		//store current item info in immediate state
		state.currentItemIndex++;
		state.currentItemCursorPos = ImGui::GetCursorPos();
		state.currentItemSize = ImVec2(ImGui::GetContentRegionAvail().x, height);
		
		//push the items unique index as an extra imgui ID for safety
		ImGui::PushID(state.currentItemIndex);
		
		//if the item is dragged
		if(state.currentItemIndex == state.draggedItemIndex){
			
			//draw black background where then item was
			ImGui::InvisibleButton("Placeholder", state.currentItemSize);
			ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(),
													  ImGui::GetItemRectMax(),
													  ImColor(.0f, .0f, .0f, .3f),
													  ImGui::GetStyle().FrameRounding,
													  ImDrawFlags_RoundCornersAll);
			
			//draw shadow around dragged item
			float shadowSize = ImGui::GetStyle().ItemSpacing.y;
			ImVec2 shadowMin(state.draggedItemAbsolutePosition.x - shadowSize / 2.0,
							 state.draggedItemAbsolutePosition.y - shadowSize / 2.0);
			ImVec2 shadowMax(state.draggedItemAbsolutePosition.x - 1.0 + state.currentItemSize.x + shadowSize / 2.0,
							 state.draggedItemAbsolutePosition.y - 1.0 + state.currentItemSize.y + shadowSize / 2.0);
			ImGui::GetForegroundDrawList()->AddRect(shadowMin, shadowMax, ImColor(.0f, .0f, .0f, .5f), shadowSize, ImDrawFlags_RoundCornersAll, shadowSize);
			
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			
			
			//start a floating window following the mouse
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
			ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(.0f, .0f, .0f, .0f));
			ImGui::SetNextWindowPos(state.draggedItemAbsolutePosition, ImGuiCond_Always);
			ImGui::SetNextWindowSize(state.currentItemSize);
			static ImGuiWindowFlags tooltipFlags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar;
			ImGui::Begin("DraggedItem", nullptr, tooltipFlags);
			drawList = ImGui::GetWindowDrawList();
			ImGui::InvisibleButton("DraggedItem", state.currentItemSize);
			ImGui::SetCursorPos(ImVec2(0,0));
			return true;
			
		}
		
		//if the item is not dragged
		else {
			
			//main gui interaction catcher
			ImGui::InvisibleButton("Item", state.currentItemSize);
			
			//if an item is being dragged
			if(state.draggedItemIndex >= 0){
				//store all items in a list for later computing of the drop target
				state.items.push_back(State::ListItem{
					.absolutePosition = ImGui::GetItemRectMin(),
					.size = state.currentItemSize,
					.index = state.currentItemIndex
				});
				//remember the current item position relative to the dragged item position
				ImVec2 currentItemCenterAbsolutePosition(ImGui::GetItemRectMin().x + state.currentItemSize.x / 2.0, ImGui::GetItemRectMin().y + state.currentItemSize.y / 2.0);
				state.items.back().b_isAboveDraggedItem = state.draggedItemCenterAbsolutePosition.y < currentItemCenterAbsolutePosition.y;
			}
			
			//if the item is fullt clipped, early out of drawing its content
			if(ImGui::IsClippedEx(ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), ImGui::GetItemID())){
				endItem(false);
				return false;
			}
			
			//if the item is pressed, store its information in the retained state
			if(ImGui::IsItemClicked()){
				storePressedItemIndex(state.currentItemIndex);
				ImVec2 mouse = ImGui::GetMousePos();
				ImVec2 item = ImGui::GetItemRectMin();
				state.pressedItemPressRelativePosition.x = mouse.x - item.x;
				state.pressedItemPressRelativePosition.y = mouse.y - item.y;
				state.draggedItemSize = state.currentItemSize;
			}
			
			//prepare for user submitted widgets and draw calls
			ImGui::PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true);
			ImGui::SetCursorPos(state.currentItemCursorPos);
			
			return true;
		}
		
	}


	inline void endItem(bool b_wasOpen){
		
		State& state = getState();
		
		//if we did not early out of the item
		if(b_wasOpen){
			if(state.currentItemIndex == state.draggedItemIndex){
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
		ImGui::SetCursorPosY(state.currentItemCursorPos.y + state.currentItemSize.y + ImGui::GetStyle().ItemSpacing.y);
	}


	inline void end(bool b_wasOpen){
		
		//early out of list
		if(!b_wasOpen){
			ImGui::EndChild();
			return;
		}
		
		State& state = getState();
						
		float dragDestinationIndex = -1;
		
		//if an item is dragged
		if(state.draggedItemIndex >= 0){
			
			//get drop index
			for(State::ListItem& item : state.items){
				if(item.b_isAboveDraggedItem) {
					dragDestinationIndex = item.index;
					break;
				}
			}
			if(dragDestinationIndex == -1) dragDestinationIndex = state.items.back().index + 1;
			if(dragDestinationIndex > state.draggedItemIndex) dragDestinationIndex--;
			
			//draw drop target
			if(dragDestinationIndex != state.draggedItemIndex){
				float dropGraphicPositionY;
				if(dragDestinationIndex >= state.items.size()) dropGraphicPositionY = state.items.back().absolutePosition.y + state.items.back().size.y;
				else dropGraphicPositionY = state.items[dragDestinationIndex].absolutePosition.y - ImGui::GetStyle().ItemSpacing.y;
				ImVec2 min(ImGui::GetWindowPos().x, dropGraphicPositionY);
				ImVec2 max(min.x + ImGui::GetContentRegionAvail().x, dropGraphicPositionY + ImGui::GetStyle().ItemSpacing.y);
				float drawMin = ImGui::GetWindowPos().y - ImGui::GetStyle().ItemSpacing.y;
				float drawMax = drawMin + ImGui::GetWindowHeight() + ImGui::GetStyle().ItemSpacing.y;
				min.y = std::max(min.y, drawMin);
				min.y = std::min(min.y, drawMax);
				max.y = std::max(max.y, drawMin);
				max.y = std::min(max.y, drawMax);
				float rounding = std::abs(max.y - min.y);
				ImGui::GetForegroundDrawList()->AddRectFilled(min, max, ImColor(1.0f, 1.0f, 1.0f, .5f), rounding, ImDrawFlags_RoundCornersAll);
			}
			
			//scroll the list while dragging
			if(state.draggedItemAbsolutePosition.y <= ImGui::GetWindowPos().y){
				float dragDistanceFromBounds = ImGui::GetMousePos().y - state.pressedItemPressRelativePosition.y - ImGui::GetWindowPos().y;
				ImGui::SetScrollY(ImGui::GetScrollY() + dragDistanceFromBounds * state.scrollSensitivity);
			}else if(state.draggedItemAbsolutePosition.y >= ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - state.draggedItemSize.y){
				float dragDistanceFromBounds = ImGui::GetMousePos().y - state.pressedItemPressRelativePosition.y + state.draggedItemSize.y - ImGui::GetWindowPos().y - ImGui::GetWindowHeight();
				ImGui::SetScrollY(1.0 + ImGui::GetScrollY() + dragDistanceFromBounds * state.scrollSensitivity);
			}
		}
		
		//clear drag & press state
		if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
			if(state.draggedItemIndex >= 0) {
				state.originIndex = state.draggedItemIndex;
				state.destinationIndex = dragDestinationIndex;
			}
			//reset the pressed item index
			storePressedItemIndex(-1);
			storeDraggedItemIndex(-1);
		}
		
		ImGui::EndChild();
		
	}


	//query reordering action
	inline bool wasReordered(int& fromIndex, int& toIndex){
		State& state = getState();
		if(!ImGui::IsMouseReleased(ImGuiMouseButton_Left) || state.originIndex == -1 || state.destinationIndex == -1 || state.originIndex == state.destinationIndex) return false;
		fromIndex = state.originIndex;
		toIndex = state.destinationIndex;
		return true;
	}

	inline bool isItemSelected(){
		return ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && getState().draggedItemIndex == -1;
	}

};
