#include <pch.h>

#include "ActuatorToServoActuator.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "NodeGraph/Device.h"

void ActuatorToServoActuator::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Controls")){
		controlGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Settings")){
		settingsGui();
		ImGui::EndTabItem();
	}
}

void ActuatorToServoActuator::controlGui(){

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	
	
	
	bool actuatorControlledExternally = isServoActuatorPinConnected();
	
	if(actuatorControlledExternally){
		ImGui::TextWrapped("Servo Actuator is Controlled by Node '%s'."
						   "\nManual controls are disabled.",
						   servoActuatorPin->getConnectedPin()->getNode()->getName());
		BEGIN_DISABLE_IMGUI_ELEMENT
	}
		
	float singleWidgetWidth = ImGui::GetContentRegionAvail().x;
	float tripleWidgetWidth = (singleWidgetWidth - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0;
	glm::vec2 tripleButtonSize(tripleWidgetWidth, ImGui::GetFrameHeight());
	glm::vec2 doubleButtonSize((singleWidgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetFrameHeight());
	glm::vec2 progressBarSize(singleWidgetWidth, ImGui::GetFrameHeight());
	glm::vec2 largeDoubleButtonSize((singleWidgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);
	
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if(servoActuator->isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Enabled", largeDoubleButtonSize);
	}else if(servoActuator->isReady()){
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Ready", largeDoubleButtonSize);
	}else{
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Disabled", largeDoubleButtonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	
	ImGui::SameLine();
	if(servoActuator->isEnabled()){
		if(ImGui::Button("Disable", largeDoubleButtonSize)) servoActuator->disable();
	}else if(servoActuator->isReady()){
		if(ImGui::Button("Enable", largeDoubleButtonSize)) servoActuator->enable();
	}else{
		BEGIN_DISABLE_IMGUI_ELEMENT
		ImGui::Button("Not Ready", largeDoubleButtonSize);
		END_DISABLE_IMGUI_ELEMENT
	}
	
	if(actuatorControlledExternally) END_DISABLE_IMGUI_ELEMENT
	
		
	bool disableManualControls = actuatorControlledExternally || !servoActuator->isEnabled();
	if(disableManualControls) BEGIN_DISABLE_IMGUI_ELEMENT

	ImGui::Text("Manual Velocity:");
	ImGui::SetNextItemWidth(singleWidgetWidth);
	ImGui::SliderFloat("##manVel", &manualVelocityDisplay, -servoActuator->getVelocityLimit(), servoActuator->getVelocityLimit());
	if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		manualVelocityDisplay = 0.0;
		setVelocityTarget(0.0);
	}
	
	//------------------------- POSITION CONTROLS --------------------------

	ImGui::Text("Manual Position Control");
	
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", targetPositionDisplay, Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	ImGui::InputFloat("##TargetPosition", &targetPositionDisplay, 0.0, 0.0, targetPositionString);
	
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char targetVelocityString[32];
	sprintf(targetVelocityString, "%.3f %s/s", targetVelocityDisplay, Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	ImGui::InputFloat("##TargetVelocity", &targetVelocityDisplay, 0.0, 0.0, targetVelocityString);
	
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputFloat("##TargetTime", &targetTimeDisplay, 0.0, 0.0, "%.3f s");
	
	if (ImGui::Button("Fast Move", tripleButtonSize))
		movetoPositionWithVelocity(targetPositionDisplay,
								   servoActuator->getVelocityLimit());
	
	ImGui::SameLine();
	if (ImGui::Button("Velocity Move", tripleButtonSize))
		movetoPositionWithVelocity(targetPositionDisplay,
								   targetVelocityDisplay);
	
	ImGui::SameLine();
	if (ImGui::Button("Timed Move", tripleButtonSize))
		moveToPositionInTime(targetPositionDisplay,
							 targetTimeDisplay);

	if (ImGui::Button("Stop", doubleButtonSize)) setVelocityTarget(0.0);
	
	ImGui::SameLine();
	if (ImGui::Button("Fast Stop", doubleButtonSize)) fastStop();
	
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Feedback");
	ImGui::PopFont();
	
	
	ImGui::Text("Velocity: ");
	static char velocityString[256];
	sprintf(velocityString, "%.3f %s/s", servoActuator->getVelocity(), Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	float velocityProgress = std::abs(servoActuator->getVelocity() / servoActuator->getVelocityLimit());
	ImGui::ProgressBar(velocityProgress, progressBarSize, velocityString);
	
	ImGui::Text("Position: ");
	static char positionString[256];
	sprintf(positionString, "%.3f %s", servoActuator->getPosition(), Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	float positionProgress = servoActuator->getPositionInRange();
	ImGui::ProgressBar(positionProgress, progressBarSize, positionString);
	
	ImGui::Text("Following Error: ");
	static char followingErrorString[256];
	sprintf(followingErrorString, "%.3f %s", servoActuator->getFollowingError(), Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	float positionErrorProgress = std::abs(servoActuator->getFollowingError() / servoActuator->maxfollowingError);
	ImGui::ProgressBar(positionErrorProgress, progressBarSize, followingErrorString);
	
	static int offset = 4;
	
	glm::vec2* positionTargetBuffer;
	size_t positionTargetPointCount = positionTargetHistory.getBuffer(&positionTargetBuffer);
	
	glm::vec2* positionBuffer;
	size_t positionPointCount = positionHistory.getBuffer(&positionBuffer);
	
	glm::vec2* positionErrorBuffer;
	size_t positionErrorPointCount = positionErrorHistory.getBuffer(&positionErrorBuffer);
	
	glm::vec2* velocityTargetBuffer;
	size_t velocityTargetPointCount = velocityTargetHistory.getBuffer(&velocityTargetBuffer);
	
	glm::vec2* velocityBuffer;
	size_t velocityPointCount = velocityHistory.getBuffer(&velocityBuffer);
	
	glm::vec2* velocityErrorBuffer;
	size_t velocityErrorPointCount = velocityErrorHistory.getBuffer(&velocityErrorBuffer);
	
	//=========================== GRAPHS =============================
	
	glm::vec2 plotSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 10.0);
	ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoChild | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoMousePos | ImPlotFlags_NoTitle;
	
	size_t pointCount = velocityErrorPointCount;
	float startTime = 0.0;
	float endTime = 0.0;
	if(pointCount > offset){
		startTime = positionTargetBuffer[offset].x;
		endTime = positionTargetBuffer[pointCount - 1].x;
	}
		
	ImPlot::SetNextPlotLimitsX(startTime, endTime, ImGuiCond_Always);
	ImPlot::FitNextPlotAxes(false, true);
	
	ImGui::Text("Position Graph");
	
	if(ImPlot::BeginPlot("Position Graph", 0, 0, plotSize, plotFlags)){
		if(pointCount > offset){
			ImPlot::SetNextLineStyle(Colors::white, 2.0);
			ImPlot::PlotLine("Position Target", &positionTargetBuffer[offset].x, &positionTargetBuffer[offset].y , pointCount - offset, 0, sizeof(glm::vec2));
			
			ImPlot::SetNextLineStyle(Colors::red, 2.0);
			ImPlot::PlotLine("Position Actual", &positionBuffer[offset].x, &positionBuffer[offset].y , pointCount - offset, 0, sizeof(glm::vec2));
		}
		ImPlot::EndPlot();
	}
	
	ImPlot::SetNextPlotLimitsX(startTime, endTime, ImGuiCond_Always);
	ImPlot::FitNextPlotAxes(false, true);
	ImPlot::SetNextPlotLimitsY(-servoActuator->getVelocityLimit() * 1.2, servoActuator->getVelocityLimit() * 1.2);
	
	ImGui::Text("Velocity Graph");
	
	if(ImPlot::BeginPlot("Velocity Graph", 0, 0, plotSize, plotFlags)){
		if(pointCount > offset){
			ImPlot::SetNextLineStyle(Colors::red, 2.0);
			ImPlot::PlotLine("Velocity Actual", &velocityBuffer[offset].x, &velocityBuffer[offset].y , pointCount - offset, 0, sizeof(glm::vec2));
			
			ImPlot::SetNextLineStyle(Colors::white, 2.0);
			ImPlot::PlotLine("Velocity Target", &velocityTargetBuffer[offset].x, &velocityTargetBuffer[offset].y , pointCount - offset, 0, sizeof(glm::vec2));
		}
		ImPlot::EndPlot();
	}
	
	ImGui::Text("Error Graph");
	
	ImPlot::SetNextPlotLimitsX(startTime, endTime, ImGuiCond_Always);
	ImPlot::SetNextPlotLimitsY(-servoActuator->maxfollowingError * 1.2, servoActuator->maxfollowingError * 1.2);
	
	if(ImPlot::BeginPlot("Error Graph", 0, 0, plotSize, plotFlags)){
		if(pointCount > offset){
			ImPlot::SetNextLineStyle(Colors::red, 1.0);
			double maxError[2] = {servoActuator->maxfollowingError, -servoActuator->maxfollowingError};
			ImPlot::PlotHLines("Following Error Threshold", &maxError[0], 2);
			
			ImPlot::SetNextLineStyle(Colors::yellow, 2.0);
			ImPlot::PlotLine("Position Error", &positionErrorBuffer[offset].x, &positionErrorBuffer[offset].y, pointCount - offset, 0, sizeof(glm::vec2));
		}
		ImPlot::EndPlot();
	}

	if(disableManualControls) END_DISABLE_IMGUI_ELEMENT
}

void ActuatorToServoActuator::settingsGui(){
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Servo Actuator");
	ImGui::PopFont();

	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Actuator Device:");
	ImGui::PopFont();
	ImGui::SameLine();
	if(isActuatorConnected()){
		std::shared_ptr<ActuatorDevice> actuatorDevice = getActuatorDevice();
		PositionUnit positionUnit = getPositionUnit();
		if(actuatorDevice->parentDevice) ImGui::Text("%s on %s", actuatorDevice->getName(), actuatorDevice->parentDevice->getName());
		else ImGui::Text("%s on Node %s", actuatorDevice->getName(), actuatorPin->getConnectedPin()->getNode()->getName());
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("Position Unit: %s", Unit::getDisplayStringPlural(positionUnit));
		ImGui::Text("Velocity Limit: %.3f %s/s", actuatorDevice->getVelocityLimit(), Unit::getAbbreviatedString(positionUnit));
		ImGui::Text("Minimum Velocity: %.3f %s/s", actuatorDevice->getMinVelocity(), Unit::getAbbreviatedString(positionUnit));
		ImGui::Text("Acceleration Limit: %.3f %s/s2", actuatorDevice->getAccelerationLimit(), Unit::getAbbreviatedString(positionUnit));
		ImGui::PopStyleColor();
		
		ImGui::Text("Acceleration for Manual Controls");
		static char manualAccelerationString[256];
		sprintf(manualAccelerationString, "%.3f %s/s2", manualAcceleration, Unit::getAbbreviatedString(positionUnit));
		ImGui::InputDouble("##manAcc", &manualAcceleration, 0.0, 0.0, manualAccelerationString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Feedback");
		ImGui::PopFont();
	
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Position Feedback Device:");
		ImGui::PopFont();
		ImGui::SameLine();
		if(isPositionFeedbackConnected()){
			std::shared_ptr<PositionFeedbackDevice> feedbackDevice = getPositionFeedbackDevice();
			if(feedbackDevice->parentDevice) ImGui::Text("%s on %s", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());
			else ImGui::Text("%s on Node %s", feedbackDevice->getName(), positionFeedbackPin->getConnectedPin()->getNode()->getName());
			
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("Position Unit: %s", Unit::getDisplayStringPlural(feedbackDevice->getPositionUnit()));
			ImGui::PopStyleColor();
			
			ImGui::Text("Feedback %s per Actuator %s", Unit::getDisplayStringPlural(feedbackDevice->getPositionUnit()), Unit::getDisplayString(servoActuator->getPositionUnit()));
			ImGui::InputDouble("##FeedbackUnitsPerActuatorUnit", &positionFeedbackUnitsPerActuatorUnit);
			if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		}else {
			ImGui::TextColored(Colors::red, "Not Connected");
		}
		
	}else {
		ImGui::TextColored(Colors::red, "Not Connected");
	}

	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position Controller");
	ImGui::PopFont();
	
	ImGui::Text("Position Loop Proportional Gain");
	ImGui::InputDouble("##pospropGain", &positionLoopProportionalGain);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Velocity Loop Proportional Gain");
	ImGui::InputDouble("##velpropGain", &velocityLoopProportionalGain);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Velocity Loop Integral Gain");
	ImGui::InputDouble("##velIntGain", &velocityLoopIntegralGain);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Max Following Error");
	static char maxErrorString[256];
	sprintf(maxErrorString, "%.3f %s", servoActuator->maxfollowingError, Unit::getDisplayStringPlural(servoActuator->getPositionUnit()));
	ImGui::InputDouble("##maxErr", &servoActuator->maxfollowingError, 0.0, 0.0, maxErrorString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Error Correction Threshold");
	static char errorThresholdString[256];
	sprintf(errorThresholdString, "%.3f %s", errorCorrectionTreshold, Unit::getDisplayStringPlural(servoActuator->getPositionUnit()));
	ImGui::InputDouble("##errtresh", &errorCorrectionTreshold, 0.0, 0.0, errorThresholdString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
}

