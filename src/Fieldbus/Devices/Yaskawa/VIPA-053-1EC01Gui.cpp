#include <pch.h>

#include "VIPA-053-1EC01.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include "Gui/Framework/Colors.h"
#include "Gui/Framework/Fonts.h"



void VIPA_053_1EC01::deviceSpecificGui() {
    if (ImGui::BeginTabItem("VIPA")) {
        ImGui::EndTabItem();
    }
}