#include <pch.h>

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include <imgui.h>

namespace Environnement::Gui{

	void machineList() {

		float availableWidth = ImGui::GetContentRegionAvail().x;
		float width = 0.0;
		float spacing = ImGui::GetStyle().ItemSpacing.x;
		bool first = true;
		
		for (auto machine : Environnement::getMachines()) {
			float machineWidth = machine->getMiniatureWidth();
			if(width + machineWidth < availableWidth) {
				if(first){
					first = false;
				}
				else ImGui::SameLine();
				width += machineWidth + spacing;
			}else{
				width = machineWidth + spacing;
			}
			machine->miniatureGui();
		}
		
		ImGui::NewLine();
	}

}
