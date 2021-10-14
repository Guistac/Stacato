#pragma once

#include <imgui.h>
#include <vector>

class DraggableList {

public:

	bool beginList(const char* id, ImVec2 size) {
		if (ImGui::BeginChild(id, size, true) == false) {
			ImGui::EndChild();
			return false;
		}
		begunTooltip = false;
		begunChild = false;
		itemDropped = false;
		items.clear();
		ImVec2 mousePosition = ImGui::GetMousePos();
		bool isMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		if (!itemIsDragging && isMouseDown && draggedItemIndex != -1 && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
			//Starts Item Dragging
			itemIsDragging = true;
		}
		else if (itemIsDragging && !isMouseDown) {
			//Triggers Dropped Item Event
			itemDropped = true;
			droppedItemIndex = draggedItemIndex;
			itemIsDragging = false;
			draggedItemIndex = -1;
		}
		return true;
	}

	bool beginItem(ImVec2 size) {
		ImVec2 position;
		currentIndex = items.size();
		ImGui::PushID(currentIndex);
		if (itemIsDragging && currentIndex == draggedItemIndex) {
			ImGui::Dummy(size);
			position = ImGui::GetItemRectMin();
			ImVec2 mousePosition = ImGui::GetMousePos();
			draggedItemPositionY = mousePosition.y - itemClickPosition.y;
			if (draggedItemPositionY < minYPosition) draggedItemPositionY = minYPosition;
			else if (draggedItemPositionY > maxYPosition) draggedItemPositionY = maxYPosition;
			ImGui::SetNextWindowPos(ImVec2(constrainedXPosition, draggedItemPositionY), ImGuiCond_Always);
			ImGui::SetNextWindowSize(size);
			ImGui::BeginTooltip();
			begunTooltip = true;
		}
		else {
			begunChild = ImGui::BeginChild("Item", size, true);
			position = ImGui::GetWindowPos();
		}
		items.push_back(DraggableItemInfo(position, size));
		if (begunTooltip || begunChild) return true;
		else {
			//BeginChild only tells us if the window is collapsed or fully clipped, we have to EndChild after each BeginChild call
			ImGui::EndChild();
			ImGui::PopID();
			return false;
		}
	}

	void endItem() {
		if (begunTooltip) {
			ImGui::EndTooltip();
		}
		else if (begunChild) {
			ImVec2 childPos = ImGui::GetWindowPos();
			ImGui::EndChild();
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				draggedItemIndex = currentIndex;
				ImVec2 mousePosition = ImGui::GetMousePos();
				itemClickPosition.x = mousePosition.x - childPos.x;
				itemClickPosition.y = mousePosition.y - childPos.y;
			}
		}
		ImGui::PopID();
		begunTooltip = false;
		begunChild = false;
	}

	void endList() {
		if (!items.empty()) {
			ImDrawList* fgDrawList = ImGui::GetForegroundDrawList();
			for (auto& item : items) {
				item.center = item.position;
				item.center.x += item.size.x * 0.5;
				item.center.y += item.size.y * 0.5;
			}
			constrainedXPosition = items.front().position.x;
			if (itemIsDragging && draggedItemIndex > -1 && draggedItemIndex < items.size()) {
				double listpositionY = ImGui::GetWindowPos().y;
				double listSizeY = ImGui::GetWindowSize().y;
				minYPosition = listpositionY;
				maxYPosition = listpositionY + listSizeY - items[draggedItemIndex].size.y;
				dropPositionIndex = getDropIndex();
				ImVec2 dropPosition;
				if (dropPositionIndex == draggedItemIndex) {
					dropPosition = items[draggedItemIndex].center;
				}
				else if (dropPositionIndex < draggedItemIndex) {
					dropPosition = items[dropPositionIndex].position;
					dropPosition.x += items[dropPositionIndex].size.x * 0.5;
				}
				else if (dropPositionIndex > draggedItemIndex) {
					dropPosition = items[dropPositionIndex].position;
					dropPosition.x += items[dropPositionIndex].size.x * 0.5;
					dropPosition.y += items[dropPositionIndex].size.y;
				}
				fgDrawList->AddCircle(dropPosition, 10.0, ImColor(ImVec4(1.0, 0.0, 0.0, 1.0)), 20, 10.0);

				float mousePosY = ImGui::GetMousePos().y;
				if (draggedItemPositionY <= minYPosition) {
					float thresholdMouseYPosition = minYPosition + itemClickPosition.y;
					float scrollOffset = std::abs(mousePosY - thresholdMouseYPosition);
					ImGui::SetScrollY(ImGui::GetScrollY() - scrollOffset * 0.1);
				}
				else if (draggedItemPositionY >= maxYPosition) {
					float thresholdMouseYPosition = maxYPosition + itemClickPosition.y;
					float scrollOffset = std::abs(mousePosY - thresholdMouseYPosition);
					ImGui::SetScrollY(ImGui::GetScrollY() + scrollOffset * 0.1);
				}
			}
		}
		ImGui::EndChild();
	}


	bool wasReordered() {
		return itemDropped && droppedItemIndex != dropPositionIndex;
	}

	void getReorderedItemIndex(int& previousItemIndex, int& newItemIndex) {
		if (itemDropped) {
			previousItemIndex = droppedItemIndex;
			newItemIndex = dropPositionIndex;
		}
	}

private:

	int getDropIndex() {
		DraggableItemInfo& draggedItem = items[draggedItemIndex];
		double draggedItemCenterY = draggedItemPositionY + draggedItem.size.y * 0.5;
		if (draggedItemCenterY < items[0].center.y) return 0;
		for (int i = 0; i < items.size() - 1; i++) {
			DraggableItemInfo& previousItem = items[i];
			DraggableItemInfo& nextItem = items[i + 1];
			if (draggedItemCenterY >= previousItem.center.y && draggedItemCenterY < nextItem.center.y) {
				if (draggedItemIndex > i) return i + 1;
				else return i;
			}
		}
		return items.size() - 1;
	};

	//persistent values
	bool itemIsDragging = false;
	int draggedItemIndex = -1;
	ImVec2 itemClickPosition;

	//currently submitted item
	bool begunTooltip = false;
	bool begunChild = false;
	int currentIndex;
	double constrainedXPosition;
	double minYPosition;
	double maxYPosition;
	double draggedItemPositionY;

	//dropped item output data
	bool itemDropped = false;
	int droppedItemIndex = -1;
	int dropPositionIndex = -1;

	struct DraggableItemInfo {
		DraggableItemInfo(ImVec2 p, ImVec2 s) : position(p), size(s), center(ImVec2(0, 0)) {}
		ImVec2 position;
		ImVec2 size;
		ImVec2 center;
	};
	//list of items gets refreshed each time the gui gets submitted
	std::vector<DraggableItemInfo> items;
};