#include <pch.h>

#include "PositionControlledMachine.h"
#include "Motion/Axis/AxisNode.h"

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
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Setup Controls");
	ImGui::PopFont();
	
	setupGui();
	
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
	ImGui::SameLine();
	if(ImGui::Button("Reset##Offset")) resetZero();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Lower Limit");
	ImGui::PopFont();
	lowerPositionLimit->gui();
	ImGui::SameLine();
	if(ImGui::Button("Reset##LowerLimit")) resetLowerLimit();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Upper Limit");
	ImGui::PopFont();
	upperPositionLimit->gui();
	ImGui::SameLine();
	if(ImGui::Button("Reset##UpperLimit")) resetUpperLimit();
	
	
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
	
	invertControlGui->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Invert Control Gui");
	ImGui::PopFont();
}

void PositionControlledMachine::axisGui() {}

void PositionControlledMachine::deviceGui() {}

void PositionControlledMachine::metricsGui() {}








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


//————————————————————————————————————————————— WIDGET GUI —————————————————————————————————————————————

void PositionControlledMachine::widgetGui(){
			
	/*
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	ImGui::BeginDisabled(!isEnabled());
	
	animatablePosition->manualControlsVerticalGui(ImGui::GetTextLineHeight() * 10.f, nullptr, invertControlGui->value);
	float controlsHeight = ImGui::GetItemRectSize().y;
	
	ImGui::SameLine();
	
	float actualEffort = 0.0;
	if(isAxisConnected()) actualEffort = getAxisInterface()->getEffortActual();
	float effortProgress = actualEffort;
	while(effortProgress > 1.0) effortProgress -= 1.0;
	if(actualEffort > 2.0) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::yellow);
	}else if(actualEffort > 1.0){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::green);
	}else{
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_FrameBg));
	}
	
	verticalProgressBar(effortProgress, ImVec2(ImGui::GetTextLineHeight() * .5f, controlsHeight));
	ImGui::PopStyleColor(2);
	
	
	
	
	
	ImGui::SameLine();
	
	//draw feedback background
	float feedbackWidth = ImGui::GetTextLineHeight() * 3.0;
	ImGui::InvisibleButton("feedback", glm::vec2(feedbackWidth, controlsHeight));
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = max - min;
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	ImGui::PushClipRect(min, max, true);
	
	if(getState() == DeviceState::OFFLINE){
		drawing->AddRectFilled(min, max, ImColor(Colors::blue));
	}else{
		
		drawing->AddRectFilled(min, max, ImColor(Colors::darkGray));
		
		auto getDrawingPosition = [&](double position) -> double{
			if(!invertControlGui->value){
				float y = min.y + size.y - animatablePosition->normalizePosition(position) * size.y;
				return std::clamp(y, min.y, max.y);
			}else{
				float y = min.y + animatablePosition->normalizePosition(position) * size.y;
				return std::clamp(y, min.y, max.y);
			}
		};
		
		glm::vec2 mouse = ImGui::GetMousePos();
		
		//draw keepout constraints
		int keepoutConstraintCount = 0;
		//count constraints to draw them side by side instead of stacking them
		for(auto& constraint : animatablePosition->getConstraints()){
			if(constraint->getType() == AnimationConstraint::Type::KEEPOUT) keepoutConstraintCount++;
		}
		float keepoutConstraintWidth = size.x / keepoutConstraintCount;
		keepoutConstraintCount = 0;
		for(auto& constraint : animatablePosition->getConstraints()){
			int constraintXMin = std::round(min.x + keepoutConstraintCount * keepoutConstraintWidth);
			if(constraint->getType() == AnimationConstraint::Type::KEEPOUT) keepoutConstraintCount++;
			int constraintXMax = std::round(min.x + keepoutConstraintCount * keepoutConstraintWidth);
			auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
			
			double minKeepout = getDrawingPosition(keepout->keepOutMinPosition);
			double maxKeepout = getDrawingPosition(keepout->keepOutMaxPosition);
			
			glm::vec2 keepoutStartPos(constraintXMin, minKeepout);
			glm::vec2 keepoutEndPos(constraintXMax, maxKeepout);
			ImColor constraintColor;
			if(!constraint->isEnabled()) constraintColor = ImColor(1.f, 1.f, 1.f, .2f);
			else constraintColor = ImColor(1.f, 0.f, 0.f, .4f);
			drawing->AddRectFilled(keepoutStartPos, keepoutEndPos, constraintColor);
			
			
			if(minKeepout > maxKeepout){
				std::swap(keepoutStartPos.y, keepoutEndPos.y);
			}
			
			if(mouse.x > keepoutStartPos.x && mouse.y > keepoutStartPos.y && mouse.x < keepoutEndPos.x && mouse.y < keepoutEndPos.y){
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0));
				ImGui::BeginTooltip();
				ImGui::Text("%s", keepout->getName().c_str());
				ImGui::EndTooltip();
				ImGui::PopStyleVar();
			}
			
			
		}
		
		float lineThickness = ImGui::GetTextLineHeight() * .15f;
		//draw rapid target
		if(animatablePosition->isInRapid()){
			float rapidTarget = animatablePosition->getRapidTarget()->toPosition()->position;
			
			float rapidTargetY = getDrawingPosition(rapidTarget);
			
			ImColor targetColor = ImColor(Colors::yellow);
			drawing->AddLine(glm::vec2(min.x, rapidTargetY), glm::vec2(max.x, rapidTargetY), targetColor, lineThickness);
			float middleX = min.x + size.x * .5f;
			float triangleSize = ImGui::GetTextLineHeight() * .4f;
			drawing->AddTriangleFilled(glm::vec2(middleX, rapidTargetY),
									   glm::vec2(middleX - triangleSize * .4f, rapidTargetY - triangleSize),
									   glm::vec2(middleX + triangleSize * .4f, rapidTargetY - triangleSize),
									   targetColor);
			drawing->AddTriangleFilled(glm::vec2(middleX, rapidTargetY),
									   glm::vec2(middleX + triangleSize * .4f, rapidTargetY + triangleSize),
									   glm::vec2(middleX - triangleSize * .4f, rapidTargetY + triangleSize),
									   targetColor);
		}
		
		//draw braking position
		float brakingPositionY = getDrawingPosition(animatablePosition->getBrakingPosition());
		float triangleSize = ImGui::GetTextLineHeight() * .75f;
		ImColor brakingPositionColor = ImColor(1.f, 1.f, 1.f, .3f);
		drawing->AddLine(glm::vec2(min.x, brakingPositionY), glm::vec2(max.x - triangleSize, brakingPositionY), brakingPositionColor, lineThickness);
		drawing->AddTriangleFilled(glm::vec2(max.x, brakingPositionY),
								   glm::vec2(max.x - triangleSize, brakingPositionY + triangleSize * .4f),
								   glm::vec2(max.x - triangleSize, brakingPositionY - triangleSize * .4f),
								   brakingPositionColor);
		
		//draw actual position
		float axisPositionY = getDrawingPosition(animatablePosition->getActualPosition());
		drawing->AddLine(glm::vec2(min.x, axisPositionY), glm::vec2(max.x - triangleSize + 1.f, axisPositionY), ImColor(Colors::white), lineThickness);
		drawing->AddTriangleFilled(glm::vec2(max.x, axisPositionY),
								   glm::vec2(max.x - triangleSize, axisPositionY + triangleSize * .4f),
								   glm::vec2(max.x - triangleSize, axisPositionY - triangleSize * .4f),
								   ImColor(Colors::white));
		
		//draw current constraint limits
		double minPositionLimit, maxPositionLimit;
		animatablePosition->getConstraintPositionLimits(minPositionLimit, maxPositionLimit);
		
		double minPosY = getDrawingPosition(minPositionLimit);
		double maxPosY = getDrawingPosition(maxPositionLimit);
		
		
		
		ImColor limitLineColor = ImColor(0.f, 0.f, 0.f, .4f);
		float limitLineThickness = ImGui::GetTextLineHeight() * .05f;
		drawing->AddLine(ImVec2(min.x, minPosY), ImVec2(max.x, minPosY), limitLineColor, limitLineThickness);
		drawing->AddLine(ImVec2(min.x, maxPosY), ImVec2(max.x, maxPosY), limitLineColor, limitLineThickness);
	
	}
	
	ImGui::PopClipRect();
	
	//draw frame contour
	float frameWidth = ImGui::GetTextLineHeight() * 0.1f;
	drawing->AddRect(min - glm::vec2(frameWidth * .5f), max + glm::vec2(frameWidth * .5f), ImColor(Colors::black), frameWidth, ImDrawFlags_RoundCornersAll, frameWidth);
	
	ImGui::EndDisabled();
	ImGui::PopStyleVar();
	 */
}






//————————————————————————————————————————————— SETUP GUI —————————————————————————————————————————————


void PositionControlledMachine::setupGui(){
	/*
	if(!isAxisConnected()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("No Axis Connected.");
		ImGui::PopStyleColor();
		return;
	}
	
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
	
	if(allowUserZeroEdit->value || allowUserLowerLimitEdit->value || allowUserUpperLimitEdit->value){
		
		float rangeDisplayWidth = ImGui::GetTextLineHeight() * 20.0;
		float rangeDisplayHeight = ImGui::GetTextLineHeight() * 7.0;
		ImGui::InvisibleButton("rangedisplaysize", glm::vec2(rangeDisplayWidth, rangeDisplayHeight));
		
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 size = ImGui::GetItemRectSize();
		
		ImGui::PushClipRect(min, max, true);
		
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		drawing->AddRectFilled(min, max, ImColor(Colors::almostBlack));
		
		auto axis = getAxisInterface();
		double maxPosition = getMaxPosition();
		double minPosition = getMinPosition();
		double lowerLimit = getLowerPositionLimit();
		double upperLimit = getUpperPositionLimit();
		double actualPosition = animatablePosition->getActualPosition();
		
		auto toRangedPositionX = [&](double position) -> double {
			double normalized = (position - minPosition) / (maxPosition - minPosition);
			double clamped = std::clamp(normalized, 0.0, 1.0);
			return min.x + clamped * size.x;
		};
		
		glm::vec2 rangeRectMin(toRangedPositionX(lowerLimit), min.y);
		glm::vec2 rangeRectMax(toRangedPositionX(upperLimit), max.y);
		double zeroPositionX = toRangedPositionX(0.0);
		double actualPositionX = toRangedPositionX(actualPosition);
		
		float rangeLineWidth = ImGui::GetTextLineHeight() * .1f;
		drawing->AddRectFilled(rangeRectMin, rangeRectMax, ImColor(Colors::darkGreen));
		drawing->AddLine(glm::vec2(rangeRectMin.x, min.y), glm::vec2(rangeRectMin.x, max.y), ImColor(Colors::green), rangeLineWidth);
		drawing->AddLine(glm::vec2(rangeRectMax.x, min.y), glm::vec2(rangeRectMax.x, max.y), ImColor(Colors::green), rangeLineWidth);
		drawing->AddLine(glm::vec2(zeroPositionX, min.y), glm::vec2(zeroPositionX, max.y), ImColor(Colors::blue), rangeLineWidth * 2.f);
		drawing->AddLine(glm::vec2(actualPositionX, min.y), glm::vec2(actualPositionX, max.y), ImColor(Colors::white), rangeLineWidth);
		
		static char rangeString[64];
		glm::vec2 displayFramePadding(ImGui::GetTextLineHeight() * .1f, 0.0);
		float frameRounding = ImGui::GetStyle().FrameRounding;
		
		float heightPerItem = rangeDisplayHeight / 6.0;
		
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
	
		
		sprintf(rangeString, "Lower limit: %.3f%s", lowerLimit, positionUnit->abbreviated);
		textAlignedBackground(rangeString,
							  glm::vec2(rangeRectMin.x, min.y + heightPerItem * 1.f),
							  TextAlignement::LEFT_TOP,
							  ImVec4(0.f, .5f, 0.f, .7f),
							  displayFramePadding,
							  frameRounding,
							  ImDrawFlags_RoundCornersRight);
		
	
		textAlignedBackground("Zero",
							  glm::vec2(zeroPositionX, min.y + heightPerItem * 2.f),
							  TextAlignement::MIDDLE_TOP,
							  Colors::blue,
							  displayFramePadding,
							  frameRounding,
							  ImDrawFlags_RoundCornersAll,
							  true,
							  min,
							  max);
		
		sprintf(rangeString, "Position: %.3f%s", actualPosition, positionUnit->abbreviated);
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::black);
		textAlignedBackground(rangeString,
							  glm::vec2(actualPositionX, min.y + heightPerItem * 3.f),
							  TextAlignement::MIDDLE_TOP,
							  Colors::white,
							  displayFramePadding,
							  frameRounding,
							  ImDrawFlags_RoundCornersAll,
							  true,
							  min,
							  max);
		ImGui::PopStyleColor();
		
		sprintf(rangeString, "Upper limit: %.3f%s", upperLimit, positionUnit->abbreviated);
		textAlignedBackground(rangeString,
							  glm::vec2(rangeRectMax.x, min.y + heightPerItem * 4.f),
							  TextAlignement::RIGHT_TOP,
							  Colors::green,
							  displayFramePadding,
							  frameRounding,
							  ImDrawFlags_RoundCornersLeft);
			
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
	*/
}
