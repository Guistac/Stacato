#include <pch.h>

#include "Gui.h"

#include <imgui.h>


void gui() {
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);
	etherCatGui();
}
