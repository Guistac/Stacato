#pragma once

#include <imgui.h>

#include "Environnement/Node.h"

namespace Environnement::NodeGraph::Gui{

	void editor(glm::vec2 size_arg);
	std::shared_ptr<Node> nodeAdderContextMenu();
	std::shared_ptr<Node> acceptDraggedNode();
	void nodeAdder();
	void DrawPinIcon(ImDrawList* drawList, const glm::vec2& a, const glm::vec2& b, int type, bool filled, ImU32 color, ImU32 innerColor);

	void showFlow();
	void centerView();

	void reset();

}
