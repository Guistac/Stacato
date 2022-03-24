#include <pch.h>

#include <imgui.h>

#include "EnvironnementGui.h"
#include "Gui/Fieldbus/EtherCatGui.h"
#include "Gui/Utilities/Log.h"

namespace Environnement::Gui{

	void gui() {
		if (ImGui::BeginTabBar("##EnvironnementTabBar")) {
			if (ImGui::BeginTabItem("Editor")) {
				editor();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Machine Manager")) {
				machineManager();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Device Manager")) {
				deviceManager();
				ImGui::EndTabItem();
			}
			/*
			if (ImGui::BeginTabItem("Stage Editor")) {
				stageEditorGui();
				ImGui::Text("3D Scene Graph / Geometry importer / Machine movement assignement");
				ImGui::EndTabItem();
			}
			 */
			if (ImGui::BeginTabItem("Fieldbus")) {
				etherCatGui();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Log")) {
				Utilies::Gui::log();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

}
