#include <pch.h>
#include "KincoFD.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void KincoFD::deviceSpecificGui() {
	if(ImGui::BeginTabItem("Kinco FD")){
		
		
		if(ImGui::Button("Enable")) b_enable = true;
		if(ImGui::Button("Disable")) b_disable = true;
		
		ImGui::Text("Actual Power State: %s", DS402Axis::getPowerStateString(axis->getActualPowerState()).c_str());
		ImGui::Text("Requested Power State: %s", DS402Axis::getTargetPowerStateString(axis->getTargetPowerState()).c_str());
		
		ImGui::Text("Actual Operating Mode: %s", DS402Axis::getOperatingModeString(axis->getOperatingModeActual()).c_str());
		ImGui::Text("Target Operating Mode: %s", DS402Axis::getOperatingModeString(axis->getOperatingModeTarget()).c_str());
		
		static int32_t min = -65536;
		static int32_t max = 65536;
		ImGui::SliderScalar("Pos", ImGuiDataType_S32, &pos, &min, &max);
		
		ImGui::EndTabItem();
	}
}
