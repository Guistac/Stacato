#include <pch.h>
#include "PositionFeedbackMachine.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Motion/SubDevice.h"
#include "Environnement/DeviceNode.h"

void PositionFeedbackMachine::controlsGui(){
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position Feedback");
	ImGui::PopFont();
	
	if(!isFeedbackConnected()){
		ImGui::TextColored(Colors::red, "No Feedback Device Connected");
		return;
	}
	
	auto feedbackDevice = getFeedbackDevice();
	
	float widgetWidth = ImGui::GetContentRegionAvail().x;
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Current Position :");
	ImGui::PopFont();
	ImGui::SameLine();
	const char * shortPositionUnitString = positionUnit->abbreviated;
	ImGui::Text("%.3f %s (in range from %.1f %s to %.1f %s)",
				position,
				shortPositionUnitString,
				feedbackPositionToMachinePosition(feedbackDevice->getMinPosition()),
				shortPositionUnitString,
				feedbackPositionToMachinePosition(feedbackDevice->getMaxPosition()),
				shortPositionUnitString);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Current Velocity :");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text("%.3f %s/s", velocity, shortPositionUnitString);
	
	static char workingRangeString[256];
	sprintf(workingRangeString, "Working Range: %.3f%%", feedbackDevice->getPositionInRange() * 100.0);
	ImGui::ProgressBar(feedbackDevice->getPositionInRange(), glm::vec2(widgetWidth, ImGui::GetFrameHeight()), workingRangeString);
	
	if(ImGui::Button("Hard Reset Position Feedback")){
		feedbackDevice->hardReset();
	}
	
}


void PositionFeedbackMachine::settingsGui(){
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Feedback Settings");
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Device :");
	ImGui::PopFont();
	ImGui::SameLine();
	if(!isFeedbackConnected()){
		ImGui::TextColored(Colors::red, "No Device Connected");
		return;
	}
	
	auto feedbackDevice = getFeedbackDevice();
	ImGui::Text("%s on %s", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Device Unit :");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text("%s", feedbackDevice->getPositionUnit()->singular);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Movement Type :");
	ImGui::PopFont();
	if (ImGui::BeginCombo("##UnitType", Enumerator::getDisplayString(movementType))) {
		for (auto& unitType : Enumerator::getTypes<MovementType>()) {
			if (ImGui::Selectable(unitType.displayString, movementType == unitType.enumerator)) {
				setMovementType(unitType.enumerator);
			}
		}
		ImGui::EndCombo();
	}

	float widgetWidth = ImGui::GetItemRectSize().x;

	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Position Unit :");
	ImGui::PopFont();
	
	if (ImGui::BeginCombo("##AxisUnit", positionUnit->singular)) {
		
		switch(movementType){
			case MovementType::ROTARY:
				for(Unit unit : Units::AngularDistance::get()){
					if (ImGui::Selectable(unit->singular, positionUnit == unit)) setPositionUnit(unit);
				}
				break;
			case MovementType::LINEAR:
				for(Unit unit : Units::LinearDistance::get()){
					if (ImGui::Selectable(unit->singular, positionUnit == unit)) setPositionUnit(unit);
				}
		}
		ImGui::EndCombo();
	}
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("%s per Feedback %s :", positionUnit->plural, feedbackDevice->getPositionUnit()->singular);
	ImGui::PopFont();
	ImGui::InputDouble("##conversionRatio", &machineUnitsPerFeedbackUnit);
	
	ImGui::Checkbox("Invert Direction of Motion", &b_invertDirection);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Position at Feedback Zero");
	ImGui::PopFont();
	static char offsetString[256];
	sprintf(offsetString, "%.3f %s", machineUnitOffset, positionUnit->abbreviated);
	ImGui::InputDouble("##posatZero", &machineUnitOffset, 0.0, 0.0, offsetString);
	
	widgetWidth = ImGui::GetContentRegionAvail().x;
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Feedback Position :");
	ImGui::PopFont();
	ImGui::SameLine();
	const char* feedbackDeviceUnitShortString = feedbackDevice->getPositionUnit()->abbreviated;
	ImGui::Text("%.3f %s (in range from %.1f %s to %.1f %s)",
				feedbackDevice->getPosition(),
				feedbackDeviceUnitShortString,
				feedbackDevice->getMinPosition(),
				feedbackDeviceUnitShortString,
				feedbackDevice->getMaxPosition(),
				feedbackDeviceUnitShortString);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Machine Position :");
	ImGui::PopFont();
	ImGui::SameLine();
	const char * positionUnitShortString = positionUnit->abbreviated;
	ImGui::Text("%.3f %s (in range from %.1f %s to %.1f %s)",
				position,
				positionUnitShortString,
				feedbackPositionToMachinePosition(feedbackDevice->getMinPosition()),
				positionUnitShortString,
				feedbackPositionToMachinePosition(feedbackDevice->getMaxPosition()),
				positionUnitShortString);
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Machine Velocity :");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text("%.3f %s/s", velocity, positionUnitShortString);
	
	static char workingRangeString[128];
	sprintf(workingRangeString, "Working Range: %.3f%%", feedbackDevice->getPositionInRange() * 100.0);
	ImGui::ProgressBar(feedbackDevice->getPositionInRange(), glm::vec2(widgetWidth, ImGui::GetFrameHeight()), workingRangeString);
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Zero & Scaling");
	ImGui::PopFont();
	
	if(ImGui::Button("Hard Reset Position Feedback")){
		feedbackDevice->hardReset();
	}
	
	static double positionScalingValue = 0.0;
	static char positionScalingString[128];
	sprintf(positionScalingString, "%.3f %s", positionScalingValue, positionUnitShortString);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
	ImGui::InputDouble("##scalingInput", &positionScalingValue, 0.0, 0.0, positionScalingString);
	ImGui::SameLine();
	if(ImGui::Button("Set Scaling")){
		setScalingPosition(positionScalingValue);
	}
}

void PositionFeedbackMachine::axisGui(){
	ImGui::Text("No Axis.");
}

void PositionFeedbackMachine::deviceGui(){
	if(ImGui::BeginTabBar("#channeldeviceTabBar")){
		std::vector<std::shared_ptr<Device>> devices;
		getDevices(devices);
		for(auto& device : devices){
			if(ImGui::BeginTabItem(device->getName())){
				if(ImGui::BeginTabBar("##deviceTabBar")){
					device->nodeSpecificGui();
					ImGui::EndTabBar();
				}
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
}

void PositionFeedbackMachine::metricsGui(){}

float PositionFeedbackMachine::getMiniatureWidth(){
	return ImGui::GetTextLineHeight() * 10.0;
}
void PositionFeedbackMachine::machineSpecificMiniatureGui(){
	
	std::shared_ptr<PositionFeedbackDevice> feedbackDevice;
	float progressNormalized = 1.0;
	static char positionString[128];
	static char positionProgressString[128];
	static char velocityString[128];
	glm::vec2 widgetSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
	bool disableResetButton = true;
	
	if(isEnabled()){
		feedbackDevice = getFeedbackDevice();
		progressNormalized = feedbackDevice->getPositionInRange();
		const char* positionUnitShortString = positionUnit->abbreviated;
		sprintf(positionString, "%.3f %s", position, positionUnitShortString);
		sprintf(velocityString, "%.3f %s/s", velocity, positionUnitShortString);
		sprintf(positionProgressString, "%.3f%%", progressNormalized * 100.0);
		disableResetButton = false;
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}else{
		sprintf(positionString, "Disabled.");
		sprintf(velocityString, "Disabled.");
		sprintf(positionProgressString, "");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
	}
	
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Position :");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text("%s", positionString);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Velocity :");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text("%s", velocityString);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Working Range :");
	ImGui::PopFont();
	ImGui::ProgressBar(progressNormalized, widgetSize, positionProgressString);
	ImGui::PopStyleColor();
	
	ImGui::Separator();
	
	ImGui::BeginDisabled(disableResetButton);
	if(ImGui::Button("Hard Reset", widgetSize)) getFeedbackDevice()->hardReset();
	ImGui::EndDisabled();
	
	
}

