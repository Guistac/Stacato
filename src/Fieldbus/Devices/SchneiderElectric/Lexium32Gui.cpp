#include <pch.h>

#include "Lexium32.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include "Gui/Framework/Colors.h"
#include "Gui/Framework/Fonts.h"



void Lexium32::deviceSpecificGui() {

    if (ImGui::BeginTabItem("Lexium32")) {    

        float doubleWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
        float maxWidgetWidth = ImGui::GetTextLineHeight() * 15.0;
        if (doubleWidgetWidth > maxWidgetWidth) doubleWidgetWidth = maxWidgetWidth;

        glm::vec2 statusDisplaySize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 2.0);

        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.0, 0.0, 0.0));
        ImGui::Button("Network Status", glm::vec2(statusDisplaySize.x, 0));
        ImGui::SameLine();
        ImGui::Button("Servo Status", glm::vec2(statusDisplaySize.x, 0));
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();

        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushFont(Fonts::robotoBold15);
        ImGui::PushStyleColor(ImGuiCol_Button, isOnline() ? Colors::green : (isDetected() ? Colors::yellow : Colors::red));

        ImGui::Button(isOnline() ? "Online" : (isDetected() ? "Detected" : "Offline"), statusDisplaySize);
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (isOnline()) {

            glm::vec4 statusButtonColor;
            switch (state) {
            case State::NotReadyToSwitchOn:
            case State::SwitchOnDisabled: statusButtonColor = Colors::red; break;
            case State::ReadyToSwitchOn: statusButtonColor = Colors::orange; break;
            case State::SwitchedOn: statusButtonColor = Colors::yellow; break;
            case State::OperationEnabled: statusButtonColor = Colors::green; break;
            case State::QuickStopActive:
            case State::FaultReactionActive:
            case State::Fault: statusButtonColor = Colors::red; break;
            }
            ImGui::PushStyleColor(ImGuiCol_Button, statusButtonColor);
            ImGui::Button(getStateChar(), statusDisplaySize);
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
            ImGui::Button("No Status", statusDisplaySize);
            ImGui::PopStyleColor();
        }

        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::PopItemFlag();
        


        glm::vec2 commandButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);

        if (b_voltageEnabled) {
            if (ImGui::Button("Disable Voltage", commandButtonSize)) disableVoltage();
        }
        else {
            if (ImGui::Button("Enable Voltage", commandButtonSize)) enableVoltage();
        }
        ImGui::SameLine();
        if (b_switchedOn) {
            if (ImGui::Button("Shut Down", commandButtonSize)) shutDown();
        }
        else {
            if (ImGui::Button("Switch On", commandButtonSize)) switchOn();
        }


        if (b_operationEnabled) {
            if (ImGui::Button("Disable Operation", commandButtonSize)) disableOperation();
        }
        else {
            if (ImGui::Button("Enable Operation", commandButtonSize)) enableOperation();
        }
        ImGui::SameLine();
        if (ImGui::Button("Quick Stop", commandButtonSize)) quickStop();

        bool hasFault = state == State::Fault;
        if (!hasFault) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
        }
        if (ImGui::Button("Fault Reset", glm::vec2(ImGui::GetTextLineHeight() * 6.0, commandButtonSize.y))) faultReset();
        if (hasFault) {
            ImGui::SameLine();
            ImGui::Text("Error Code:", lastErrorCode);
        }
        if (!hasFault) {
            ImGui::PopItemFlag();
            ImGui::PopStyleColor();
        }

        if (ImGui::BeginTabBar("LexiumTabBar")) {

            if (ImGui::BeginTabItem("Control")) {
                ImGui::PushFont(Fonts::robotoBold20);
                ImGui::Text("Device Mode");
                ImGui::PopFont();

                if (ImGui::BeginCombo("##ModeSelector", getOperatingMode()->displayName, ImGuiComboFlags_HeightLargest)) {
                    for (OperatingMode& availableMode : availableOperatingModes) {
                        if (ImGui::Selectable(availableMode.displayName, availableMode.id == modeID)) modeIDCommand = availableMode.id;
                    }
                    ImGui::EndCombo();
                }
                
                OperatingMode* operatingMode = getOperatingMode();

                if (operatingMode == nullptr) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
                    ImGui::TextWrapped("Current Operating Mode is Unknown");
                    ImGui::PopStyleColor();
                }
                else if (operatingMode->mode == OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION) {
                    ImGui::TextWrapped("In this mode, the drive is controlled by the position command input. No manual controls are available.");
                }
                else if (operatingMode->mode == OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY) {
                    ImGui::TextWrapped("This mode is exclusively designed for manual drive velocity control. When the drive is in this mode, it will only respond to the following velocity parameters.");
                    float maxV = maxVelocity_rpm;
                    ImGui::SliderFloat("##manualVelocity", &manualVelocity_rpm, -maxVelocity_rpm, maxVelocity_rpm, "%.3frpm");
                    float velocityFraction = (actualVelocity->getReal() + maxVelocity_rpm) / (2 * maxVelocity_rpm);
                    static char actualVelocityString[32];
                    sprintf(actualVelocityString, "%.1frpm", actualVelocity->getReal());
                    ImGui::ProgressBar(velocityFraction, ImGui::GetItemRectSize(), actualVelocityString);
                    ImGui::InputFloat("##manualAcceleration", &manualAcceleration_rpm2, 0.0, (float)maxAcceleration_rpm2, "Acceleration: %.3f rpm2");
                    if (manualAcceleration_rpm2 > maxAcceleration_rpm2) manualAcceleration_rpm2 = maxAcceleration_rpm2;
                    if (ImGui::Button("Stop Movement", glm::vec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeight() * 2.0))) manualVelocity_rpm = 0.0;
                }
                else if (operatingMode->mode == OperatingMode::Mode::TUNING) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
                    ImGui::TextWrapped("Tuning Mode is not yet supported");
                    ImGui::PopStyleColor();
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
                    ImGui::TextWrapped("Current Operating Mode is Not Supported");
                    ImGui::PopStyleColor();
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Limits")) {

                ImGui::PushFont(Fonts::robotoBold20);
                ImGui::Text("Profile Generator Limits");
                ImGui::PopFont();

                ImGui::Text("Max Velocity");
                ImGui::InputDouble("##maxV", &maxVelocity_rpm, 0.0, 0.0, "%.1frpm");
                ImGui::Text("Max Acceleration");
                ImGui::InputDouble("##maxA", &maxAcceleration_rpm2, 0.0, 0.0, "%.1frpm2");

                ImGui::Separator();

                ImGui::PushFont(Fonts::robotoBold20);
                ImGui::Text("Load Limit");
                ImGui::PopFont();

                ImGui::Text("Max Current");
                ImGui::InputDouble("##maxI", &maxCurrent_amps, 0.0, 0.0, "%.1f Amperes");

                ImGui::Separator();

                ImGui::PushFont(Fonts::robotoBold20);
                ImGui::Text("Limit Signals");
                ImGui::PopFont();
                        
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Feedback Configuration")) {
                
                ImGui::PushFont(Fonts::robotoBold20);
                ImGui::Text("Encoder Settings");
                ImGui::PopFont();

                ImGui::Text("Encoder increments per motor shaft revolution");
                ImGui::InputInt("##incrementsPerRotation", &encoderIncrementsPerShaftRotation, 0, 0);
                ImGui::Text("Encoder Multiturn Resolution");
                ImGui::InputInt("##MtResolution", &encoderMultiturnResolution, 0, 0);

                ImGui::EndTabItem();
            }


            ImGui::EndTabBar();
        }



        ImGui::EndTabItem();
    }

    /*
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

        ImGui::Text("Position: %f", actualPosition->getReal());
        ImGui::Text("Torque: %f", actualTorque->getReal());

        ImGui::Text("Digital Inputs: %i %i %i %i %i %i", digitalIn0->getBoolean(), digitalIn1->getBoolean(), digitalIn2->getBoolean(), digitalIn3->getBoolean(), digitalIn4->getBoolean(), digitalIn5->getBoolean());

        ImGui::Separator();

        ImGui::Text("Current Operating Mode: %s (%i)", modeChar, mode);

        if (ImGui::BeginCombo("Mode Switch", modelist[modeCommand].c_str(), ImGuiComboFlags_HeightLargest)) {
            for (const auto& [key, value] : modelist) {
                if (key == 0) continue;
                if (ImGui::Selectable(value.c_str(), false)) modeCommand = key;

            }
            ImGui::EndCombo();
        }


        bool DQ0 = digitalOut0->getBoolean();
        bool DQ1 = digitalOut1->getBoolean();
        bool DQ2 = digitalOut2->getBoolean();

        ImGui::Checkbox("DQ0", &DQ0);
        ImGui::SameLine();
        ImGui::Checkbox("DQ1", &DQ1);
        ImGui::SameLine();
        ImGui::Checkbox("DQ2", &DQ2);

        digitalOut0->set(DQ0);
        digitalOut1->set(DQ1);
        digitalOut2->set(DQ2);

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
    */
}