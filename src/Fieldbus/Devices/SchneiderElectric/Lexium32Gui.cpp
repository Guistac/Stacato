#include <pch.h>

#include "Lexium32.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include "Gui/Framework/Colors.h"
#include "Gui/Framework/Fonts.h"



void Lexium32::deviceSpecificGui() {

    if (ImGui::BeginTabItem("Lexium32")) {    

        statusGui();

        if (ImGui::BeginTabBar("LexiumTabBar")) {
            if (ImGui::BeginTabItem("Control")) {
                controlsGui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Limits")) {
                limitsGui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Feedback Configuration")) {
                feedbackConfigurationGui();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }



        ImGui::EndTabItem();
    }
}



void Lexium32::statusGui() {

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
        ImGui::Text("Error Code: %X", lastErrorCode);
    }
    if (!hasFault) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }
}


void Lexium32::controlsGui() {
    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Device Mode");
    ImGui::PopFont();

    if (ImGui::BeginCombo("##ModeSelector", getOperatingMode(actualOperatingMode)->displayName, ImGuiComboFlags_HeightLargest)) {
        for (OperatingMode& availableMode : availableOperatingModes) {
            if (ImGui::Selectable(availableMode.displayName, actualOperatingMode == availableMode.mode)) requestedOperatingMode = availableMode.mode;
        }
        ImGui::EndCombo();
    }


    if (actualOperatingMode == OperatingMode::Mode::UNKNOWN) {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
        ImGui::TextWrapped("Current Operating Mode is Unknown");
        ImGui::PopStyleColor();
    }
    else if (actualOperatingMode == OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION) {
        ImGui::TextWrapped("In this mode, the drive is controlled by the position command input. No manual controls are available.");
    }
    else if (actualOperatingMode == OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY) {
        ImGui::TextWrapped("This mode is exclusively designed for manual drive velocity control. When the drive is in this mode, it will only respond to the following velocity parameters.");
        float maxV = maxVelocity_rpm;
        ImGui::SliderFloat("##manualVelocity", &manualVelocityCommand_rpm, -maxVelocity_rpm, maxVelocity_rpm, "%.3frpm");
        float velocityFraction = (actualVelocity->getReal() + maxVelocity_rpm) / (2 * maxVelocity_rpm);
        static char actualVelocityString[32];
        sprintf(actualVelocityString, "%.1frpm", actualVelocity->getReal());
        ImGui::ProgressBar(velocityFraction, ImGui::GetItemRectSize(), actualVelocityString);
        ImGui::InputFloat("##manualAcceleration", &manualAcceleration_rpm2, 0.0, (float)maxAcceleration_rpm2, "Acceleration: %.3f rpm2");
        if (manualAcceleration_rpm2 > maxAcceleration_rpm2) manualAcceleration_rpm2 = maxAcceleration_rpm2;
        if (ImGui::Button("Stop Movement", glm::vec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeight() * 2.0))) manualVelocityCommand_rpm = 0.0;
    }
    else if (actualOperatingMode == OperatingMode::Mode::TUNING) {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
        ImGui::TextWrapped("Tuning Mode is not yet supported");
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
        ImGui::TextWrapped("Current Operating Mode is Not Supported");
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Miscellaneous Device Status Flags");
    ImGui::PopFont();

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
}







void Lexium32::limitsGui() {
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

    bool disableUploadButton = !isDetected();
    if (disableUploadButton) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    }
    if (ImGui::Button("Upload Setting")) {
        std::thread maxCurrentUploader([this]() {
            b_uploadingMaxCurrent = true;
            EtherCatCoeData CTRL_I_max(0x3011, 0xC, EtherCatData::Type::UINT16_T);
            uint16_t current = maxCurrent_amps * 100.0;
            CTRL_I_max.set(current);
            if (CTRL_I_max.write(getSlaveIndex())) {
                b_uploadMaxCurrentSuccess = true;
                Logger::info("Successfully uploaded Max Current Value");
            }
            else {
                b_uploadMaxCurrentSuccess = false;
                Logger::warn("Failed to upload Max Current Value");
            }
            b_uploadingMaxCurrent = false;
        });
        maxCurrentUploader.detach();
    }
    if (!b_uploadingMaxCurrent) {
        ImGui::SameLine();
        ImGui::Text(b_uploadMaxCurrentSuccess ? "Upload Success" : "Upload Failed");
    }
    if (disableUploadButton) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Limit Signals");
    ImGui::PopFont();


    static auto showInputPinFunctionCombo = [](InputPinFunction::Type& pinFunctionType) {
        if (ImGui::BeginCombo("##DI", getInputPinFunction(pinFunctionType)->displayName)) {
            for (InputPinFunction& function : inputPinFunctions) {
                if (ImGui::Selectable(function.displayName, pinFunctionType == function.type)) {
                    pinFunctionType = function.type;
                }
            }
            ImGui::EndCombo();
        }
    };

    ImGui::Text("Input 0 (DI0)");
    ImGui::PushID("DI0");
    showInputPinFunctionCombo(inputPin0Function);
    ImGui::PopID();

    ImGui::Text("Input 1 (DI1)");
    ImGui::PushID("DI1");
    showInputPinFunctionCombo(inputPin1Function);
    ImGui::PopID();

    ImGui::Text("Input 2 (DI2)");
    ImGui::PushID("DI2");
    showInputPinFunctionCombo(inputPin2Function);
    ImGui::PopID();

    ImGui::Text("Input 3 (DI3)");
    ImGui::PushID("DI3");
    showInputPinFunctionCombo(inputPin3Function);
    ImGui::PopID();

    ImGui::Text("Input 4 (DI4)");
    ImGui::PushID("DI4");
    showInputPinFunctionCombo(inputPin4Function);
    ImGui::PopID();

    ImGui::Text("Input 5 (DI5)");
    ImGui::PushID("DI5");
    showInputPinFunctionCombo(inputPin5Function);
    ImGui::PopID();

    ImGui::TextWrapped("Settings can only be uploaded while the power stage is disabled");

    if (disableUploadButton) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    }
    if (ImGui::Button("Upload Settings")) {
        std::thread inputPinAssigmentUploader([this]() {
            auto setPinAssigement = [](InputPinFunction::Type function, uint8_t subindex, int slaveIndex) -> bool {
                EtherCatCoeData IOfunct_DIx(0x3007, subindex, EtherCatData::Type::UINT16_T);
                uint16_t value;
                switch (function) {
                    case InputPinFunction::Type::NEGATIVE_LIMIT:
                        value = 23;
                        IOfunct_DIx.set(value);
                        break;
                    case InputPinFunction::Type::POSITIVE_LIMIT:
                        value = 22;
                        IOfunct_DIx.set(value);
                        break;
                    case InputPinFunction::Type::UNASSIGNED:
                        value = 1;
                        IOfunct_DIx.set(value);
                        break;
                }
                return IOfunct_DIx.write(slaveIndex);
            };
            b_uploadingPinAssignements = true;
            b_uploadPinAssignementSuccess = true;
            if(!setPinAssigement(inputPin0Function, 0x1, getSlaveIndex())) { b_uploadPinAssignementSuccess = false; Logger::warn("Failed To Upload Input Pin 0 Assignement"); }
            if(!setPinAssigement(inputPin1Function, 0x2, getSlaveIndex())) { b_uploadPinAssignementSuccess = false; Logger::warn("Failed To Upload Input Pin 1 Assignement"); }
            if(!setPinAssigement(inputPin2Function, 0x3, getSlaveIndex())) { b_uploadPinAssignementSuccess = false; Logger::warn("Failed To Upload Input Pin 2 Assignement"); }
            if(!setPinAssigement(inputPin3Function, 0x4, getSlaveIndex())) { b_uploadPinAssignementSuccess = false; Logger::warn("Failed To Upload Input Pin 3 Assignement"); }
            if(!setPinAssigement(inputPin4Function, 0x5, getSlaveIndex())) { b_uploadPinAssignementSuccess = false; Logger::warn("Failed To Upload Input Pin 4 Assignement"); }
            if(!setPinAssigement(inputPin5Function, 0x6, getSlaveIndex())) { b_uploadPinAssignementSuccess = false; Logger::warn("Failed To Upload Input Pin 5 Assignement"); }
            b_uploadingPinAssignements = false;
            Logger::info("Successfully Uploaded Input Pin Assignement");
        });
        inputPinAssigmentUploader.detach();
    }
    if (!b_uploadingPinAssignements) {
        ImGui::SameLine();
        ImGui::Text(b_uploadPinAssignementSuccess ? "Upload Success" : "Upload Failed");
    }
    if (disableUploadButton) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }
}







void Lexium32::feedbackConfigurationGui() {

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Encoder Settings");
    ImGui::PopFont();

    ImGui::Text("Used Encoder");
    if (ImGui::BeginCombo("##encoderAssignement", getEncoderAssignement(encoderAssignement)->displayName)) {
        for (EncoderAssignement& assignement : encoderAssignements) {
            if (ImGui::Selectable(assignement.displayName, assignement.type == encoderAssignement)) {
                encoderAssignement = assignement.type;
            }
        }
        ImGui::EndCombo();
    }

    if (encoderAssignement == EncoderAssignement::Type::INTERNAL_ENCODER) {

        ImGui::Text("Encoder increments per motor shaft revolution");
        ImGui::InputInt("##incrementsPerRotation", &encoderIncrementsPerShaftRotation, 0, 0);
        ImGui::Text("Encoder Multiturn Resolution");
        ImGui::InputInt("##MtResolution", &encoderMultiturnResolution, 0, 0);

    }
    else if (encoderAssignement == EncoderAssignement::Type::ENCODER_MODULE) {

        if (ImGui::Button("Detect Module Type")) {}

        ImGui::PushFont(Fonts::robotoBold15);
        ImGui::Text("%s detected", getEncoderModule(encoderModuleType)->displayName);
        ImGui::PopFont();

        switch (encoderModuleType) {
        case EncoderModule::Type::ANALOG_MODULE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("Analog Encoder Modules are not yet supported.");
            ImGui::PopStyleColor();
            break;
        case EncoderModule::Type::DIGITAL_MODULE:

            if (ImGui::BeginCombo("##EncoderType", getEncoderType(encoderType)->displayName)) {
                for (EncoderType& encoder : encoderTypes) {
                    if (ImGui::Selectable(encoder.displayName, encoder.type == encoderType)) {
                        encoderType = encoder.type;
                    }
                }
                ImGui::EndCombo();
            }

            switch (encoderType) {
            case EncoderType::Type::SSI_ROTARY:
                ImGui::Text("Single Turn Resolution (bits)");
                ImGui::InputScalar("##STres", ImGuiDataType_U8, &singleTurnResolutionBits);
                ImGui::Text("Multi Turn Resolution (bits)");
                ImGui::InputScalar("##MTres", ImGuiDataType_U8, &multiTurnResoltuionBits);
                ImGui::Text("Encoder Revolutions per Motor Shaft Revolution");
                ImGui::InputDouble("#ratio", &encoderRevolutionsPerMotorShaftRevolution);
                break;
            case EncoderType::Type::NONE:
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
                ImGui::TextWrapped("Other Encoder Types are not yet supported");
                ImGui::PopStyleColor();
                break;
            }
            break;
        case EncoderModule::Type::RESOLVER_MODULE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("Resolver Encoder Modules are not yet supported.");
            ImGui::PopStyleColor();
            break;
        case EncoderModule::Type::NONE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("No Encoder Module was detected in module slot 2.");
            ImGui::PopStyleColor();
            break;
        }

    }
}