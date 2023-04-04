#pragma once

#include <imgui.h>

#include "Environnement/NodeGraph/Node.h"

namespace Environnement::NodeGraph::Gui{

	void DrawPinIcon(ImDrawList* drawList, const glm::vec2& a, const glm::vec2& b, int type, bool filled, ImU32 color, ImU32 innerColor);

}
