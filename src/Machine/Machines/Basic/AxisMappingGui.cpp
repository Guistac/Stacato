#include "AxisMapping.h"

#include "Gui/Utilities/CustomWidgets.h"

void AxisMapping::controlGui(){
	
	ImGui::BeginGroup();
		
	std::shared_ptr<AxisInterface> axis = nullptr;
	bool b_disableControlGui = true;
	if(axisPin->isConnected()) {
		axis = axisPin->getConnectedPin()->getSharedPointer<AxisInterface>();
		b_disableControlGui = axis->getState() != DeviceState::ENABLED;
	}
	bool b_invertControlGui = false;
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));

	ImGui::BeginDisabled(b_disableControlGui);
	animatablePosition->manualControlsVerticalGui(ImGui::GetTextLineHeight() * 10.f, nullptr, b_invertControlGui);
	ImGui::EndDisabled();
	float controlsHeight = ImGui::GetItemRectSize().y;

	ImGui::SameLine();

	float actualEffort = 0.0;
	if(axis) actualEffort = axis->getEffortActual();
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

	if(axis == nullptr || axis->getState() == DeviceState::OFFLINE){
		drawing->AddRectFilled(min, max, ImColor(Colors::blue));
	}else{

	drawing->AddRectFilled(min, max, ImColor(Colors::darkGray));

	auto getDrawingPosition = [&](double position) -> double{
		if(!b_invertControlGui){
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
		
	//draw profiler position
	float profilerPositionY = getDrawingPosition(animatablePosition->motionProfile.getPosition());
	drawing->AddLine(glm::vec2(min.x, axisPositionY), glm::vec2(max.x, axisPositionY), ImColor(Colors::black), 1.0);

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

	
	ImGui::PopStyleVar();
	
	ImGui::EndGroup();
}


void AxisMapping::angularControlGui(){
	float displayDiameter = ImGui::GetTextLineHeight() * 8.0;
	
	
	ImGui::InvisibleButton("rotatingAxisDisplay", glm::vec2(displayDiameter));
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
	
	
	if(!isAxisConnected() || getAxis()->getState() == DeviceState::OFFLINE){
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
		double positionDegrees = Units::convert(animatablePosition->getActualPosition(), animatablePosition->getUnit(), Units::AngularDistance::Degree);
		
		int fullTurns = std::floor(positionDegrees / 360.0);
		double singleTurnPosition = positionDegrees - fullTurns * 360.0;
		char positiveAngleString[64];
		//snprintf(positiveAngleString, 64, "%iR+%.1f°", fullTurns, singleTurnPosition);
        snprintf(positiveAngleString, 64, "%.1f°", positionDegrees); //quick fix for starmania since they prefer seeing the values in raw degrees
		
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
	if(isAxisConnected()) actualEffort = getAxis()->getEffortActual();
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



void AxisMapping::setupGui(){
	
	
	if(ImGui::Button("Capture Zero")) captureUserZero();
	ImGui::SameLine();
	if(ImGui::Button("Reset##Zero")) resetUserZero();
	
	
	if(ImGui::Button("Capture Lower Limit")) captureLowerUserLimit();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
	if(ImGui::InputDouble("##LowerLimit", &userLowerPositionLimit, 0.0, 0.0, "%.3f")) setUserLowerLimit(userLowerPositionLimit);
	ImGui::SameLine();
	if(ImGui::Button("Reset##LowerLimit")) resetLowerUserLimit();
	
	if(ImGui::Button("Capture Upper Limit")) captureUpperUserLimit();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
	if(ImGui::InputDouble("##UpperLimit", &userUpperPositionLimit, 0.0, 0.0, "%.3f")) setUserUpperLimit(userUpperPositionLimit);
	ImGui::SameLine();
	if(ImGui::Button("Reset##UpperLimit")) resetUpperUserLimit();
	
		
	float rangeDisplayWidth = ImGui::GetTextLineHeight() * 20.0;
	float rangeDisplayHeight = ImGui::GetTextLineHeight() * 7.0;
	ImGui::InvisibleButton("rangedisplaysize", glm::vec2(rangeDisplayWidth, rangeDisplayHeight));
	
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = ImGui::GetItemRectSize();
	
	ImGui::PushClipRect(min, max, true);
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	drawing->AddRectFilled(min, max, ImColor(Colors::almostBlack));
	
	auto axis = getAxis();
	double actualPosition = animatablePosition->getActualPosition();
	Unit positionUnit = axis->getPositionUnit();
	
	auto toRangedPositionX = [&](double position) -> double {
		double normalized = (position - axisLowerPositionLimit) / (axisUpperPositionLimit - axisLowerPositionLimit);
		double clamped = std::clamp(normalized, 0.0, 1.0);
		return min.x + clamped * size.x;
	};
	
	glm::vec2 rangeRectMin(toRangedPositionX(userLowerPositionLimit), min.y);
	glm::vec2 rangeRectMax(toRangedPositionX(userUpperPositionLimit), max.y);
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
	
	snprintf(rangeString, 64, "Min position: %.3f%s", axisLowerPositionLimit, positionUnit->abbreviated);
	textAlignedBackground(rangeString,
						  glm::vec2(min.x, min.y),
						  TextAlignement::LEFT_TOP,
						  ImVec4(0.f, 0.f, 0.f, .7f),
						  displayFramePadding,
						  frameRounding,
						  ImDrawFlags_RoundCornersBottomRight);
	
	snprintf(rangeString, 64, "Max position: %.3f%s", axisUpperPositionLimit, positionUnit->abbreviated);
	textAlignedBackground(rangeString,
						  glm::vec2(max.x, max.y),
						  TextAlignement::RIGHT_BOTTOM,
						  ImVec4(0.f, 0.f, 0.f, .7f),
						  displayFramePadding,
						  frameRounding,
						  ImDrawFlags_RoundCornersTopLeft);

	
	snprintf(rangeString, 64, "Lower limit: %.3f%s", userLowerPositionLimit, positionUnit->abbreviated);
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
	
	snprintf(rangeString, 64, "Position: %.3f%s", actualPosition, positionUnit->abbreviated);
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
	
	snprintf(rangeString, 64, "Upper limit: %.3f%s", userUpperPositionLimit, positionUnit->abbreviated);
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
