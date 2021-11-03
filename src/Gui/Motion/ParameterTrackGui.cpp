#include <pch.h>

#include "Motion/ParameterTrack.h"

#include <imgui.h>
#include <imgui_internal.h>


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
	auto compatibleSequenceTypes = getCompatibleSequenceTypes();
	bool disableCombo = compatibleSequenceTypes.size() < 2;
	if(disableCombo) BEGIN_DISABLE_IMGUI_ELEMENT
	if (ImGui::BeginCombo("##SequenceTypeSelector", getSequenceType(sequenceType)->displayName)) {
		for (auto& st : compatibleSequenceTypes) {
			if(st == SequenceType::Type::ANIMATED_MOVE) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::Selectable(getSequenceType(st)->displayName, st == sequenceType)) {
				setSequenceType(st);
				valueChanged = true;
			}
			if(st == SequenceType::Type::ANIMATED_MOVE) END_DISABLE_IMGUI_ELEMENT
		}
		ImGui::EndCombo();
	}
	if (disableCombo) END_DISABLE_IMGUI_ELEMENT
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

bool ParameterTrack::constraintInputGui() {
	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			valueChanged = ImGui::InputDouble("##constraint", &timeConstraint, 0.0, 0.0, "%.3f s");
			break;
		case SequenceType::Type::VELOCITY_MOVE:
			valueChanged = ImGui::InputDouble("##constraint", &velocityConstraint, 0.0, 0.0, "%.3f u/s");
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
			valueChanged = ImGui::InputDouble("##timeOffset", &timeOffset, 0.0, 0.0, "%.3f s");
			break;
	}
	return valueChanged;
}


bool ParameterTrack::rampIntInputGui() {
	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::VELOCITY_MOVE:
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
		case SequenceType::Type::VELOCITY_MOVE:
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
		case SequenceType::Type::VELOCITY_MOVE:
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