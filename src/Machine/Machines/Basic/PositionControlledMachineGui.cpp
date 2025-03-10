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
	
	if(isAxisConnected()){
		Units::Type axisUnitType = getAxisInterface()->getPositionUnit()->unitType;
		if(axisUnitType == Units::Type::ANGULAR_DISTANCE) angularWidgetGui();
		else if(axisUnitType == Units::Type::LINEAR_DISTANCE) {
			if(linearWidgetOrientation_parameter->value == linearWidgetOrientation_vertical.getInt()){
				verticalWidgetGui();
			}
			else if(linearWidgetOrientation_parameter->value == linearWidgetOrientation_horizontal.getInt()){
				horizontalWidgetGui();
			}
		}
		ImGui::Separator();
	}
	
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
	if(ImGui::Button("Reset##Offset")) resetUserZero();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Lower Limit");
	ImGui::PopFont();
	lowerPositionLimit->gui();
	ImGui::SameLine();
	if(ImGui::Button("Reset##LowerLimit")) resetLowerUserLimit();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Upper Limit");
	ImGui::PopFont();
	upperPositionLimit->gui();
	ImGui::SameLine();
	if(ImGui::Button("Reset##UpperLimit")) resetUpperUserLimit();

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
	
	allowModuloPositionShifting->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("%s", allowModuloPositionShifting->getName());
	ImGui::PopFont();
	
	ImGui::Separator();
	
	displayModuloturns_param->gui(Fonts::sansBold15);
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Control Widget");
	ImGui::PopFont();
	
	if(positionUnit->unitType == Units::Type::LINEAR_DISTANCE){
		linearWidgetOrientation_parameter->gui(Fonts::sansBold15);
	}
	
	invertControlGui->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Invert Control Widget");
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
	
	Units::Type axisUnitType = machine->getAxisInterface()->getPositionUnit()->unitType;
	if(axisUnitType == Units::Type::ANGULAR_DISTANCE) machine->angularWidgetGui();
	else if(axisUnitType == Units::Type::LINEAR_DISTANCE){
		if(machine->linearWidgetOrientation_parameter->value == machine->linearWidgetOrientation_vertical.getInt()){
			machine->verticalWidgetGui();
		}
		else if(machine->linearWidgetOrientation_parameter->value == machine->linearWidgetOrientation_horizontal.getInt()){
			machine->horizontalWidgetGui();
		}
	}

	ImGui::EndGroup();
	
	float widgetWidth = ImGui::GetItemRectSize().x;
	machine->machineHeaderGui(headerCursorPos, widgetWidth);
	machine->machineStateControlGui(widgetWidth);
}


//————————————————————————————————————————————— WIDGET GUI —————————————————————————————————————————————

void PositionControlledMachine::verticalWidgetGui(){
		
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	ImGui::BeginDisabled(!isEnabled());

	animatablePosition->manualControlsVerticalGui(ImGui::GetTextLineHeight() * 10.f, nullptr, invertControlGui->value);
	float controlsHeight = ImGui::GetItemRectSize().y;
	if(controlsHeight <= 0.0) controlsHeight = 1.0;

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
	
}

void PositionControlledMachine::horizontalWidgetGui(){
	
	float sliderHeight = ImGui::GetTextLineHeight() * 12.0;
	double separatorWidth = ImGui::GetTextLineHeight() * .25f;
	float feedbackWidth = ImGui::GetTextLineHeight() * 2.f;
	
	
	auto getNormalizedPosition = [this](double pos) -> double {
		float minPos = animatablePosition->lowerPositionLimit;
		float maxPos = animatablePosition->upperPositionLimit;
		double norm = (pos - minPos) / (maxPos - minPos);
		return std::clamp(norm, 0.0, 1.0);
	};
	
	auto getNormalizedDistance = [this](double distance) -> double{
		float minPos = animatablePosition->lowerPositionLimit;
		float maxPos = animatablePosition->upperPositionLimit;
		if(maxPos == minPos) return 0.0;
		return std::abs(distance / (maxPos - minPos));
	};
	
	//draw control sliders and widgets
	animatablePosition->manualControlsHorizontalGui(sliderHeight);
	
	
	
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y + separatorWidth);
	float controlsWidth = ImGui::GetItemRectSize().x;
	
	ImGui::BeginGroup();
	ImGui::InvisibleButton("LinearAxisDisplay", glm::vec2(controlsWidth, feedbackWidth));
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = max - min;
				
	//position indicator background
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	ImGui::PushClipRect(min, max, true);
	
	if(getState() == DeviceState::OFFLINE){
		drawing->AddRectFilled(min, max, ImColor(Colors::blue));
	}else{
		
		drawing->AddRectFilled(min, max, ImColor(Colors::darkGray));
		
		//draw keepout constraints
		for(auto& constraint : animatablePosition->getConstraints()){
			if(constraint->getType() != AnimationConstraint::Type::KEEPOUT) continue;
			auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
			double constraintMinX = min.x + size.x * getNormalizedPosition(keepout->keepOutMinPosition);
			double constraintMaxX = min.x + size.x * getNormalizedPosition(keepout->keepOutMaxPosition);
			drawing->AddRectFilled(glm::vec2(constraintMinX, min.y),
								   glm::vec2(constraintMaxX, max.y),
								   constraint->isEnabled() ? ImColor(1.f, 0.f, 0.f, .4f) : ImColor(1.f, 1.f, 1.f, .2f));
		}
		
		//draw constraint limit lines
		ImColor limitLineColor = ImColor(.0f, .0f, .0f, 1.f);
		float limitLineThickness = ImGui::GetTextLineHeight() * .05f;
		
		double axis1ConstraintMin, axis1ConstraintMax;
		animatablePosition->getConstraintPositionLimits(axis1ConstraintMin, axis1ConstraintMax);
		double axis1ConstraintMinX = min.x + size.x * getNormalizedPosition(axis1ConstraintMin);
		double axis1ConstraintMaxX = min.x + size.x * getNormalizedPosition(axis1ConstraintMax);
		drawing->AddLine(glm::vec2(axis1ConstraintMinX, min.y),
						 glm::vec2(axis1ConstraintMinX, max.y),
						 limitLineColor, limitLineThickness);
		drawing->AddLine(glm::vec2(axis1ConstraintMaxX, min.y),
						 glm::vec2(axis1ConstraintMaxX, max.y),
						 limitLineColor, limitLineThickness);
		
		
		//draw rapid target
		ImColor targetColor = ImColor(Colors::yellow);
		float targetLineThickness = ImGui::GetTextLineHeight() * .05f;
		float targetTriangleSize = ImGui::GetTextLineHeight() * .4f;
		
		if(animatablePosition->isInRapid()){
			float targetX = min.x + size.x * getNormalizedPosition(animatablePosition->getRapidTarget()->toPosition()->position);
			float triangleY = min.y + size.y * .25f;
			drawing->AddLine(glm::vec2(targetX, min.y), glm::vec2(targetX, max.y), targetColor, targetLineThickness);
			drawing->AddTriangleFilled(glm::vec2(targetX, triangleY),
									   glm::vec2(targetX - targetTriangleSize, triangleY + targetTriangleSize * .4f),
									   glm::vec2(targetX - targetTriangleSize, triangleY - targetTriangleSize * .4f),
									   targetColor);
			drawing->AddTriangleFilled(glm::vec2(targetX, triangleY),
									   glm::vec2(targetX + targetTriangleSize, triangleY - targetTriangleSize * .4f),
									   glm::vec2(targetX + targetTriangleSize, triangleY + targetTriangleSize * .4f),
									   targetColor);
		}
		
		
		//draw position arrow indicators
		float lineThickness = ImGui::GetTextLineHeight() * .05f;
		float triangleSize = ImGui::GetTextLineHeight() * .5f;
		ImColor positionIndicatorColor = ImColor(Colors::white);
		ImColor brakingPositionIndicatorColor = ImColor(1.f, 1.f, 1.f, .3f);
		
		double axis1_X = min.x + size.x * getNormalizedPosition(animatablePosition->getActualPosition());
		double axis1BrakingPositionX = min.x + size.x * getNormalizedPosition(animatablePosition->getBrakingPosition());
		
		drawing->AddLine(glm::vec2(axis1BrakingPositionX, min.y),
						 glm::vec2(axis1BrakingPositionX, max.y - triangleSize + 1.f),
						 brakingPositionIndicatorColor, lineThickness);
		drawing->AddTriangleFilled(glm::vec2(axis1BrakingPositionX, max.y),
								   glm::vec2(axis1BrakingPositionX - triangleSize * .4f, max.y - triangleSize),
								   glm::vec2(axis1BrakingPositionX + triangleSize * .4f, max.y - triangleSize),
								   brakingPositionIndicatorColor);
		
		drawing->AddLine(glm::vec2(axis1_X, min.y), glm::vec2(axis1_X, max.y - triangleSize + 1.f), positionIndicatorColor, lineThickness);
		drawing->AddTriangleFilled(glm::vec2(axis1_X, max.y),
								   glm::vec2(axis1_X - triangleSize * .4f, max.y - triangleSize),
								   glm::vec2(axis1_X + triangleSize * .4f, max.y - triangleSize),
								   positionIndicatorColor);
	}
	
	//draw frame outline
	float frameWidth = ImGui::GetTextLineHeight() * 0.05;
	drawing->AddRect(min - glm::vec2(frameWidth * .5f), max + glm::vec2(frameWidth * .5f), ImColor(Colors::black), frameWidth, ImDrawFlags_RoundCornersAll, frameWidth);
	
	ImGui::PopClipRect();
	 
	ImGui::EndGroup();
				
}

void PositionControlledMachine::angularWidgetGui(){
	float displayDiameter = ImGui::GetTextLineHeight() * 8.0;
	
	ImGui::InvisibleButton("rotatingAxisDisplay", glm::vec2(displayDiameter));
	
	ImRect rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && rect.Contains(ImGui::GetMousePos())) {
		displayModuloturns_param->overwrite(!displayModuloturns_param->value);
	}
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = max - min;
	glm::vec2 middle = (max + min) / 2.0;
	float radius = displayDiameter / 2.0;
				
	float triangleSize = ImGui::GetTextLineHeight() * .5f;
	float lineWidth = ImGui::GetTextLineHeight() * .1f;
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	auto drawArrowRotated = [&](glm::vec2 center, float startRadius, float endRadius, float angleRadians, ImVec4 color){
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
		
		drawing->AddLine(start, end, ImColor(color), lineWidth);
		drawing->AddTriangleFilled(trianglePoints[0],
							 trianglePoints[1],
							 trianglePoints[2],
							 ImColor(color));
		
	};
	
	
	if(getState() == DeviceState::OFFLINE){
		drawing->AddCircleFilled(middle, size.x * .5f, ImColor(Colors::blue));
	}else{
		
		drawing->AddCircleFilled(middle, size.x / 2.0, ImColor(Colors::darkGray));
		
		ImColor borderColor = ImColor(Colors::black);
		float borderWidth = ImGui::GetTextLineHeight() * .05f;
		
		//get visualizer angles
		double positionAngle = animatablePosition->getActualPosition();
		double breakingPositionAngle = animatablePosition->getBrakingPosition();
		double displayPositionAngle = Units::convert(positionAngle, animatablePosition->getUnit(), Units::AngularDistance::Radian) - M_PI_2;
		double displayBreakingPositionAngle = Units::convert(breakingPositionAngle, animatablePosition->getUnit(), Units::AngularDistance::Radian) - M_PI_2;
		
		//background & zero tick mark
		drawing->AddCircle(middle, radius, borderColor, 64, borderWidth);
		float zeroTickLength = ImGui::GetTextLineHeight() * .25f;
		drawing->AddLine(middle, middle + glm::vec2(0, -radius), ImColor(0.f, 0.f, 0.f, .2f), borderWidth);
		
		//draw visualizer arrows
		drawArrowRotated(middle, 0.0, radius, displayBreakingPositionAngle, Colors::gray);
		drawArrowRotated(middle, 0.0, radius, displayPositionAngle, Colors::white);
		
		//display position string on turntables
		
		char positiveAngleString[64];
		if(displayModuloturns_param->value){
			double positionDegrees = Units::convert(animatablePosition->getActualPosition(), animatablePosition->getUnit(), Units::AngularDistance::Degree);
			int fullTurns = std::floor(positionDegrees / 360.0);
			double singleTurnPosition = positionDegrees - fullTurns * 360.0;
			snprintf(positiveAngleString, 64, "%iR+%.1f°", fullTurns, singleTurnPosition);
		}else{
			snprintf(positiveAngleString, 64, "%.2f%s", animatablePosition->getActualPosition(), animatablePosition->getUnit()->abbreviated);
		}
		
		drawing->AddCircleFilled(middle, radius * .5, ImColor(Colors::veryDarkGray), 64);
		drawing->AddLine(middle + glm::vec2(0, -radius * .5f), middle + glm::vec2(0, -radius * .5f + zeroTickLength), ImColor(Colors::white), borderWidth);
		ImGui::PushFont(Fonts::sansBold15);
		textAligned(positiveAngleString, middle, TextAlignement::MIDDLE_MIDDLE);
		ImGui::PopFont();
	}
	
	glm::vec2 rangeDisplaySize(radius * 2.0, ImGui::GetTextLineHeight() * .5);
	float rangeProgress = animatablePosition->getActualPositionNormalized();
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::gray);
	ImGui::ProgressBar(rangeProgress, rangeDisplaySize, "");
	ImGui::PopStyleColor();
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::Text("Position in working range");
		ImGui::EndTooltip();
	}
	
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

	ImGui::ProgressBar(effortProgress, ImVec2(displayDiameter, ImGui::GetTextLineHeight() * 0.5), "");
	ImGui::PopStyleColor(2);
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::Text("Control Effort");
		ImGui::EndTooltip();
	}
	
	animatablePosition->manualControlsHorizontalGui(displayDiameter, nullptr);

}







//————————————————————————————————————————————— SETUP GUI —————————————————————————————————————————————


void PositionControlledMachine::setupGui(){
	
	if(!isAxisConnected()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("No Axis Connected.");
		ImGui::PopStyleColor();
		return;
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetStyle().ItemSpacing.y));
	
	if(allowUserZeroEdit->value){
		if(ImGui::Button("Capture Zero")) captureUserZero();
		ImGui::SameLine();
		if(ImGui::Button("Reset##Zero")) resetUserZero();
	}
	
	if(allowUserLowerLimitEdit->value){
		if(ImGui::Button("Capture Lower Limit")) captureLowerUserLimit();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
		lowerPositionLimit->gui();
		ImGui::SameLine();
		if(ImGui::Button("Reset##LowerLimit")) resetLowerUserLimit();
	}
	
	if(allowUserUpperLimitEdit->value){
		if(ImGui::Button("Capture Upper Limit")) captureUpperUserLimit();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
		upperPositionLimit->gui();
		ImGui::SameLine();
		if(ImGui::Button("Reset##UpperLimit")) resetUpperUserLimit();
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


		ImVec2 homingButtonSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
		
		ImVec2 buttonCursorPos = ImGui::GetCursorPos();

		ImGui::BeginDisabled(!canStartHoming());
		if(isHoming()){
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			if(customButton("Stop Homing", homingButtonSize, ImGui::GetStyle().Colors[ImGuiCol_FrameBg], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTop))
				stopHoming();
			ImGui::PopStyleColor();
		}else{
			if(customButton("Start Homing", homingButtonSize, ImGui::GetStyle().Colors[ImGuiCol_FrameBg], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTop))
				startHoming();
		}
		ImGui::EndDisabled();

		ImVec4 progressIndicatorColor = Colors::darkGray;
		if(isHoming()) progressIndicatorColor = Colors::orange;
		else if(didHomingSucceed()) progressIndicatorColor = Colors::green;
		else if(didHomingFail()) progressIndicatorColor = Colors::red;
		
		ImGui::SetCursorPos(ImVec2(buttonCursorPos.x, buttonCursorPos.y + ImGui::GetItemRectSize().y));

		glm::vec2 homingProgressSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
		std::string homingString = getHomingString();
		backgroundText(homingString.c_str(), homingButtonSize, progressIndicatorColor, Colors::white, ImDrawFlags_RoundCornersBottom);
	}
		
		
		
		
	if(allowUserEncoderRangeReset->value){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("User Encoder Range Reset");
		ImGui::PopStyleColor();
	}
	
	if(allowModuloPositionShifting->value){
		if(isAxisConnected() && getAxisInterface()->getPositionUnit() == Units::AngularDistance::Degree){
			
			ImVec2 buttonSize(ImGui::GetTextLineHeight() * 3.0, ImGui::GetFrameHeight());
			ImGui::BeginGroup();
			bool b_disabled = animatablePosition->isMoving() || animatablePosition->hasAnimation();
			ImGui::BeginDisabled(b_disabled);
			auto axis = getAxisInterface();
			
			ImGui::BeginDisabled(!canSetTurnOffset(turnOffset + 1));
			if(ImGui::Button("+1R", buttonSize)) setTurnOffset(turnOffset + 1);
			ImGui::EndDisabled();
			
			ImGui::BeginDisabled(!canSetTurnOffset(turnOffset - 1));
			if(ImGui::Button("-1R", buttonSize)) setTurnOffset(turnOffset - 1);
			ImGui::EndDisabled();
			
			ImGui::EndDisabled();
			ImGui::EndGroup();
			
			ImGui::SameLine();
			
			ImDrawList* drawing = ImGui::GetWindowDrawList();
			ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, buttonSize.y * 2.0 + ImGui::GetStyle().ItemSpacing.y));
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			drawing->AddRectFilled(min, max, ImColor(Colors::darkGray));
			
			double axisMin = axis->getLowerPositionLimit();
			double axisMax = axis->getUpperPositionLimit();
			auto toAxisXCoords = [&](double input) -> double {
				double norm = (input - axisMin) / (axisMax - axisMin);
				return (max.x - min.x) * norm + min.x;
			};
			
			double machineMin = lowerPositionLimit->value - turnOffset * 360.0;
			double machineMax = upperPositionLimit->value - turnOffset * 360.0;

			drawing->AddRectFilled(ImVec2(toAxisXCoords(machineMin), min.y),
								   ImVec2(toAxisXCoords(machineMax), max.y),
								   ImColor(Colors::green));
			double machinePos = animatablePosition->getActualPosition() - turnOffset * 360.0;
			drawing->AddLine(ImVec2(toAxisXCoords(machinePos), min.y),
							 ImVec2(toAxisXCoords(machinePos), max.y),
							 ImColor(Colors::white), 2.0);
			
			if(!isinf(axisMin) && !isinf(axisMax)){
				for(double i = std::ceil(axisMin / 360.0) * 360.0; i < axisMax; i += 360.0){
					double xPos = toAxisXCoords(i);
					drawing->AddLine(ImVec2(xPos, min.y), ImVec2(xPos, max.y), ImColor(ImVec4(0.0, 0.0, 0.0, 0.2)), 1.0);
				}
			}
			
			drawing->AddRect(min, max, ImColor(Colors::black), 0.0, ImDrawFlags_None, 1.0);
		}
	}
	
	ImGui::PopStyleVar();
	
}



#include "Stacato/StacatoGui.h"

void PositionControlledMachine::ProgrammingWidget::gui(){
	
	ImVec2 switchSize = ImVec2(ImGui::GetTextLineHeight() * 3.0, ImGui::GetTextLineHeight()*2.0);
	ImVec2 targetSize = ImVec2(ImGui::GetTextLineHeight() * 5.0, ImGui::GetTextLineHeight()*2.0);
	ImVec2 captureSize = ImVec2(ImGui::GetTextLineHeight() * 3.5, ImGui::GetTextLineHeight()*2.0);
	
	float sliderWidth = ImGui::GetTextLineHeight() * 10.0;
	
	ImGui::PushFont(Fonts::sansBold26);
	char buf[32];
	snprintf(buf, 32, "Vel: %.2fm/s", machine->animatablePosition->getActualVelocity());
	ImVec2 velSize(ImGui::GetTextLineHeight() * 8.0, ImGui::GetTextLineHeight()*1.2);
	backgroundText(buf, velSize, Colors::veryDarkGray);
	ImGui::SameLine();
	snprintf(buf, 32, "Pos: %.3fm", machine->animatablePosition->getActualPosition());
	ImVec2 posSize(ImGui::GetTextLineHeight() * 8.0, ImGui::GetTextLineHeight()*1.2);
	backgroundText(buf, posSize, Colors::veryDarkGray);
	ImGui::PopFont();
	
	for(int i = 0; i < targets.size(); i++){
		ImGui::PushID(i);
		
		if(targets[i].modeSwitch.draw("##mode", targets[i].useTime, "Time", "Velocity", switchSize)){
			targets[i].useTime = !targets[i].useTime;
			Stacato::Gui::save();
		}
		
		ImGui::SameLine();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0, 9.0));
		ImGui::SetNextItemWidth(sliderWidth);
		if(targets[i].useTime){
			ImGui::SliderFloat("##Time", &targets[i].time, 0.0, 60.0, "Time: %.1fs");
		}
		else{
			float maxVel = machine->animatablePosition->velocityLimit;
			ImGui::SliderFloat("##Vel", &targets[i].velocity, 0.0, maxVel, "Velocity: %.2fm/s");
		}
		ImGui::PopStyleVar();
		if(ImGui::IsItemDeactivatedAfterEdit()){
			targets[i].time = std::clamp(targets[i].time, 0.0f, 60.0f);
			targets[i].velocity = std::clamp(targets[i].velocity, 0.0f, float(machine->animatablePosition->velocityLimit));
			Stacato::Gui::save();
		}
		
		ImGui::SameLine();
		if(ImGui::Button("Capture", captureSize)){
			targets[i].position = std::clamp(machine->animatablePosition->getActualPosition(),
											 machine->animatablePosition->lowerPositionLimit,
											 machine->animatablePosition->upperPositionLimit);
			Stacato::Gui::save();
		}
		
		ImGui::SameLine();
		char buffer[32];
		snprintf(buffer, 32, "%.3f", targets[i].position);
		ImGui::PushFont(Fonts::sansBold26);
		backgroundText(buffer, targetSize, Colors::black);
		
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansBold26);
		if(customButton("GO", targetSize, Colors::green, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll)){
			if(targets[i].useTime){
				machine->animatablePosition->moveToPositionInTime(targets[i].position, targets[i].time);
			}
			else{
				machine->animatablePosition->moveToPositionWithVelocity(targets[i].position, targets[i].velocity);
			}
		}
		ImGui::PopFont();
		
		ImGui::PopID();
	}
}
