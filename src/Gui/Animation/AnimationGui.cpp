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

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"

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
	if(Stacato::Editor::getCurrentProject()->isPlotEditLocked()){
		if(ImGui::BeginTable("##TrackParameters", 5, tableFlags)){
			ImGui::TableSetupColumn("Playback");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Current");
			ImGui::TableSetupColumn("Target");
			return true;
		}
	}
	else{
		if(ImGui::BeginTable("##TrackParameters", 6, tableFlags)){
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Playback");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Current");
			ImGui::TableSetupColumn("Target");
			return true;
		}
	}
	return false;
}

bool TargetAnimation::beginTrackSheetTable(ImGuiTableFlags tableFlags){
	
	if(Stacato::Editor::getCurrentProject()->isPlotEditLocked()){
		if(ImGui::BeginTable("##TrackParameters", 7, tableFlags)){
			ImGui::TableSetupColumn("Playback");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Current");
			ImGui::TableSetupColumn("Target");			//position or other
			ImGui::TableSetupColumn("Constraint");		//time or velocity
			ImGui::TableSetupColumn("Ramps");			//for kinematic or bezier
			return true;
		}
	}else{
		if(ImGui::BeginTable("##TrackParameters", 9, tableFlags)){
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Playback");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Current");
			//ImGui::TableSetupColumn("Interpolation");	//kinematic, linear, step, bezier
			ImGui::TableSetupColumn("Target");			//position or other
			ImGui::TableSetupColumn("Using");			//time vs velocity
			ImGui::TableSetupColumn("Constraint");		//time or velocity
			ImGui::TableSetupColumn("Ramps");			//for kinematic or bezier
			return true;
		}
	}
	

	return false;
}

bool SequenceAnimation::beginTrackSheetTable(ImGuiTableFlags tableFlags){
	if(Stacato::Editor::getCurrentProject()->isPlotEditLocked()){
		if(ImGui::BeginTable("##TrackParameters", 8, tableFlags)){
			ImGui::TableSetupColumn("Playback");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Current");
			ImGui::TableSetupColumn("Start");		//sequencer start
			ImGui::TableSetupColumn("End");			//sequencer end
			ImGui::TableSetupColumn("Duration");
			ImGui::TableSetupColumn("Ramps");
			return true;
		}
	}else{
		if(ImGui::BeginTable("##TrackParameters", 10, tableFlags)){
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Playback");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Current");
			//ImGui::TableSetupColumn("Interpolation");
			ImGui::TableSetupColumn("Start");		//sequencer start
			ImGui::TableSetupColumn("End");			//sequencer end
			ImGui::TableSetupColumn("Duration");
			ImGui::TableSetupColumn("Time Offset");
			ImGui::TableSetupColumn("Ramps");
			return true;
		}
	}
	return false;
}

void Animation::baseTrackSheetRowGui(){
	
	if(Stacato::Editor::getCurrentProject()->isPlotEditLocked()){
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
		
		//[3] "Current"
		ImGui::TableSetColumnIndex(3);
		backgroundText(getAnimatable()->getTargetValueString().c_str(),
					   glm::vec2(ImGui::GetTextLineHeight() * 4.0, ImGui::GetFrameHeight()),
					   Colors::black, Colors::white);
		
	}else{
		bool b_showValidationErrorPopup = false;
			
		//[2] "Machine"
		ImGui::TableSetColumnIndex(2);
		
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
		
		//[3] "Parameter"
		ImGui::TableSetColumnIndex(3);
		backgroundText(animatable->getName(), b_valid ? Colors::darkGreen : Colors::red, b_valid ? Colors::white : Colors::white);
		if(ImGui::IsItemHovered() && !b_valid) validationErrorPopup();
		
		//[4] "Current"
		ImGui::TableSetColumnIndex(4);
		backgroundText(getAnimatable()->getTargetValueString().c_str(),
					   glm::vec2(ImGui::GetTextLineHeight() * 4.0, ImGui::GetFrameHeight()),
					   Colors::black, Colors::white);
	}
}

void AnimationKey::playbackGui(){
	ImGui::BeginDisabled(!canRapidToPlaybackPosition());
	bool atKey = isAtPlaybackPosition();
	if(atKey) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
	if(buttonArrowDownStop("goToPos", ImGui::GetFrameHeight())) rapidToPlaybackPosition();
	if(atKey) ImGui::PopStyleColor();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!canStop());
	if(buttonStop("StopPlayback")) stop();
	ImGui::EndDisabled();
}

void TargetAnimation::playbackGui(){
	ImGui::BeginDisabled(!canRapidToTarget());
	bool atTarget = isAtTarget();
	if(atTarget) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
	if(buttonArrowRightStop("goToTarget")) rapidToTarget();
	if(atTarget) ImGui::PopStyleColor();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!canStartPlayback());
	if(buttonPlay("StartPlayback")) startPlayback();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!canStop());
	if(buttonStop("StopPlayback")) stop();
	ImGui::EndDisabled();
}

void SequenceAnimation::playbackGui(){
	ImGui::BeginDisabled(!canRapidToStart());
	bool atStart = isAtStart();
	if(atStart) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
	if(buttonArrowLeftStop("goToStart")) rapidToStart();
	if(atStart) ImGui::PopStyleColor();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!canRapidToTarget());
	bool atTarget = isAtTarget();
	if(atTarget) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
	if(buttonArrowRightStop("goToTarget")) rapidToTarget();
	if(atTarget) ImGui::PopStyleColor();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if(isPlaying()){
		ImGui::BeginDisabled(!canPausePlayback());
		if(buttonPause("PausePlayback")) {
			pausePlayback();
		}
		ImGui::EndDisabled();
	}
	else{
		ImGui::BeginDisabled(!canStartPlayback());
		if(buttonPlay("StartPlayback")) {
			startPlayback();
		}
		ImGui::EndDisabled();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!canStop());
	if(buttonStop("StopPlayback")) stop();
	ImGui::EndDisabled();
}

void AnimationKey::trackSheetRowGui(){
	
	if(Stacato::Editor::getCurrentProject()->isPlotEditLocked()){
		
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		
		//[4] "Target"			//position or other
		ImGui::TableSetColumnIndex(4);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		target->gui();
		if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		
		ImGui::PopItemFlag();
		
	}else{
		//[5] "Target"			//position or other
		ImGui::TableSetColumnIndex(5);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		target->gui();
		if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		ImGui::SameLine();
		if(ImGui::Button("Capture")) captureTarget();
	}
	
}

void TargetAnimation::trackSheetRowGui(){
	
	
	if(Stacato::Editor::getCurrentProject()->isPlotEditLocked()){
		
		
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		
		//[4] "Target"			//position or other
		ImGui::TableSetColumnIndex(4);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		target->gui();
		if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		
		//[5] "Constraint"			//time vs velocity
		ImGui::TableSetColumnIndex(5);
		
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 7.0);
		if(getConstraintType() == Constraint::TIME) timeConstraint->gui();
		else if(getConstraintType() == Constraint::VELOCITY) {
			velocityConstraint->gui();
			if(!velocityConstraint->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		}
		
		//[6] "Ramps"			//for kinematic or bezier
		ImGui::TableSetColumnIndex(6);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		inAcceleration->gui();
		if(!inAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		outAcceleration->gui();
		if(!outAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		
		ImGui::PopItemFlag();
		
	}else{
	
		/*
		//[4] "Interpolation"	//kinematic, linear, step, bezier
		ImGui::TableSetColumnIndex(4);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10.0);
		auto& compatibleTypes = getAnimatable()->getCompatibleInterpolationTypes();
		interpolationType->combo(compatibleTypes.data(), compatibleTypes.size());
		 */
		
		//[5] "Target"			//position or other
		ImGui::TableSetColumnIndex(5);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		target->gui();
		if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		ImGui::SameLine();
		if(ImGui::Button("Capture")) captureTarget();
		
		//[6] "Using"			//time vs velocity
		ImGui::TableSetColumnIndex(6);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		constraintType->gui();

		//[7] "Constraint"		//time or velocity
		ImGui::TableSetColumnIndex(7);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 7.0);
		if(getConstraintType() == Constraint::TIME) {
			timeConstraint->gui();
		}
		else if(getConstraintType() == Constraint::VELOCITY) {
			velocityConstraint->gui();
			if(!velocityConstraint->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		}
		
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
}

void SequenceAnimation::trackSheetRowGui(){
	
	if(Stacato::Editor::getCurrentProject()->isPlotEditLocked()){
		
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		
		//[4] "Start"
		ImGui::TableSetColumnIndex(4);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		start->gui();
		if(!start->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		
		//[5] "End"
		ImGui::TableSetColumnIndex(5);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		target->gui();
		if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		
		//[6] "Duration"
		ImGui::TableSetColumnIndex(6);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		duration->gui();
		
		//[7] "Ramps"			//for kinematic or bezier
		ImGui::TableSetColumnIndex(7);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		inAcceleration->gui();
		if(!inAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		outAcceleration->gui();
		if(!outAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		
		ImGui::PopItemFlag();
		
	}else{
		/*
		//[4] "Interpolation"	//kinematic, linear, step, bezier
		ImGui::TableSetColumnIndex(4);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10.0);
		auto& compatibleTypes = getAnimatable()->getCompatibleInterpolationTypes();
		interpolationType->combo(compatibleTypes.data(), compatibleTypes.size());
		*/
		
		//[5] "Start"
		ImGui::TableSetColumnIndex(5);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		start->gui();
		if(!start->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		ImGui::SameLine();
		if(ImGui::Button("Capture##Start")) captureStart();
		
		//[6] "End"
		ImGui::TableSetColumnIndex(6);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		target->gui();
		if(!target->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		ImGui::SameLine();
		if(ImGui::Button("Capture##Target")) captureTarget();
		
		//[7] "Duration"
		ImGui::TableSetColumnIndex(7);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		duration->gui();
		ImGui::SameLine();
		if(ImGui::Button("Min")){
			if(!isComplex()){
				duration->overwrite(theoreticalShortestDuration);
				duration->onEdit();
			}
		}
		if(ImGui::IsItemHovered()){
			ImGui::BeginTooltip();
			std::string minTimeString = TimeStringConversion::secondsToTimecodeString(theoreticalShortestDuration);
			ImGui::Text("Theoretical minimum time: %s", minTimeString.c_str());
			if(isComplex()){
				ImGui::TextColored(Colors::red, "Cannot set minimum duration :");
				ImGui::TextColored(Colors::red, "The sequence has additional control points.");
			}
			ImGui::EndTooltip();
		}
		
		//[8] "Time Offset"
		ImGui::TableSetColumnIndex(8);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		timeOffset->gui();
		
		//[9] "Ramps"			//for kinematic or bezier
		ImGui::TableSetColumnIndex(9);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		inAcceleration->gui();
		if(!inAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
		outAcceleration->gui();
		if(!outAcceleration->isValid() && ImGui::IsItemHovered()) validationErrorPopup();
	}
	
	
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
	
	if(edited) validate();
}






void TargetAnimation::drawCurveControls(){
	
}



void Animation::drawCurves(){
	double startTime, endTime;
	
	auto& curves = getCurves();
	int curveCount = curves.size();
	
	for (int i = 0; i < curveCount; i++) {
		auto& curve = curves[i];
		
		if(!curve->b_visibleInEditor) continue;
		if(curve->getPoints().size() < 2) continue;
		
		bool b_selected = getManoeuvre()->isCurveSelectedInEditor(curve);
		
		//TODO: TEMPORARY CURVE NAME ASSIGNEMENT
		const char* curveName;
		if(curveCount == 1) curveName = getAnimatable()->getName();
		else {
			if(i == 0) curveName = std::string(std::string(getAnimatable()->getName()) + ".x").c_str();
			else if(i == 1) curveName = std::string(std::string(getAnimatable()->getName()) + ".y").c_str();
			else curveName = std::string(std::string(getAnimatable()->getName()) + ".z").c_str();
		}
		
		auto startPoint = curve->getStart();
		if (startPoint->time > startTime) {
			std::vector<glm::vec2> headerPoints;
			headerPoints.push_back(glm::vec2(startTime, startPoint->position));
			headerPoints.push_back(glm::vec2(startPoint->time, startPoint->position));
			ImPlot::PlotLine(curveName, &headerPoints.front().x, &headerPoints.front().y, headerPoints.size(), 0, sizeof(glm::vec2));
		}
		
		
		for (auto& interpolation : curve->getInterpolations()) {
			if (interpolation->b_valid) {
				std::vector<Motion::Point>& points = interpolation->displayPoints;
				if(b_selected) ImPlot::SetNextLineStyle(ImColor(Colors::white), ImGui::GetTextLineHeight() * .15f);
				ImPlot::PlotLine(curveName, &points.front().time, &points.front().position, points.size(), 0, sizeof(Motion::Point));
				std::vector<Motion::Point>& inflectionPoints = interpolation->displayInflectionPoints;
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Cross);
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
		
		auto endPoint = curve->getEnd();
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

	auto& curves = getCurves();
	int curveCount = curves.size();
		
	for (int c = 0; c < curveCount; c++) {
		auto& curve = curves[c];
		if(!curve->b_visibleInEditor) continue;
		auto& controlPoints = curve->getPoints();
		int controlPointCount = controlPoints.size();
		
		ImGui::PushID(c);
		
		for (int i = 0; i < controlPointCount; i++) {
			
			//don't allow editing of the first and last points in the curve editor
			if(i == 0 || i == controlPointCount - 1) continue;
			//don't draw the first control point of a step interpolation sequence, since we can't edit it anyway
			if(i == 0 && interpolationType->value == InterpolationType::STEP) continue;
			
			auto& controlPoint = controlPoints[i];
			ImGui::PushID(controlPoint->id);
			
			ImColor controlPointColor;
			if(!controlPoint->b_valid && controlPoint->b_selected) controlPointColor = ImColor(Colors::orange);
			else if(!controlPoint->b_valid) controlPointColor = ImColor(Colors::red);
			else if(controlPoint->b_selected) controlPointColor = ImColor(Colors::yellow);
			else controlPointColor = ImColor(Colors::white);
			float controlPointSize = ImGui::GetTextLineHeight() * .25f;
			
			bool controlPointEdited = ImPlot::DragPoint("", &controlPoint->time, &controlPoint->position, true, controlPointColor, controlPointSize);
			if(controlPointEdited) edited = true;
			
			//limit the movement of control points to the range of the first and last points
			if(controlPointEdited){
				if(controlPoint->time >= curve->getEnd()->time) controlPoint->time = curve->getEnd()->time - 0.001;
				if(controlPoint->time <= curve->getStart()->time) controlPoint->time = curve->getStart()->time + 0.001;
			}
			
			if(ImGui::IsItemClicked()) getManoeuvre()->selectControlPoint(controlPoint);

			if (!controlPoint->b_valid) {
				ImPlot::Annotate(controlPoint->time, controlPoint->position, glm::vec2(30, -30), glm::vec4(0.5, 0.0, 0.0, 0.5), "%s", Enumerator::getDisplayString(controlPoint->validationError));
			}
			
			ImGui::PopID();
		}

		ImGui::PopID();
	}
	 
	if(edited) updateAfterCurveEdit();
}

