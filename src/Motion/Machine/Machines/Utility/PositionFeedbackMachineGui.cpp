#include <pch.h>
#include "PositionFeedbackMachine.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Motion/SubDevice.h"
#include "NodeGraph/Device.h"

void PositionFeedbackMachine::controlsGui(){
	
	if(!isFeedbackConnected()){
		ImGui::TextColored(Colors::red, "No Feedback Device Connected");
		return;
	}
	
	auto feedbackDevice = getFeedbackDevice();
	
	float widgetWidth = ImGui::GetContentRegionAvail().x;
	
	ImGui::Text("Current Position (in range from %.1f %s to %.1f %s",
				feedbackPositionToMachinePosition(feedbackDevice->getMinPosition()),
				getPositionUnit(positionUnit)->shortForm,
				feedbackPositionToMachinePosition(feedbackDevice->getMaxPosition()),
				getPositionUnit(positionUnit)->shortForm);
	static char positionString[256];
	sprintf(positionString, "%.3f %s", position, getPositionUnit(positionUnit)->shortForm);
	ImGui::ProgressBar(feedbackDevice->getPositionInRange(), glm::vec2(widgetWidth, ImGui::GetFrameHeight()), positionString);
	
	ImGui::Text("Current Velocity: %.3f %s/s", velocity, getPositionUnit(positionUnit)->shortForm);
	
	if(ImGui::Button("Hard Reset Position Feedback")){
		feedbackDevice->hardReset();
	}
	
}


void PositionFeedbackMachine::settingsGui(){
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Encoder Channel");
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Feedback Device :");
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
	ImGui::Text("%s", getPositionUnit(feedbackDevice->getPositionUnit())->displayName);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Movement Type :");
	ImGui::PopFont();
	if (ImGui::BeginCombo("##UnitType", getPositionUnitType(movementType)->displayName)) {
		for (PositionUnitType& unitType : getPositionUnitTypes()) {
			if (ImGui::Selectable(unitType.displayName, movementType == unitType.type)) {
				movementType = unitType.type;
				//if the machine type is changed but the machine unit is of the wrong type
				//change the machine unit to the first correct type automatically
				if (getPositionUnit(positionUnit)->type != unitType.type) {
					switch (unitType.type) {
					case PositionUnit::Type::ANGULAR:
						positionUnit = getAngularPositionUnits().front().unit;
						break;
					case PositionUnit::Type::LINEAR:
						positionUnit = getLinearPositionUnits().front().unit;
						break;
					}
				}
			}
		}
		ImGui::EndCombo();
	}

	float widgetWidth = ImGui::GetItemRectSize().x;

	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Position Unit :");
	ImGui::PopFont();
	if (ImGui::BeginCombo("##AxisUnit", getPositionUnit(positionUnit)->displayName)) {
		if (movementType == PositionUnit::Type::LINEAR) {
			for (PositionUnit& unit : getLinearPositionUnits()) {
				if (ImGui::Selectable(unit.displayName, positionUnit == unit.unit)) positionUnit = unit.unit;
			}
		}
		else if (movementType == PositionUnit::Type::ANGULAR) {
			for (PositionUnit& unit : getAngularPositionUnits()) {
				if (ImGui::Selectable(unit.displayName, positionUnit == unit.unit)) positionUnit = unit.unit;
			}
		}
		ImGui::EndCombo();
	}
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("%s per Feedback %s :", getPositionUnit(positionUnit)->displayNamePlural, getPositionUnit(feedbackDevice->getPositionUnit())->displayName);
	ImGui::PopFont();
	ImGui::InputDouble("##conversionRatio", &unitsPerFeedbackUnit);
	
	ImGui::Checkbox("Invert Direction of Motion", &b_invertDirection);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Position at Feedback Zero");
	ImGui::PopFont();
	static char offsetString[256];
	sprintf(offsetString, "%.3f %s", unitOffset, getPositionUnit(positionUnit)->shortForm);
	ImGui::InputDouble("##posatZero", &unitOffset, 0.0, 0.0, offsetString);
	
	widgetWidth = ImGui::GetContentRegionAvail().x;
	
	ImGui::Text("Current Position (in range from %.1f %s to %.1f %s",
				feedbackPositionToMachinePosition(feedbackDevice->getMinPosition()),
				getPositionUnit(positionUnit)->shortForm,
				feedbackPositionToMachinePosition(feedbackDevice->getMaxPosition()),
				getPositionUnit(positionUnit)->shortForm);
	static char positionString[256];
	sprintf(positionString, "%.3f %s", position, getPositionUnit(positionUnit)->shortForm);
	ImGui::ProgressBar(feedbackDevice->getPositionInRange(), glm::vec2(widgetWidth, ImGui::GetFrameHeight()), positionString);
	
	ImGui::Text("Current Velocity: %.3f %s/s", velocity, getPositionUnit(positionUnit)->shortForm);
	
	if(ImGui::Button("Hard Reset Position Feedback")){
		feedbackDevice->hardReset();
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
		sprintf(positionString, "%.3f %s", position, getPositionUnit(feedbackDevice->getPositionUnit())->shortForm);
		sprintf(velocityString, "%.3f %s/s", velocity, getPositionUnit(feedbackDevice->getPositionUnit())->shortForm);
		sprintf(positionProgressString, "%.1f%%", progressNormalized * 100.0);
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
	
	if(disableResetButton) BEGIN_DISABLE_IMGUI_ELEMENT
	if(ImGui::Button("Hard Reset", widgetSize)) getFeedbackDevice()->hardReset();
	if(disableResetButton) END_DISABLE_IMGUI_ELEMENT
	
	
}

