#include <pch.h>

#include "PD4_E.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void PD4_E::deviceSpecificGui() {


    if (ImGui::BeginTabItem("PD4-E")) {

        statusGui();

        if (ImGui::BeginTabBar("##PD4ETabBar")) {

            if (ImGui::BeginTabItem("Control")) {
                ImGui::BeginChild("Control");
                controlGui();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Limits")) {
                ImGui::BeginChild("Limits");
                limitsGui();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Gpio")) {
                ImGui::BeginChild("Gpio");
                gpioGui();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Auto Setup")) {
                ImGui::BeginChild("Auto Setup");
                autosetupGui();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Separator();








        ImGui::EndTabItem();
    }
}

void PD4_E::statusGui() {
    float doubleWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
    float maxDoubleWidgetWidth = ImGui::GetTextLineHeight() * 15.0;
    if (doubleWidgetWidth > maxDoubleWidgetWidth) doubleWidgetWidth = maxDoubleWidgetWidth;

    float tripleWidgetWidth = (ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
    float maxTripleWidgetWidth = ImGui::GetTextLineHeight() * 15.0;
    if (tripleWidgetWidth > maxTripleWidgetWidth) tripleWidgetWidth = maxTripleWidgetWidth;

    glm::vec2 statusDisplaySize(tripleWidgetWidth, ImGui::GetTextLineHeight() * 2.0);

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.0, 0.0, 0.0));
    ImGui::Button("Network Status", glm::vec2(statusDisplaySize.x, 0));
    ImGui::SameLine();
    ImGui::Button("EtherCAT status", glm::vec2(statusDisplaySize.x, 0));
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
    if (!isDetected()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
    else if (isStateBootstrap() || isStateInit()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
    else if (isStatePreOperational() || isStateSafeOperational()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
    else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
    ImGui::Button(getEtherCatStateChar(), statusDisplaySize);
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if (isOnline()) {

        glm::vec4 statusButtonColor;
        switch (actualPowerState) {
        case DS402::PowerState::State::NOT_READY_TO_SWITCH_ON:
        case DS402::PowerState::State::SWITCH_ON_DISABLED: statusButtonColor = Colors::red; break;
        case DS402::PowerState::State::READY_TO_SWITCH_ON: statusButtonColor = Colors::orange; break;
        case DS402::PowerState::State::SWITCHED_ON: statusButtonColor = Colors::yellow; break;
        case DS402::PowerState::State::OPERATION_ENABLED: statusButtonColor = Colors::green; break;
        case DS402::PowerState::State::QUICKSTOP_ACTIVE:
        case DS402::PowerState::State::FAULT_REACTION_ACTIVE:
        case DS402::PowerState::State::FAULT: statusButtonColor = Colors::red; break;
        }
        ImGui::PushStyleColor(ImGuiCol_Button, statusButtonColor);
        ImGui::Button(DS402::getPowerState(actualPowerState)->displayName, statusDisplaySize);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
        ImGui::Button("No Status", statusDisplaySize);
    }

    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::PopItemFlag();

    glm::vec2 commandButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);


    bool disableCommandButton = !isOnline();
    if (disableCommandButton) BEGIN_DISABLE_IMGUI_ELEMENT
        if (isEnabled()) {
            if (ImGui::Button("Disable Operation", commandButtonSize)) {
                servoMotor->disable();
            }
        }
        else {
            if (ImGui::Button("Enable Operation", commandButtonSize)) {
                servoMotor->enable();
            }
        }
    ImGui::SameLine();
    if (ImGui::Button("Quick Stop", commandButtonSize)) {
        servoMotor->quickstop();
    }
    if (disableCommandButton) END_DISABLE_IMGUI_ELEMENT
}




void PD4_E::controlGui() {
    ImGui::Text("Control Mode");

    if (ImGui::BeginCombo("##ControlModeSelector", getControlMode(actualControlMode)->displayName)) {
        for (auto& controlMode : getControlModes()) {
            if (ImGui::Selectable(controlMode.displayName, actualControlMode == controlMode.mode)) {
                actualControlMode = controlMode.mode;
            }
        }
        ImGui::EndCombo();
    }

    float widgetWidth = ImGui::GetItemRectSize().x;

    switch (actualControlMode) {
        case ControlMode::Mode::MANUAL_VELOCITY_CONTROL: {
            ImGui::SetNextItemWidth(widgetWidth);
            ImGui::InputDouble("##manualAcceleration", &manualAcceleration_revolutionPerSecondSquared, 0.0, 0.0, "%.1f rev/s\xc2\xb2");
            float vTarget = manualVelocityTarget_revolutionsPerSecond;
            ImGui::SetNextItemWidth(widgetWidth);
            ImGui::SliderFloat("##Velocity", &vTarget, -servoMotor->velocityLimit_positionUnitsPerSecond, servoMotor->velocityLimit_positionUnitsPerSecond, "%.1f rev/s");
            if (ImGui::IsItemActive()) manualVelocityTarget_revolutionsPerSecond = vTarget;
            else manualVelocityTarget_revolutionsPerSecond = 0.0;
            }break;
        case ControlMode::Mode::EXTERNAL_CONTROL:
            break;
    }

    static char velocityDisplayString[64];
    sprintf(velocityDisplayString, "%.3f rev/s", actualVelocity_revolutionsPerSecond);
    float velocityProgress = std::abs(servoMotor->getVelocity() / servoMotor->velocityLimit_positionUnitsPerSecond);
    ImGui::ProgressBar(velocityProgress, glm::vec2(widgetWidth, ImGui::GetFrameHeight()), velocityDisplayString);

    static char positionDisplayString[64];
    sprintf(positionDisplayString, "%.3f rev", servoMotor->getPosition());
    float positionProgress = (servoMotor->getPosition() - servoMotor->getMinPosition()) / (servoMotor->getMaxPosition() - servoMotor->getMinPosition());
    ImGui::ProgressBar(positionProgress, glm::vec2(widgetWidth, ImGui::GetFrameHeight()), positionDisplayString);

    ImGui::Separator();

    ImGui::Text("warning: %s", warning ? "yes" : "no");
    ImGui::Text("fault: %s", fault ? "yes" : "no");
    ImGui::Text("isRemoteControlled: %s", isRemoteControlled ? "yes" : "no");
    ImGui::Text("targetReached: %s", targetReached ? "yes" : "no");
    ImGui::Text("internalLimit: %s", internalLimit ? "yes" : "no");
    ImGui::Text("controllerSyncedWithFieldbus: %s", controllerSyncedWithFieldbus ? "yes" : "no");
    ImGui::Text("controllerFollowsTarget: %s", controllerFollowsTarget ? "yes" : "no");
    ImGui::Text("followingError: %s", followingError ? "yes" : "no");
    ImGui::Text("closedLoopActive: %s", closedLoopActive ? "yes" : "no");
    ImGui::Text("following error: %.6f", actualFollowingError_revolutions);
    ImGui::Text("Motor Moving: %s", servoMotor->isMoving() ? "yes" : "no");
}





void PD4_E::limitsGui() {

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("General Motion Limits");
    ImGui::PopFont();

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    ImGui::Text("Max Motor Velicocity is %.3f rev/s", maxVelocity_revolutionsPerSecond);
    ImGui::PopStyleColor();
    
    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Velocity Limit");
    ImGui::PopFont();
    ImGui::InputDouble("##velocityLimit", &servoMotor->velocityLimit_positionUnitsPerSecond, 0.0, 0.0, "%.3f rev/s");
    if (servoMotor->velocityLimit_positionUnitsPerSecond > maxVelocity_revolutionsPerSecond) servoMotor->velocityLimit_positionUnitsPerSecond = maxVelocity_revolutionsPerSecond;
    else if (servoMotor->velocityLimit_positionUnitsPerSecond < 0.0) servoMotor->velocityLimit_positionUnitsPerSecond = 0.0;
    
    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Acceleration Limit");
    ImGui::PopFont();
    ImGui::InputDouble("##accelerationLimit", &servoMotor->accelerationLimit_positionUnitsPerSecondSquared, 0.0, 0.0, "%.3f rev/s\xc2\xb2");
    if (servoMotor->accelerationLimit_positionUnitsPerSecondSquared < 0.0) servoMotor->accelerationLimit_positionUnitsPerSecondSquared = 0.0;
    
    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Default Manual Acceleration");
    ImGui::PopFont();
    ImGui::InputDouble("##defmanacc", &defaultManualAcceleration_revolutionsPerSecondSquared, 0.0, 0.0, "%.3f rev/s\xc2\xb2");
    if (defaultManualAcceleration_revolutionsPerSecondSquared > servoMotor->accelerationLimit_positionUnitsPerSecondSquared) defaultManualAcceleration_revolutionsPerSecondSquared = servoMotor->accelerationLimit_positionUnitsPerSecondSquared;
    else if (defaultManualAcceleration_revolutionsPerSecondSquared < 0.0) defaultManualAcceleration_revolutionsPerSecondSquared = 0.0;

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Max Following Error");
    ImGui::PopFont();
    ImGui::InputDouble("##maxfollowerror", &maxFollowingError_revolutions, 0.0, 0.0, "%.3f revolutions");
    if (maxFollowingError_revolutions < 0.0) maxFollowingError_revolutions = 0.0;

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Position Controller Proportional Gain");
    ImGui::PopFont();
    ImGui::InputDouble("##kp", &positionControllerProportionalGain);
    if (positionControllerProportionalGain < 0.0) positionControllerProportionalGain = 0.0;

    ImGui::Checkbox("##invertdir", &invertDirectionOfMotion);
    ImGui::SameLine();
    ImGui::Text("Invert Direction of Motor Motion");

}


void PD4_E::gpioGui() {
    
    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Digital Input Settings");
    ImGui::PopFont();

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    ImGui::Text("All Pins use 5V logic by default.");
    ImGui::PopStyleColor();

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Digital Input 1");
    ImGui::PopFont();
    ImGui::Checkbox("##negativeLimit", &negativeLimitSwitchOnDigitalIn1);
    ImGui::SameLine();
    ImGui::Text("Use as Negative Limit Signal");
    ImGui::Checkbox("##D1Invert", &invertDigitalInput1);
    ImGui::SameLine();
    ImGui::Text("Invert");

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Digital Input 2");
    ImGui::PopFont();
    ImGui::Checkbox("##positiveLimit", &positiveLimitSwitchOnDigitalIn2);
    ImGui::SameLine();
    ImGui::Text("Use as Positive Limit Signal");
    ImGui::Checkbox("##D2Invert", &invertDigitalInput2);
    ImGui::SameLine();
    ImGui::Text("Invert");

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Digital Input 3");
    ImGui::PopFont();
    ImGui::Checkbox("##D3Invert", &invertDigitalInput3);
    ImGui::SameLine();
    ImGui::Text("Invert");
    
    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Digital Input 4");
    ImGui::PopFont();
    ImGui::Checkbox("##D4Invert", &invertDigitalInput4);
    ImGui::SameLine();
    ImGui::Text("Invert");

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Digital Input 5");
    ImGui::PopFont();
    ImGui::Checkbox("##D5Invert", &invertDigitalInput5);
    ImGui::SameLine();
    ImGui::Text("Invert");

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Digital Input 6");
    ImGui::PopFont();
    ImGui::Checkbox("##D6Invert", &invertDigitalInput6);
    ImGui::SameLine();
    ImGui::Text("Invert");
}


void PD4_E::autosetupGui() {
    if (ImGui::Button("Start Auto Setup")) {
        b_startAutoSetup = true;
        b_autoSetupComplete = false;
    }

    if (b_autoSetupActive) {
        ImGui::Text("Auto Setup is Active");
    }
    else if (b_autoSetupComplete) {
        ImGui::Text("Auto Setup is Complete");
    }

}
