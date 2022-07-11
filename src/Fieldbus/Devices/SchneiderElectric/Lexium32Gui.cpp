#include <pch.h>

#include "Lexium32.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"
#include "Gui/Utilities/CustomWidgets.h"

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
                    generalSettingsGui();
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

    
    ImGui::PushFont(Fonts::sansBold15);
	
	const char* networkStatusString = isConnected() ? "Online" : (isDetected() ? "Detected" : "Offline");
	glm::vec4 networkStatusColor = isConnected() ? Colors::green : (isDetected() ? Colors::yellow : Colors::red);
	backgroundText(networkStatusString, statusDisplaySize, networkStatusColor);

    ImGui::SameLine();
	backgroundText(getEtherCatStateChar(), statusDisplaySize, getEtherCatStateColor());

    ImGui::SameLine();
	
	backgroundText(getShortStatusString().c_str(), statusDisplaySize, getStatusColor());
	
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("%s", getStatusString().c_str());
		ImGui::PopStyleColor();
		ImGui::EndTooltip();
	}

    ImGui::PopFont();

    glm::vec2 commandButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);

	ImGui::BeginDisabled(!servoMotor->isReady() || servoMotorPin->isConnected());
	if (servoMotor->isEnabled()) {
		if (ImGui::Button("Disable", commandButtonSize)) servoMotor->disable();
	}
	else if(b_hasFault){
		if(ImGui::Button("Reset Faults & Enable", commandButtonSize)) servoMotor->enable();
	}
	else {
		if (ImGui::Button("Enable", commandButtonSize)) servoMotor->enable();
	}
	ImGui::SameLine();
	if (ImGui::Button("Quick Stop", commandButtonSize)) servoMotor->quickstop();
	ImGui::EndDisabled();
    
}











void Lexium32::controlsGui() {

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Device Control");
	ImGui::PopFont();
	
	ImGui::BeginDisabled(!servoMotor->isEnabled());
	
	if(servoMotorPin->isConnected()){
		ImGui::TextWrapped("The device is controlled by its Servo Motor Node Pin."
						   "\nTo control the device manually, disconnect the Node Pin.");
	}else{
		ImGui::Text("Manual Velocity Control :");
		
		float velocityTarget = manualVelocityCommand_rps;
		
		ImGui::SliderFloat("##manualVelocity", &velocityTarget, -servoMotor->velocityLimit, servoMotor->velocityLimit, "%.1f rev/s");
		if (!ImGui::IsItemActive()) velocityTarget = 0.0; //only set the command if the slider is held down
		velocityTarget = std::min(velocityTarget, (float)servoMotor->velocityLimit);
		velocityTarget = std::max(velocityTarget, -(float)servoMotor->velocityLimit);
		manualVelocityCommand_rps = velocityTarget;
		
		static char accelerationString[32];
		sprintf(accelerationString, "Acceleration: %.2f rev/s\xc2\xb2", manualAcceleration_rpsps);
		ImGui::InputFloat("##manualAcceleration", &manualAcceleration_rpsps, 0.0, servoMotor->accelerationLimit, accelerationString);
		if (manualAcceleration_rpsps > servoMotor->accelerationLimit) manualAcceleration_rpsps = servoMotor->accelerationLimit;
	}
	
	float widgetWidth = ImGui::GetItemRectSize().x;
	float widgetHeight = ImGui::GetItemRectSize().y;

    ImGui::Separator();

    //------------------------- FEEDBACK ------------------------
	
	ImGui::EndDisabled();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Feedback");
	ImGui::PopFont();
	
    float velocityFraction;
    static char actualVelocityString[32];
    if (!servoMotor->isOnline()) {
        sprintf(actualVelocityString, "Offline");
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
        velocityFraction = 1.0;
    }
    else {
        double velocity = servoMotor->getVelocity();
        sprintf(actualVelocityString, "%.2f rev/s", velocity);
        velocityFraction = std::abs(velocity) / servoMotor->velocityLimit;
        if(velocityFraction >= 1.0) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
        else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
    }
    ImGui::Text("Motor Velocity :");
    ImGui::ProgressBar(velocityFraction, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), actualVelocityString);
    ImGui::PopStyleColor();

    char encoderPositionString[64];
    double range = servoMotor->getPositionInWorkingRange();
    if (!servoMotor->isOnline()) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
        range = 1.0;
        sprintf(encoderPositionString, "Offline");
    }
    else if (range < 1.0 && range > 0.0) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
        sprintf(encoderPositionString, "%.3f rev", servoMotor->getPosition());
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
        double distanceOutsideRange = range > 1.0 ? servoMotor->getPosition() - servoMotor->getMaxPosition() : servoMotor->getPosition() - servoMotor->getMinPosition();
        sprintf(encoderPositionString, "Encoder Outside Working Range by %.3f rev)", distanceOutsideRange);
        range = 1.0;
    }
	
    ImGui::Text("Encoder Position in Working Range : (%.2f rev to %.2f rev)", servoMotor->getMinPosition(), servoMotor->getMaxPosition());
    ImGui::ProgressBar(range, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), encoderPositionString);
    ImGui::PopStyleColor();

    double tripleWidgetWidth = (widgetWidth - 2.0 * ImGui::GetStyle().ItemSpacing.x) / 3.0;

	ImGui::BeginDisabled(servoMotorPin->isConnected());
	
    ImGui::Text("Soft Setting of Encoder Position (Current Offset: %.2f)", servoMotor->positionOffset);
    ImGui::SetNextItemWidth(tripleWidgetWidth);
    ImGui::InputDouble("##encoderPosition", &newEncoderPosition, 0.0, 0.0, "%.3f rev");
    ImGui::SameLine();
    if (ImGui::Button("Set", glm::vec2(tripleWidgetWidth, widgetHeight))) servoMotor->softOverridePosition(newEncoderPosition);
    ImGui::SameLine();
    if (ImGui::Button("Reset", glm::vec2(tripleWidgetWidth, widgetHeight))) servoMotor->positionOffset = 0.0;
	
	ImGui::Text("Load :");
	static char loadString[64];
	float loadProgress;
	if (!servoMotor->isOnline()) {
		loadProgress = 1.0;
		sprintf(loadString, "Offline");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
	}else{
		sprintf(loadString, "%.1f%%", servoMotor->getLoad() * 100.0);
		loadProgress = servoMotor->getLoad();
		if(loadProgress >= 1.0) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		else if(loadProgress > 0.8) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::orange);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	ImGui::ProgressBar(loadProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), loadString);
	ImGui::PopStyleColor();
	
	
	ImGui::Text("Following Error :");
	static char followingErrorString[64];
	float followingErrorProgress;
	if(!servoMotor->isOnline()){
		followingErrorProgress = 1.0;
		sprintf(followingErrorString, "Offline");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
	}else{
		sprintf(followingErrorString, "%.2f revs", servoMotor->followingError);
		followingErrorProgress = servoMotor->getFollowingErrorInRange();
		followingErrorProgress = std::min(followingErrorProgress, 1.0f);
		followingErrorProgress = std::max(followingErrorProgress, 0.0f);
		if(followingErrorProgress > 0.95) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		else if(followingErrorProgress > 0.75) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::orange);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	ImGui::ProgressBar(followingErrorProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), followingErrorString);
	ImGui::PopStyleColor();
	
	ImGui::EndDisabled();
	
	
	if(ImGui::Button("Start Homing")) b_startHoming = true;
	ImGui::BeginDisabled(!b_isHoming);
	if(ImGui::Button("Stop Homing")) b_isHoming = false;
	ImGui::EndDisabled();
		
}












void Lexium32::generalSettingsGui() {

    ImGui::PushFont(Fonts::sansBold20);
    ImGui::Text("Profile Generator Limits");
    ImGui::PopFont();

    ImGui::SameLine();
    if (beginHelpMarker("(help)")) {
        ImGui::TextWrapped("These settings are not stored on the drive, but regulate the speed and position commands sent to the drive.");
        endHelpMarker();
    }


    ImGui::Text("Velocity Limit");
    ImGui::InputDouble("##maxV", &servoMotor->velocityLimit, 0.0, 0.0, "%.1f rev/s");
    ImGui::Text("Acceleration Limit");
    ImGui::InputDouble("##maxA", &servoMotor->accelerationLimit, 0.0, 0.0, "%.1f rev/s\xc2\xb2");

    ImGui::Separator();

    ImGui::PushFont(Fonts::sansBold20);
    ImGui::Text("General Settings");
    ImGui::PopFont();

    ImGui::SameLine();
    if (beginHelpMarker("(help)")) {
        ImGui::TextWrapped("These settings are stored on the drive and represent general system limits.");
        endHelpMarker();
    }

    bool disableTransferButton = !isDetected();
	ImGui::BeginDisabled(disableTransferButton);
    if (ImGui::Button("Download Settings From Drive")) {
        std::thread generalParameterDownloader([this]() { downloadGeneralParameters(); });
        generalParameterDownloader.detach();
    }
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(generalParameterDownloadState));

	ImGui::BeginDisabled(disableTransferButton);
	if (ImGui::Button("Upload Settings To Drive")) {
		std::thread generalParameterUploader([this]() { uploadGeneralParameters(); });
		generalParameterUploader.detach();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(generalParameterUploadState));

	if (maxMotorVelocity == 0.0) {
		ImGui::Text("Max Motor Velocity is unknown : Download the value from the drive.");
	}
	else {
		ImGui::Text("Max Motor Velocity: %.3f rev/s", maxMotorVelocity);
		if (servoMotor->getVelocityLimit() > maxMotorVelocity) servoMotor->velocityLimit = maxMotorVelocity;
	}
	
    ImGui::Checkbox("##dir", &b_invertDirectionOfMotorMovement);
    ImGui::SameLine();
    ImGui::Text("Invert Direction of Movement");

	ImGui::Text("Max Position Following Error");
	if (ImGui::InputDouble("##MaxError", &maxFollowingError, 0.0, 0.0, "%.3f revolutions")) generalParameterUploadState = DataTransferState::NO_TRANSFER;
	
    ImGui::Text("Max Current");
    if (ImGui::InputDouble("##maxI", &maxCurrent_amps, 0.0, 0.0, "%.1f Amperes")) generalParameterUploadState = DataTransferState::NO_TRANSFER;
 

    ImGui::Text("Quick Stop Reaction");
    if (ImGui::BeginCombo("##QuickStopReaction", Enumerator::getDisplayString(quickstopReaction))) {
        for (auto& type : Enumerator::getTypes<QuickStopReaction>()) {
            if (ImGui::Selectable(type.displayString, type.enumerator == quickstopReaction)) {
                quickstopReaction = type.enumerator;
            }
            if (ImGui::IsItemHovered()) {
                if (beginHelpTooltip()) {
                    switch (type.enumerator) {
                        case QuickStopReaction::TORQUE_RAMP:
                            ImGui::TextWrapped("The Drives will come to a stop using the specified braking current.");
                            break;
                        case QuickStopReaction::DECELERATION_RAMP:
                            ImGui::TextWrapped("The Drive will come to a stop using the provided deceleration value.");
                            break;
                    }
                    endHelpTooltip();
                }
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    if (beginHelpMarker("(help)")) {
        ImGui::TextWrapped("QuickStop is a fast controlled stopping of motion."
            "\nHere the Drive will use power to brake the motion to a full stop. This is more controlled, and potentially quicker than a STO stop."
            "\nSTO stops (Safe Torque Off) are triggered by the emergency stop signal and will simply remove all torque from the drive."
            "\nA Quickstop is triggered automatically when the drive hits a limit signal. Quickstops can also be triggered manually.");
        endHelpMarker();
    }

    switch (quickstopReaction) {
        case QuickStopReaction::TORQUE_RAMP:
            ImGui::Text("Stopping Current");
            if (ImGui::InputDouble("##maxqi", &maxQuickstopCurrent_amps, 0.0, 0.0, "%.1f Amperes")) generalParameterUploadState = DataTransferState::NO_TRANSFER;
            break;
        case QuickStopReaction::DECELERATION_RAMP: {
            static char decelRampString[64];
            sprintf(decelRampString, "%.3f rev/s\xc2\xb2", quickStopDeceleration_revolutionsPerSecondSquared);
            ImGui::Text("Stopping Deceleration");
            if (ImGui::InputDouble("##rampA", &quickStopDeceleration_revolutionsPerSecondSquared, 0.0, 0.0, decelRampString)) generalParameterUploadState = DataTransferState::NO_TRANSFER;
            }break;
    }


  
}











void Lexium32::gpioGui() {

    ImGui::PushFont(Fonts::sansBold20);
    ImGui::Text("Limit Signals");
    ImGui::PopFont();

    ImGui::SameLine();
    if (beginHelpMarker("(help)")) {
        ImGui::TextWrapped("When the drive hits a limit switch it will trigger a quickstop."
            "\nUnassigned Output Pins are set to be freely available."
            "\nSettings can only be uploaded while the drive is disabled."
            "\nThe Drive needs to be rebooted after uploading new settings.");   
        endHelpMarker();
    }

    bool disableTransferButton = !isDetected();
	ImGui::BeginDisabled(disableTransferButton);
    if (ImGui::Button("Download Settings from Drive")) {
        std::thread pinAssigmentDownloader([this]() { downloadPinAssignements(); });
        pinAssigmentDownloader.detach();
    }
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(pinAssignementDownloadState));
	ImGui::BeginDisabled(disableTransferButton);
        if (ImGui::Button("Upload Settings To Drive")) {
            std::thread pinAssigmentUploader([this]() { uploadPinAssignements(); });
            pinAssigmentUploader.detach();
        }
	ImGui::EndDisabled();
	ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(pinAssignementUploadState));


    ImGui::Text("Negative Limit Switch");
    if (ImGui::BeginCombo("##negativeLimitSwitch", Enumerator::getDisplayString(negativeLimitSwitchPin))) {
        for (auto& type : Enumerator::getTypes<InputPin>()) {
            if (positiveLimitSwitchPin != InputPin::NONE && positiveLimitSwitchPin == type.enumerator) continue;
            if(ImGui::Selectable(type.displayString, negativeLimitSwitchPin == type.enumerator)) negativeLimitSwitchPin = type.enumerator;
        }
        ImGui::EndCombo();
    }
    if (negativeLimitSwitchPin != InputPin::NONE) {
        ImGui::Checkbox("##negsig", &b_negativeLimitSwitchNormallyClosed);
        ImGui::SameLine();
        ImGui::Text("Normally Closed");
    }

    ImGui::Text("Positive Limit Switch");
    if (ImGui::BeginCombo("##positiveLimitSwitch", Enumerator::getDisplayString(positiveLimitSwitchPin))) {
        for (auto& type : Enumerator::getTypes<InputPin>()) {
            if (negativeLimitSwitchPin != InputPin::NONE && negativeLimitSwitchPin == type.enumerator) continue;
            if (ImGui::Selectable(type.displayString, positiveLimitSwitchPin == type.enumerator)) positiveLimitSwitchPin = type.enumerator;
        }
        ImGui::EndCombo();
    }
    if (positiveLimitSwitchPin != InputPin::NONE) {
        ImGui::Checkbox("##possig", &b_positiveLimitSwitchNormallyClosed);
        ImGui::SameLine();
        ImGui::Text("Normally Closed");
    }

    ImGui::Separator();

    ImGui::PushFont(Fonts::sansBold20);
    ImGui::Text("Signal Inversion");
    ImGui::PopFont();

    ImGui::Checkbox("##di0", &b_invertDI0);
    ImGui::SameLine();
    ImGui::Text("Invert DI0");

    ImGui::Checkbox("##di1", &b_invertDI1);
    ImGui::SameLine();
    ImGui::Text("Invert DI1");

    ImGui::Checkbox("##di2", &b_invertDI2);
    ImGui::SameLine();
    ImGui::Text("Invert DI2");

    ImGui::Checkbox("##di3", &b_invertDI3);
    ImGui::SameLine();
    ImGui::Text("Invert DI3");

    ImGui::Checkbox("##di4", &b_invertDI4);
    ImGui::SameLine();
    ImGui::Text("Invert DI4");

    ImGui::Checkbox("##di5", &b_invertDI5);
    ImGui::SameLine();
    ImGui::Text("Invert DI5");
}




void Lexium32::encoderGui() {

    ImGui::PushFont(Fonts::sansBold20);
    ImGui::Text("Encoder Settings");
    ImGui::PopFont();

    ImGui::SameLine();
    if (beginHelpMarker("(help)")) {
        ImGui::TextWrapped("All Encoder settings are stored in the drive."
            "\nFor the settings to take effect, the drive has to be restared. "
            "\nChanging these settings will invalidate all current position references of the corresponding machine, homing procedures will need to be reexecuted.");
        endHelpMarker();
    }
   
    bool disableTransferButton = !isDetected();
	ImGui::BeginDisabled(disableTransferButton);
    if (ImGui::Button("Download Settings From Drive")) {
        std::thread encoderSettingsDownloader([this]() { downloadEncoderSettings(); });
        encoderSettingsDownloader.detach();
    }
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(encoderSettingsDownloadState));
	ImGui::BeginDisabled(disableTransferButton);
	if (ImGui::Button("Upload Settings To Drive")) {
		std::thread encoderSettingsUploader([this]() {
			uploadEncoderSettings();
			});
		encoderSettingsUploader.detach();
	}
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(encoderSettingsUploadState));

    ImGui::Text("Main Encoder used for absolute positionning:");
    if (ImGui::BeginCombo("##encoderAssignement", Enumerator::getDisplayString(encoderAssignement))) {
        for (auto type : Enumerator::getTypes<EncoderAssignement>()) {
            if (ImGui::Selectable(type.displayString, type.enumerator == encoderAssignement)) {
                encoderAssignement = type.enumerator;
            }
        }
        ImGui::EndCombo();
    }

    float doublewidgetWidth = (ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;

    if (encoderAssignement == EncoderAssignement::INTERNAL_ENCODER) {
        ImGui::TextWrapped("The Resolution of the internal motor encoder is 17 bits singleturn, 12 bits multiturn.");
    }
    else if (encoderAssignement == EncoderAssignement::ENCODER_MODULE) {
       
		ImGui::BeginDisabled(disableTransferButton);
        ImGui::SameLine();
        if (ImGui::Button("Detect Module")) {
            std::thread encoderModuleDetector([this]() { detectEncoderModule(); });
            encoderModuleDetector.detach();
        }
		ImGui::EndDisabled();

        switch (encoderModuleType) {
        case EncoderModule::DIGITAL_MODULE:

            ImGui::Text("Digital Encoder Type:");
            if (ImGui::BeginCombo("##EncoderType", Enumerator::getDisplayString(encoderType))) {
                for (auto type : Enumerator::getTypes<EncoderType>()) {
                    if (ImGui::Selectable(type.displayString, type.enumerator == encoderType)) {
                        encoderType = type.enumerator;
                    }
                }
                ImGui::EndCombo();
            }

            switch (encoderType) {
            case EncoderType::SSI_ROTARY:

                ImGui::Text("Coding");
                if (ImGui::BeginCombo("##Encoding", Enumerator::getDisplayString(encoderCoding))) {
                    for (auto& type : Enumerator::getTypes<EncoderCoding>()) {
                        if(ImGui::Selectable(type.displayString, encoderCoding == type.enumerator)) encoderCoding = type.enumerator;
                    }
                    ImGui::EndCombo();
                }
                ImGui::Text("Voltage");
                if (ImGui::BeginCombo("##Voltage", Enumerator::getDisplayString(encoderVoltage))) {
                    for (auto& type : Enumerator::getTypes<EncoderVoltage>()) {
                        if (ImGui::Selectable(type.displayString, encoderVoltage == type.enumerator)) encoderVoltage = type.enumerator;
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
            case EncoderType::NONE:
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
                ImGui::TextWrapped("Other Encoder Types are not yet supported");
                ImGui::PopStyleColor();
                break;
            }
            break;
        case EncoderModule::ANALOG_MODULE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("Analog Encoder Modules are not yet supported.");
            ImGui::PopStyleColor();
            break;
        case EncoderModule::RESOLVER_MODULE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("Resolver Encoder Modules are not yet supported.");
            ImGui::PopStyleColor();
            break;
        case EncoderModule::NONE:
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
            ImGui::TextWrapped("No Encoder Module was detected in module slot 2."
                                "\nRestart the drive and detect the module again. Never insert or remove modules while the drive is powered on.");
            ImGui::PopStyleColor();
            break;
        }
    }

    ImGui::Spacing();
    ImGui::Checkbox("##shifting", &b_encoderRangeShifted);
    ImGui::SameLine();
    ImGui::TextWrapped("Center the encoder working range around 0.");
    ImGui::Text("Working Range : %.1f to %.1f motor revolutions", servoMotor->minWorkingRange, servoMotor->maxWorkingRange);

    ImGui::Separator();


    ImGui::PushFont(Fonts::sansBold20);
    ImGui::Text("Manual Absolute Position setting");
    ImGui::PopFont();

    ImGui::SameLine();
    if (beginHelpMarker("(help)")) {
        ImGui::TextWrapped("Overwrite the hard absolute position of the current encoder as stored on the drive."
            "\nUseful to get the encoder back into its working range and prevent it from exceeding it during normal operation."
            "\nThe drive needs to be restarted for this change to take effect.");
        endHelpMarker();
    }

    ImGui::PushFont(Fonts::sansBold15);
    switch(encoderAssignement) {
        case EncoderAssignement::INTERNAL_ENCODER:
            ImGui::Text("Assign hard absolute position of Internal Encoder");
            break;
        case EncoderAssignement::ENCODER_MODULE:
            ImGui::Text("Assign hard absolute position of Module Encoder");
            break;
    }
    ImGui::PopFont();
    ImGui::InputFloat("##manualabsolute", &manualAbsoluteEncoderPosition_revolutions, 0.0, 0.0, "%.3f motor revolutions");

	ImGui::BeginDisabled(disableTransferButton);
    ImGui::Spacing();
    if (ImGui::Button("Upload New Absolute Position")) {
        std::thread absolutePositionAssigner([this]() { uploadManualAbsoluteEncoderPosition(); });
        absolutePositionAssigner.detach();
    }
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(encoderAbsolutePositionUploadState));
}


void Lexium32::tuningGui() {

    ImGui::PushFont(Fonts::sansBold20);
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
            ImGui::PushFont(Fonts::sansBold15);
            ImGui::Text("Auto Tuning Succeeded");
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Text("(%s)", Enumerator::getDisplayString(autoTuningSaveState));
        }

        if(b_autoTuningSucceeded){
            ImGui::Separator();
            ImGui::PushFont(Fonts::sansBold15);
            ImGui::Text("Tuning Results:");
            ImGui::PopFont();
            ImGui::Text("Friction Torque: %.2f Amperes", tuning_frictionTorque_amperes);
            ImGui::Text("Constant Load Torque: %.2f Amperes", tuning_constantLoadTorque_amperes);
            ImGui::Text("Moment of Inertia: %.1f kg*cm2", tuning_momentOfInertia_kilogramcentimeter2);
        }
    }
    else {
        bool disableButton = isConnected() || !isDetected();
		ImGui::BeginDisabled(disableButton);
        if (ImGui::Button("Start Auto Tuning")) startAutoTuning();
		ImGui::EndDisabled();
    }
}


void Lexium32::miscellaneousGui() {
    
    ImGui::PushFont(Fonts::sansBold15);
    ImGui::Text("Station Alias Assignement");
    ImGui::PopFont();
    ImGui::TextWrapped("Assigns a new station alias to the drive. The alias becomes active after the drive has been restarted.");
    static uint16_t newStationAlias = 0;
    ImGui::InputScalar("##alias", ImGuiDataType_U16, &newStationAlias);
    bool disableTransferButton = !isDetected();
	ImGui::BeginDisabled(disableTransferButton);
    if (ImGui::Button("Set Station Alias")) {
        std::thread stationAliasSettingHandler([this]() {setStationAlias(newStationAlias);});
        stationAliasSettingHandler.detach();
    }
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(stationAliasUploadState));

    ImGui::Separator();
    ImGui::PushFont(Fonts::sansBold15);
    ImGui::Text("Factory Reset");
    ImGui::PopFont();
    ImGui::TextWrapped("Resets all internal drive settings to the original factory configuration.");
	ImGui::BeginDisabled(disableTransferButton);
    if (ImGui::Button("Reset Lexium32 to Factory Settings")) {
        std::thread factoryResetHandler([this]() {factoryReset();});
        factoryResetHandler.detach();
    }
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s", Enumerator::getDisplayString(factoryResetTransferState));
    
}
