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
	
	if(!isAxisConnected()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("No Axis is Connected.");
		ImGui::PopStyleColor();
		return;
	}
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis Settings");
	ImGui::PopFont();
	
	invertAxis->gui();
	ImGui::SameLine();
	ImGui::TextWrapped("Invert Axis");
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Axis Offset");
	ImGui::PopFont();
	axisOffset->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Lower Limit");
	ImGui::PopFont();
	lowerPositionLimit->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Upper Limit");
	ImGui::PopFont();
	upperPositionLimit->gui();
	
	
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Setup Options");
	ImGui::PopFont();
	
	allowUserZeroEdit->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Allow user to modify zero");
	ImGui::PopFont();
	
	allowUserLowerLimitEdit->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Allow user to modify lower position limit");
	ImGui::PopFont();
	
	allowUserUpperLimitEdit->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Allow user to upper position limit");
	ImGui::PopFont();
	
	allowUserHoming->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Allow user to home machine");
	ImGui::PopFont();
	
	allowUserEncoderRangeReset->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Allow user to reset encoder range");
	ImGui::PopFont();
	
	allowUserEncoderValueOverride->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Allow user to override encoder position");
	ImGui::PopFont();
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
	int keepoutConstraintCount = 0;
	//count constraints to draw them side by side instead of stacking them
	for(auto& constraint : animatablePosition->getConstraints()){
		if(constraint->getType() == AnimationConstraint::Type::KEEPOUT) keepoutConstraintCount++;
	}
	float keepoutConstraintWidth = size.x / keepoutConstraintCount;
	keepoutConstraintCount = 0;
	for(auto& constraint : animatablePosition->getConstraints()){
		int constraintX = min.x + keepoutConstraintCount * keepoutConstraintWidth;
		if(constraint->getType() == AnimationConstraint::Type::KEEPOUT) keepoutConstraintCount++;
		auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
		double minKeepout = animatablePosition->normalizePosition(keepout->keepOutMinPosition);
		double maxKeepout = animatablePosition->normalizePosition(keepout->keepOutMaxPosition);
		double keepoutSize = maxKeepout - minKeepout;
		glm::vec2 keepoutStartPos(constraintX, max.y - size.y * minKeepout);
		glm::vec2 keepoutEndPos(constraintX + keepoutConstraintWidth, max.y - size.y * maxKeepout);
		ImColor constraintColor;
		if(!constraint->isEnabled()) constraintColor = ImColor(1.f, 1.f, 1.f, .2f);
		else constraintColor = ImColor(1.f, 0.f, 0.f, .4f);
		drawing->AddRectFilled(keepoutStartPos, keepoutEndPos, constraintColor);
	}
	
	float lineThickness = ImGui::GetTextLineHeight() * .15f;
	//draw rapid target
	if(animatablePosition->isInRapid()){
		float rapidTarget = animatablePosition->getRapidTarget()->toPosition()->position;
		float rapidTargetNormalized = animatablePosition->normalizePosition(rapidTarget);
		float height = max.y - (max.y - min.y) * rapidTargetNormalized;
		ImColor targetColor = ImColor(Colors::yellow);
		drawing->AddLine(glm::vec2(min.x, height), glm::vec2(max.x, height), targetColor, lineThickness);
		float middleX = min.x + size.x * .5f;
		float triangleSize = ImGui::GetTextLineHeight() * .4f;
		drawing->AddTriangleFilled(glm::vec2(middleX, height),
								   glm::vec2(middleX - triangleSize * .4f, height - triangleSize),
								   glm::vec2(middleX + triangleSize * .4f, height - triangleSize),
								   targetColor);
		drawing->AddTriangleFilled(glm::vec2(middleX, height),
								   glm::vec2(middleX + triangleSize * .4f, height + triangleSize),
								   glm::vec2(middleX - triangleSize * .4f, height + triangleSize),
								   targetColor);
	}
	
	//draw braking position
	float brakingPositionY = min.y + size.y - size.y * animatablePosition->normalizePosition(animatablePosition->getBrakingPosition());
	float triangleSize = ImGui::GetTextLineHeight() * .75f;
	ImColor brakingPositionColor = ImColor(1.f, 1.f, 1.f, .3f);
	drawing->AddLine(glm::vec2(min.x, brakingPositionY), glm::vec2(max.x - triangleSize, brakingPositionY), brakingPositionColor, lineThickness);
	drawing->AddTriangleFilled(glm::vec2(max.x, brakingPositionY),
							   glm::vec2(max.x - triangleSize, brakingPositionY + triangleSize * .4f),
							   glm::vec2(max.x - triangleSize, brakingPositionY - triangleSize * .4f),
							   brakingPositionColor);
	
	//draw actual position
	float axisPositionY = min.y + size.y - size.y * animatablePosition->getActualPositionNormalized();
	drawing->AddLine(glm::vec2(min.x, axisPositionY), glm::vec2(max.x - triangleSize + 1.f, axisPositionY), ImColor(Colors::white), lineThickness);
	drawing->AddTriangleFilled(glm::vec2(max.x, axisPositionY),
							   glm::vec2(max.x - triangleSize, axisPositionY + triangleSize * .4f),
							   glm::vec2(max.x - triangleSize, axisPositionY - triangleSize * .4f),
							   ImColor(Colors::white));
	
	//draw current constraint limits
	double minPositionLimit, maxPositionLimit;
	animatablePosition->getConstraintPositionLimits(minPositionLimit, maxPositionLimit);
	double minPosition = max.y - size.y * animatablePosition->normalizePosition(minPositionLimit);
	double maxPosition = max.y - size.y * animatablePosition->normalizePosition(maxPositionLimit);
	ImColor limitLineColor = ImColor(0.f, 0.f, 0.f, .4f);
	float limitLineThickness = ImGui::GetTextLineHeight() * .05f;
	drawing->AddLine(ImVec2(min.x, minPosition), ImVec2(max.x, minPosition), limitLineColor, limitLineThickness);
	drawing->AddLine(ImVec2(min.x, maxPosition), ImVec2(max.x, maxPosition), limitLineColor, limitLineThickness);

	
	ImGui::PopClipRect();
	
	//draw frame contour
	float frameWidth = ImGui::GetTextLineHeight() * 0.05;
	drawing->AddRect(min - glm::vec2(frameWidth * .5f), max + glm::vec2(frameWidth * .5f), ImColor(Colors::black), frameWidth, ImDrawFlags_RoundCornersAll, frameWidth);
	
	ImGui::EndDisabled();
	ImGui::PopStyleVar();
}

void PositionControlledMachine::setupGui(){
	
	if(!isAxisConnected()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("No Axis Connected.");
		ImGui::PopStyleColor();
	}
	
	ImGui::BeginGroup();
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetStyle().ItemSpacing.y));
	
	if(allowUserZeroEdit->value){
		if(ImGui::Button("Capture Zero")) captureZero();
		ImGui::SameLine();
		if(ImGui::Button("Reset##Zero")) resetZero();
	}
	
	if(allowUserLowerLimitEdit->value){
		if(ImGui::Button("Capture Lower Limit")) captureLowerLimit();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
		lowerPositionLimit->gui();
		ImGui::SameLine();
		if(ImGui::Button("Reset##LowerLimit")) resetLowerLimit();
	}
	
	if(allowUserUpperLimitEdit->value){
		if(ImGui::Button("Capture Upper Limit")) captureUpperLimit();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
		upperPositionLimit->gui();
		ImGui::SameLine();
		if(ImGui::Button("Reset##UpperLimit")) resetUpperLimit();
	}
	
	ImGui::EndGroup();
	
	if(allowUserZeroEdit->value || allowUserLowerLimitEdit->value || allowUserUpperLimitEdit->value){
		
		ImGui::InvisibleButton("rangedisplaysize", glm::vec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeight() * 6.0));
		
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 size = ImGui::GetItemRectSize();
		
		ImGui::PushClipRect(min, max, true);
		
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		drawing->AddRectFilled(min, max, ImColor(Colors::almostBlack));
		
		auto axis = getAxis();
		double maxPosition = getMaxPosition();
		double minPosition = getMinPosition();
		double lowerLimit = getLowerPositionLimit();
		double upperLimit = getUpperPositionLimit();
		
		auto toRangedPositionX = [&](double position) -> double {
			double normalized = (position - minPosition) / (maxPosition - minPosition);
			double clamped = std::clamp(normalized, 0.0, 1.0);
			return min.x + clamped * size.x;
		};
		
		glm::vec2 rangeRectMin(toRangedPositionX(lowerLimit), min.y);
		glm::vec2 rangeRectMax(toRangedPositionX(upperLimit), max.y);
		double zeroPositionX = toRangedPositionX(0.0);
		double actualPositionX = toRangedPositionX(animatablePosition->getActualPosition());
		
		float rangeLineWidth = ImGui::GetTextLineHeight() * .1f;
		drawing->AddRectFilled(rangeRectMin, rangeRectMax, ImColor(Colors::darkGreen));
		drawing->AddLine(glm::vec2(rangeRectMin.x, min.y), glm::vec2(rangeRectMin.x, max.y), ImColor(Colors::green), rangeLineWidth);
		drawing->AddLine(glm::vec2(rangeRectMax.x, min.y), glm::vec2(rangeRectMax.x, max.y), ImColor(Colors::green), rangeLineWidth);
		drawing->AddLine(glm::vec2(zeroPositionX, min.y), glm::vec2(zeroPositionX, max.y), ImColor(0.f, 0.f, 1.f, 1.f), rangeLineWidth * 2.f);
		drawing->AddLine(glm::vec2(actualPositionX, min.y), glm::vec2(actualPositionX, max.y), ImColor(1.f, 1.f, 1.f, 1.f), rangeLineWidth);
		
		static char rangeString[64];
		glm::vec2 displayFramePadding(ImGui::GetTextLineHeight() * .1f, 0.0);
		float frameRounding = ImGui::GetStyle().FrameRounding;
		
		bool b_hasLowerLimit = lowerPositionLimit->value != 0.0;
		bool b_hasUpperLimit = upperPositionLimit->value != 0.0;
		TextAlignement zeroTextAlignement;
		ImDrawFlags zeroDrawFlags;
		float zeroTextX;
		if((b_hasLowerLimit && b_hasUpperLimit) || (!b_hasUpperLimit && !b_hasLowerLimit)) {
			zeroTextAlignement = TextAlignement::MIDDLE_MIDDLE;
			zeroDrawFlags = ImDrawFlags_RoundCornersAll;
			zeroTextX = zeroPositionX;
		}
		else if(b_hasUpperLimit) {
			zeroTextAlignement = TextAlignement::LEFT_MIDDLE;
			zeroDrawFlags = ImDrawFlags_RoundCornersRight;
			zeroTextX = zeroPositionX + rangeLineWidth * .5f;
		}
		else{
			zeroTextAlignement = TextAlignement::RIGHT_MIDDLE;
			zeroDrawFlags = ImDrawFlags_RoundCornersLeft;
			zeroTextX = zeroPositionX - rangeLineWidth * .5f;
		}
		
		sprintf(rangeString, "Min position: %.3f%s", minPosition, positionUnit->abbreviated);
		textAlignedBackground(rangeString,
							  glm::vec2(min.x, min.y),
							  TextAlignement::LEFT_TOP,
							  ImVec4(0.f, 0.f, 0.f, .7f),
							  displayFramePadding,
							  frameRounding,
							  ImDrawFlags_RoundCornersBottomRight);
		
		sprintf(rangeString, "Max position: %.3f%s", maxPosition, positionUnit->abbreviated);
		textAlignedBackground(rangeString,
							  glm::vec2(max.x, max.y),
							  TextAlignement::RIGHT_BOTTOM,
							  ImVec4(0.f, 0.f, 0.f, .7f),
							  displayFramePadding,
							  frameRounding,
							  ImDrawFlags_RoundCornersTopLeft);
		
		textAlignedBackground("Zero",
							  glm::vec2(zeroTextX, min.y + size.y * .5f),
							  zeroTextAlignement,
							  ImVec4(.0f, .0f, .5f, .7f),
							  displayFramePadding,
							  frameRounding,
							  zeroDrawFlags);
		
		if(b_hasLowerLimit){
			sprintf(rangeString, "Lower limit: %.3f%s", lowerLimit, positionUnit->abbreviated);
			textAlignedBackground(rangeString,
								  glm::vec2(rangeRectMin.x + rangeLineWidth * .5f, min.y + size.y * .3f),
								  TextAlignement::LEFT_MIDDLE,
								  ImVec4(0.f, .5f, 0.f, .7f),
								  displayFramePadding,
								  frameRounding,
								  ImDrawFlags_RoundCornersRight);
		}
	
		if(b_hasUpperLimit){
			sprintf(rangeString, "Upper limit: %.3f%s", upperLimit, positionUnit->abbreviated);
			textAlignedBackground(rangeString,
								  glm::vec2(rangeRectMax.x - rangeLineWidth * .5f, min.y + size.y * .7f),
								  TextAlignement::RIGHT_MIDDLE,
								  ImVec4(0.f, .5f, 0.f, .7f),
								  displayFramePadding,
								  frameRounding,
								  ImDrawFlags_RoundCornersLeft);
		}
			
		ImGui::PopClipRect();
		
		float framethickness = ImGui::GetTextLineHeight() * .1f;
		drawing->AddRect(min - glm::vec2(framethickness * .5f),
						 max + glm::vec2(framethickness * .5f),
						 ImColor(Colors::black),
						 framethickness,
						 ImDrawFlags_RoundCornersAll,
						 framethickness);
	}
	
	if(allowUserHoming->value){

		glm::vec2 homingButtonSize(ImGui::GetTextLineHeight() * 6.0, ImGui::GetFrameHeight());
		glm::vec2 homingProgressSize(ImGui::GetTextLineHeight() * 8.0, ImGui::GetFrameHeight());
		ImGui::BeginDisabled(!canStartHoming());
		if(isHoming()){
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			if(ImGui::Button("Stop Homing", homingButtonSize)) stopHoming();
			ImGui::PopStyleColor();
		}else{
			if(ImGui::Button("Start Homing", homingButtonSize)) startHoming();
		}
		ImGui::EndDisabled();


		ImGui::SameLine();

		ImVec4 progressIndicatorColor = Colors::darkGray;
		if(isHoming()) progressIndicatorColor = Colors::orange;
		else if(didHomingSucceed()) progressIndicatorColor = Colors::green;
		else if(didHomingFail()) progressIndicatorColor = Colors::red;

		backgroundText(getHomingString(), homingProgressSize, Colors::darkGray);

	}
		
		
		
		
	if(allowUserEncoderRangeReset->value){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("User Encoder Range Reset");
		ImGui::PopStyleColor();
	}
	
	ImGui::PopStyleVar();
	
}
