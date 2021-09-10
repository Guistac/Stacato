#pragma once

#include <imgui.h>

#include "NodeGraph/ioNode.h"

namespace ImGuiNodeEditor {
	void CreateContext();
	void DestroyContext();
}

void nodeGraph();

std::shared_ptr<ioNode> nodeAdderContextMenu();

std::shared_ptr<ioNode> acceptDraggedNode();

void nodeAdder();

void DrawIcon(ImDrawList* drawList, const glm::vec2& a, const glm::vec2& b, int type, bool filled, ImU32 color, ImU32 innerColor);