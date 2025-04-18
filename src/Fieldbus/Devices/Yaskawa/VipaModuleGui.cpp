#include <pch.h>

#include "VipaModule.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"


void VipaModule::moduleGui(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("%s", getDisplayName());
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansRegular20);
	ImGui::Text("%s", getSaveName());
	ImGui::PopFont();
	moduleParameterGui();
}

void VipaModule::moduleParameterGui(){
	ImGui::Text("No Settings Are Available for this SLIO Module");
}
