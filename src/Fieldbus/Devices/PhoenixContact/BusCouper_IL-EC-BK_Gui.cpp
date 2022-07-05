#include <pch.h>

#include "BusCouper_IL-EC-BK.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"


void IL_EC_BK_BusCoupler::deviceSpecificGui() {
	if(ImGui::BeginTabItem("MicroFlex e190")){
		ImGui::EndTabItem();
	}
}
