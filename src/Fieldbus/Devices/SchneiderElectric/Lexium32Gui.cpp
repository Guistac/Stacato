#include <pch.h>

#include "Lexium32.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

void Lexium32::deviceSpecificGui() {

    if (ImGui::BeginTabItem("Lexium32 Options")) {
        ImGui::Separator();

        if (ImGui::Button("Enable Voltage")) enableVoltage();
        ImGui::SameLine();
        if (ImGui::Button("Disable Voltage")) disableVoltage();

        if (ImGui::Button("Switch On")) switchOn();
        ImGui::SameLine();
        if (ImGui::Button("Shut Down")) shutDown();

        if (ImGui::Button("Enable Operation")) enableOperation();

        ImGui::SameLine();
        if (ImGui::Button("Disable Operation")) disableOperation();

        if (ImGui::Button("Fault Reset")) faultReset();
        ImGui::SameLine();
        if (ImGui::Button("Quick Stop")) quickStop();

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

        ImGui::Text("Position: %f", actualPosition.getReal());
        ImGui::Text("Velocity: %f", actualVelocity.getReal());
        ImGui::Text("Torque: %f", actualTorque.getReal());

        ImGui::Text("Digital Inputs: %i %i %i %i %i %i", digitalIn0.getBoolean(), digitalIn1.getBoolean(), digitalIn2.getBoolean(), digitalIn3.getBoolean(), digitalIn4.getBoolean(), digitalIn5.getBoolean());

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

        float velocityFraction = ((actualVelocity.getReal() / 7000.0) + 1.0) / 2.0;
        ImGui::ProgressBar(velocityFraction, ImVec2(0, 0), "velocity");

        bool DQ0 = digitalOut0.getBoolean();
        bool DQ1 = digitalOut1.getBoolean();
        bool DQ2 = digitalOut2.getBoolean();

        ImGui::Checkbox("DQ0", &DQ0);
        ImGui::SameLine();
        ImGui::Checkbox("DQ1", &DQ1);
        ImGui::SameLine();
        ImGui::Checkbox("DQ2", &DQ2);

        digitalOut0.set(DQ0);
        digitalOut1.set(DQ1);
        digitalOut2.set(DQ2);

        if (ImGui::Button("<<--")) {}
        ImGui::SameLine();
        if (ImGui::Button("<--")) {}
        ImGui::SameLine();
        if (ImGui::Button("0")) {}
        ImGui::SameLine();
        if (ImGui::Button("-->")) {}
        ImGui::SameLine();
        if (ImGui::Button("-->>")) {}

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