#include <pch.h>

#include "PositionControlledMachine.h"
#include "Motion/Axis/PositionControlledAxis.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/NodeGraph/DeviceNode.h"

#include "Environnement/Environnement.h"

static bool b_invalidValue = false;
static void pushInvalidValue(bool doit){
	if(doit){
		b_invalidValue = true;
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::sansBold15);
	}
}

static void popInvalidValue(){
	if(b_invalidValue){
		b_invalidValue = false;
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
}

void PositionControlledMachine::controlsGui() {
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	
	widgetGui();
	
	ImGui::Separator();
	
	/*
	ImGui::BeginChild("##manualMachineControls", ImGui::GetContentRegionAvail());
	
	if(!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		ImGui::EndChild();
		return;
	}
	
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	
	ImGui::BeginDisabled(!isEnabled());

	float widgetWidth = ImGui::GetContentRegionAvail().x;
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Manual Velocity Control");
	ImGui::PopFont();

	float manualVelocityTarget = manualVelocityTarget_machineUnitsPerSecond;
	ImGui::SetNextItemWidth(widgetWidth);
	ImGui::SliderFloat("##velTar", &manualVelocityTarget, -axis->getVelocityLimit(), axis->getVelocityLimit());
	if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityTarget);
	else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(0.0);

	ImGui::Separator();

	ImGui::PushFont(Fonts::sansBold20);
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

		ImGui::PushFont(Fonts::sansBold20);
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

	ImGui::EndDisabled();
	 
	
	
	
	//-------------------------------- FEEDBACK --------------------------------
		
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Feedback");
	ImGui::PopFont();

	glm::vec2 progressBarSize = ImGui::GetContentRegionAvail();
	progressBarSize.y = ImGui::GetFrameHeight();
	
	//actual position in range
	double minPosition = 0.0;
	double maxPosition = 0.0;
	double positionProgress = 0.0;
	static char positionString[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		positionProgress = 1.0;
		sprintf(positionString, "Axis Disabled");
	}
	else {
		minPosition = getLowPositionLimit();
		maxPosition = getHighPositionLimit();
		positionProgress = getPositionNormalized();
		double positionValue;
		if(isSimulating()) positionValue = motionProfile.getPosition();
		else positionValue = axisPositionToMachinePosition(axis->getActualPosition());
		if (positionProgress < 0.0 || positionProgress > 1.0) {
			sprintf(positionString, "Axis out of limits : %.2f %s", positionValue, axis->getPositionUnit()->abbreviated);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		}
		else {
			sprintf(positionString, "%.2f %s", positionValue, axis->getPositionUnit()->abbreviated);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		}
	}
	const char* shortPositionUnitString = axis->getPositionUnit()->abbreviated;
	ImGui::Text("Current Position : (in range from %.2f %s to %.2f %s)", minPosition, shortPositionUnitString, maxPosition, shortPositionUnitString);
	ImGui::ProgressBar(positionProgress, progressBarSize, positionString);
	ImGui::PopStyleColor();


	//actual velocity
	float velocityProgress;
	static char velocityString[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		velocityProgress = 1.0;
		sprintf(velocityString, "Axis Disabled");
	}
	else {
		velocityProgress = std::abs(getVelocityNormalized());
		double actualVelocity;
		if(isSimulating()) actualVelocity = motionProfile.getVelocity();
		else actualVelocity = axisVelocityToMachineVelocity(axis->getActualVelocity());
		sprintf(velocityString, "%.2f %s/s", actualVelocity, axis->getPositionUnit()->abbreviated);
		if (velocityProgress > 1.0)
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	ImGui::Text("Current Velocity : (max %.2f%s/s)", axis->getVelocityLimit(), axis->getPositionUnit()->abbreviated);
	ImGui::ProgressBar(velocityProgress, progressBarSize, velocityString);
	ImGui::PopStyleColor();

	float positionErrorProgress;
	float maxfollowingError = 0.0;
	static char positionErrorString[32];
	if(!isEnabled()){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		positionErrorProgress = 1.0;
		sprintf(positionErrorString, "Axis Disabled");
	}else{
		if(isSimulating()){
			positionErrorProgress = 1.0;
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
			sprintf(positionErrorString, "Simulating");
		}else{
			positionErrorProgress = std::abs(axis->getActualFollowingErrorNormalized());
			if(positionErrorProgress < 1.0) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
			else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
			maxfollowingError = axis->getFollowingErrorLimit();
			double followingError = axis->getActualFollowingError();
			sprintf(positionErrorString, "%.3f %s", followingError, axis->getPositionUnit()->abbreviated);
		}
	}

	if(isSimulating()) ImGui::Text("Position Following Error:");
	else ImGui::Text("Position Following Error : (max %.3f%s)", maxfollowingError, axis->getPositionUnit()->abbreviated);
	ImGui::ProgressBar(positionErrorProgress, progressBarSize, positionErrorString);
	ImGui::PopStyleColor();

	//target movement progress
	float targetProgress;
	double movementSecondsLeft = 0.0;
	static char movementProgressChar[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "Axis Disabled");
		targetProgress = 1.0;
	}
	else{
		targetProgress = motionProfile.getInterpolationProgress(Environnement::getTime_seconds());
		if(targetProgress > 0.0 && targetProgress < 1.0){
			movementSecondsLeft = motionProfile.getRemainingInterpolationTime(Environnement::getTime_seconds());
			sprintf(movementProgressChar, "%.2fs", movementSecondsLeft);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
		}else{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
			sprintf(movementProgressChar, "No Target Movement");
			targetProgress = 1.0;
		}
	}
	ImGui::Text("Movement Time Remaining :");
	ImGui::ProgressBar(targetProgress, progressBarSize, movementProgressChar);
	ImGui::PopStyleColor();
	
	ImGui::EndChild();
	 */
}


void PositionControlledMachine::settingsGui() {
	
	//ImGui::InputDouble("rapid velocity", &animatablePosition->rapidVelocity);
	//ImGui::InputDouble("rapid acceleration", &animatablePosition->rapidAcceleration);
	
	ImGui::Separator();
	
	invertAxis->gui();
	ImGui::SameLine();
	ImGui::TextWrapped("Invert Axis");
	
	ImGui::Text("Axis Offset");
	axisOffset->gui();
	
	/*
	if (!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Machine Limits :");
	ImGui::PopFont();

	ImGui::Text("Position Unit:");
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("%s", axis->getPositionUnit()->singular);
	ImGui::PopFont();
	
	bool b_positionRangeIsZero = getHighPositionLimit() - getLowPositionLimit() == 0.0;
	
	ImGui::Text("Low Position Limit:");
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	pushInvalidValue(b_positionRangeIsZero);
	ImGui::Text("%.3f%s", getLowPositionLimit(), axis->getPositionUnit()->abbreviated);
	popInvalidValue();
	ImGui::PopFont();
	
	ImGui::Text("High Position Limit:");
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	pushInvalidValue(b_positionRangeIsZero);
	ImGui::Text("%.3f%s", getHighPositionLimit(), axis->getPositionUnit()->abbreviated);
	popInvalidValue();
	ImGui::PopFont();
	
	ImGui::Text("Velocity Limit:");
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	pushInvalidValue(axis->getVelocityLimit() <= 0.0);
	ImGui::Text("%.3f%s/s", axis->getVelocityLimit(), axis->getPositionUnit()->abbreviated);
	popInvalidValue();
	ImGui::PopFont();
	
	ImGui::Text("Acceleration Limit:");
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	pushInvalidValue(axis->getAccelerationLimit() <= 0.0);
	ImGui::Text("%.3f%s\xC2\xB2", axis->getAccelerationLimit(), axis->getPositionUnit()->abbreviated);
	popInvalidValue();
	ImGui::PopFont();
	
	ImGui::Separator();
	

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Rapids");
	ImGui::PopFont();

	static char rapidVelocityString[128];
	static char rapidAccelerationString[128];

	ImGui::Text("Velocity for rapid movements :");
	sprintf(rapidVelocityString, "%.3f %s/s", rapidVelocity_machineUnitsPerSecond, axis->getPositionUnit()->abbreviated);
	pushInvalidValue(rapidVelocity_machineUnitsPerSecond == 0.0);
	ImGui::InputDouble("##velRapid", &rapidVelocity_machineUnitsPerSecond, 0.0, 0.0, rapidVelocityString);
	popInvalidValue();
	rapidVelocity_machineUnitsPerSecond = std::min(rapidVelocity_machineUnitsPerSecond, axis->getVelocityLimit());

	ImGui::Text("Acceleration for rapid movements :");
	sprintf(rapidAccelerationString, "%.3f %s/s\xC2\xB2", rapidAcceleration_machineUnitsPerSecond, axis->getPositionUnit()->abbreviated);
	pushInvalidValue(rapidAcceleration_machineUnitsPerSecond == 0.0);
	ImGui::InputDouble("##accRapid", &rapidAcceleration_machineUnitsPerSecond, 0.0, 0.0, rapidAccelerationString);
	popInvalidValue();
	rapidAcceleration_machineUnitsPerSecond = std::min(rapidAcceleration_machineUnitsPerSecond, axis->getAccelerationLimit());
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
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
*/
}

void PositionControlledMachine::axisGui() {
	/*
	if (!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	ImGui::PushFont(Fonts::sansBold20);
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
	 */
}

void PositionControlledMachine::deviceGui() {
	/*
	std::vector<std::shared_ptr<Device>> devices;
	getDevices(devices);

	auto deviceTabBar = [](std::shared_ptr<Device> device) {
		ImGui::PushFont(Fonts::sansBold20);
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
	 */
}

void PositionControlledMachine::metricsGui() {
	/*
	if (!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	axis->metricsGui();
	 */
}








void PositionControlledMachine::ControlWidget::gui(){
	glm::vec2 headerCursorPos = machine->reserveSpaceForMachineHeaderGui();
	
	if(!machine->isAxisConnected()) {
		machine->machineHeaderGui(headerCursorPos);
		ImGui::TextWrapped("No Axis Connected");
		return;
	}
	
	ImGui::BeginGroup();
	
	
	machine->widgetGui();
	
	
	ImGui::EndGroup();
	
	float widgetWidth = ImGui::GetItemRectSize().x;
	machine->machineHeaderGui(headerCursorPos, widgetWidth);
	machine->machineStateControlGui(widgetWidth);
}

void PositionControlledMachine::widgetGui(){
			
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	ImGui::BeginDisabled(!isEnabled());
	
	animatablePosition->manualControlsVerticalGui(ImGui::GetTextLineHeight() * 10.f);
	float controlsHeight = ImGui::GetItemRectSize().y;
	
	ImGui::SameLine();
	
	//draw feedback background
	float feedbackWidth = ImGui::GetTextLineHeight() * 3.0;
	ImGui::InvisibleButton("feedback", glm::vec2(feedbackWidth, controlsHeight));
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = max - min;
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	drawing->AddRectFilled(min, max, ImColor(Colors::darkGray));
	
	ImGui::PushClipRect(min, max, true);
	
	//draw keepout constraints
	for(auto& constraint : animatablePosition->getConstraints()){
		auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
		double minKeepout = animatablePosition->normalizePosition(keepout->keepOutMinPosition);
		double maxKeepout = animatablePosition->normalizePosition(keepout->keepOutMaxPosition);
		double keepoutSize = maxKeepout - minKeepout;
		glm::vec2 keepoutStartPos(min.x, max.y - size.y * minKeepout);
		glm::vec2 keepoutEndPos(max.x, max.y - size.y * maxKeepout);
		ImColor constraintColor;
		if(!constraint->isEnabled()) constraintColor = ImColor(1.f, 1.f, 1.f, .2f);
		else constraintColor = ImColor(1.f, 0.f, 0.f, .4f);
		drawing->AddRectFilled(keepoutStartPos, keepoutEndPos, constraintColor);
	}
	
	//draw current constraint limits
	double minPositionLimit, maxPositionLimit;
	animatablePosition->getConstraintPositionLimits(minPositionLimit, maxPositionLimit);
	double minPosition = max.y - size.y * animatablePosition->normalizePosition(minPositionLimit);
	double maxPosition = max.y - size.y * animatablePosition->normalizePosition(maxPositionLimit);
	drawing->AddLine(ImVec2(min.x, minPosition), ImVec2(max.x, minPosition), ImColor(1.f, 1.f, 1.f, 1.f));
	drawing->AddLine(ImVec2(min.x, maxPosition), ImVec2(max.x, maxPosition), ImColor(1.f, 1.f, 1.f, 1.f));
	
	float lineThickness = ImGui::GetTextLineHeight() * .15f;
	
	//draw rapid target
	if(animatablePosition->isInRapid()){
		float rapidTarget = animatablePosition->getRapidTarget()->toPosition()->position;
		float rapidTargetNormalized = animatablePosition->normalizePosition(rapidTarget);
		float height = max.y - (max.y - min.y) * rapidTargetNormalized;
		ImGui::GetWindowDrawList()->AddLine(glm::vec2(min.x, height), glm::vec2(max.x, height), ImColor(Colors::yellow), lineThickness);
	}
	
	
	
	//draw actual position
	float axisPositionY = min.y + size.y - size.y * animatablePosition->getActualPositionNormalized();
	float triangleSize = ImGui::GetTextLineHeight() * .75f;
	drawing->AddLine(glm::vec2(min.x, axisPositionY), glm::vec2(max.x - triangleSize + 1.f, axisPositionY), ImColor(Colors::white), lineThickness);
	drawing->AddTriangleFilled(glm::vec2(max.x, axisPositionY),
							   glm::vec2(max.x - triangleSize, axisPositionY + triangleSize * .4f),
							   glm::vec2(max.x - triangleSize, axisPositionY - triangleSize * .4f),
							   ImColor(Colors::white));

	
	ImGui::PopClipRect();
	
	//draw frame contour
	float frameWidth = ImGui::GetTextLineHeight() * 0.05;
	drawing->AddRect(min - glm::vec2(frameWidth * .5f), max + glm::vec2(frameWidth * .5f), ImColor(Colors::black), frameWidth, ImDrawFlags_RoundCornersAll, frameWidth);
	
	ImGui::EndDisabled();
	ImGui::PopStyleVar();
}
