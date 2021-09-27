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
        ImGui::Separator();

        if (ImGui::BeginTabBar("LexiumTabBar")) {
            if (ImGui::BeginTabItem("Control")) {
                controlsGui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Limits")) {
                limitsGui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Encoder")) {
                encoderGui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tuning")) {
                tuningGui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Misc")) {
                miscellaneousGui();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }



        ImGui::EndTabItem();
    }
}



void Lexium32::statusGui() {

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
    bool isOffline = !isOnline();
    if (isOffline) ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    if (!isOnline()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
    else if (isStateBootstrap() || isStateInit()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
    else if (isStatePreOperational() || isStateSafeOperational()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
    else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
    ImGui::Button(getEtherCatStateChar(), statusDisplaySize);
    ImGui::PopStyleColor();
    if (isOffline) ImGui::PopStyleColor();

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

    if (isEmergencyStopActive()) {
        int millis = Timing::getTime_seconds() * 1000.0;
        if (millis % 1000 < 500) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
        else ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkRed);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushFont(Fonts::robotoBold15);
        ImGui::Button("E-STOP", commandButtonSize);
        ImGui::SameLine();
        ImGui::Button("E-STOP", commandButtonSize);
        ImGui::PopFont();
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }
    else {
        if (isEnabled()) { if (ImGui::Button("Disable Operation", commandButtonSize)) disable(); }
        else { if (ImGui::Button("Enable Operation", commandButtonSize)) enable(); }
    }
    ImGui::SameLine();
    if (ImGui::Button("Quick Stop", commandButtonSize)) quickStop();
}





void Lexium32::controlsGui() {
    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Device Mode");
    ImGui::PopFont();

    bool disableModeSwitch = !isOnline();

    if (disableModeSwitch) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    }

    if (ImGui::BeginCombo("##ModeSelector", getOperatingMode(actualOperatingMode)->displayName, ImGuiComboFlags_HeightLargest)) {
        for (OperatingMode& availableMode : availableOperatingModes) {
            if (ImGui::Selectable(availableMode.displayName, actualOperatingMode == availableMode.mode)) requestedOperatingMode = availableMode.mode;
            if (ImGui::IsItemHovered()) {
                ImGui::SetNextWindowSize(glm::vec2(ImGui::GetTextLineHeight() * 20.0, 0));
                ImGui::BeginTooltip();
                switch (availableMode.mode) {
                case OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION:
                    ImGui::TextWrapped("Used for position control. No manual controls are available. The drive is completely controlled by the position command.");
                    break;
                case OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY:
                    ImGui::TextWrapped("Used exclusively for manual velocity control. When the drive is in this mode, it will only respond to the modes parameters.");
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
                    ImGui::TextWrapped("Caution: Except for the drives internal limits signals, any position and collision limits will be ignored.");
                    ImGui::PopStyleColor();
                    break;
                }
                ImGui::EndTooltip();
            }
        }
        ImGui::EndCombo();
    }

    if (disableModeSwitch) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }

    ImGui::Separator();


    if (actualOperatingMode == OperatingMode::Mode::UNKNOWN) {

    }
    else if (actualOperatingMode == OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION) {
        float velocityFraction = (actualVelocity->getReal() + maxVelocity_rpm) / (2 * maxVelocity_rpm);
        static char actualVelocityString[32];
        sprintf(actualVelocityString, "%.1frpm", actualVelocity->getReal());
        ImGui::ProgressBar(velocityFraction, ImVec2(ImGui::GetContentRegionAvail().x, 0), actualVelocityString);
    }
    else if (actualOperatingMode == OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY) {
        float maxV = maxVelocity_rpm;
        float vCommand_rpm = manualVelocityCommand_rpm;

        bool disableManualControls = !isEnabled();
        if (disableManualControls) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
        }
        ImGui::SliderFloat("##manualVelocity", &vCommand_rpm, -maxVelocity_rpm, maxVelocity_rpm, "%.3frpm");
        if (vCommand_rpm > maxVelocity_rpm) vCommand_rpm = maxVelocity_rpm;
        else if (vCommand_rpm < -maxVelocity_rpm) vCommand_rpm = -maxAcceleration_rps2;
        manualVelocityCommand_rpm = vCommand_rpm;
        float velocityFraction = (actualVelocity->getReal() + maxVelocity_rpm) / (2 * maxVelocity_rpm);
        static char actualVelocityString[32];
        sprintf(actualVelocityString, "%.1frpm", actualVelocity->getReal());
        ImGui::ProgressBar(velocityFraction, ImGui::GetItemRectSize(), actualVelocityString);
        ImGui::InputFloat("##manualAcceleration", &manualAcceleration_rps2, 0.0, (float)maxAcceleration_rps2, "Acceleration: %.3f rps2");
        if (manualAcceleration_rps2 > maxAcceleration_rps2) manualAcceleration_rps2 = maxAcceleration_rps2;
        if (ImGui::Button("Stop Movement", glm::vec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeight() * 2.0))) manualVelocityCommand_rpm = 0.0;
        if (disableManualControls) {
            ImGui::PopItemFlag();
            ImGui::PopStyleColor();
        }
    }
}







void Lexium32::limitsGui() {
    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Profile Generator Limits");
    ImGui::PopFont();

    ImGui::Text("Max Velocity");
    ImGui::InputDouble("##maxV", &maxVelocity_rpm, 0.0, 0.0, "%.1frpm");
    ImGui::Text("Max Acceleration");
    ImGui::InputDouble("##maxA", &maxAcceleration_rps2, 0.0, 0.0, "%.1frps2");

    ImGui::Separator();

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Load Limit");
    ImGui::PopFont();

    ImGui::Text("Max Current");
    if (ImGui::InputDouble("##maxI", &maxCurrent_amps, 0.0, 0.0, "%.1f Amperes")) maxCurrentParameterTransferState = DataTransferState::State::NO_TRANSFER;

    bool disableUploadButton = !isDetected();
    if (disableUploadButton) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    }
    if (ImGui::Button("Upload Setting")) {
        std::thread maxCurrentUploader([this]() { uploadMaxCurrentParameters(); });
        maxCurrentUploader.detach();
    }
    if (disableUploadButton) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(maxCurrentParameterTransferState)->displayName);

    ImGui::Separator();

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Limit Signals");
    ImGui::PopFont();


    static auto showInputPinFunctionCombo = [this](InputPinFunction::Type& pinFunctionType) {
        if (ImGui::BeginCombo("##DI", getInputPinFunction(pinFunctionType)->displayName)) {
            for (InputPinFunction& function : inputPinFunctions) {
                if (ImGui::Selectable(function.displayName, pinFunctionType == function.type)) {
                    pinFunctionType = function.type;
                    pinAssignementParameterTransferState = DataTransferState::State::NO_TRANSFER;
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

    ImGui::TextWrapped("Settings can only be uploaded while the power stage is disabled. The Drive needs to be rebooted after uploading new settings.");

    if (disableUploadButton) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    }
    if (ImGui::Button("Upload Settings")) {
        std::thread pinAssigmentUploader([this]() { uploadPinAssignementParameters(); });
        pinAssigmentUploader.detach();
    }
    if (disableUploadButton) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(pinAssignementParameterTransferState)->displayName);
}







void Lexium32::encoderGui() {

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Encoder Settings");
    ImGui::PopFont();

    ImGui::Text("Main Encoder:");
    if (ImGui::BeginCombo("##encoderAssignement", getEncoderAssignement(encoderAssignement)->displayName)) {
        for (EncoderAssignement& assignement : encoderAssignements) {
            if (ImGui::Selectable(assignement.displayName, assignement.type == encoderAssignement)) {
                encoderAssignement = assignement.type;
            }
        }
        ImGui::EndCombo();
    }

    if (encoderAssignement == EncoderAssignement::Type::INTERNAL_ENCODER) {

        ImGui::Separator();

        ImGui::PushFont(Fonts::robotoBold20);
        ImGui::Text("Internal Encoder");
        ImGui::PopFont();

        ImGui::Text("Encoder increments per motor shaft revolution");
        ImGui::InputInt("##incrementsPerRotation", &encoderIncrementsPerShaftRotation, 0, 0);
        ImGui::Text("Encoder Multiturn Resolution");
        ImGui::InputInt("##MtResolution", &encoderMultiturnResolution, 0, 0);

    }
    else if (encoderAssignement == EncoderAssignement::Type::ENCODER_MODULE) {
       
        ImGui::SameLine();
        if (ImGui::Button("Detect Module")) {
            std::thread encoderModuleDetector([this]() { detectEncoderModule(); });
            encoderModuleDetector.detach();
        }

        ImGui::Separator();

        ImGui::PushFont(Fonts::robotoBold20);
        ImGui::Text("%s", getEncoderModule(encoderModuleType)->displayName);
        ImGui::PopFont();

        switch (encoderModuleType) {
        case EncoderModule::Type::DIGITAL_MODULE:

            ImGui::Text("Encoder Type:");
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

                ImGui::Text("Coding");
                if (ImGui::BeginCombo("##Encoding", getEncoderCoding(encoderCoding)->displayName)) {
                    for (auto& coding : encoderCodings) {
                        if(ImGui::Selectable(coding.displayName, encoderCoding == coding.type)) encoderCoding = coding.type;
                    }
                    ImGui::EndCombo();
                }
                ImGui::Text("Voltage");
                if (ImGui::BeginCombo("##Voltage", getEncoderVoltage(encoderVoltage)->displayName)) {
                    for (auto& voltage : encoderVoltages) {
                        if (ImGui::Selectable(voltage.displayName, encoderVoltage == voltage.voltage)) encoderVoltage = voltage.voltage;
                    }
                    ImGui::EndCombo();
                }

                ImGui::Text("Single Turn Resolution (bits)");
                ImGui::InputScalar("##STres", ImGuiDataType_U8, &encoder2_singleTurnResolutionBits);
                ImGui::Text("Multi Turn Resolution (bits)");
                ImGui::InputScalar("##MTres", ImGuiDataType_U8, &encoder2_multiTurnResolutionBits);
                ImGui::Text("Full Encoder Revolution amount per");
                ImGui::InputInt("##ratio", &encoder2_encoderRevolutionsPer);
                ImGui::Text("Per Full Motor Revolution amount");
                ImGui::InputInt("##ratio2", &encoder2_perMotorRevolutions);
                ImGui::Text("Invert Encoder Motion Direction");
                ImGui::Checkbox("##invert", &encoder2_invertDirection);
                ImGui::Text("Max Motor Encoder Deviation from Module Encoder (motor revolutions)");
                ImGui::InputFloat("##maxdev", &encoder2_maxDifferenceToMotorEncoder_rotations);
                break;
            case EncoderType::Type::NONE:
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
                ImGui::TextWrapped("Other Encoder Types are not yet supported");
                ImGui::PopStyleColor();
                break;
            }
            break;
        case EncoderModule::Type::ANALOG_MODULE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("Analog Encoder Modules are not yet supported.");
            ImGui::PopStyleColor();
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

    if (ImGui::Button("Upload Settings")) {
        std::thread encoderSettingsUploader([this]() {
            setEncoderSettings();
        });
        encoderSettingsUploader.detach();
    }
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(encoderSettingsTransferState)->displayName);
}


void Lexium32::tuningGui() {

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Automatic Drive Controller Tuning");
    ImGui::PopFont();

    ImGui::TextWrapped("This functionnality is used to automatically tune the drive motion controller to the specific loads of the system. "
                      "Auto Tuning should be executed once all loads have been installed."
                      "\nAuto Tuning can only be started while the device is detected and not while the whole system is running.");

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
    ImGui::TextWrapped("CAUTION: In this mode, the motor will automatically move back and forth. Before starting auto tuning, ensure the motor has space to move in both directions and always have an emergency stop button ready.");
    ImGui::PopStyleColor();

    ImGui::Separator();

    if (isAutoTuning() || b_autoTuningSucceeded) {
        if (isAutoTuning()) {
            if (ImGui::Button("Stop Auto Tuning")) stopAutoTuning();
            ImGui::ProgressBar(tuningProgress);
        }
        else {
            if (ImGui::Button("Restart Auto Tuning")) startAutoTuning();
        }
        if (b_autoTuningSucceeded) {
            ImGui::PushFont(Fonts::robotoBold15);
            ImGui::Text("Auto Tuning Succeeded");
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Text("(%s)", getDataTransferState(autoTuningSaveState)->displayName);
        }

        if(b_autoTuningSucceeded){
            ImGui::Separator();
            ImGui::PushFont(Fonts::robotoBold15);
            ImGui::Text("Tuning Results:");
            ImGui::PopFont();
            ImGui::Text("Friction Torque: %.2f Amperes", tuning_frictionTorque_amperes);
            ImGui::Text("Constant Load Torque: %.2f Amperes", tuning_constantLoadTorque_amperes);
            ImGui::Text("Moment of Inertia: %.1f kg*cm2", tuning_momentOfInertia_kilogramcentimeter2);
        }
    }
    else {
        bool disableButton = isOnline() || !isDetected();
        if (disableButton) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
        }
        if (ImGui::Button("Start Auto Tuning")) startAutoTuning();
        if (disableButton) {
            ImGui::PopItemFlag();
            ImGui::PopStyleColor();
        }
    }
}


void Lexium32::miscellaneousGui() {
    
    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Station Alias Assignement");
    ImGui::PopFont();
    ImGui::TextWrapped("Assigns a new station alias to the drive. The alias becomes active after the drive has been restarted.");
    static uint16_t newStationAlias = 0;
    ImGui::InputScalar("##alias", ImGuiDataType_U16, &newStationAlias);
    if (ImGui::Button("Set Station Alias")) {
        std::thread stationAliasSettingHandler([this]() {
            setStationAlias(newStationAlias);
            });
        stationAliasSettingHandler.detach();
    }
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(stationAliasUploadState)->displayName);

    ImGui::Separator();
    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Factory Reset");
    ImGui::PopFont();
    ImGui::TextWrapped("Resets all internal drive settings to the original factory configuration.");
    if (ImGui::Button("Reset Lexium32 to Factory Settings")) {
        std::thread factoryResetHandler([this]() {
            factoryReset();
        });
        factoryResetHandler.detach();
    }
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(factoryResetTransferState)->displayName);
    
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