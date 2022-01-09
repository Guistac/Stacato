#include <pch.h>

#include "PositionControlledSingleAxisMachine.h"
#include "Motion/Axis/PositionControlledAxis.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "NodeGraph/Device.h"

#include "Project/Environnement.h"


void PositionControlledSingleAxisMachine::controlsGui() {
	if(!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	glm::vec2 manualControlsSize = ImGui::GetContentRegionAvail();
	manualControlsSize.y -= axis->getFeedbackGuiHeight();
	ImGui::BeginChild("##manualMachineControls", manualControlsSize);

	float widgetWidth = ImGui::GetContentRegionAvail().x;
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Velocity Control");
	ImGui::PopFont();

	float manualVelocityTarget = manualVelocityTarget_machineUnitsPerSecond;
	ImGui::SetNextItemWidth(widgetWidth);
	ImGui::SliderFloat("##velTar", &manualVelocityTarget, -axis->getVelocityLimit_axisUnitsPerSecond(), axis->getVelocityLimit_axisUnitsPerSecond());
	if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityTarget);
	else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(0.0);

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Position Control");
	ImGui::PopFont();

	static char positionTargetString[128];
	sprintf(positionTargetString, "%.3f %s", manualPositionTarget_machineUnits, getPositionUnit(axis->positionUnit)->shortForm);
	ImGui::SetNextItemWidth(widgetWidth);
	ImGui::InputDouble("##postar", &manualPositionTarget_machineUnits, 0.0, 0.0, positionTargetString);

	glm::vec2 halfButtonSize((widgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);

	if (ImGui::Button("Move To Position", halfButtonSize)) {
		moveToPosition(manualPositionTarget_machineUnits);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop", halfButtonSize)) {
		setVelocityTarget(0.0);
	}

	ImGui::Separator();

	if (axis->isHomeable()) {

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Homing Controls");
		ImGui::PopFont();

		if (ImGui::Button("Start Homing", halfButtonSize)) {
		
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel Homing", halfButtonSize)) {
			
		}

		static char homingStateString[256];
		sprintf(homingStateString, "Homing State: %s", getHomingStep(axis->homingStep)->displayName);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
		ImGui::Button(homingStateString, glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 1.5));
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();


	}

	ImGui::EndChild();

	ImGui::Separator();

	axis->feedbackGui();
	
	//position
	//velocity
	//movement time
	//working range

	
	
}


void PositionControlledSingleAxisMachine::settingsGui() {
	if (!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("General Information :");
	ImGui::PopFont();

	if(ImGui::BeginTable("##machineInfo", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit)){
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Position Unit");
		ImGui::PopFont();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", getPositionUnit(axis->positionUnit)->displayName);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Lower Position Limit: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s", axis->getLowPositionLimit(), getPositionUnit(axis->positionUnit)->shortForm);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Upper Position Limit: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s", axis->getHighPositionLimit(), getPositionUnit(axis->positionUnit)->shortForm);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Velocity Limit: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s/s", axis->getVelocityLimit_axisUnitsPerSecond(), getPositionUnit(axis->positionUnit)->shortForm);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Acceleration Limit: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s/s\xC2\xB2", axis->getAccelerationLimit_axisUnitsPerSecondSquared(), getPositionUnit(axis->positionUnit)->shortForm);

		ImGui::EndTable();
	}

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Rapids");
	ImGui::PopFont();

	static char rapidVelocityString[128];
	static char rapidAccelerationString[128];

	ImGui::Text("Velocity for rapid movements :");
	sprintf(rapidVelocityString, "%.3f %s/s", rapidVelocity_machineUnitsPerSecond, getPositionUnit(axis->positionUnit)->shortForm);
	ImGui::InputDouble("##velRapid", &rapidVelocity_machineUnitsPerSecond, 0.0, 0.0, rapidVelocityString);
	rapidVelocity_machineUnitsPerSecond = std::min(rapidVelocity_machineUnitsPerSecond, axis->getVelocityLimit_axisUnitsPerSecond());

	ImGui::Text("Acceleration for rapid movements :");
	sprintf(rapidAccelerationString, "%.3f %s/s\xC2\xB2", rapidAcceleration_machineUnitsPerSecond, getPositionUnit(axis->positionUnit)->shortForm);
	ImGui::InputDouble("##accRapid", &rapidAcceleration_machineUnitsPerSecond, 0.0, 0.0, rapidAccelerationString);
	rapidAcceleration_machineUnitsPerSecond = std::min(rapidAcceleration_machineUnitsPerSecond, axis->getAccelerationLimit_axisUnitsPerSecondSquared());
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Machine Zero");
	ImGui::PopFont();
	
	ImGui::Text("Machine Zero (Axis Units) :");
	static char machineZeroString[128];
	sprintf(machineZeroString, "%.3f %s", machineZero_axisUnits, getPositionUnit(axis->positionUnit)->shortForm);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
	ImGui::InputDouble("##axisUnitOffset", &machineZero_axisUnits, 0.0, 0.0, machineZeroString);
	
	ImGui::SameLine();
	if(ImGui::Button("Capture Machine Zero")){
		captureMachineZero();
	}
	
	ImGui::Checkbox("Invert Axis Direction", &b_invertDirection);
	
	/*
	double axisPosition = simulationMotionProfile.getPosition();
	double axisVelocity = simulationMotionProfile.getVelocity();
	double axisLowLimit = axis->getLowPositionLimit();
	double axisHighLimit = axis->getHighPositionLimit();
	const char* axisUnitStringPlural = getPositionUnit(axis->positionUnit)->displayNamePlural;
	
	ImGui::Text("Axis Position: %.3f %s", axisPosition, axisUnitStringPlural);
	ImGui::Text("Axis Velocity: %.3f %s/s", axisVelocity, axisUnitStringPlural);
	ImGui::Text("Axis Lower Position Limit: %.3f %s", axisLowLimit, axisUnitStringPlural);
	ImGui::Text("Axis Upper Position Limit: %.3f %s", axisHighLimit, axisUnitStringPlural);
	
	double machinePosition = axisPositionToMachinePosition(axisPosition);
	double machineVelocity = axisVelocityToMachineVelocity(axisVelocity);
	double machineLowLimit = axisPositionToMachinePosition(axisLowLimit);
	double machineHighLimit = axisPositionToMachinePosition(axisHighLimit);
	
	ImGui::Separator();
	
	ImGui::Text("Machine Position: %.3f %s", machinePosition, axisUnitStringPlural);
	ImGui::Text("Machine Velocity: %.3f %s/s", machineVelocity, axisUnitStringPlural);
	ImGui::Text("Machine Lower Position Limit: %.3f %s", machineLowLimit, axisUnitStringPlural);
	ImGui::Text("Machine Upper Position Limit: %.3f %s", machineHighLimit, axisUnitStringPlural);
	
	double recurseAxisPosition = machinePositionToAxisPosition(machinePosition);
	double recurseAxisVelocity = machineVelocityToAxisVelocity(machineVelocity);
	double recurseAxisLowLimit = machinePositionToAxisPosition(machineLowLimit);
	double recurseAxisHighLimit = machinePositionToAxisPosition(machineHighLimit);
	
	ImGui::Separator();
	
	ImGui::Text("Recursive Axis Position: %.3f %s", recurseAxisPosition, axisUnitStringPlural);
	ImGui::Text("Recursive Axis Velocity: %.3f %s/s", recurseAxisVelocity, axisUnitStringPlural);
	ImGui::Text("Recursive Axis Lower Position Limit: %.3f %s", recurseAxisLowLimit, axisUnitStringPlural);
	ImGui::Text("Recursive Axis Upper Position Limit: %.3f %s", recurseAxisHighLimit, axisUnitStringPlural);
	*/
}

void PositionControlledSingleAxisMachine::axisGui() {
	if (!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("%s", axis->getName());
	ImGui::PopFont();

	if (ImGui::BeginTabBar("AxisTabBar")) {
		if (ImGui::BeginTabItem("Settings")) {
			ImGui::BeginChild("SettingsChild");
			axis->settingsGui();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Devices")) {
			ImGui::BeginChild("DevicesChild");
			axis->devicesGui();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PositionControlledSingleAxisMachine::deviceGui() {
	std::vector<std::shared_ptr<Device>> devices;
	getDevices(devices);

	auto deviceTabBar = [](std::shared_ptr<Device> device) {
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("%s", device->getName());
		ImGui::PopFont();
		if (ImGui::BeginTabBar(device->getName())) {
			device->nodeSpecificGui();
			ImGui::EndTabBar();
		}
	};

	if (devices.size() == 1) deviceTabBar(devices.front());
	else if (!devices.empty()) {
		if (ImGui::BeginTabBar("DevicesTabBar")) {
			for (auto& device : devices) {
				if (ImGui::BeginTabItem(device->getName())) {
					deviceTabBar(device);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
	}
}

void PositionControlledSingleAxisMachine::metricsGui() {
	if (!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	axis->metricsGui();
}

float PositionControlledSingleAxisMachine::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 8.0;
}

void PositionControlledSingleAxisMachine::machineSpecificMiniatureGui() {
		float bottomControlsHeight = ImGui::GetTextLineHeight() * 4.4;
		float sliderHeight = ImGui::GetContentRegionAvail().y - bottomControlsHeight;
		float tripleWidgetWidth = (ImGui::GetContentRegionAvail().x - 2.0 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
		glm::vec2 verticalSliderSize(tripleWidgetWidth, sliderHeight);
		
		float positionProgress = 1.0;
		float velocityProgress = 1.0;
		float velocityLimit = 1.0;
		const char* positionUnitShortFormString;
		float motionProgress = 0.0;
		static char velocityTargetString[32];
		static char actualVelocityString[32];
		static char actualPositionString[32];
		bool disableControls = true;

		if (isAxisConnected()) {
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			velocityLimit = axis->getVelocityLimit_axisUnitsPerSecond();
			
			if(!isSimulating()){
				positionProgress = axis->getActualPosition_normalized();
				velocityProgress = std::abs(axis->getActualVelocityNormalized());
				if (velocityProgress > 1.0) velocityProgress = 1.0;
				positionUnitShortFormString = getPositionUnitStringShort(axis->positionUnit);
				motionProgress = axis->targetInterpolation->getProgressAtTime(axis->profileTime_seconds);
				sprintf(velocityTargetString, "%.1f%s/s", manualVelocityTarget_machineUnitsPerSecond, positionUnitShortFormString);
				sprintf(actualVelocityString, "%.1f%s/s", axis->getActualVelocity_axisUnitsPerSecond(), positionUnitShortFormString);
				sprintf(actualPositionString, "%.1f%s", axis->getActualPosition_axisUnits(), positionUnitShortFormString);
			}else{
				
				//SIMULATION TEST
				double lowLimit = axis->getLowPositionLimit();
				double highLimit = axis->getHighPositionLimit();
				positionProgress = (simulationMotionProfile.getPosition() - lowLimit) / (highLimit - lowLimit);
				velocityProgress = std::abs(simulationMotionProfile.getVelocity() / velocityLimit);
				if (velocityProgress > 1.0) velocityProgress = 1.0;
				positionUnitShortFormString = getPositionUnitStringShort(axis->positionUnit);
				motionProgress = simulationTargetInterpolation->getProgressAtTime(Environnement::getTime_seconds());
				sprintf(velocityTargetString, "%.1f%s/s", manualVelocityTarget_machineUnitsPerSecond, positionUnitShortFormString);
				sprintf(actualVelocityString, "%.1f%s/s", simulationMotionProfile.getVelocity(), positionUnitShortFormString);
				sprintf(actualPositionString, "%.1f%s", simulationMotionProfile.getPosition(), positionUnitShortFormString);
				
			}
			disableControls = !isEnabled();
		}
		else {
			sprintf(velocityTargetString, "-");
			sprintf(actualVelocityString, "-");
			sprintf(actualPositionString, "-");
			positionUnitShortFormString = "u";
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		}

		if(disableControls) BEGIN_DISABLE_IMGUI_ELEMENT

		float manualVelocityTarget = manualVelocityTarget_machineUnitsPerSecond;
		ImGui::VSliderFloat("##ManualVelocity", verticalSliderSize, &manualVelocityTarget, -velocityLimit, velocityLimit, "");
		clampValue(manualVelocityTarget, -velocityLimit, velocityLimit);
		if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityTarget);
		else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(0.0);
		ImGui::SameLine();
		verticalProgressBar(velocityProgress, verticalSliderSize);
		ImGui::SameLine();
		verticalProgressBar(positionProgress, verticalSliderSize);
		

		ImGui::PushFont(Fonts::robotoRegular12);
		glm::vec2 feedbackButtonSize(verticalSliderSize.x, ImGui::GetTextLineHeight());
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0, 0));
		ImGui::Button(velocityTargetString, feedbackButtonSize);
		ImGui::SameLine();
		ImGui::Button(actualVelocityString, feedbackButtonSize);
		ImGui::SameLine();
		ImGui::Button(actualPositionString, feedbackButtonSize);
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();
		ImGui::PopFont();

		float framePaddingX = ImGui::GetStyle().FramePadding.x;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(framePaddingX, ImGui::GetTextLineHeight() * 0.1));

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		static char targetPositionString[32];
		sprintf(targetPositionString, "%.1f %s", manualPositionTarget_machineUnits, positionUnitShortFormString);
		ImGui::InputDouble("##TargetPosition", &manualPositionTarget_machineUnits, 0.0, 0.0, targetPositionString);

		if (motionProgress > 0.0 && motionProgress < 1.0) {
			glm::vec2 targetmin = ImGui::GetItemRectMin();
			glm::vec2 targetmax = ImGui::GetItemRectMax();
			glm::vec2 targetsize = ImGui::GetItemRectSize();
			glm::vec2 progressBarMax(targetmin.x + targetsize.x * motionProgress, targetmax.y);
			ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
		}

		ImGui::PopStyleVar();

		float doubleWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
		glm::vec2 doubleButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);

		if (ImGui::Button("Move", doubleButtonSize)) moveToPosition(manualPositionTarget_machineUnits);

		ImGui::SameLine();

		if (ImGui::Button("Stop", doubleButtonSize)) setVelocityTarget(0.0);

		if (!isAxisConnected()) {
			ImGui::PopStyleColor();
		}

		if(disableControls) END_DISABLE_IMGUI_ELEMENT

}
