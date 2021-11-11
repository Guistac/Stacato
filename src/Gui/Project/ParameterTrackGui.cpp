#include <pch.h>

#include "Motion/Manoeuvre/ParameterTrack.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Motion/Curve/Curve.h"
#include "Motion/AnimatableParameter.h"

bool ParameterTrack::interpolationTypeSelectorGui() {
	bool valueChanged = false;
	auto compatibleInterpolations = parameter->getCompatibleInterpolationTypes();
	bool disableCombo = compatibleInterpolations.size() < 2;
	if(disableCombo) BEGIN_DISABLE_IMGUI_ELEMENT
	if (ImGui::BeginCombo("##InterpolationSelector", getInterpolationType(interpolationType)->displayName)) {
		for (auto& it : compatibleInterpolations) {
			if(it == InterpolationType::Type::BEZIER) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::Selectable(getInterpolationType(it)->displayName, it == interpolationType)) {
				setInterpolationType(it);
				valueChanged = true;
			}
			if(it == InterpolationType::Type::BEZIER) END_DISABLE_IMGUI_ELEMENT
		}
		ImGui::EndCombo();
	}
	if (disableCombo) END_DISABLE_IMGUI_ELEMENT
	return valueChanged;
}

bool ParameterTrack::sequenceTypeSelectorGui() {
	bool valueChanged = false;
	if (ImGui::BeginCombo("##SequenceTypeSelector", getSequenceType(sequenceType)->displayName)) {
		for (auto& st : getSequenceTypes()) {
			if(st.type == SequenceType::Type::ANIMATED_MOVE) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::Selectable(st.displayName, st.type == sequenceType)) {
				setSequenceType(st.type);
				valueChanged = true;
			}
			if(st.type == SequenceType::Type::ANIMATED_MOVE) END_DISABLE_IMGUI_ELEMENT
		}
		ImGui::EndCombo();
	}
	return valueChanged;
}


bool ParameterTrack::originIsPreviousTargetCheckboxGui() {
	return ImGui::Checkbox("##ChainPrevious", &originIsPreviousTarget);
}

bool ParameterTrack::targetIsNextOriginCheckboxGui() {
	return ImGui::Checkbox("##ChainNext", &targetIsNextOrigin);
}


bool ParameterTrack::originInputGui() {
	bool valueChanged = false;
	if (originIsPreviousTarget) BEGIN_DISABLE_IMGUI_ELEMENT
	if (sequenceType == SequenceType::Type::ANIMATED_MOVE) BEGIN_DISABLE_IMGUI_ELEMENT
	ImGui::PushID("Origin");
	valueChanged = origin.inputFieldGui(ImGui::GetTextLineHeight() * 5.0);
	ImGui::PopID();
	if (sequenceType == SequenceType::Type::ANIMATED_MOVE) END_DISABLE_IMGUI_ELEMENT
	if (originIsPreviousTarget) END_DISABLE_IMGUI_ELEMENT
	return valueChanged;
}

bool ParameterTrack::targetInputGui() {
	bool valueChanged = false;
	if (sequenceType == SequenceType::Type::ANIMATED_MOVE) BEGIN_DISABLE_IMGUI_ELEMENT
	ImGui::PushID("Target");
	valueChanged = target.inputFieldGui(ImGui::GetTextLineHeight() * 5.0);
	ImGui::PopID();
	if (sequenceType == SequenceType::Type::ANIMATED_MOVE) END_DISABLE_IMGUI_ELEMENT
	return valueChanged;
}

bool ParameterTrack::timeInputGui() {
	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			valueChanged = ImGui::InputDouble("##constraint", &movementTime, 0.0, 0.0, "%.3f s");
			break;
		default:
			break;
	}
	return valueChanged;
}

bool ParameterTrack::timeOffsetInputGui() {
	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::ANIMATED_MOVE:
			break;
		default:
			switch (interpolationType) {
				case InterpolationType::Type::STEP:
					break;
				default:
					valueChanged = ImGui::InputDouble("##timeOffset", &timeOffset, 0.0, 0.0, "%.3f s");
					break;
			}
			break;
	}
	return valueChanged;
}


bool ParameterTrack::rampIntInputGui() {
	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			switch (interpolationType) {
				case InterpolationType::Type::BEZIER:
				case InterpolationType::Type::TRAPEZOIDAL:
					valueChanged = ImGui::InputDouble("##rampIn", &rampIn, 0.0, 0.0, "%.3f u");
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return valueChanged;
}

bool ParameterTrack::rampOutInputGui() {
	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			switch (interpolationType) {
				case InterpolationType::Type::BEZIER:
					break;
				case InterpolationType::Type::TRAPEZOIDAL:
					if (rampsAreEqual) BEGIN_DISABLE_IMGUI_ELEMENT
					valueChanged = ImGui::InputDouble("##rampOut", &rampOut, 0.0, 0.0, "%.3f u");
					if (rampsAreEqual) END_DISABLE_IMGUI_ELEMENT
					break;
				default:
					break;
			}
			break;
	default:
		break;
	}
	return valueChanged;
}

bool ParameterTrack::equalRampsCheckboxGui() {
	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			switch (interpolationType) {
				case InterpolationType::Type::BEZIER:
				case InterpolationType::Type::TRAPEZOIDAL:
					valueChanged = ImGui::Checkbox("##rampEqual", &rampsAreEqual);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return valueChanged;
}


void ParameterTrack::drawCurves(double startTime, double endTime) {
	for (auto& curve : curves) {
		auto& startPoint = curve->getStart();
		if (startPoint->time > startTime) {
			std::vector<glm::vec2> headerPoints;
			headerPoints.push_back(glm::vec2(startTime, startPoint->position));
			headerPoints.push_back(glm::vec2(startPoint->time, startPoint->position));
			ImPlot::PlotLine(curve->name, &headerPoints.front().x, &headerPoints.front().y, headerPoints.size(), 0, sizeof(glm::vec2));
		}
		for (auto& interpolation : curve->interpolations) {
			if (interpolation->isDefined) {
				std::vector<Motion::CurvePoint>& points = interpolation->displayPoints;
				ImPlot::PlotLine(curve->name, &points.front().time, &points.front().position, points.size(), 0, sizeof(Motion::CurvePoint));
			}
			else {
				std::vector<Motion::CurvePoint> errorPoints;
				errorPoints.push_back(*interpolation->inPoint);
				errorPoints.push_back(*interpolation->outPoint);
				ImPlot::SetNextLineStyle(Colors::red, ImGui::GetTextLineHeight() * 0.2);
				ImPlot::PlotLine(curve->name, &errorPoints.front().time, &errorPoints.front().position, errorPoints.size(), 0, sizeof(Motion::CurvePoint));
			}
		}
		auto& endPoint = curve->getEnd();
		if (endPoint->time < endTime) {
			std::vector<glm::vec2> trailerPoints;
			trailerPoints.push_back(glm::vec2(endPoint->time, endPoint->position));
			trailerPoints.push_back(glm::vec2(endTime, endPoint->position));
			ImPlot::PlotLine(curve->name, &trailerPoints.front().x, &trailerPoints.front().y, trailerPoints.size(), 0, sizeof(glm::vec2));
		}
	}
}

bool ParameterTrack::drawControlPoints() {
	bool pointEdited = false;
	for (auto& curve : curves) {
		for (int i = 0; i < curve->getPoints().size(); i++) {
			//don't draw the first control point of a step interpolation sequence, since we can't edit it anyway
			if (sequenceType == SequenceType::Type::TIMED_MOVE && interpolationType == InterpolationType::Type::STEP && i == 0) continue;
			auto& controlPoint = curve->getPoints()[i];
			if (ImPlot::DragPoint(controlPoint->name, &controlPoint->time, &controlPoint->position, true, Colors::white, 10.0)) {
				refreshAfterCurveEdit();
				pointEdited = true;
			}
		}
	}
	return pointEdited;
}