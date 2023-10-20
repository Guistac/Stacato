#include <pch.h>

#include "AxisNode.h"
#include "Motion/Interfaces.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"

#include "Gui/Utilities/CustomWidgets.h"
#include "Fieldbus/EtherCatFieldbus.h"

void AxisNode::nodeSpecificGui(){
	controlTab();
	configurationTab();
	devicesTab();
	axisInterfaceTab();
}

void AxisNode::controlTab(){
	
	if(ImGui::BeginTabItem("Control")){
		
		
		std::string stateString;
		glm::vec4 stateColor;
		switch(axisInterface->getState()){
			case DeviceState::OFFLINE:
				stateString = "Offline";
				stateColor = Colors::blue;
				break;
			case DeviceState::NOT_READY:
				stateString = "Not Ready";
				stateColor = Colors::red;
				break;
			case DeviceState::READY:
				stateString = "Ready";
				stateColor = Colors::yellow;
				break;
			case DeviceState::DISABLING:
				stateString = "Disabling...";
				stateColor = Colors::darkYellow;
				break;
			case DeviceState::ENABLING:
				stateString = "Enabling...";
				stateColor = Colors::darkGreen;
				break;
			case DeviceState::ENABLED:
				stateString = "Enabled";
				stateColor = Colors::green;
				break;
		}
		
		if(axisInterface->configuration.controlMode == AxisInterface::ControlMode::NONE){
			ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 2.0);
			ImGui::PushFont(Fonts::sansBold15);
			backgroundText(stateString.c_str(), buttonSize, stateColor);
			ImGui::PopFont();
		}else{
			ImVec2 buttonSize(ImGui::GetContentRegionAvail().x / 2.0, ImGui::GetTextLineHeight() * 2.0);
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::BeginDisabled(!axisInterface->isOnline());
			if(axisInterface->isEnabled()){
				if(customButton("Disable", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft)){
					axisInterface->disable();
				}
			}else{
				if(customButton("Enable", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft)){
					axisInterface->enable();
				}
			}
			ImGui::EndDisabled();
			ImGui::SameLine(0.0, 0.0);
			backgroundText(stateString.c_str(), buttonSize, stateColor, Colors::black, ImDrawFlags_RoundCornersRight);
			ImGui::PopFont();
		}
		
		ImVec2 progressBarSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5);
		
		ImGui::BeginDisabled(!axisInterface->isEnabled() || axisPin->isConnected());
		
		if(axisInterface->configuration.controlMode != AxisInterface::ControlMode::NONE){
			
			float sliderVelocityTarget = 0.0;
			std::ostringstream manVelString;
			manVelString << std::fixed << std::setprecision(2) << "Manual Velocity Target : " << internalVelocityTarget;
			
			double velLim = axisInterface->getVelocityLimit();
			
			ImGui::SetNextItemWidth(progressBarSize.x);
			ImGui::SliderFloat("##vel", &sliderVelocityTarget, -velLim, velLim, manVelString.str().c_str());
			
			if(ImGui::IsItemActive()) {
				sliderVelocityTarget = getFilteredVelocity(sliderVelocityTarget);
				setManualVelocityTarget(sliderVelocityTarget);
			}
			if(ImGui::IsItemDeactivatedAfterEdit()) {
				setManualVelocityTarget(0.0);
			}
			
		}
		
		if(axisInterface->configuration.controlMode == AxisInterface::ControlMode::POSITION_CONTROL){
			
			float cellWidth = (ImGui::GetContentRegionAvail().x - 3 * ImGui::GetStyle().ItemSpacing.x) / 4;
			ImVec2 cellSize(cellWidth, ImGui::GetTextLineHeight());
			ImVec2 cellButtonSize(cellWidth, ImGui::GetTextLineHeight()*2);
			
			if(ImGui::BeginTable("##posTar", 4)){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				centeredText("Target Position", cellSize);
				ImGui::TableSetColumnIndex(1);
				centeredText("Target Velocity", cellSize);
				ImGui::TableSetColumnIndex(2);
				centeredText("Target Time", cellSize);
				ImGui::TableSetColumnIndex(3);
				centeredText("Target Acceleration", cellSize);
				
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(cellWidth);
				ImGui::InputFloat("##tarpos", &manualPositionEntry);
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(cellWidth);
				ImGui::InputFloat("##tarvel", &manualVelocityEntry);
				ImGui::TableSetColumnIndex(2);
				ImGui::SetNextItemWidth(cellWidth);
				ImGui::InputFloat("##tartim", &manualTimeEntry);
				ImGui::TableSetColumnIndex(3);
				ImGui::SetNextItemWidth(cellWidth);
				ImGui::InputFloat("##taracc", &manualAccelerationEntry);
				
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if(ImGui::Button("Fast Move", cellButtonSize)){
					moveToManualPositionTargetWithTime(manualPositionEntry, 0.0, manualAccelerationEntry);
				}
				ImGui::TableSetColumnIndex(1);
				if(ImGui::Button("Velocity Move", cellButtonSize)){
					moveToManualPositionTargetWithVelocity(manualPositionEntry, manualVelocityEntry, manualAccelerationEntry);
				}
				ImGui::TableSetColumnIndex(2);
				if(ImGui::Button("Timed Move", cellButtonSize)){
					moveToManualPositionTargetWithTime(manualPositionEntry, manualTimeEntry, manualAccelerationEntry);
				}
				ImGui::TableSetColumnIndex(3);
				if(ImGui::Button("Stop", cellButtonSize)){
					setManualVelocityTarget(0.0);
				}
				
				ImGui::EndTable();
			}
			
			
			float interpolationProgress = 0.0;
			std::string interpolationProgressString;
			if(internalControlMode == InternalControlMode::MANUAL_POSITION_INTERPOLATION){
				double now = EtherCatFieldbus::getCycleProgramTime_seconds();
				if(motionProfile.isInterpolationFinished(now)){
					interpolationProgress = 1.0;
					interpolationProgressString = "Movement Finished";
				}else{
					interpolationProgress = motionProfile.getInterpolationProgress(now);
					std::ostringstream msg;
					msg << std::fixed << std::setprecision(1) << "Movement Time Remaining : "
					<< motionProfile.getRemainingInterpolationTime(now);
					interpolationProgressString = msg.str();
				}
			}else{
				interpolationProgress = 0.0;
				interpolationProgressString = "No Movement in progress";
			}
			ImGui::ProgressBar(interpolationProgress, progressBarSize, interpolationProgressString.c_str());
			
		}
		
		if(controlMode == VELOCITY_CONTROL){
			
			ImVec4 inactiveBackgroundColor = Colors::darkYellow;
			ImVec4 activeBackgroundColor = Colors::yellow;
			ImVec4 inactiveTextColor = Colors::veryDarkGray;
			ImVec4 activeTextColor = Colors::black;
			
			if(limitSignalType == SIGNAL_AT_LOWER_AND_UPPER_LIMITS){
				ImVec2 indicatorSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetFrameHeight());
				backgroundText("Lower Limit", indicatorSize,
							   *lowerLimitSignal ? activeBackgroundColor : inactiveBackgroundColor,
							   *lowerLimitSignal ? activeTextColor : inactiveTextColor);
				ImGui::SameLine();
				backgroundText("Upper Limit", indicatorSize,
							   *upperLimitSignal ? activeBackgroundColor : inactiveBackgroundColor,
							   *upperLimitSignal ? activeTextColor : inactiveTextColor);
			}
			else if(limitSignalType == LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS){
				ImVec2 indicatorSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3.0) / 4.0, ImGui::GetFrameHeight());
				backgroundText("Lower Limit", indicatorSize,
							   *lowerLimitSignal ? activeBackgroundColor : inactiveBackgroundColor,
							   *lowerLimitSignal ? activeTextColor : inactiveTextColor);
				ImGui::SameLine();
				backgroundText("Lower Slowdown", indicatorSize,
							   *lowerSlowdownSignal ? activeBackgroundColor : inactiveBackgroundColor,
							   *lowerSlowdownSignal ? activeTextColor : inactiveTextColor);
				ImGui::SameLine();
				backgroundText("Upper Slowdown", indicatorSize,
							   *upperSlowdownSignal ? activeBackgroundColor : inactiveBackgroundColor,
							   *upperSlowdownSignal ? activeTextColor : inactiveTextColor);
				ImGui::SameLine();
				backgroundText("Upper Limit", indicatorSize,
							   *upperLimitSignal ? activeBackgroundColor : inactiveBackgroundColor,
							   *upperLimitSignal ? activeTextColor : inactiveTextColor);
			}
		}
		
		if(axisInterface->supportsHoming()){
			ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 2.0);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
			if(axisInterface->isHoming()){
				if(customButton("Stop Homing", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTop)){
					axisInterface->stopHoming();
				}
			}else{
				ImGui::BeginDisabled(!axisInterface->canStartHoming());
				if(customButton("Start Homing", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTop)){
					axisInterface->startHoming();
				}
				ImGui::EndDisabled();
			}
			ImGui::PopStyleVar();
			
			if(axisInterface->isHoming()){
				backgroundText(getHomingStepString().c_str(), buttonSize, Colors::gray, Colors::black, ImDrawFlags_RoundCornersBottom);
			}else if(axisInterface->didHomingSucceed()){
				backgroundText("Homing Finished", buttonSize, Colors::green, Colors::black, ImDrawFlags_RoundCornersBottom);
			}else{
				backgroundText("No Homing in progress", buttonSize, Colors::darkGray, Colors::black, ImDrawFlags_RoundCornersBottom);
			}
		}
		
		
		if(axisInterface->configuration.b_supportsPositionFeedback){
			ImVec2 halfWidgetSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5, ImGui::GetFrameHeight());
			ImGui::SetNextItemWidth(halfWidgetSize.x);
			std::ostringstream positionOverrideString;
			positionOverrideString << std::fixed << std::setprecision(3) << newPositionForFeedbackRatio << axisInterface->getPositionUnit()->abbreviated;
			ImGui::InputDouble("##posovrd", &newPositionForFeedbackRatio, 0, 0, positionOverrideString.str().c_str());
			ImGui::SameLine();
			if(ImGui::Button("Set Axis Position", halfWidgetSize)) updateFeedbackRatioToMatchPosition();
		}
			
		ImGui::EndDisabled(); //end disabled when axis not enabled or axis pin connected
		
		ImGui::Separator();
		
		if(axisInterface->configuration.b_supportsPositionFeedback){
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Position Feedback");
			ImGui::PopFont();
			std::ostringstream positionString;
			positionString << std::fixed << std::setprecision(3)
			<< selectedPositionFeedbackMapping->getFeedbackInterface()->getPosition() / selectedPositionFeedbackMapping->deviceUnitsPerAxisUnits->value
			<< " u";
			
			
			

			
			
			double pos = axisInterface->getPositionActual();
			double lowerLimit = lowerPositionLimitWithoutClearance;
			double upperLimit = upperPositionLimitWithoutClearance;
			double posNormalized = std::clamp((pos - lowerLimit) / (upperLimit - lowerLimit), 0.0, 1.0);
			double lowerClearanceNormalized = std::clamp((axisInterface->getLowerPositionLimit() - lowerLimit) / (upperLimit - lowerLimit), 0.0, 1.0);
			double upperClearanceNormalized = std::clamp((axisInterface->getUpperPositionLimit() - lowerLimit) / (upperLimit - lowerLimit), 0.0, 1.0);
			
			ImDrawList* canvas = ImGui::GetWindowDrawList();
			
			ImGui::ProgressBar(posNormalized, progressBarSize, positionString.str().c_str());
			ImVec2 minPos = ImGui::GetItemRectMin();
			ImVec2 maxPos = ImGui::GetItemRectMax();
			ImVec2 size = ImGui::GetItemRectSize();
			canvas->AddLine(ImVec2(minPos.x + lowerClearanceNormalized * size.x, minPos.y),
							ImVec2(minPos.x + lowerClearanceNormalized * size.x, maxPos.y), ImColor(Colors::white));
			canvas->AddLine(ImVec2(minPos.x + upperClearanceNormalized * size.x, minPos.y),
							ImVec2(minPos.x + upperClearanceNormalized * size.x, maxPos.y), ImColor(Colors::white));
			
			
			
		}
		if(axisInterface->configuration.b_supportsVelocityFeedback){
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Velocity Feedback");
			ImGui::PopFont();
			double vel = selectedVelocityFeedbackMapping->getFeedbackInterface()->getVelocity() / selectedVelocityFeedbackMapping->deviceUnitsPerAxisUnits->value;
			std::ostringstream velocityString;
			velocityString << std::fixed << std::setprecision(3) << vel << " u/s";
			ImGui::ProgressBar(std::abs(axisInterface->getVelocityNormalizedToLimits()), progressBarSize, velocityString.str().c_str());
		}
		if(axisInterface->configuration.b_supportsEffortFeedback){
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Effort Feedback");
			ImGui::PopFont();
			
			double axisEffort = axisInterface->processData.effortActual;
			std::ostringstream axisEffortString;
			axisEffortString << "Axis : " << std::fixed << std::setprecision(1) << axisEffort * 100.0 << "%";
			ImGui::ProgressBar(axisEffort, progressBarSize, axisEffortString.str().c_str());
			
			for(int i = 0; i < actuatorMappings.size(); i++){
				if(!actuatorMappings[i]->isActuatorConnected()) continue;
				auto actuator = actuatorMappings[i]->getActuatorInterface();
				double actuatorEffort = actuator->getEffort();
				std::ostringstream actuatorEffortString;
				actuatorEffortString << actuator->getName() << " : " << std::fixed << std::setprecision(1) << actuatorEffort * 100.0 << "%";
				ImGui::ProgressBar(actuatorEffort, progressBarSize, actuatorEffortString.str().c_str());
			}
			
		}
		if(loadSensorPin->isConnected()){
			
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Force Feedback");
			ImGui::PopFont();
			
			float forceProgress = axisInterface->processData.forceActual / upperForceLimit->value;
			std::ostringstream msg;
			msg << std::fixed << std::setprecision(1) << "Force : " << axisInterface->processData.forceActual << "N";
			std::string forceProgressString = msg.str();
			ImGui::ProgressBar(forceProgress, progressBarSize, forceProgressString.c_str());
		}
		
		if(selectedPositionFeedbackMapping && selectedPositionFeedbackMapping->isFeedbackConnected()){
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Position feedback working range");
			ImGui::PopFont();
			
			auto feedback = selectedPositionFeedbackMapping->getFeedbackInterface();
			double fbRatio = selectedPositionFeedbackMapping->deviceUnitsPerAxisUnits->value;
			double wrMin = feedback->getPositionLowerWorkingRangeBound() / fbRatio;
			double wrMax = feedback->getPositionUpperWorkingRangeBound() / fbRatio;
			double pos = axisInterface->getPositionActual();
			double min = axisInterface->getLowerPositionLimit();
			double max = axisInterface->getUpperPositionLimit();
			double posNorm = (pos - wrMin) / (wrMax - wrMin);
			double minPosNorm = (min - wrMin) / (wrMax - wrMin);
			double maxPosNorm = (max - wrMin) / (wrMax - wrMin);
			ImDrawList* canvas = ImGui::GetWindowDrawList();
			ImGui::Dummy(progressBarSize);
			glm::vec2 minPos = ImGui::GetItemRectMin();
			glm::vec2 maxPos = ImGui::GetItemRectMax();
			glm::vec2 size = ImGui::GetItemRectSize();
			canvas->AddRectFilled(minPos, maxPos, ImGui::GetColorU32(ImGuiCol_FrameBg), ImGui::GetStyle().FrameRounding);
			canvas->AddRectFilled(ImVec2(minPos.x + size.x * minPosNorm, minPos.y),
								  ImVec2(minPos.x + size.x * maxPosNorm, maxPos.y),
								  ImGui::GetColorU32(ImGuiCol_PlotHistogram), ImGui::GetStyle().FrameRounding);
			canvas->AddLine(ImVec2(minPos.x + size.x * posNorm, minPos.y),
							ImVec2(minPos.x + size.x * posNorm, maxPos.y),
							ImColor(Colors::white));
		}
		
		
		
		if(axisInterface->configuration.controlMode == AxisInterface::ControlMode::POSITION_CONTROL){
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Position Following Error");
			ImGui::PopFont();
			std::ostringstream positionErrorString;
			positionErrorString << std::fixed << std::setprecision(4) << positionFollowingError << " u";
			double errorNormalized = std::abs(positionFollowingError / positionLoop_maxError->value);
			ImGui::ProgressBar(errorNormalized, progressBarSize, positionErrorString.str().c_str());
			double minErrorNormalized = std::abs(positionLoop_minError->value) / std::abs(positionLoop_maxError->value);
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			ImVec2 size = ImGui::GetItemRectSize();
			ImDrawList* canvas = ImGui::GetWindowDrawList();
			canvas->AddLine(ImVec2(min.x + size.x * minErrorNormalized, min.y),
							ImVec2(min.x + size.x * minErrorNormalized, max.y), ImColor(Colors::white));
		}
		
		ImGui::EndTabItem();
	}

}

void AxisNode::configurationTab(){
	if(ImGui::BeginTabItem("Configuration")){
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("General")){
			ImGui::PopFont();
			movementTypeParameter->gui(Fonts::sansBold15);
			positionUnitParameter->gui(Fonts::sansBold15);
			controlModeParameter->gui(Fonts::sansBold15);
			limitSignalTypeParameter->gui(Fonts::sansBold15);
		}else ImGui::PopFont();
	
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Actuators")){
			ImGui::PopFont();
			actuatorControlSettingsGui();
		}else ImGui::PopFont();
		
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Motion Feedback")){
			ImGui::PopFont();
			motionFeedbackSettingsGui();
		}else ImGui::PopFont();
		
		
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Limits")){
			ImGui::PopFont();
			limitSettingsGui();
		}else ImGui::PopFont();
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Position Control")){
			ImGui::PopFont();
			positionControlSettingsGui();
		}else ImGui::PopFont();
		
		if(controlModeParameter->value == ControlMode::POSITION_CONTROL){
			ImGui::PushFont(Fonts::sansBold20);
			if(ImGui::CollapsingHeader("Homing")){
				ImGui::PopFont();
				homingSettingsGui();
			}else ImGui::PopFont();
		}
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Miscellaneous")){
			ImGui::PopFont();
			maxEnableTimeSeconds->gui(Fonts::sansBold15);
		}else ImGui::PopFont();
		
		ImGui::EndTabItem();
	}
}

void AxisNode::motionFeedbackSettingsGui(){
	
	if(ImGui::Button("Add Feedback Mapping")) addNewFeedbackMapping();
	
	
	std::shared_ptr<FeedbackMapping> removedMapping = nullptr;
	for(int i = 0; i < feedbackMappings.size(); i++){
		
		ImGui::Separator();
		
		ImGui::PushID(i);
		auto feedbackMapping = feedbackMappings[i];
		
		ImGui::PushFont(Fonts::sansBold20);
		if(buttonCross("##remove")) removedMapping = feedbackMapping;
		ImGui::SameLine();
		ImGui::Text("%s :", feedbackMapping->feedbackPin->displayString);
		ImGui::SameLine();
		if(!feedbackMapping->isFeedbackConnected()) ImGui::TextColored(Colors::red, "Not Connected");
		else ImGui::Text("%s", feedbackMapping->getName().c_str());
		ImGui::PopFont();
		
		ImGui::Text("Device Units per Axis Units");
		feedbackMapping->deviceUnitsPerAxisUnits->gui();

		if(feedbackMapping->isFeedbackConnected()){
			auto feedbackDevice = feedbackMapping->getFeedbackInterface();
			if(ImGui::BeginTable("##feedbackProperties", 2, ImGuiTableFlags_Borders)){
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Supports Position Feedback");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", feedbackDevice->supportsPosition() ? "Yes" : "No");
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Supports Velocity Feedback");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", feedbackDevice->supportsVelocity() ? "Yes" : "No");
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Position Working Range Min");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", feedbackDevice->getPositionLowerWorkingRangeBound(), feedbackDevice->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Position Working Range Max");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", feedbackDevice->getPositionUpperWorkingRangeBound(), feedbackDevice->getPositionUnit()->abbreviated);
				
				ImGui::EndTable();
			}
		}
		
		ImGui::PopID();
	}
	if(removedMapping) removeFeedbackMapping(removedMapping);
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Feedback Selection");
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Position Feedback");
	ImGui::PopFont();
	std::string selectedPositionFeedbackMappingName;
	if(selectedPositionFeedbackMapping == nullptr) selectedPositionFeedbackMappingName = "None";
	else selectedPositionFeedbackMappingName = selectedPositionFeedbackMapping->getName();
	if(ImGui::BeginCombo("##PositionFeedbackMapping", selectedPositionFeedbackMappingName.c_str())){
		for(auto feedbackMapping : feedbackMappings){
			ImGui::BeginDisabled(!feedbackMapping->isFeedbackConnected() || !feedbackMapping->getFeedbackInterface()->supportsPosition());
			if(ImGui::Selectable(feedbackMapping->getName().c_str(), feedbackMapping == selectedPositionFeedbackMapping)){
				selectPositionFeedbackMapping(feedbackMapping);
			}
			ImGui::EndDisabled();
		}
		for(auto actuatorMapping : actuatorMappings){
			ImGui::BeginDisabled(!actuatorMapping->isActuatorConnected() || !actuatorMapping->getActuatorInterface()->supportsPosition());
			if(ImGui::Selectable(actuatorMapping->getName().c_str(), actuatorMapping == selectedPositionFeedbackMapping)){
				selectPositionFeedbackMapping(actuatorMapping);
			}
			ImGui::EndDisabled();
		}
		ImGui::EndCombo();
	}
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Velocity Feedback");
	ImGui::PopFont();
	std::string selectedVelocityFeedbackMappingName;
	if(selectedVelocityFeedbackMapping == nullptr) selectedVelocityFeedbackMappingName = "None";
	else selectedVelocityFeedbackMappingName = selectedVelocityFeedbackMapping->getName();
	if(ImGui::BeginCombo("##VelocityFeedbackMapping", selectedVelocityFeedbackMappingName.c_str())){
		for(auto feedbackMapping : feedbackMappings){
			ImGui::BeginDisabled(!feedbackMapping->isFeedbackConnected() || !feedbackMapping->getFeedbackInterface()->supportsVelocity());
			if(ImGui::Selectable(feedbackMapping->getName().c_str(), feedbackMapping == selectedVelocityFeedbackMapping)){
				selectVelocityFeedbackMapping(feedbackMapping);
			}
			ImGui::EndDisabled();
		}
		for(auto actuatorMapping : actuatorMappings){
			ImGui::BeginDisabled(!actuatorMapping->isActuatorConnected() || !actuatorMapping->getActuatorInterface()->supportsVelocity());
			if(ImGui::Selectable(actuatorMapping->getName().c_str(), actuatorMapping == selectedVelocityFeedbackMapping)){
				selectVelocityFeedbackMapping(actuatorMapping);
			}
			ImGui::EndDisabled();
		}
		ImGui::EndCombo();
	}
	
	ImGui::Separator();
	
	useExternalLoadSensor_Param->gui(Fonts::sansBold15);
	forceSensorMultiplier_Param->gui(Fonts::sansBold15);
	forceSensorOffset_Param->gui(Fonts::sansBold15);
	ImGui::SameLine();
	if(ImGui::Button("Tare Sensor")) b_shouldTareForceSensor = true;
	
}

void AxisNode::actuatorControlSettingsGui(){
	
	if(ImGui::Button("Add Actuator Mapping")) addNewActuatorMapping();
	
	std::shared_ptr<ActuatorMapping> removedMapping = nullptr;
	for(int i = 0; i < actuatorMappings.size(); i++){
		
		ImGui::Separator();
		
		ImGui::PushID(i);
		auto actuatorMapping = actuatorMappings[i];
		
		ImGui::PushFont(Fonts::sansBold20);
		if(buttonCross("##remove")) removedMapping = actuatorMapping;
		ImGui::SameLine();
		ImGui::Text("%s :", actuatorMapping->actuatorPin->displayString);
		ImGui::SameLine();
		if(!actuatorMapping->isFeedbackConnected()) ImGui::TextColored(Colors::red, "Not Connected");
		else ImGui::Text("%s", actuatorMapping->getName().c_str());
		ImGui::PopFont();
		
		ImGui::Text("Device Units per Axis Units");
		actuatorMapping->deviceUnitsPerAxisUnits->gui();
		
		ImGui::Text("Actuator Control Mode");
		actuatorMapping->controlModeParameter->gui();
		
		if(actuatorMapping->isActuatorConnected()){
			auto actuatorDevice = actuatorMapping->getActuatorInterface();
			if(ImGui::BeginTable("##feedbackProperties", 2, ImGuiTableFlags_Borders)){
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Supports Position Control");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", actuatorDevice->supportsPositionControl() ? "Yes" : "No");
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Supports Velocity Control");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", actuatorDevice->supportsVelocityControl() ? "Yes" : "No");
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Max Velocity");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", actuatorDevice->getVelocityLimit(), actuatorDevice->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Max Acceleration");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", actuatorDevice->getAccelerationLimit(), actuatorDevice->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Supports Position Feedback");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", actuatorDevice->supportsPosition() ? "Yes" : "No");
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Supports Velocity Feedback");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", actuatorDevice->supportsVelocity() ? "Yes" : "No");
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Position Working Range Min");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", actuatorDevice->getPositionLowerWorkingRangeBound(), actuatorDevice->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Position Working Range Max");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", actuatorDevice->getPositionUpperWorkingRangeBound(), actuatorDevice->getPositionUnit()->abbreviated);
				
				ImGui::EndTable();
			}
		}
		
		ImGui::PopID();
	}
	if(removedMapping) removeActuatorMapping(removedMapping);
	
}

void AxisNode::limitSettingsGui(){

	velocityLimit->gui(Fonts::sansBold15);
	std::ostringstream actVelLimString;
	actVelLimString << "Max: " << std::fixed << std::setprecision(3) << actuatorVelocityLimit << " u/s";
	ImGui::SameLine();
	backgroundText(actVelLimString.str().c_str(), Colors::gray, Colors::black);
	
	accelerationLimit->gui(Fonts::sansBold15);
	std::ostringstream actAccLimString;
	actAccLimString << "Max: " << std::fixed << std::setprecision(3) << actuatorAccelerationLimit << " u/s\xc2\xb2";
	ImGui::SameLine();
	backgroundText(actAccLimString.str().c_str(), Colors::gray, Colors::black);
	
	upperForceLimit->gui(Fonts::sansBold15);
	lowerForceLimit->gui(Fonts::sansBold15);
	
	ImGui::Separator();
	
	ImGui::BeginDisabled(controlMode != ControlMode::POSITION_CONTROL);
	
	enableLowerPositionLimit->gui(Fonts::sansBold15);
	lowerPositionLimit->gui(Fonts::sansBold15);
	std::ostringstream lowPosLimString;
	lowPosLimString << "Min: " << std::fixed << std::setprecision(3) << feedbackLowerPositionLimit << " u";
	ImGui::SameLine();
	backgroundText(lowPosLimString.str().c_str(), Colors::gray, Colors::black);
	
	lowerPositionLimitClearance->gui(Fonts::sansBold15);
	
	ImGui::Separator();
	
	enableUpperPositionLimit->gui(Fonts::sansBold15);
	upperPositionLimit->gui(Fonts::sansBold15);
	std::ostringstream highPosLimString;
	highPosLimString << "Max: " << std::fixed << std::setprecision(3) << feedbackUpperPositionLimit << " u";
	ImGui::SameLine();
	backgroundText(highPosLimString.str().c_str(), Colors::gray, Colors::black);
	
	upperPositionLimitClearance->gui(Fonts::sansBold15);
	
	ImGui::EndDisabled();
}

void AxisNode::positionControlSettingsGui(){
	
	ImGui::BeginDisabled(controlModeParameter->value != ControlMode::POSITION_CONTROL);
	positionLoop_velocityFeedForward->gui(Fonts::sansBold15);
	positionLoop_proportionalGain->gui(Fonts::sansBold15);
	positionLoop_maxError->gui(Fonts::sansBold15);
	positionLoop_minError->gui(Fonts::sansBold15);
	positionLoop_errorTimeout_milliseconds->gui(Fonts::sansBold15);
	ImGui::EndDisabled();
	
	//velocityLoop_maxError->gui(Fonts::sansBold15);
	ImGui::BeginDisabled(limitSignalTypeParameter->value != LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS);
	limitSlowdownVelocity->gui(Fonts::sansBold15);
	ImGui::EndDisabled();
}

void AxisNode::homingSettingsGui(){
	homingDirectionParameter->gui(Fonts::sansBold15);
	signalApproachParameter->gui(Fonts::sansBold15);
	homingVelocityCoarse->gui(Fonts::sansBold15);
	homingVelocityFine->gui(Fonts::sansBold15);
	maxHomingDistanceCoarse->gui(Fonts::sansBold15);
	maxHomingDistanceFine->gui(Fonts::sansBold15);
}

void AxisNode::devicesTab(){
	
	if(ImGui::BeginTabItem("Devices")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Connected Devices");
		ImGui::PopFont();
		
		for(auto device : connectedDeviceInterfaces){
			backgroundText(device->getName().c_str(), ImVec2(0,0), Colors::darkGray, Colors::white, ImDrawFlags_RoundCornersLeft);
			glm::vec4 deviceStatusColor;
			std::string deviceStatusString;
			switch(device->getState()){
				case DeviceState::OFFLINE:
					deviceStatusColor = Colors::blue;
					deviceStatusString = "Offline";
					break;
				case DeviceState::NOT_READY:
					deviceStatusColor = Colors::red;
					deviceStatusString = "Not Ready";
					break;
				case DeviceState::READY:
					deviceStatusColor = Colors::yellow;
					deviceStatusString = "Ready";
					break;
				case DeviceState::ENABLING:
					deviceStatusColor = Colors::yellow;
					deviceStatusString = "Enabling...";
					break;
				case DeviceState::DISABLING:
					deviceStatusColor = Colors::yellow;
					deviceStatusString = "Disabling...";
					break;
					break;
				case DeviceState::ENABLED:
					deviceStatusColor = Colors::green;
					deviceStatusString = "Enabled";
					break;
			}
			ImGui::SameLine(0.0,0.0);
			backgroundText(deviceStatusString.c_str(), ImVec2(0,0), deviceStatusColor, Colors::black, ImDrawFlags_RoundCornersRight);
			ImGui::TreePush();
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("%s", device->getStatusString().c_str());
			ImGui::PopStyleColor();
			ImGui::TreePop();
		}
		
		ImGui::EndTabItem();
	}

}

void AxisNode::axisInterfaceTab(){
	if(ImGui::BeginTabItem("Axis Interface")){
		
		
		std::string stateString;
		switch(axisInterface->getState()){
			case DeviceState::OFFLINE: 		stateString = "Offline"; break;
			case DeviceState::NOT_READY:	stateString = "Not Ready"; break;
			case DeviceState::READY:		stateString = "Ready"; break;
			case DeviceState::DISABLING:	stateString = "Disabling"; break;
			case DeviceState::ENABLING:		stateString = "Enabling"; break;
			case DeviceState::ENABLED:		stateString = "Enabled"; break;
				break;
		}
		ImGui::Text("State: %s", stateString.c_str());
		
		auto& config = axisInterface->configuration;
		if(ImGui::BeginTable("##config", 2, ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit)){
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_supportsPositionFeedback");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", config.b_supportsPositionFeedback ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_supportsVelocityFeedback");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", config.b_supportsVelocityFeedback ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_supportsForceFeedback");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", config.b_supportsForceFeedback ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_supportsEffortFeedback");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", config.b_supportsEffortFeedback ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_supportsHoming");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", config.b_supportsHoming ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("lowerPositionLimit");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", config.lowerPositionLimit);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("upperPositionLimit");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", config.upperPositionLimit);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("velocityLimit");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", config.velocityLimit);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("accelerationLimit");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", config.accelerationLimit);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("decelerationLimit");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", config.decelerationLimit);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("controlMode");
			ImGui::TableSetColumnIndex(1);
			switch(config.controlMode){
				case AxisInterface::ControlMode::POSITION_CONTROL:	ImGui::Text("Position Control"); break;
				case AxisInterface::ControlMode::VELOCITY_CONTROL:	ImGui::Text("Velocity Control"); break;
				case AxisInterface::ControlMode::NONE:				ImGui::Text("No Control"); break;
			}
			ImGui::EndTable();
		}
		
		auto& process = axisInterface->processData;
		if(ImGui::BeginTable("##process", 2, ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit)){
			
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_canStartHoming");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", process.b_canStartHoming ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_startHoming");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", process.b_startHoming ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_stopHoming");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", process.b_stopHoming ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_isHoming");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", process.b_isHoming ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_didHomingSucceed");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", process.b_didHomingSucceed ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_isEmergencyStopActive");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", process.b_isEmergencyStopActive ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_enable");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", process.b_enable ? "true" : "false");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("b_disable");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", process.b_disable ? "true" : "false");
			
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("positionActual");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", process.positionActual);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("velocityActual");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", process.velocityActual);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("forceActual");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", process.forceActual);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("effortActual");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", process.effortActual);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("positionTarget");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", process.positionTarget);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("velocityTarget");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", process.velocityTarget);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("accelerationTarget");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f", process.accelerationTarget);

			ImGui::EndTable();
		}
		
		ImGui::EndTabItem();
	}
}
