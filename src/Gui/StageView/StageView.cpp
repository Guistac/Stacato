#include "StageView.h"

#include <imgui.h>

#include "Environnement/StageVisualizer.h"

namespace StageView {

	void draw() {
		Environnement::StageVisualizer::canvas(ImGui::GetContentRegionAvail(), 1.0, 5.0);
	}

}
