#include <pch.h>

#include "Lexium32.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

void Lexium32::deviceSpecificGui() {

    if (ImGui::BeginTabItem("Lexium32 Options")) {
        ImGui::Separator();

        if (ImGui::Button("Enable Voltage")) enableVoltage = true;
        ImGui::SameLine();
        if (ImGui::Button("Disable Voltage")) disableVoltage = true;

        if (ImGui::Button("Switch On")) switchOn = true;
        ImGui::SameLine();
        if (ImGui::Button("Shut Down")) shutdown = true;

        if (ImGui::Button("Enable Operation")) {
            enableOperation = true;
            positionCommand = position;
            counter = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Disable Operation")) disableOperation = true;

        if (ImGui::Button("Fault Reset")) performFaultReset = true;
        ImGui::SameLine();
        if (ImGui::Button("Quick Stop")) performQuickStop = true;

        const char* stateChar;
        switch (state) {
        case State::Fault:					stateChar = "Fault"; break;
        case State::FaultReactionActive:	stateChar = "Fault Reaction Active"; break;
        case State::NotReadyToSwitchOn:		stateChar = "Not Ready To Switch On"; break;
        case State::OperationEnabled:		stateChar = "Operation Enabled"; break;
        case State::QuickStopActive:		stateChar = "Quick Stop Active"; break;
        case State::ReadyToSwitchOn:		stateChar = "Ready To Switch On"; break;
        case State::SwitchedOn:				stateChar = "Switched On"; break;
        case State::SwitchOnDisabled:		stateChar = "Switch On Disabled"; break;
        }
        ImGui::Text("State: %s", stateChar);
        if (state == State::Fault) {
            ImGui::SameLine();
            ImGui::Text("(%X)", lastErrorCode);
        }

        ImGui::Separator();

        ImGui::Text("Position: %i", position);
        ImGui::Text("Velocity: %i", velocity);
        ImGui::Text("Torque: %i", torque);

        ImGui::Text("Digital Inputs: %i %i %i %i %i %i", DI0, DI1, DI2, DI3, DI4, DI5);

        ImGui::Separator();

        ImGui::Text("Current Operating Mode: %s (%i)", modeChar, mode);

        if (ImGui::BeginCombo("Mode Switch", modelist[modeCommand].c_str(), ImGuiComboFlags_HeightLargest)) {
            for (const auto& [key, value] : modelist) {
                if (key == 0) continue;
                if (ImGui::Selectable(value.c_str(), false)) modeCommand = key;

            }
            ImGui::EndCombo();
        }

        ImPlot::SetNextPlotLimitsX(positions.newest().x - 1000.0, positions.newest().x, ImGuiCond_Always);
        ImPlot::SetNextPlotFormatY("%g ticks");
        ImPlot::FitNextPlotAxes(false, true);

        if (ImPlot::BeginPlot("positions", NULL, NULL, ImVec2(-1, 600))) {
            ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
            ImPlot::PlotLine("Position", &positions.front().x, &positions.front().y, positions.size(), positions.offset(), positions.stride());
            ImPlot::EndPlot();
        }

        ImGui::Checkbox("Invert", &b_inverted);

        int displayPosition = position;
        int displayVelocity = velocity;
        int displayTorque = torque;

        float velocityFraction = (((double)displayVelocity / 7000.0) + 1.0) / 2.0;
        ImGui::ProgressBar(velocityFraction, ImVec2(0, 0), "velocity");


        ImGui::Checkbox("DQ0", &DQ0);
        ImGui::SameLine();
        ImGui::Checkbox("DQ1", &DQ1);
        ImGui::SameLine();
        ImGui::Checkbox("DQ2", &DQ2);

        if (ImGui::Button("<<--")) {
            jog = true;
            direction = false;
            fast = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("<--")) {
            jog = true;
            direction = false;
            fast = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("0")) {
            stop = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("-->")) {
            jog = true;
            direction = true;
            fast = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("-->>")) {
            jog = true;
            direction = true;
            fast = true;
        }

        ImGui::Separator();

        ImGui::Text("motorVoltagePresent : %i", motorVoltagePresent);
        ImGui::Text("class0error : %i", class0error);
        ImGui::Text("halted : %i", halted);
        ImGui::Text("fieldbusControlActive : %i", fieldbusControlActive);
        ImGui::Text("targetReached : %i", targetReached);
        ImGui::Text("internalLimitActive : %i", internalLimitActive);
        ImGui::Text("operatingModeSpecificFlag : %i", operatingModeSpecificFlag);
        ImGui::Text("stoppedByError : %i", stoppedByError);
        ImGui::Text("operatingModeFinished : %i", operatingModeFinished);
        ImGui::Text("validPositionReference : %i", validPositionReference);

        ImGui::EndTabItem();
    }
}