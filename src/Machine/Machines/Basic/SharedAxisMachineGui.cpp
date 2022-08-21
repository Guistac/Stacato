#include <pch.h>

#include "SharedAxisMachine.h"
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

void SharedAxisMachine::controlsGui() {
	
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


void SharedAxisMachine::settingsGui() {
		
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("Both axes need to have:");
	ImGui::TreePush();
	ImGui::Text("- the same position unit");
	ImGui::Text("- the same position scaling");
	ImGui::Text("- the same signed direction");
	ImGui::TreePop();
	ImGui::PopStyleColor();
	
	if(!areAxesConnected()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::TextWrapped("Axes are not connected.");
		ImGui::PopStyleColor();
		return;
	}else if(!axesHaveSamePositionUnit()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::TextWrapped("Axes don't have the same position unit.");
		ImGui::PopStyleColor();
		return;
	}
	
	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	ImGui::Separator(); //————————————————————————
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis 1");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansLight20);
	ImGui::Text("(%s)", axis1->getName());
	ImGui::PopFont();
	
	invertAxis1->gui();
	ImGui::SameLine();
	ImGui::Text("Invert Direction");
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Axis Offset");
	ImGui::PopFont();
	axis1Offset->gui();
	
	
	ImGui::Separator(); //————————————————————————
	
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis 2");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansLight20);
	ImGui::Text("(%s)", axis2->getName());
	ImGui::PopFont();
	
	invertAxis2->gui();
	ImGui::SameLine();
	ImGui::Text("Invert Direction");
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Axis Offset");
	ImGui::PopFont();
	axis2Offset->gui();
	
	
	ImGui::Separator(); //————————————————————————
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Motion Limits");
	ImGui::PopFont();
	
	double axisVelocityLimit = std::min(axis1->getVelocityLimit(), axis2->getVelocityLimit());
	double axisAccelerationLimit = std::min(axis1->getAccelerationLimit(), axis2->getAccelerationLimit());
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::Text("Axes Limit Velocity to %.3f %s/s", axisVelocityLimit, axis1->getPositionUnit()->abbreviated);
	ImGui::Text("Axes Limit Acceleration to %.3f %s/s2", axisAccelerationLimit, axis1->getPositionUnit()->abbreviated);
	ImGui::PopStyleColor();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Velocity Limit");
	ImGui::PopFont();
	velocityLimit->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Acceleration Limit");
	ImGui::PopFont();
	accelerationLimit->gui();
		
	
	ImGui::Separator(); //————————————————————————
	
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Collision Avoidance");
	ImGui::PopFont();
	
	enableAntiCollision->gui();
	ImGui::SameLine();
	ImGui::Text("Enable Collision Avoidance");
	if(enableAntiCollision->value){
		axis1isAboveAxis2->gui();
		ImGui::SameLine();
		ImGui::TextWrapped("Axis Order: %s", getAxis1()->getName());
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", axis1isAboveAxis2->value ? ">" : "<");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TextWrapped("%s", getAxis2()->getName());
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Minimum Distance Between Axes");
		ImGui::PopFont();
		minimumDistanceBetweenAxes->gui();
	}
	
	
	ImGui::Separator(); //————————————————————————
	
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Synchronous Control");
	ImGui::PopFont();
	
	enableSynchronousControl->gui();
	ImGui::SameLine();
	ImGui::Text("Enable Synchronous Control");
	
	if(enableSynchronousControl->value){
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Synchronisation master selection");
		ImGui::PopFont();
		axis1isMaster->gui();
		ImGui::SameLine();
		ImGui::Text("%s is Master", axis1isMaster->value ? axis1->getName() : axis2->getName());
	}
	
}

void SharedAxisMachine::axisGui() {
	if (!areAxesConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}

	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	if(ImGui::BeginTabBar("AxesTabBar")){
		
		if(ImGui::BeginTabItem(axis1->getName())){
			if (ImGui::BeginTabBar("AxisTabBar")) {
				axis1->nodeSpecificGui();
				ImGui::EndTabBar();
			}
			ImGui::EndTabItem();
		}
		
		if(ImGui::BeginTabItem(axis2->getName())){
			if (ImGui::BeginTabBar("AxisTabBar")) {
				axis2->nodeSpecificGui();
				ImGui::EndTabBar();
			}
			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}

}

void SharedAxisMachine::deviceGui() {
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

void SharedAxisMachine::metricsGui() {
	/*
	if (!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	axis->metricsGui();
	 */
}





void SharedAxisMachine::ControlWidget::gui(){
	machine->widgetGui();
}

void SharedAxisMachine::widgetGui(){
		
	glm::vec2 contentSize = controlWidget->getFixedContentSize();
	glm::vec2 contentMin = ImGui::GetCursorPos();
	glm::vec2 contentMax = contentMin + contentSize;
	
	//machineHeaderGui(contentSize.x);
	glm::vec2 headerCursor = reserveSpaceForMachineHeaderGui();
	
	//Begin main widget content group
	ImGui::BeginGroup();
	
	if(!areAxesConnected()) {
		ImGui::Text("Axe are not connected.");
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	ImGui::BeginDisabled(!isEnabled());
	
	float sliderHeight = ImGui::GetTextLineHeight() * 10.0;
	float sliderWidth = ImGui::GetTextLineHeight() * 1.75;
	float velocityDisplayWidth = ImGui::GetTextLineHeight() * 0.75;
	glm::vec2 verticalSliderSize(sliderWidth, sliderHeight);
	glm::vec2 velocityDisplaySize(velocityDisplayWidth, sliderHeight);
	float channelWidth = verticalSliderSize.x + velocityDisplaySize.x + ImGui::GetStyle().ItemSpacing.x;
	
	static double min = -1.0;
	static double max = 1.0;
	double separatorWidth = ImGui::GetTextLineHeight() * .3f;
	double separatorHeight = sliderHeight;
	
	auto drawAxisControls = [&](std::shared_ptr<AnimatablePosition> animatable, const char* customName = nullptr){
	 
		//--- Animatable Name
		ImGui::PushFont(Fonts::sansRegular12);
		glm::vec2 nameFrameSize(channelWidth, ImGui::GetTextLineHeight());
		if(customName) backgroundText(customName, nameFrameSize, Colors::darkGray);
		else backgroundText(animatable->getName(), nameFrameSize, Colors::darkGray);
		ImGui::PopFont();
		
		//--- Velocity Slider & Feedback
		ImGui::VSliderScalar("##ManualVelocity", verticalSliderSize, ImGuiDataType_Double, &animatable->velocitySliderDisplayValue, &min, &max, "");
		if (ImGui::IsItemActive()) animatable->setManualVelocityTarget(animatable->velocitySliderDisplayValue);
		else if (ImGui::IsItemDeactivatedAfterEdit()) {
			animatable->setManualVelocityTarget(0.0);
			animatable->velocitySliderDisplayValue = 0.0;
		}
		ImGui::SameLine();
		verticalProgressBar(std::abs(animatable->getActualVelocityNormalized()), velocityDisplaySize);
		
		//--- Numerical Velocity & Position Feedback
		ImGui::PushFont(Fonts::sansRegular12);
		glm::vec2 feedbackFrameSize(channelWidth, ImGui::GetTextLineHeight() * 2.0);
		static char feedbackString[32];
		const char *positionUnitAbbreviated = animatable->getUnit()->abbreviated;
		sprintf(feedbackString, "%.3f%s\n%.2f%s/s",
				animatable->getActualPosition(),
				positionUnitAbbreviated,
				animatable->getActualVelocity(),
				positionUnitAbbreviated);
		backgroundText(feedbackString, feedbackFrameSize, Colors::darkGray);
		ImGui::PopFont();
		
		//--- Rapid Target Position Entry Box
		ImGui::SetNextItemWidth(channelWidth);
		static char targetPositionString[32];
		sprintf(targetPositionString, "%.3f %s", animatable->rapidTargetPositionDisplayValue, positionUnitAbbreviated);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetStyle().FramePadding.x, ImGui::GetTextLineHeight() * 0.1));
		ImGui::InputDouble("##TargetPosition", &animatable->rapidTargetPositionDisplayValue, 0.0, 0.0, targetPositionString);
		ImGui::PopStyleVar();
		animatable->rapidTargetPositionDisplayValue = std::clamp(animatable->rapidTargetPositionDisplayValue, animatable->lowerPositionLimit, animatable->upperPositionLimit);
		if(animatable->isInRapid()){
			//display rapid progress if in rapid
			float rapidProgress = animatable->getRapidProgress();
			glm::vec2 targetmin = ImGui::GetItemRectMin();
			glm::vec2 targetmax = ImGui::GetItemRectMax();
			glm::vec2 targetsize = ImGui::GetItemRectSize();
			glm::vec2 progressBarMax(targetmin.x + targetsize.x * rapidProgress, targetmax.y);
			ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
		}
		
		//--- Stop and Rapid Buttons
		if(animatable->isControlledManuallyOrByAnimation()){
			if (ImGui::Button("Stop", ImVec2(channelWidth, ImGui::GetTextLineHeight() * 1.5))) {
				animatable->stopMovement();
			}
		}else{
			if (ImGui::Button("Rapid", ImVec2(channelWidth, ImGui::GetTextLineHeight() * 1.5))) {
				animatable->setManualPositionTargetWithVelocity(animatable->rapidTargetPositionDisplayValue, animatable->velocityLimit);
			}
		}
		
	};
	
	ImGui::BeginGroup();
	ImGui::PushID(axis1Animatable->getName());
	drawAxisControls(axis1Animatable);
	ImGui::PopID();
	ImGui::EndGroup();
	
	ImGui::SameLine(.0f, separatorWidth);
	
	ImGui::BeginGroup();
	ImGui::PushID(axis2Animatable->getName());
	drawAxisControls(axis2Animatable);
	ImGui::PopID();
	ImGui::EndGroup();
	
	
	if(enableSynchronousControl->value){
		ImGui::SameLine(.0f, separatorWidth);

		ImGui::BeginGroup();
		ImGui::PushID(axis2Animatable->getName());
		drawAxisControls(synchronizedAnimatable, "Synchro");
		ImGui::PopID();
		ImGui::EndGroup();
		
	}
	
	ImGui::SameLine(.0f, separatorWidth);
	
	ImGui::BeginGroup();
	if(positionUnit->unitType == Units::Type::LINEAR_DISTANCE){
		
		//——— title card
		ImGui::PushFont(Fonts::sansRegular12);
		glm::vec2 nameFrameSize(channelWidth, ImGui::GetTextLineHeight());
		backgroundText("Feedback", nameFrameSize, Colors::darkGray);
		ImGui::PopFont();
		
		ImGui::InvisibleButton("LinearAxisDisplay", glm::vec2(channelWidth, sliderHeight));
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 size = max - min;
		
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		drawing->AddRectFilled(min, max, ImColor(Colors::gray));
		
		
		
		
		/*
		 
		 ImGui::SameLine();
		 verticalProgressBar(animatable->getActualPositionNormalized(), verticalSliderSize);
		 
		 //--- Draw Constraints
		 glm::vec2 minPosProg = ImGui::GetItemRectMin();
		 glm::vec2 maxPosProg = ImGui::GetItemRectMax();
		 glm::vec2 progSize = maxPosProg - minPosProg;
		 auto& constraints = animatable->getConstraints();
		 ImDrawList* drawing = ImGui::GetWindowDrawList();
		 for(auto& constraint : constraints){
			 auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
			 double minKeepout = animatable->normalizePosition(keepout->keepOutMinPosition);
			 double maxKeepout = animatable->normalizePosition(keepout->keepOutMaxPosition);
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
		 animatable->getConstraintPositionLimits(minPositionLimit, maxPositionLimit);
		 double minPosition = maxPosProg.y - progSize.y * animatable->normalizePosition(minPositionLimit);
		 double maxPosition = maxPosProg.y - progSize.y * animatable->normalizePosition(maxPositionLimit);
			 drawing->AddLine(ImVec2(minPosProg.x, minPosition), ImVec2(maxPosProg.x, minPosition), ImColor(1.f, 1.f, 1.f, 1.f));
			 drawing->AddLine(ImVec2(minPosProg.x, maxPosition), ImVec2(maxPosProg.x, maxPosition), ImColor(1.f, 1.f, 1.f, 1.f));
		 }

		 //--- Draw Rapid Target
		 if(animatable->isInRapid()){
			 float normalizedPositionTarget = animatable->normalizePosition(animatable->getRapidTarget()->toPosition()->position);
			 float height = minPosProg.y + progSize.y * (1.0 - normalizedPositionTarget);
			 glm::vec2 lineStart(minPosProg.x, height);
			 glm::vec2 lineEnd(maxPosProg.x, height);
			 drawing->AddLine(lineStart, lineEnd, ImColor(Colors::white));
		 }
		 
		 static char actualPositionString[32];
		 sprintf(actualPositionString, "%.7f%s", animatable->getActualPosition(), positionUnitAbbreviated);
		 
		 ImGui::SameLine();
		 ImGui::Button(actualPositionString, feedbackButtonSize);
		 */
		
		
		
		
		
		
	}else if(positionUnit->unitType == Units::Type::ANGULAR_DISTANCE){
		float displayDiameter = verticalSliderSize.y;
		
		//——— title card
		ImGui::PushFont(Fonts::sansRegular12);
		glm::vec2 nameFrameSize(displayDiameter, ImGui::GetTextLineHeight());
		backgroundText("Feedback", nameFrameSize, Colors::darkGray);
		ImGui::PopFont();
		
		ImGui::InvisibleButton("rotatingAxisDisplay", glm::vec2(verticalSliderSize.y));
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 size = max - min;
		glm::vec2 middle = (max + min) / 2.0;
		float radius = verticalSliderSize.y / 2.0;
					
		float triangleSize = ImGui::GetTextLineHeight() * .5f;
		float lineWidth = ImGui::GetTextLineHeight() * .1f;
		
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		
		auto drawArrowRotated = [&](glm::vec2 center, float startRadius, float endRadius, float angleRadians){
			float lineEndRadius = endRadius - triangleSize * .5f;
			glm::vec2 start(startRadius * std::cos(angleRadians), startRadius * std::sin(angleRadians));
			glm::vec2 end(lineEndRadius * std::cos(angleRadians), lineEndRadius * std::sin(angleRadians));
			start += center;
			end += center;
			
			glm::vec2 trianglePoints[3] = {
				center + glm::rotate(glm::vec2(endRadius, 0), angleRadians),
				center + glm::rotate(glm::vec2(endRadius - triangleSize, triangleSize * .4f), angleRadians),
				center + glm::rotate(glm::vec2(endRadius - triangleSize, -triangleSize * .4f), angleRadians)
			};
			
			drawing->AddLine(start, end, ImColor(Colors::white), lineWidth);
			drawing->AddTriangleFilled(trianglePoints[0],
								 trianglePoints[1],
								 trianglePoints[2],
								 ImColor(Colors::white));
			
		};
		
		//draw visualizer background
		ImColor backgroundColor = ImColor(Colors::darkGray);
		ImColor borderColor = ImColor(Colors::black);
		float borderWidth = ImGui::GetTextLineHeight() * .05f;
		drawing->AddCircleFilled(middle, size.x / 2.0, backgroundColor);

		//get visualizer angles
		double angle1 = axis1Animatable->getActualPosition() + Timing::getProgramTime_seconds() * 30.0;
		double angle2 = axis2Animatable->getActualPosition() + Timing::getProgramTime_seconds() * -40.0;
		double displayAngle1 = Units::convert(angle1, positionUnit, Units::AngularDistance::Radian) - M_PI_2;
		double displayAngle2 = Units::convert(angle2, positionUnit, Units::AngularDistance::Radian) - M_PI_2;
		
		//draw visualizer arrows
		drawing->AddCircle(middle, radius, borderColor, 64, borderWidth);
		drawArrowRotated(middle, radius * .5f, radius, displayAngle1);
		drawing->AddCircle(middle, radius * .5f, borderColor, 64, borderWidth);
		drawArrowRotated(middle, 0.0, radius * .5f, displayAngle2);
		drawing->AddCircleFilled(middle, lineWidth, borderColor, 64);

		//zero tick mark
		float zeroTickLength = ImGui::GetTextLineHeight() * .25f;
		drawing->AddLine(middle, middle + glm::vec2(0, -radius), ImColor(0.f, 0.f, 0.f, .2f), borderWidth);
		drawing->AddLine(middle + glm::vec2(0, -radius), middle + glm::vec2(0, -radius + zeroTickLength), ImColor(Colors::white), borderWidth);
		drawing->AddLine(middle + glm::vec2(0, -radius * .5f), middle + glm::vec2(0, -radius * .5f + zeroTickLength), ImColor(Colors::white), borderWidth);
		
		
		//display position string on turntables
		static char angle1String[64];
		static char angle2String[64];
		if(positionUnit == Units::AngularDistance::Degree){
			auto getDegreeString = [](char* output, double angleDegrees){
				int extraRevs = floor(std::abs(angleDegrees) / 360.0);
				if(angleDegrees < 0.0) extraRevs++;
				float angleSingleTurn;
				if(angleDegrees >= 0.0) angleSingleTurn = fmod(angleDegrees, 360.0);
				else angleSingleTurn = 360.0 + fmod(angleDegrees, 360.0);
				sprintf(output, "%.1f%s%s%i", angleSingleTurn,  Units::AngularDistance::Degree->abbreviated, angleDegrees > 0.0 ? "+" : "-", extraRevs);
			};
			getDegreeString(angle1String, angle1);
			getDegreeString(angle2String, angle2);
		}else if(positionUnit == Units::AngularDistance::Radian){
			sprintf(angle1String, "%.3f%s", fmod(angle1, 2.0 * M_PI), positionUnit->abbreviated);
			sprintf(angle2String, "%.3f%s", fmod(angle2, 2.0 * M_PI), positionUnit->abbreviated);
		}else{
			sprintf(angle1String, "%.3f%s", angle1, positionUnit->abbreviated);
			sprintf(angle2String, "%.3f%s", angle2, positionUnit->abbreviated);
		}
		ImGui::PushFont(Fonts::sansBold15);
		glm::vec2 textSize;
		glm::vec2 textPos;
		textSize = ImGui::CalcTextSize(angle1String);
		textPos = middle + glm::vec2(0, - 3.0 * radius / 4.0) - textSize / 2.0;
		drawing->AddText(textPos, ImColor(0.f, 0.f, 0.f, .6f), angle1String);
		textSize = ImGui::CalcTextSize(angle2String);
		textPos = middle + glm::vec2(0, -radius / 4.0) - textSize / 2.0;
		drawing->AddText(textPos, ImColor(0.f, 0.f, 0.f, .6f), angle2String);
		ImGui::PopFont();
		
		glm::vec2 rangeDisplaySize(radius * 2.0, ImGui::GetTextLineHeight());
		static char rangeString[32];
		float rangeProgress = axis1Animatable->getActualPositionNormalized();
		sprintf(rangeString, "%s : %.1f%%", axis1Animatable->getName(), rangeProgress * 100.0);
		ImGui::ProgressBar(rangeProgress, rangeDisplaySize, rangeString);
		rangeProgress = axis2Animatable->getActualPositionNormalized();
		sprintf(rangeString, "%s : %.1f%%", axis2Animatable->getName(), rangeProgress * 100.0);
		ImGui::ProgressBar(rangeProgress, rangeDisplaySize, rangeString);
		
	}
	ImGui::EndGroup();

	
	
	//end main widget content group
	ImGui::EndGroup();
	
	ImGui::EndDisabled();
	
	float widgetWidth_final = ImGui::GetItemRectSize().x;
	machineHeaderGui(headerCursor, widgetWidth_final);
	
	machineStateControlGui(widgetWidth_final);
	
	ImGui::PopStyleVar();
}
