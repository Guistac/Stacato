#include <pch.h>
#include "EnvironnementGui.h"

#include "Environnement/Environnement.h"
#include "Scripting/Script.h"

namespace Environnement::Gui{

	void stageEditor(){
		
		ImVec2 half = ImGui::GetContentRegionAvail();
		half.x /= 2.0;
		
		Environnement::stageVisualizerScript->editor(half);
	}

}
