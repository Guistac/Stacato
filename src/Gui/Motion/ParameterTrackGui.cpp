#include <pch.h>

#include "Motion/Manoeuvre/ParameterTrack.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Motion/Curve/Curve.h"
#include "Machine/AnimatableParameter.h"
#include "Machine/Machine.h"
#include "Motion/Manoeuvre/Manoeuvre.h"

#include "Gui/Utilities/CustomWidgets.h"


void ParameterTrack::baseTrackSheetRowGui(){

	bool b_showValidationErrorPopup = false;
	
	//[1] "Machine"
	ImGui::TableSetColumnIndex(1);
	
	auto machine = parameter->getMachine();
	glm::vec4 machineColor;
	glm::vec4 machineTextColor = Colors::white;
	if(machine->isEnabled()) machineColor = Colors::green;
	else if(machine->isReady()) machineColor = Colors::yellow;
	else {
		machineColor = Colors::red;
		machineTextColor = Colors::black;
	}
	if (!hasParentGroup()) backgroundText(parameter->getMachine()->getName(), machineColor, machineTextColor);
	
	//[2] "Parameter"
	ImGui::TableSetColumnIndex(2);
	backgroundText(parameter->getName(), b_valid ? Colors::darkGreen : Colors::red, b_valid ? Colors::white : Colors::white);
	if(ImGui::IsItemHovered() && !b_valid) validationErrorPopup();
}

void ParameterTrack::validationErrorPopup(){
	if(b_valid) return;
	
	ImGui::BeginTooltip();
	ImGui::Text("Track is not valid.");
	
	ImGui::Separator();
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
	if(!validationErrorString.empty()) ImGui::Text("%s", validationErrorString.c_str());
	ImGui::PopStyleColor();
	
	ImGui::EndTooltip();
}

void ParameterTrackGroup::trackSheetRowGui(){}

void KeyParameterTrack::trackSheetRowGui(){
	
	//[3] "Target"			//position or other
	ImGui::TableSetColumnIndex(3);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	target->gui();
	if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	if(ImGui::Button("Capture")) captureCurrentValueAsTarget();
	
}

void TargetParameterTrack::trackSheetRowGui(){
	
	//[3] "Interpolation"	//kinematic, linear, step, bezier
	ImGui::TableSetColumnIndex(3);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10.0);
	auto& compatibleTypes = getAnimatableParameter()->getCompatibleInterpolationTypes();
	interpolationType->combo(compatibleTypes.data(), compatibleTypes.size());
	 
	//[4] "Target"			//position or other
	ImGui::TableSetColumnIndex(4);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	target->gui();
	if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	if(ImGui::Button("Capture")) captureCurrentValueAsTarget();
	
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
	
	//[7] "Time Offset" 	//seconds
	ImGui::TableSetColumnIndex(7);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	timeOffset->gui();
	
	//[7] "Ramps"			//for kinematic or bezier
	ImGui::TableSetColumnIndex(8);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	inAcceleration->gui();
	if(!inAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	outAcceleration->gui();
	if(!outAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	
}

void SequenceParameterTrack::trackSheetRowGui(){
	
	//[3] "Interpolation"	//kinematic, linear, step, bezier
	ImGui::TableSetColumnIndex(3);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10.0);
	auto& compatibleTypes = getAnimatableParameter()->getCompatibleInterpolationTypes();
	interpolationType->combo(compatibleTypes.data(), compatibleTypes.size());
	
	//[4] "Start"
	ImGui::TableSetColumnIndex(4);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	start->gui();
	if(!start->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	if(ImGui::Button("Capture##Start")) captureCurrentValueAsStart();
	
	//[5] "End"
	ImGui::TableSetColumnIndex(5);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
	target->gui();
	if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	ImGui::SameLine();
	if(ImGui::Button("Capture##Target")) captureCurrentValueAsTarget();
	
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
















void KeyParameterTrack::drawCurveControls(){
	bool edited = false;
	
	auto animatable = getAnimatableParameter();
	int curveCount = animatable->getCurveCount();
	auto targetValue = animatable->getParameterValue(target);
	auto targetValues = getAnimatableParameter()->getCurvePositionsFromParameterValue(targetValue);
	
	for (int i = 0; i < curveCount; i++) {
		
		//TODO: TEMPORARY CURVE NAME ASSIGNEMENT
		const char* curveName;
		if(curveCount == 1) curveName = getParameter()->getName();
		else {
			if(i == 0) curveName = std::string(std::string(getParameter()->getName()) + ".x").c_str();
			else if(i == 1) curveName = std::string(std::string(getParameter()->getName()) + ".y").c_str();
			else curveName = std::string(std::string(getParameter()->getName()) + ".z").c_str();
		}
				
		if(ImPlot::DragLineY(curveName, &targetValues[i], true, Colors::gray, 4.0)) edited = true;
	}
	
	if(edited){
		validate();
	}
}





void TargetParameterTrack::drawCurves(){
	//only display when parameter track is playing
}
void TargetParameterTrack::drawCurveControls(){
	//display horizontal lines for targets
}



void SequenceParameterTrack::drawCurves(){
	double startTime, endTime;
	
	auto& curves = getCurves();
	int curveCount = curves.size();
	
	for (int i = 0; i < curveCount; i++) {
		auto& curve = curves[i];
		
		if(curve.getPoints().size() < 2) return;
		
		//TODO: TEMPORARY CURVE NAME ASSIGNEMENT
		const char* curveName;
		if(curveCount == 1) curveName = getParameter()->getName();
		else {
			if(i == 0) curveName = std::string(std::string(getParameter()->getName()) + ".x").c_str();
			else if(i == 1) curveName = std::string(std::string(getParameter()->getName()) + ".y").c_str();
			else curveName = std::string(std::string(getParameter()->getName()) + ".z").c_str();
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

void SequenceParameterTrack::drawCurveControls(){
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
			if(i == 0 && interpolationType->value == Motion::Interpolation::Type::STEP) continue;

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

