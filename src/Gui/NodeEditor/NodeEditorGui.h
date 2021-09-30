#pragma once

#include <imgui.h>

#include "NodeGraph/Node.h"

namespace ImGuiNodeEditor {
	void CreateContext();
	void DestroyContext();
}

void nodeGraph();

std::shared_ptr<Node> nodeAdderContextMenu();

std::shared_ptr<Node> acceptDraggedNode();

void nodeAdder();

void DrawPinIcon(ImDrawList* drawList, const glm::vec2& a, const glm::vec2& b, int type, bool filled, ImU32 color, ImU32 innerColor);