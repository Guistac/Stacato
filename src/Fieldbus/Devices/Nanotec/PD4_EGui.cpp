#include <pch.h>

#include "PD4_E.h"

#include <imgui.h>

void PD4_E::deviceSpecificGui() {


    if (ImGui::BeginTabItem("PD4-E")) {

        ImGui::Text("Nanotec Drive Gui");

        ImGui::EndTabItem();
    }
}






struct OperatingMode {
	enum class Mode {
		AUTO_SETUP,
		NO_MODE_CHANGE_OR_NONE_ASSIGNED,
		PROFILE_POSITION,
		VELOCITY,
		PROFILE_VELOCITY,
		PROFILE_TORQUE,
		HOMING,
		INTERPOLATED_POSITION,
		CYCLIC_SYNCHRONOUS_POSITION,
		CYCLIC_SYNCHRONOUS_VELOCITY,
		CYCLIC_SYNCHRONOUS_TORQUE
	};
	int8_t value;
	Mode mode;
	const char displayName[64];
};

std::vector<PD4_E::OperatingMode> operatingModes = {
		{PD4_E::OperatingMode::Mode::AUTO_SETUP, -2, "Auto Setup"},
		{PD4_E::OperatingMode::Mode::NO_MODE_CHANGE_OR_NONE_ASSIGNED, 0, "None"},
		{PD4_E::OperatingMode::Mode::PROFILE_POSITION, 1, "Profile Position"},
		{PD4_E::OperatingMode::Mode::VELOCITY, 2, "Velocity"},
		{PD4_E::OperatingMode::Mode::PROFILE_VELOCITY, 3, "Profile Velocity"},
		{PD4_E::OperatingMode::Mode::PROFILE_TORQUE, 4, "Profile Torque"},
		{PD4_E::OperatingMode::Mode::HOMING, 6, "Homing"},
		{PD4_E::OperatingMode::Mode::INTERPOLATED_POSITION, 7, "Interpolated Position"},
		{PD4_E::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION, 8, "Cyclic Synchronous Position"},
		{PD4_E::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY, 9, "Cyclic Synchronous Velocity"},
		{PD4_E::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_TORQUE, 10, "Cyclic Synchronous Torque"}
};

std::vector<PD4_E::OperatingMode>& PD4_E::getOperatingModes() {
	return operatingModes;
}
PD4_E::OperatingMode* PD4_E::getOperatingMode(int8_t value) {
	for (auto& mode : operatingModes) {
		if (value == mode.value) return &mode;
	}
	return nullptr;
}
PD4_E::OperatingMode* PD4_E::getOperatingMode(OperatingMode::Mode m) {
	for (auto& mode : operatingModes) {
		if (m == mode.mode) return &mode;
	}
	return nullptr;
}