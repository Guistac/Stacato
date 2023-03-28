#include <pch.h>
#include "KincoFD.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void KincoFD::deviceSpecificGui() {
	if(ImGui::BeginTabItem("Kinco FD")){
		ImGui::EndTabItem();
	}
}
