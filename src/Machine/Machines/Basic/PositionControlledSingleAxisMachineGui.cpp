#include <pch.h>

#include "PositionControlledSingleAxisMachine.h"
#include "Motion/Axis/PositionControlledAxis.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/DeviceNode.h"

#include "Environnement/Environnement.h"


void PositionControlledSingleAxisMachine::controlsGui() {
	if(!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	glm::vec2 manualControlsSize = ImGui::GetContentRegionAvail();
	//manualControlsSize.y -= axis->getFeedbackGuiHeight();
	ImGui::BeginChild("##manualMachineControls", manualControlsSize);

	float widgetWidth = ImGui::GetContentRegionAvail().x;
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Velocity Control");
	ImGui::PopFont();

	float manualVelocityTarget = manualVelocityTarget_machineUnitsPerSecond;
	ImGui::SetNextItemWidth(widgetWidth);
	ImGui::SliderFloat("##velTar", &manualVelocityTarget, -axis->getVelocityLimit(), axis->getVelocityLimit());
	if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityTarget);
	else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(0.0);

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Position Control");
	ImGui::PopFont();

	static char positionTargetString[128];
	sprintf(positionTargetString, "%.3f %s", manualPositionTarget_machineUnits, axis->getPositionUnit()->abbreviated);
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
		sprintf(homingStateString, "Homing State: %s", Enumerator::getDisplayString(getAxis()->homingStep));
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
		ImGui::Button(homingStateString, glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 1.5));
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();


	}

	ImGui::EndChild();

	ImGui::Separator();

	//axis->feedbackGui();
	
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
	ImGui::Text("Machine Limits :");
	ImGui::PopFont();

	if(ImGui::BeginTable("##machineInfo", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit)){
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Position Unit");
		ImGui::PopFont();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", axis->getPositionUnit()->singular);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Lower Position Limit: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s", getLowPositionLimit(), axis->getPositionUnit()->abbreviated);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Upper Position Limit: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s", getHighPositionLimit(), axis->getPositionUnit()->abbreviated);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Velocity Limit: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s/s", axis->getVelocityLimit(), axis->getPositionUnit()->abbreviated);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Acceleration Limit: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s/s\xC2\xB2", axis->getAccelerationLimit(), axis->getPositionUnit()->abbreviated);

		ImGui::EndTable();
	}
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Current State :");
	ImGui::PopFont();
		
	if(ImGui::BeginTable("##currentMachineState", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit)){
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Target Position: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s", motionProfile.getPosition(), axis->getPositionUnit()->abbreviated);
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Target Velocity: ");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.20f %s/s", motionProfile.getVelocity(), axis->getPositionUnit()->abbreviated);
		
		ImGui::EndTable();
	}

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Rapids");
	ImGui::PopFont();

	static char rapidVelocityString[128];
	static char rapidAccelerationString[128];

	ImGui::Text("Velocity for rapid movements :");
	sprintf(rapidVelocityString, "%.3f %s/s", rapidVelocity_machineUnitsPerSecond, axis->getPositionUnit()->abbreviated);
	ImGui::InputDouble("##velRapid", &rapidVelocity_machineUnitsPerSecond, 0.0, 0.0, rapidVelocityString);
	rapidVelocity_machineUnitsPerSecond = std::min(rapidVelocity_machineUnitsPerSecond, axis->getVelocityLimit());

	ImGui::Text("Acceleration for rapid movements :");
	sprintf(rapidAccelerationString, "%.3f %s/s\xC2\xB2", rapidAcceleration_machineUnitsPerSecond, axis->getPositionUnit()->abbreviated);
	ImGui::InputDouble("##accRapid", &rapidAcceleration_machineUnitsPerSecond, 0.0, 0.0, rapidAccelerationString);
	rapidAcceleration_machineUnitsPerSecond = std::min(rapidAcceleration_machineUnitsPerSecond, axis->getAccelerationLimit());
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Machine Zero");
	ImGui::PopFont();
	
	ImGui::Text("Machine Zero (Axis Units) :");
	static char machineZeroString[128];
	sprintf(machineZeroString, "%.3f %s", machineZero_axisUnits, axis->getPositionUnit()->abbreviated);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
	ImGui::InputDouble("##axisUnitOffset", &machineZero_axisUnits, 0.0, 0.0, machineZeroString);
	
	ImGui::SameLine();
	if(ImGui::Button("Capture Machine Zero")){
		captureMachineZero();
	}
	
	ImGui::Checkbox("Invert Axis Direction", &b_invertDirection);
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
		double minPosition = 0.0;
		double maxPosition = 0.0;
		bool b_hasPositionTarget = hasManualPositionTarget();
		double positionTargetNormalized = 0.0;

		if (isAxisConnected()) {
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			velocityLimit = axis->getVelocityLimit();
			
			minPosition = getLowPositionLimit();
			maxPosition = getHighPositionLimit();
			positionProgress = getPositionNormalized();
			velocityProgress = std::abs(getVelocityNormalized());
			if (velocityProgress > 1.0) velocityProgress = 1.0;
			positionUnitShortFormString = axis->getPositionUnit()->abbreviated;
			motionProgress = targetInterpolation->getProgressAtTime(Environnement::getTime_seconds());
			if(b_hasPositionTarget) positionTargetNormalized = (getManualPositionTarget() - minPosition) / (maxPosition - minPosition);
			disableControls = !isEnabled();
			
			if(isSimulating()){
				sprintf(velocityTargetString, "%.2f%s/s", manualVelocityTarget_machineUnitsPerSecond, positionUnitShortFormString);
				sprintf(actualVelocityString, "%.2f%s/s", motionProfile.getVelocity(), positionUnitShortFormString);
				sprintf(actualPositionString, "%.3f%s", motionProfile.getPosition(), positionUnitShortFormString);
			}else{
				sprintf(velocityTargetString, "%.2f%s/s", manualVelocityTarget_machineUnitsPerSecond, positionUnitShortFormString);
				sprintf(actualVelocityString, "%.2f%s/s", axisVelocityToMachineVelocity(axis->getActualVelocity()), positionUnitShortFormString);
				sprintf(actualPositionString, "%.3f%s", axisPositionToMachinePosition(axis->getActualPosition()), positionUnitShortFormString);
			}
		}
		else {
			sprintf(velocityTargetString, "-");
			sprintf(actualVelocityString, "-");
			sprintf(actualPositionString, "-");
			positionUnitShortFormString = "u";
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		}

		ImGui::BeginDisabled(disableControls);

		float manualVelocityTarget = manualVelocityTarget_machineUnitsPerSecond;
		ImGui::VSliderFloat("##ManualVelocity", verticalSliderSize, &manualVelocityTarget, -velocityLimit, velocityLimit, "");
		std::clamp(manualVelocityTarget, -velocityLimit, velocityLimit);
		if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityTarget);
		else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(0.0);
		ImGui::SameLine();
		verticalProgressBar(velocityProgress, verticalSliderSize);
		ImGui::SameLine();
		verticalProgressBar(positionProgress, verticalSliderSize);
	
		if(b_hasPositionTarget){
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			float height = max.y - (max.y - min.y) * positionTargetNormalized;
			glm::vec2 lineStart(min.x, height);
			glm::vec2 lineEnd(max.x, height);
			ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, ImColor(Colors::white));
		}
		

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
		sprintf(targetPositionString, "%.3f %s", manualPositionTarget_machineUnits, positionUnitShortFormString);
		ImGui::InputDouble("##TargetPosition", &manualPositionTarget_machineUnits, 0.0, 0.0, targetPositionString);
		manualPositionTarget_machineUnits = std::min(manualPositionTarget_machineUnits, maxPosition);
		manualPositionTarget_machineUnits = std::max(manualPositionTarget_machineUnits, minPosition);

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

		ImGui::EndDisabled();

}
