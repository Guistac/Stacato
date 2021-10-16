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
                if (ImGui::BeginChild("Control")) {
                    controlsGui();
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("General")) {
                if (ImGui::BeginChild("General")) {
                    generalGui();
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("I/O")) {
                if (ImGui::BeginChild("I/O")) {
                    gpioGui();
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Encoder")) {
                if (ImGui::BeginChild("Encoder")) {
                    encoderGui();
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tuning")) {
                if (ImGui::BeginChild("Tuning")) {
                    tuningGui();
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Other")) {
                if (ImGui::BeginChild("Other")) {
                    miscellaneousGui();
                    ImGui::EndChild();
                }
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
    if (!isDetected()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
    else if (isStateBootstrap() || isStateInit()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
    else if (isStatePreOperational() || isStateSafeOperational()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
    else ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
    ImGui::Button(getEtherCatStateChar(), statusDisplaySize);
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
        ImGui::Button("No Status", statusDisplaySize);
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
        bool disableCommandButton = !isOnline();
        if (disableCommandButton) BEGIN_DISABLE_IMGUI_ELEMENT
        if (isEnabled()) { if (ImGui::Button("Disable Operation", commandButtonSize)) disable(); }
        else { if (ImGui::Button("Enable Operation", commandButtonSize)) enable(); }
        ImGui::SameLine();
        if (ImGui::Button("Quick Stop", commandButtonSize)) quickStop();
        if (disableCommandButton) END_DISABLE_IMGUI_ELEMENT
    }
}











void Lexium32::controlsGui() {
    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Device Mode");
    ImGui::PopFont();

    bool disableModeSwitch = !isOnline();
    if (disableModeSwitch) BEGIN_DISABLE_IMGUI_ELEMENT
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
    float widgetWidth = ImGui::GetItemRectSize().x;
    if (disableModeSwitch) END_DISABLE_IMGUI_ELEMENT

    float maxV = servoMotorDevice->velocityLimit_positionUnitsPerSecond;
    float maxA = servoMotorDevice->accelerationLimit_positionUnitsPerSecondSquared;

    switch (actualOperatingMode) {
        case OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY: {
            float vCommand_rps = manualVelocityCommand_rps;
            bool disableManualControls = !isEnabled();
            if (disableManualControls) BEGIN_DISABLE_IMGUI_ELEMENT
            ImGui::SliderFloat("##manualVelocity", &vCommand_rps, -maxV, maxV, "%.1frps");
            if (!ImGui::IsItemActive()) vCommand_rps = 0.0; //only set the command if the slider is held down
            if (vCommand_rps > maxV) vCommand_rps = maxV;
            else if (vCommand_rps < -maxV) vCommand_rps = -maxV;
            manualVelocityCommand_rps = vCommand_rps;
            ImGui::InputFloat("##manualAcceleration", &manualAcceleration_rpsps, 0.0, maxA, "Acceleration: %.3f rps/s");
            if (manualAcceleration_rpsps > maxA) manualAcceleration_rpsps = maxA;
            if (disableManualControls) END_DISABLE_IMGUI_ELEMENT
        }break;
        default: break;
    }


    ImGui::Separator();

    //------------------------- FEEDBACK ------------------------

    float velocityFraction;
    static char actualVelocityString[32];
    if (!isReady()) {
        sprintf(actualVelocityString, "Not Ready");
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
        velocityFraction = 1.0;
    }
    else {
        double velocity = servoMotorDevice->getVelocity();
        sprintf(actualVelocityString, "%.1f rps", velocity);
        velocityFraction = std::abs(velocity) / maxV;
        if(velocityFraction >= 1.0) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
        else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
    }
    ImGui::Text("Motor Velocity :");
    ImGui::ProgressBar(velocityFraction, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), actualVelocityString);
    ImGui::PopStyleColor();

    char rangeString[64];

    double range = servoMotorDevice->getPositionInRange();
    if (!isReady()) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
        range = 1.0;
        sprintf(rangeString, "Not ready");
    }
    else if (range < 1.0 && range > 0.0) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
        sprintf(rangeString, "%.3f%%", range * 100.0);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
        sprintf(rangeString, "Encoder Outside Working Range ! (%.3f%%)", range * 100.0);
        range = 1.0;
    }

    ImGui::Text("Encoder Position in Working Range :");
    ImGui::ProgressBar(range, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), rangeString);
    ImGui::PopStyleColor();

    if (servoMotorDevice->isReady()) {
        ImGui::Text("Current Position is %.3f revolutions", servoMotorDevice->positionRaw_positionUnits);
        ImGui::Text("Range is from %.3f to %.3f revolutions", servoMotorDevice->rangeMin_positionUnits, servoMotorDevice->rangeMax_positionUnits);
    }



}












void Lexium32::generalGui() {
    bool disableTransferButton = !isDetected();

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Profile Generator Limits");
    ImGui::PopFont();

    ImGui::TextWrapped("These settings are not stored on the drive, but regulate the speed and position commands sent to the drive.");

    if (maxMotorVelocity_rps != 0.0) {
        ImGui::TextWrapped("The Maximum Velocity of the Motor is %.3f rotations per second ", maxMotorVelocity_rps);
        if (servoMotorDevice->velocityLimit_positionUnitsPerSecond > maxMotorVelocity_rps) servoMotorDevice->velocityLimit_positionUnitsPerSecond = maxMotorVelocity_rps;
    }

    ImGui::Text("Velocity Limit");
    ImGui::InputDouble("##maxV", &servoMotorDevice->velocityLimit_positionUnitsPerSecond, 0.0, 0.0, "%.1f rev/s");
    ImGui::Text("Acceleration Limit");
    ImGui::InputDouble("##maxA", &servoMotorDevice->accelerationLimit_positionUnitsPerSecondSquared, 0.0, 0.0, u8"%.1f rev/s²");

    ImGui::Text("Default Manual Acceleration");
    ImGui::InputFloat("##defmaxacc", &defaultManualAcceleration_rpsps, 0.0, 0.0, u8"%.1f rev/s²");

    ImGui::Separator();

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("General Settings");
    ImGui::PopFont();

    if (disableTransferButton) BEGIN_DISABLE_IMGUI_ELEMENT
    if (ImGui::Button("Download Settings From Drive")) {
        std::thread maxCurrentDownloader([this]() { downloadGeneralParameters(); });
        maxCurrentDownloader.detach();
    }
    if (disableTransferButton) END_DISABLE_IMGUI_ELEMENT
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(generalParameterDownloadState)->displayName);

    ImGui::TextWrapped("General Settings that are stored on the drive.");

    ImGui::Checkbox("##dir", &b_invertDirectionOfMotorMovement);
    ImGui::SameLine();
    ImGui::Text("Invert Direction of Movement");

    ImGui::Text("Max Current");
    if (ImGui::InputDouble("##maxI", &maxCurrent_amps, 0.0, 0.0, "%.1f Amperes")) generalParameterUploadState = DataTransferState::State::NO_TRANSFER;
    
    ImGui::Text("Max Quickstop Current");
    if (ImGui::InputDouble("##maxqi", &maxQuickstopCurrent_amps, 0.0, 0.0, "%.1f Amperes")) generalParameterUploadState = DataTransferState::State::NO_TRANSFER;

    if (disableTransferButton) BEGIN_DISABLE_IMGUI_ELEMENT
    if (ImGui::Button("Upload Setting")) {
        std::thread maxCurrentUploader([this]() { uploadGeneralParameters(); });
        maxCurrentUploader.detach();
    }
    if (disableTransferButton) END_DISABLE_IMGUI_ELEMENT
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(generalParameterUploadState)->displayName);
}











void Lexium32::gpioGui() {
    
    bool disableTransferButton = !isDetected();

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Limit Signals");
    ImGui::PopFont();



    if (disableTransferButton) BEGIN_DISABLE_IMGUI_ELEMENT
    if (ImGui::Button("Download Settings from Drive")) {
        std::thread pinAssigmentDownloader([this]() { downloadPinAssignements(); });
        pinAssigmentDownloader.detach();
    }
    if (disableTransferButton) END_DISABLE_IMGUI_ELEMENT
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(pinAssignementDownloadState)->displayName);

    ImGui::Text("Negative Limit Switch");
    if (ImGui::BeginCombo("##negativeLimitSwitch", getInputPin(negativeLimitSwitchPin)->displayName)) {
        for (auto& inputPin : getInputPins()) {
            if (positiveLimitSwitchPin != InputPin::Pin::NONE && positiveLimitSwitchPin == inputPin.pin) continue;
            if(ImGui::Selectable(inputPin.displayName, negativeLimitSwitchPin == inputPin.pin)) negativeLimitSwitchPin = inputPin.pin;
        }
        ImGui::EndCombo();
    }
    if (negativeLimitSwitchPin != InputPin::Pin::NONE) {
        ImGui::Checkbox("##negsig", &b_negativeLimitSwitchNormallyClosed);
        ImGui::SameLine();
        ImGui::Text("Normally Closed");
    }

    ImGui::Text("Positive Limit Switch");
    if (ImGui::BeginCombo("##positiveLimitSwitch", getInputPin(positiveLimitSwitchPin)->displayName)) {
        for (auto& inputPin : getInputPins()) {
            if (negativeLimitSwitchPin != InputPin::Pin::NONE && negativeLimitSwitchPin == inputPin.pin) continue;
            if (ImGui::Selectable(inputPin.displayName, positiveLimitSwitchPin == inputPin.pin)) positiveLimitSwitchPin = inputPin.pin;
        }
        ImGui::EndCombo();
    }
    if (positiveLimitSwitchPin != InputPin::Pin::NONE) {
        ImGui::Checkbox("##possig", &b_positiveLimitSwitchNormallyClosed);
        ImGui::SameLine();
        ImGui::Text("Normally Closed");
    }

    ImGui::TextWrapped("Output Pins are set to be freely available by default."
                       "\nSettings can only be uploaded while the drive is disabled. The Drive needs to be rebooted after uploading new settings.");

    if (disableTransferButton) BEGIN_DISABLE_IMGUI_ELEMENT
    if (ImGui::Button("Upload Settings")) {
        std::thread pinAssigmentUploader([this]() { uploadPinAssignements(); });
        pinAssigmentUploader.detach();
    }
    if (disableTransferButton) END_DISABLE_IMGUI_ELEMENT
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(pinAssignementUploadState)->displayName);
}




void Lexium32::encoderGui() {

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Encoder Settings");
    ImGui::PopFont();

    ImGui::TextWrapped("All Encoder settings are stored in the drive."
                        "\nFor the settings to take effect, the drive has to be restared. "
                        "\nChanging these settings will invalidate all current position references of the corresponding machine, homing procedures will need to be reexecuted.");

    if (ImGui::Button("Download Settings From Drive")) {
        std::thread encoderSettingsDownloader([this]() { downloadEncoderSettings(); });
        encoderSettingsDownloader.detach();
    }
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(encoderSettingsDownloadState)->displayName);

    ImGui::Separator();

    ImGui::Text("Main Encoder used for absolute positionning:");
    if (ImGui::BeginCombo("##encoderAssignement", getEncoderAssignement(encoderAssignement)->displayName)) {
        for (EncoderAssignement& assignement : encoderAssignements) {
            if (ImGui::Selectable(assignement.displayName, assignement.type == encoderAssignement)) {
                encoderAssignement = assignement.type;
            }
        }
        ImGui::EndCombo();
    }

    float doublewidgetWidth = (ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
    bool disableEncoderUploadButton = false;

    if (encoderAssignement == EncoderAssignement::Type::INTERNAL_ENCODER) {

        ImGui::Separator();
        ImGui::PushFont(Fonts::robotoBold20);
        ImGui::Text("Internal Encoder");
        ImGui::PopFont();
        ImGui::TextWrapped("The Resolution of the internal motor encoder is 17 bits singleturn, 12 bits multiturn.");

    }
    else if (encoderAssignement == EncoderAssignement::Type::ENCODER_MODULE) {
       
        bool disableDetectButton = !isDetected();
        if (disableDetectButton) BEGIN_DISABLE_IMGUI_ELEMENT
        ImGui::SameLine();
        if (ImGui::Button("Detect Module")) {
            std::thread encoderModuleDetector([this]() { detectEncoderModule(); });
            encoderModuleDetector.detach();
        }
        if(disableDetectButton) END_DISABLE_IMGUI_ELEMENT

        ImGui::Separator();

        ImGui::PushFont(Fonts::robotoBold20);
        ImGui::Text("%s", getEncoderModule(encoderModuleType)->displayName);
        ImGui::PopFont();

        switch (encoderModuleType) {
        case EncoderModule::Type::DIGITAL_MODULE:

            ImGui::Text("Digital Encoder Type:");
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

                ImGui::Text("Encoder Revolution to Motor Revolution Ratio");
                ImGui::SetNextItemWidth(doublewidgetWidth);
                ImGui::InputInt("##ratio", &encoder2_EncoderToMotorRatioNumerator);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(doublewidgetWidth);
                ImGui::InputInt("##ratio2", &encoder2_EncoderToMotorRatioDenominator);

                if (encoder2_EncoderToMotorRatioNumerator < 1) encoder2_EncoderToMotorRatioNumerator = 1;
                if (encoder2_EncoderToMotorRatioDenominator < 1) encoder2_EncoderToMotorRatioDenominator = 1;

                ImGui::Checkbox("##invert", &encoder2_invertDirection);
                ImGui::SameLine();
                ImGui::Text("Invert Encoder Motion Direction");
                ImGui::Text("Max Motor Encoder Deviation from Module Encoder");
                ImGui::InputDouble("##maxdev", &encoder2_maxDifferenceToMotorEncoder_rotations, 0.0, 0.0,"%.2f motor revolutions");
                break;
            case EncoderType::Type::NONE:
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
                ImGui::TextWrapped("Other Encoder Types are not yet supported");
                ImGui::PopStyleColor();
                disableEncoderUploadButton = true;
                break;
            }
            break;
        case EncoderModule::Type::ANALOG_MODULE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("Analog Encoder Modules are not yet supported.");
            ImGui::PopStyleColor();
            disableEncoderUploadButton = true;
            break;
        case EncoderModule::Type::RESOLVER_MODULE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("Resolver Encoder Modules are not yet supported.");
            ImGui::PopStyleColor();
            disableEncoderUploadButton = true;
            break;
        case EncoderModule::Type::NONE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("No Encoder Module was detected in module slot 2."
                                "\nRestart the drive and detect the module again. Never insert or remove modules while the drive is powered on.");
            ImGui::PopStyleColor();
            disableEncoderUploadButton = true;
            break;
        }
    }

    ImGui::Spacing();
    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Shift Encoder Working Range");
    ImGui::PopFont();
    ImGui::Checkbox("##shifting", &b_encoderRangeShifted);
    ImGui::SameLine();
    ImGui::TextWrapped("Center the encoder working range around 0.");
    float low, high;
    getEncoderWorkingRange(low, high);
    ImGui::Text("Working Range : %.1f to %.1f motor revolutions", low, high);

    if (disableEncoderUploadButton) BEGIN_DISABLE_IMGUI_ELEMENT
    if (ImGui::Button("Upload Encoder Settings")) {
        std::thread encoderSettingsUploader([this]() {
            uploadEncoderSettings();
        });
        encoderSettingsUploader.detach();
    }
    if (disableEncoderUploadButton) END_DISABLE_IMGUI_ELEMENT
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(encoderSettingsUploadState)->displayName);

    ImGui::Separator();




    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Manual Absolute Position setting");
    ImGui::PopFont();

    ImGui::TextWrapped("Overwrite the absolute position of the current encoder."
        "\nUseful to get the encoder back into its working range and prevent it from exceeding it during normal operation.");

    ImGui::PushFont(Fonts::robotoBold15);
    switch(encoderAssignement) {
        case EncoderAssignement::Type::INTERNAL_ENCODER:
            ImGui::Text("Assign absolute position of Internal Encoder");
            break;
        case EncoderAssignement::Type::ENCODER_MODULE:
            ImGui::Text("Assign absolute position of Module Encoder");
            break;
    }
    ImGui::PopFont();
    ImGui::InputFloat("##manualabsolute", &manualAbsoluteEncoderPosition_revolutions, 0.0, 0.0, "%.3f motor revolutions");


    ImGui::Spacing();
    if (ImGui::Button("Upload New Absolute Position")) {
        std::thread absolutePositionAssigner([this]() { uploadManualAbsoluteEncoderPosition(); });
        absolutePositionAssigner.detach();
    }
    ImGui::SameLine();
    ImGui::Text(getDataTransferState(encoderAbsolutePositionUploadState)->displayName);
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
        if (disableButton) BEGIN_DISABLE_IMGUI_ELEMENT
        if (ImGui::Button("Start Auto Tuning")) startAutoTuning();
        if (disableButton) END_DISABLE_IMGUI_ELEMENT
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