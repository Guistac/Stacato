#include <pch.h>

#include "Animation/Animation.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Motion/Curve/Curve.h"
#include "Animation/Animatable.h"
#include "Machine/Machine.h"
#include "Animation/Manoeuvre.h"

#include "Gui/Utilities/CustomWidgets.h"

bool Animation::beginTrackSheetTable(ManoeuvreType type, ImGuiTableFlags tableFlags){
	switch(type){
		case ManoeuvreType::KEY:
			return AnimationKey::beginTrackSheetTable(tableFlags);
		case ManoeuvreType::TARGET:
			return TargetAnimation::beginTrackSheetTable(tableFlags);
		case ManoeuvreType::SEQUENCE:
			return SequenceAnimation::beginTrackSheetTable(tableFlags);
	}
}

bool AnimationKey::beginTrackSheetTable(ImGuiTableFlags tableFlags){
	if(ImGui::BeginTable("##TrackParameters", 4, tableFlags)){
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Machine");
		ImGui::TableSetupColumn("Parameter");
		ImGui::TableSetupColumn("Target");
		return true;
	}
	return false;
}

bool TargetAnimation::beginTrackSheetTable(ImGuiTableFlags tableFlags){
	if(ImGui::BeginTable("##TrackParameters", 8, tableFlags)){
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Machine");
		ImGui::TableSetupColumn("Parameter");
		ImGui::TableSetupColumn("Interpolation");	//kinematic, linear, step, bezier
		ImGui::TableSetupColumn("Target");			//position or other
		ImGui::TableSetupColumn("Using");			//time vs velocity
		ImGui::TableSetupColumn("Constraint");		//time or velocity
		ImGui::TableSetupColumn("Ramps");			//for kinematic or bezier
		return true;
	}
	return false;
}

bool SequenceAnimation::beginTrackSheetTable(ImGuiTableFlags tableFlags){
	if(ImGui::BeginTable("##TrackParameters", 9, tableFlags)){
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Machine");
		ImGui::TableSetupColumn("Parameter");
		ImGui::TableSetupColumn("Interpolation");
		ImGui::TableSetupColumn("Start");		//sequencer start
		ImGui::TableSetupColumn("End");			//sequencer end
		ImGui::TableSetupColumn("Duration");
		ImGui::TableSetupColumn("Time Offset");
		ImGui::TableSetupColumn("Ramps");
		return true;
	}
	return false;
}

void Animation::baseTrackSheetRowGui(){

	bool b_showValidationErrorPopup = false;
	
	//[1] "Machine"
	ImGui::TableSetColumnIndex(1);
	
	auto machine = animatable->getMachine();
	glm::vec4 machineColor;
	glm::vec4 machineTextColor = Colors::white;
	if(machine->isEnabled()) machineColor = Colors::green;
	else if(machine->isReady()) machineColor = Colors::yellow;
	else {
		machineColor = Colors::red;
		machineTextColor = Colors::black;
	}
	if (!hasParentComposite()) backgroundText(animatable->getMachine()->getName(), machineColor, machineTextColor);
	
	//[2] "Parameter"
	ImGui::TableSetColumnIndex(2);
	backgroundText(animatable->getName(), b_valid ? Colors::darkGreen : Colors::red, b_valid ? Colors::white : Colors::white);
	if(ImGui::IsItemHovered() && !b_valid) validationErrorPopup();
}

void AnimationKey::trackSheetRowGui(){
	
	//[3] "Target"			//position or other
	ImGui::TableSetColumnIndex(3);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	target->gui();
	if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	if(ImGui::Button("Capture")) captureTarget();
	
}

void TargetAnimation::trackSheetRowGui(){
	
	//[3] "Interpolation"	//kinematic, linear, step, bezier
	ImGui::TableSetColumnIndex(3);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10.0);
	auto& compatibleTypes = getAnimatable()->getCompatibleInterpolationTypes();
	interpolationType->combo(compatibleTypes.data(), compatibleTypes.size());
	 
	//[4] "Target"			//position or other
	ImGui::TableSetColumnIndex(4);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	target->gui();
	if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	if(ImGui::Button("Capture")) captureTarget();
	
	//[5] "Using"			//time vs velocity
	ImGui::TableSetColumnIndex(5);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	constraintType->gui();

	//[6] "Constraint"		//time or velocity
	ImGui::TableSetColumnIndex(6);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 7.0);
	if(getConstraintType() == Constraint::TIME) {
		timeConstraint->gui();
	}
	else if(getConstraintType() == Constraint::VELOCITY) {
		velocityConstraint->gui();
		if(!velocityConstraint->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	}
	
	//[7] "Ramps"			//for kinematic or bezier
	ImGui::TableSetColumnIndex(7);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	inAcceleration->gui();
	if(!inAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	outAcceleration->gui();
	if(!outAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	
}

void SequenceAnimation::trackSheetRowGui(){
	
	//[3] "Interpolation"	//kinematic, linear, step, bezier
	ImGui::TableSetColumnIndex(3);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10.0);
	auto& compatibleTypes = getAnimatable()->getCompatibleInterpolationTypes();
	interpolationType->combo(compatibleTypes.data(), compatibleTypes.size());
	
	//[4] "Start"
	ImGui::TableSetColumnIndex(4);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	start->gui();
	if(!start->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	if(ImGui::Button("Capture##Start")) captureStart();
	
	//[5] "End"
	ImGui::TableSetColumnIndex(5);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	target->gui();
	if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	if(ImGui::Button("Capture##Target")) captureTarget();
	
	//[6] "Duration"
	ImGui::TableSetColumnIndex(6);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	duration->gui();
	
	//[7] "Time Offset"
	ImGui::TableSetColumnIndex(7);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	timeOffset->gui();
	
	//[8] "Ramps"			//for kinematic or bezier
	ImGui::TableSetColumnIndex(8);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	inAcceleration->gui();
	if(!inAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	outAcceleration->gui();
	if(!outAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
}

void Animation::validationErrorPopup(){
	if(b_valid) return;
	
	ImGui::BeginTooltip();
	ImGui::Text("Track is not valid.");
	
	ImGui::Separator();
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
	if(!validationErrorString.empty()) ImGui::Text("%s", validationErrorString.c_str());
	ImGui::PopStyleColor();
	
	ImGui::EndTooltip();
}













void AnimationKey::drawCurveControls(){
	bool edited = false;
	
	auto animatable = getAnimatable();
	int curveCount = animatable->getCurveCount();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	auto targetValues = animatable->getCurvePositionsFromAnimationValue(targetValue);
	
	for (int i = 0; i < curveCount; i++) {
		
		//TODO: TEMPORARY CURVE NAME ASSIGNEMENT
		const char* curveName;
		if(curveCount == 1) curveName = animatable->getName();
		else {
			if(i == 0) curveName = std::string(std::string(animatable->getName()) + ".x").c_str();
			else if(i == 1) curveName = std::string(std::string(animatable->getName()) + ".y").c_str();
			else curveName = std::string(std::string(animatable->getName()) + ".z").c_str();
		}
				
		if(ImPlot::DragLineY(curveName, &targetValues[i], true, Colors::gray, 4.0)) edited = true;
	}
	
	if(edited){
		validate();
	}
}






void TargetAnimation::drawCurveControls(){
	
}



void Animation::drawCurves(){
	double startTime, endTime;
	
	auto& curves = getCurves();
	int curveCount = curves.size();
	
	for (int i = 0; i < curveCount; i++) {
		auto& curve = curves[i];
		
		if(curve.getPoints().size() < 2) return;
		
		//TODO: TEMPORARY CURVE NAME ASSIGNEMENT
		const char* curveName;
		if(curveCount == 1) curveName = getAnimatable()->getName();
		else {
			if(i == 0) curveName = std::string(std::string(getAnimatable()->getName()) + ".x").c_str();
			else if(i == 1) curveName = std::string(std::string(getAnimatable()->getName()) + ".y").c_str();
			else curveName = std::string(std::string(getAnimatable()->getName()) + ".z").c_str();
		}
		
		auto startPoint = curve.getStart();
		if (startPoint->time > startTime) {
			std::vector<glm::vec2> headerPoints;
			headerPoints.push_back(glm::vec2(startTime, startPoint->position));
			headerPoints.push_back(glm::vec2(startPoint->time, startPoint->position));
			ImPlot::PlotLine(curveName, &headerPoints.front().x, &headerPoints.front().y, headerPoints.size(), 0, sizeof(glm::vec2));
		}
		
		
		for (auto& interpolation : curve.getInterpolations()) {
			if (interpolation->b_valid) {
				std::vector<Motion::Point>& points = interpolation->displayPoints;
				ImPlot::PlotLine(curveName, &points.front().time, &points.front().position, points.size(), 0, sizeof(Motion::Point));
				std::vector<Motion::Point>& inflectionPoints = interpolation->displayInflectionPoints;
				if(!inflectionPoints.empty())
					ImPlot::PlotScatter("##inflectionPoints", &inflectionPoints.front().time, &inflectionPoints.front().position, inflectionPoints.size(), 0, sizeof(Motion::Point));
			}
			else {
				std::vector<Motion::Point> errorPoints;
				errorPoints.push_back(interpolation->inPoint->toPoint());
				errorPoints.push_back(interpolation->outPoint->toPoint());
				ImPlot::SetNextLineStyle(Colors::red, ImGui::GetTextLineHeight() * 0.2);
				ImPlot::PlotLine(curveName, &errorPoints.front().time, &errorPoints.front().position, errorPoints.size(), 0, sizeof(Motion::Point));
				glm::vec2 averagePosition = glm::vec2(interpolation->inPoint->time, interpolation->inPoint->position) + glm::vec2(interpolation->outPoint->time, interpolation->outPoint->position);
				averagePosition /= 2.0;
				ImGui::PushFont(Fonts::sansBold15);
				ImPlot::Annotate(averagePosition.x, averagePosition.y, glm::vec2(0, 0), glm::vec4(0.5, 0.0, 0.0, 0.5), "%s", Enumerator::getDisplayString(interpolation->validationError));
				
				ImGui::PopFont();
			}
		}
		
		auto endPoint = curve.getEnd();
		if (endPoint->time < endTime) {
			std::vector<glm::vec2> trailerPoints;
			trailerPoints.push_back(glm::vec2(endPoint->time, endPoint->position));
			trailerPoints.push_back(glm::vec2(endTime, endPoint->position));
			ImPlot::PlotLine(curveName, &trailerPoints.front().x, &trailerPoints.front().y, trailerPoints.size(), 0, sizeof(glm::vec2));
		}
	}
}

void SequenceAnimation::drawCurveControls(){
	bool edited = false;
	
	static float controlPointLarge = ImGui::GetTextLineHeight() * 0.5;
	static float controlPointMedium = ImGui::GetTextLineHeight() * 0.3;

	auto& curves = getCurves();
	int curveCount = curves.size();
		
	for (int c = 0; c < curveCount; c++) {

		auto& curve = curves[c];
		auto& controlPoints = curve.getPoints();
		int controlPointCount = controlPoints.size();
		
		for (int i = 0; i < controlPointCount; i++) {
			auto& controlPoint = controlPoints[i];
			bool controlPointEdited = false;
			
			//don't draw the first control point of a step interpolation sequence, since we can't edit it anyway
			if(i == 0 && interpolationType->value == InterpolationType::STEP) continue;

			ImGui::BeginDisabled();
			if (controlPoint->b_valid) controlPointEdited = ImPlot::DragPoint("", &controlPoint->time, &controlPoint->position, true, Colors::white, controlPointMedium);
			else controlPointEdited = ImPlot::DragPoint("", &controlPoint->time, &controlPoint->position, true, Colors::red, controlPointLarge);
			ImGui::EndDisabled();

			if (!controlPoint->b_valid) {
				ImPlot::Annotate(controlPoint->time, controlPoint->position, glm::vec2(30, -30), glm::vec4(0.5, 0.0, 0.0, 0.5), "%s", Enumerator::getDisplayString(controlPoint->validationError));
			}

			
			/*
			if (!pointIsChained && (controlPointEdited || ImGui::IsItemHovered())) {
				double lowLimit, highLimit;
				if (parameter->machine->getCurveLimitsAtTime(parameter, curves, controlPoint->time, curve, lowLimit, highLimit)) {

					//draw manoeuvre bounds
					glm::vec2 plotBoundsMin(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().Y.Max);
					glm::vec2 plotBoundsMax(ImPlot::GetPlotLimits().X.Max, ImPlot::GetPlotLimits().Y.Min);

					glm::vec2 lowLimitPoints[2] = {
						glm::vec2(plotBoundsMin.x, lowLimit),
						glm::vec2(plotBoundsMax.x, lowLimit)
					};
					glm::vec2 highLimitPoints[2] = {
						glm::vec2(plotBoundsMin.x, highLimit),
						glm::vec2(plotBoundsMax.x, highLimit)
					};

					ImPlot::SetNextFillStyle(Colors::black, 0.4);
					ImPlot::PlotShaded("##limitLowShaded", &lowLimitPoints[0].x, &lowLimitPoints[0].y, 2, -INFINITY, 0, sizeof(glm::vec2));
					ImPlot::SetNextFillStyle(Colors::black, 0.4);
					ImPlot::PlotShaded("##limitHighShaded", &highLimitPoints[0].x, &highLimitPoints[0].y, 2, INFINITY, 0, sizeof(glm::vec2));
					ImPlot::SetNextLineStyle(Colors::black);
			 
					ImPlot::PlotHLines("##positionLimits", &lowLimit, 1, 0, sizeof(double));
					ImPlot::SetNextLineStyle(Colors::black);
					ImPlot::PlotHLines("##positionLimits", &highLimit, 1, 0, sizeof(double));

					if (controlPointEdited) {
						if (controlPoint->position < lowLimit) controlPoint->position = lowLimit;
						else if (controlPoint->position > highLimit) controlPoint->position = highLimit;
					}
				}
			}
			 */

			/*
			if (controlPointEdited) {
				refreshAfterCurveEdit();
				edited = true;
			}
			 */
		}

	}
	 
	if(edited) updateAfterCurveEdit();
}

