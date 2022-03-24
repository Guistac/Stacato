#include "StageView.h"

#include <imgui.h>

namespace StageView {

	void draw() {

		glm::vec2 canvasSize = ImGui::GetContentRegionAvail();
		ImGui::InvisibleButton("##MagnumCanvas", canvasSize);

	}

}
