#include <pch.h>

#include "Motion/ParameterTrack.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Framework/Colors.h"

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


bool ParameterTrack::chainPreviousTargetCheckboxGui() {
	return ImGui::Checkbox("##Chain", &originIsPreviousTarget);
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


void ParameterTrack::drawCurves() {
	for (auto& curve : curves) {
		for (auto& interpolation : curve->interpolations) {
			if (interpolation->isDefined) {
				std::vector<Motion::CurvePoint>& points = interpolation->displayPoints;
				ImPlot::PlotLine("test", &points.front().time, &points.front().position, points.size(), 0, sizeof(Motion::CurvePoint));
			}
			else {
				std::vector<Motion::CurvePoint> errorPoints;
				errorPoints.push_back(*interpolation->inPoint);
				errorPoints.push_back(*interpolation->outPoint);
				ImPlot::SetNextLineStyle(Colors::red, ImGui::GetTextLineHeight() * 0.2);
				ImPlot::PlotLine("error", &errorPoints.front().time, &errorPoints.front().position, errorPoints.size(), 0, sizeof(Motion::CurvePoint));
			}
		}
	}
}

bool ParameterTrack::drawControlPoints() {
	bool pointEdited = false;
	for (auto& curve : curves) {
		for (auto& point : curve->points) {
			if (ImPlot::DragPoint(point->name, &point->time, &point->position, true, Colors::white, 10.0)) {
				updateParametersAfterCurveEdit();
				pointEdited = true;
			}
		}
	}
	return pointEdited;
}