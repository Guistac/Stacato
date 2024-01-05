#include <pch.h>

#include "MultiAxisMachine.h"

#include "Motion/Axis/AxisNode.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/NodeGraph/DeviceNode.h"

#include "Environnement/Environnement.h"


void MultiAxisMachine::controlsGui() {
	widgetGui();
}

void MultiAxisMachine::settingsGui() {
	
	
	ImGui::Checkbox("Allow User Homing", &b_allowUserHoming);
	ImGui::Checkbox("Show Load", &b_showLoad);
	ImGui::Checkbox("Allow User Limit Setting", &b_allowUserLimitSettings);
	ImGui::Checkbox("Allow User Underload Surveillance Toggle", &b_allowUserUnderloadSurveillanceToggle);
	
	if(ImGui::Button("Add Axis")) addAxisMapping();
	
	std::shared_ptr<AxisMapping> deletedMapping = nullptr;
	
	for(int i = 0; i < axisMappings.size(); i++){
		
		ImGui::PushID(i);
		
		auto axisMapping = axisMappings[i];
		
		ImGui::PushFont(Fonts::sansBold20);
		if(buttonCross("##delete", ImGui::GetTextLineHeight())) deletedMapping = axisMapping;
		ImGui::PopFont();
		
		ImGui::SameLine();
		
		if(!axisMapping->axisPin->isConnected()){
			
			ImGui::PushFont(Fonts::sansBold20);
			ImGui::Text("Axis %i :", i);
			ImGui::SameLine();
			ImGui::TextColored(Colors::red, "No Axis Connected");
			ImGui::PopFont();
			
		}
		else{
			
			auto axis = axisMapping->axisPin->getConnectedPin()->getSharedPointer<AxisInterface>();
			auto animatable = axisMapping->animatablePosition;
			
			ImGui::PushFont(Fonts::sansBold20);
			ImGui::Text("Axis %i : %s", i, axis->getName().c_str());
			ImGui::PopFont();
			
			if(ImGui::BeginTable("##axisProperties", 2, ImGuiTableFlags_Borders)){
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Lower Position Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", axis->getLowerPositionLimit(), axis->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Upper Position Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", axis->getUpperPositionLimit(), axis->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Velocity Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s/s", axis->getVelocityLimit(), axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();
				
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Acceleration Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s/s\xc2\xb2", axis->getAccelerationLimit(), axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Animatable Lower Position Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", animatable->lowerPositionLimit, axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();
				
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Animatable Upper Position Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", animatable->upperPositionLimit, axis->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Animatable Velocity Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s/s", animatable->velocityLimit, axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();
				
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Animatable Acceleration Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s/s\xc2\xb2", animatable->accelerationLimit, axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();

				
				
				ImGui::EndTable();
			}
			
			if(ImGui::Checkbox("Invert Direction", &axisMapping->b_invertDirection)){
				axisMapping->updateAnimatableParameters();
			}
			ImGui::Text("Position Offset");
			if(ImGui::InputDouble("##PositionOffset", &axisMapping->userPositionOffset)){
				axisMapping->updateAnimatableParameters();
			}
			ImGui::Text("Minimum Load");
			ImGui::InputDouble("##MinimumLoad", &axisMapping->minimumLoad_Kilograms, 0, 0, "%.0fKg");
			
		}
		
		ImGui::PopID();
		
		ImGui::Separator();
		
	}
	if(deletedMapping) removeAxisMapping(deletedMapping);
	
}



void MultiAxisMachine::axisGui() {}
void MultiAxisMachine::deviceGui() {}
void MultiAxisMachine::metricsGui() {}


void MultiAxisMachine::ControlWidget::gui(){
	
	glm::vec2 headerCursorPos = machine->reserveSpaceForMachineHeaderGui();
	
	ImGui::BeginGroup();
	machine->widgetGui();
	ImGui::EndGroup();
	
	float widgetWidth = ImGui::GetItemRectSize().x;
	machine->machineHeaderGui(headerCursorPos, widgetWidth);
	machine->machineStateControlGui(widgetWidth);
	
}

void MultiAxisMachine::widgetGui(){
	
	if(axisMappings.size() == 0){
		ImGui::TextColored(Colors::red, "No Axis Configured");
		return;
	}
	
	
	float controlsHeight = ImGui::GetTextLineHeight() * 10.0;
	
	for(int i = 0; i < axisMappings.size(); i++){
		auto mapping = axisMappings[i];
		auto animatable = mapping->animatablePosition;
		bool b_axisConnected = mapping->axisPin->isConnected();
		Units::Type unitType = b_axisConnected ? mapping->getAxis()->getPositionUnit()->unitType : Units::Type::LINEAR_DISTANCE;
		
		ImGui::PushID(i);
		ImGui::BeginGroup();
		
		ImGui::BeginDisabled(!b_axisConnected);
		if(unitType == Units::Type::LINEAR_DISTANCE) mapping->controlGui();
		else if(unitType == Units::Type::ANGULAR_DISTANCE) mapping->angularControlGui();
		ImGui::EndDisabled();
		
		float controlsWidth = ImGui::GetItemRectSize().x;
		
		if(b_showLoad){
			ImVec2 loadStatusSize(controlsWidth, ImGui::GetTextLineHeightWithSpacing());
			static char loadString[64];
			if(b_axisConnected) snprintf(loadString, 64, "%.0fKg", mapping->getAxis()->getForceActual() / 10.0);
			else snprintf(loadString, 64, "---");
			backgroundText(loadString, loadStatusSize, Colors::darkGray, Colors::white);
		}
		
		bool b_disableAxisStateControl;
		bool b_axisEnabled;
		std::string stateString;
		glm::vec4 stateColor;
		if(!b_axisConnected) {
			stateString = "No Axis";
			stateColor = Colors::darkRed;
			b_disableAxisStateControl = true;
			b_axisEnabled = false;
		}else{
			switch(mapping->getAxis()->getState()){
				case DeviceState::OFFLINE:
					stateString = "Offline";
					stateColor = Colors::blue;
					b_disableAxisStateControl = true;
					b_axisEnabled = false;
					break;
				case DeviceState::NOT_READY:
					stateString = "Not Ready";
					stateColor = Colors::red;
					b_disableAxisStateControl = true;
					b_axisEnabled = false;
					break;
				case DeviceState::READY:
					stateString = "Ready";
					stateColor = Colors::yellow;
					b_disableAxisStateControl = false;
					b_axisEnabled = false;
					break;
				case DeviceState::DISABLING:
					stateString = "Disabling...";
					stateColor = Colors::darkYellow;
					b_disableAxisStateControl = false;
					b_axisEnabled = false;
					break;
				case DeviceState::ENABLING:
					stateString = "Enabling...";
					stateColor = Colors::darkGreen;
					b_disableAxisStateControl = false;
					b_axisEnabled = false;
					break;
				case DeviceState::ENABLED:
					stateString = "Enabled";
					stateColor = Colors::green;
					b_disableAxisStateControl = false;
					b_axisEnabled = true;
					break;
			}
		}
		
		ImVec2 buttonSize(controlsWidth / 2.0, ImGui::GetTextLineHeight() * 2.0);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::BeginDisabled(b_disableAxisStateControl || b_enableGroupSurveillance);
		if(b_axisEnabled){
			if(customButton("Disable", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft)){
				mapping->disableAxis();
			}
		}else{
			if(customButton("Enable", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft)){
				mapping->enableAxis();
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine(0.0, 0.0);
		backgroundText(stateString.c_str(), buttonSize, stateColor, Colors::black, ImDrawFlags_RoundCornersRight);
		ImGui::PopFont();
		
		
		glm::vec2 nameBoxSize(controlsWidth, ImGui::GetTextLineHeightWithSpacing());
		if(b_axisConnected){
			std::string axisName = mapping->axisPin->getConnectedPin()->getSharedPointer<AxisInterface>()->getName();
			ImVec4 backgroundColor;
			if(!b_enableGroupSurveillance || !b_enableUnderloadSurveillance) backgroundColor = Timing::getBlink(0.1) ? Colors::red : Colors::yellow;
			else backgroundColor = Colors::darkGray;
			
			if(!b_enableGroupSurveillance && Timing::getBlink(1.0)) backgroundText("No Group Safety", nameBoxSize, backgroundColor);
			else if(!b_enableUnderloadSurveillance && Timing::getBlink(1.0)) backgroundText("No Underload", nameBoxSize, backgroundColor);
			else backgroundText(axisName.c_str(), nameBoxSize, backgroundColor);
		}
		else{
			backgroundText("No Axis", nameBoxSize, Colors::darkRed, Colors::red);
		}
		
		ImGui::EndGroup();
		ImGui::PopID();
	
		if(i < axisMappings.size() - 1) ImGui::SameLine();
	}
	
	ImGui::SameLine();
	
	float masterControlHeight = ImGui::GetItemRectSize().y - ImGui::GetTextLineHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y;
	
	ImGui::BeginGroup();
	static double min = -1.0;
	static double max = 1.0;
	ImVec2 masterSliderSize(ImGui::GetTextLineHeight() * 3.0, masterControlHeight);
	if(ImGui::VSliderScalar("##ManualVelocity", masterSliderSize, ImGuiDataType_Double, &masterVelocityCommand, &min, &max, "")){
		setMasterVelocityTarget(masterVelocityCommand);
	}
	if(ImGui::IsItemDeactivatedAfterEdit()){
		masterVelocityCommand = 0.0;
		setMasterVelocityTarget(0.0);
	}
	backgroundText("Master", ImVec2(masterSliderSize.x, ImGui::GetTextLineHeightWithSpacing()), Colors::darkGray, Colors::white);
	ImGui::EndGroup();
	
}

void MultiAxisMachine::setupGui(){
	
	ImGui::Checkbox("Enable Group Surveillance", &b_enableGroupSurveillance);
	
	if(b_allowUserUnderloadSurveillanceToggle){
		ImGui::Checkbox("Enable Underload Surveilllance", &b_enableUnderloadSurveillance);
	}
	
	for(int i = 0; i < axisMappings.size(); i++){
        ImGui::PushID(i);
		auto axisMapping = axisMappings[i];
		ImGui::Separator();
		if(!axisMapping->isAxisConnected()) ImGui::TextColored(Colors::red, "No Axis Connected");
		else {
			ImGui::PushFont(Fonts::sansBold15);
			backgroundText(axisMappings[i]->getAxis()->getName().c_str(), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()), Colors::darkGray);
			ImGui::PopFont();
			
			if(b_allowUserLimitSettings){
				ImGui::PushID(i);
				axisMappings[i]->setupGui();
				ImGui::PopID();
			}
		}
		if(b_allowUserHoming && axisMapping->isAxisConnected()){
			
			auto axis = axisMapping->getAxis();
			
			glm::vec2 homingButtonSize(ImGui::GetTextLineHeight() * 6.0, ImGui::GetFrameHeight());
			
			if(axis->isHoming()){
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				if(ImGui::Button("Stop Homing", homingButtonSize)) axis->stopHoming();
				ImGui::PopStyleColor();
			}else{
                ImGui::BeginDisabled(!axis->canStartHoming());
				if(ImGui::Button("Start Homing", homingButtonSize)) axis->startHoming();
                ImGui::EndDisabled();
			}


			ImGui::SameLine();

			ImVec4 progressIndicatorColor = Colors::darkGray;
			if(axis->isHoming()) progressIndicatorColor = Colors::orange;
			else if(axis->didHomingSucceed()) progressIndicatorColor = Colors::green;
			else if(!axis->isHoming() && !axis->didHomingSucceed()) progressIndicatorColor = Colors::red;

			glm::vec2 homingProgressSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
			std::string homingString = axis->getHomingStepString();
			backgroundText(homingString.c_str(), homingProgressSize, Colors::darkGray);
			
			
			
		}
        ImGui::PopID();
	}
	
}
