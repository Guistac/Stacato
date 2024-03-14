#include <pch.h>

#include "EtherCatFiberConverter.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void ECAT_2511_A_FiberConverter::deviceSpecificGui() {
    if(ImGui::BeginTabItem("ECAT-2511-A")){
        ImGui::Text("This device has no settings or controls");
        ImGui::EndTabItem();
    }
}

void ECAT_2511_B_FiberConverter::deviceSpecificGui() {
    if(ImGui::BeginTabItem("ECAT-2511-B")){
        ImGui::Text("This device has no settings or controls");
        ImGui::EndTabItem();
    }
}

void ECAT_2515_6PortJunction::deviceSpecificGui() {
	if(ImGui::BeginTabItem("ECAT-2515")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void ECAT_2517_7PortJunction::deviceSpecificGui() {}
