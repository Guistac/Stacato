#include <pch.h>
#include "EnvironnementGui.h"

#include "Environnement/Environnement.h"
#include "Scripting/Script.h"

#include "Environnement/StageVisualizer.h"

namespace Environnement::Gui{

	void stageEditor(){
		
		ImVec2 half = ImGui::GetContentRegionAvail();
		half.x /= 2.0;
		StageVisualizer::editor(half);
		
		ImGui::SameLine();
		StageVisualizer::canvas(ImGui::GetContentRegionAvail(), 5.0);
		
	}

}
