#include <pch.h>

#include "Motion/Axis/VelocityControlledAxis.h"

#include <imgui.h>

void VelocityControlledAxis::nodeSpecificGui() {
	if (ImGui::BeginTabItem("Controls")) {
		if (ImGui::BeginChild("Controls")) {
			controlsGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Settings")) {
		if (ImGui::BeginChild("Settings")) {
			settingsGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Devices")) {
		if (ImGui::BeginChild("Devices")) {
			devicesGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Metrics")) {
		if (ImGui::BeginChild("Metrics")) {
			metricsGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
}

void VelocityControlledAxis::settingsGui() {}
void VelocityControlledAxis::metricsGui() {}
void VelocityControlledAxis::devicesGui() {}
void VelocityControlledAxis::controlsGui() {}