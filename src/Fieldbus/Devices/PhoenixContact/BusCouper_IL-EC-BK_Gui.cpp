#include <pch.h>

#include "BusCouper_IL-EC-BK.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"


namespace PhoenixContact{

void BusCoupler::deviceSpecificGui() {
	if(ImGui::BeginTabItem("Bus Coupler")){
		moduleManagerGui();
		
		ImGui::Separator();
		if(ImGui::Button("Configure PDOs")){
			configureModules();
		}
		if(ImGui::Button("download txPdo")){
			uint32_t o, i;
			int ret = ec_readPDOmap(getSlaveIndex(), &o, &i);
		}
		
		
		
		
		
		ImGui::EndTabItem();
	}
}


}
