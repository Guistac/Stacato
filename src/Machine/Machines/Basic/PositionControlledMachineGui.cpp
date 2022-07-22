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
	
	
	ImGui::InputDouble("rapid velocity", &animatablePosition->rapidVelocity);
	ImGui::InputDouble("rapid acceleration", &animatablePosition->rapidAcceleration);
	
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







glm::vec2 PositionControlledMachine::ControlWidget::getFixedContentSize(){
	float lineHeight = ImGui::GetTextLineHeight();
	return glm::vec2(lineHeight * 10.0, lineHeight * 20);
}

void PositionControlledMachine::ControlWidget::gui(){
	machine->widgetGui();
}

void PositionControlledMachine::widgetGui(){
		
	
	glm::vec2 contentSize = controlWidget->getFixedContentSize();
	glm::vec2 contentMin = ImGui::GetCursorPos();
	glm::vec2 contentMax = contentMin + contentSize;
	
	machineHeaderGui(contentSize.x);
	
	if(!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	
	float bottomControlsHeight = ImGui::GetTextLineHeight() * 4.4;
	float sliderHeight = contentSize.y - bottomControlsHeight;
	float tripleWidgetWidth = (contentSize.x - 2.0 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
	glm::vec2 verticalSliderSize(tripleWidgetWidth, sliderHeight);
	
	
	ImGui::BeginDisabled(!isEnabled());

	static double min = -1.0;
	static double max = 1.0;
	ImGui::VSliderScalar("##ManualVelocity", verticalSliderSize, ImGuiDataType_Double, &velocitySliderValue, &min, &max, "");
	if (ImGui::IsItemActive()) animatablePosition->setManualVelocityTarget(velocitySliderValue);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		animatablePosition->setManualVelocityTarget(0.0);
		velocitySliderValue = 0.0;
	}
		
	ImGui::SameLine();
	verticalProgressBar(std::abs(animatablePosition->getActualVelocityNormalized()), verticalSliderSize);
	ImGui::SameLine();
	verticalProgressBar(animatablePosition->getActualPositionNormalized(), verticalSliderSize);
	
	glm::vec2 minPosProg = ImGui::GetItemRectMin();
	glm::vec2 maxPosProg = ImGui::GetItemRectMax();
	glm::vec2 progSize = maxPosProg - minPosProg;
	
	auto& constraints = animatablePosition->getConstraints();
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	for(auto& constraint : constraints){
		auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
		double minKeepout = animatablePosition->normalizePosition(keepout->keepOutMinPosition);
		double maxKeepout = animatablePosition->normalizePosition(keepout->keepOutMaxPosition);
		double keepoutSize = maxKeepout - minKeepout;
		glm::vec2 keepoutStartPos(minPosProg.x, maxPosProg.y - progSize.y * minKeepout);
		glm::vec2 keepoutEndPos(maxPosProg.x, maxPosProg.y - progSize.y * maxKeepout);
		ImColor constraintColor;
		if(!constraint->isEnabled()) constraintColor = ImColor(1.f, 1.f, 1.f, .3f);
		else constraintColor = ImColor(1.f, 0.f, 0.f, .5f);
		drawing->AddRectFilled(keepoutStartPos, keepoutEndPos, constraintColor);
	}
	
	{
	double minPositionLimit, maxPositionLimit;
	animatablePosition->getConstraintPositionLimits(minPositionLimit, maxPositionLimit);
	double minPosition = maxPosProg.y - progSize.y * animatablePosition->normalizePosition(minPositionLimit);
	double maxPosition = maxPosProg.y - progSize.y * animatablePosition->normalizePosition(maxPositionLimit);
		drawing->AddLine(ImVec2(minPosProg.x, minPosition), ImVec2(maxPosProg.x, minPosition), ImColor(1.f, 1.f, 1.f, 1.f));
		drawing->AddLine(ImVec2(minPosProg.x, maxPosition), ImVec2(maxPosProg.x, maxPosition), ImColor(1.f, 1.f, 1.f, 1.f));
	}
		
	/*
	if(b_hasPositionTarget){
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float height = max.y - (max.y - min.y) * positionTargetNormalized;
		glm::vec2 lineStart(min.x, height);
		glm::vec2 lineEnd(max.x, height);
		ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, ImColor(Colors::white));
	}
	 */
	

	static char actualVelocityString[32];
	static char actualPositionString[32];
	const char *positionUnitAbbreviated = animatablePosition->getUnit()->abbreviated;
	sprintf(actualVelocityString, "%.2f%s/s", animatablePosition->getActualVelocity(), positionUnitAbbreviated);
	sprintf(actualPositionString, "%.7f%s", animatablePosition->getActualPosition(), positionUnitAbbreviated);
	
	ImGui::PushFont(Fonts::sansRegular12);
	glm::vec2 feedbackButtonSize(verticalSliderSize.x, ImGui::GetTextLineHeight());
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0, 0));
	//ImGui::Button(velocityTargetString, feedbackButtonSize);
	ImGui::Dummy(feedbackButtonSize);
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

	ImGui::SetNextItemWidth(contentSize.x);
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", positionTargetValue, positionUnitAbbreviated);
	ImGui::InputDouble("##TargetPosition", &positionTargetValue, 0.0, 0.0, targetPositionString);
	positionTargetValue = std::clamp(positionTargetValue, animatablePosition->lowerPositionLimit, animatablePosition->upperPositionLimit);

	/*
	if (motionProgress > 0.0 && motionProgress < 1.0) {
		glm::vec2 targetmin = ImGui::GetItemRectMin();
		glm::vec2 targetmax = ImGui::GetItemRectMax();
		glm::vec2 targetsize = ImGui::GetItemRectSize();
		glm::vec2 progressBarMax(targetmin.x + targetsize.x * motionProgress, targetmax.y);
		ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	}
	 */

	ImGui::PopStyleVar();

	float doubleWidgetWidth = (contentSize.x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
	glm::vec2 doubleButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);

	if (ImGui::Button("Move", doubleButtonSize)) animatablePosition->setManualPositionTargetWithVelocity(positionTargetValue, animatablePosition->rapidVelocity);

	ImGui::SameLine();

	if (ImGui::Button("Stop", doubleButtonSize)) animatablePosition->stopMovement();


	ImGui::EndDisabled();
		
	machineStateControlGui(contentSize.x);
	
	ImGui::PopStyleVar();
}
