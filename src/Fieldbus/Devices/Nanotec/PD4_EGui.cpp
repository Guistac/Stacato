#include <pch.h>

#include "PD4_E.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Framework/Fonts.h"
#include "Gui/Framework/Colors.h"

void PD4_E::deviceSpecificGui() {


    if (ImGui::BeginTabItem("PD4-E")) {


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
                if (ImGui::Button("Disable Operation", commandButtonSize)) requestedPowerState = DS402::PowerState::State::READY_TO_SWITCH_ON;
            }
            else { 
                if (ImGui::Button("Enable Operation", commandButtonSize)) requestedPowerState = DS402::PowerState::State::OPERATION_ENABLED;
            }
        ImGui::SameLine();
        if (ImGui::Button("Quick Stop", commandButtonSize)) requestedPowerState = DS402::PowerState::State::QUICKSTOP_ACTIVE;
        if (disableCommandButton) END_DISABLE_IMGUI_ELEMENT

        
        ImGui::Separator();


		float maxVelocity = 2000;
		float velocityControl = targetVelocity;
		ImGui::SliderFloat("Velocity", &velocityControl, -maxVelocity, maxVelocity, "%.1f rpm");
		targetVelocity = velocityControl;
		float velocityProgress = std::abs((float)actualVelocity / maxVelocity);
		ImGui::ProgressBar(velocityProgress);


        ImGui::InputDouble("target position", &targetPosition_revolution, 0.1, 1.0);

        ImGui::Text("warning: %s", warning ? "yes" : "no");
        ImGui::Text("fault: %s", fault ? "yes" : "no");
        ImGui::Text("isRemoteControlled: %s", isRemoteControlled ? "yes" : "no");
        ImGui::Text("targetReached: %s", targetReached ? "yes" : "no");
        ImGui::Text("internalLimit: %s", internalLimit ? "yes" : "no");

        ImGui::Text("controllerSyncedWithFieldbus: %s", controllerSyncedWithFieldbus ? "yes" : "no");
        ImGui::Text("controllerFollowsTarget: %s", controllerFollowsTarget ? "yes" : "no");
        ImGui::Text("followingError: %s", followingError ? "yes" : "no");
        ImGui::Text("closedLoopActive: %s", closedLoopActive ? "yes" : "no");

        ImGui::Text("OpMode: %i", ds402status.operatingModeDisplay);
        if (ImGui::Button("Auto Setup")) {
            b_autoSetup = true;
        }
        ImGui::Checkbox("start setup", &b_startSetup);
        if (ImGui::Button("Stop Auto Setup")) {
            b_autoSetup = false;
        }
        ImGui::Text("auto setup status: %s", ds402status.getOperationModeSpecificByte12() ? "yes" : "no");
        ImGui::Text("encoder index found: %s", ds402status.getOperationModeSpecificByte10() ? "yes" : "no");

        ImGui::Text("following error: %.6f", followingError_revolutions);

        ImGui::Text("Motor Moving: %s", servoMotor->isMoving() ? "yes" : "no");

        ImGui::EndTabItem();
    }
}

