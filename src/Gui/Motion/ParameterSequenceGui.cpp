#include <pch.h>

#include "Motion/ParameterSequence.h"

#include <imgui.h>
#include <imgui_internal.h>


void ParameterSequence::interpolationTypeSelectorGui() {
	if (ImGui::BeginCombo("##InterpolationSelector", getInterpolationType(curve->interpolationType)->displayName)) {
		for (auto& interpolationType : parameter->getCompatibleInterpolationTypes()) {
			if (ImGui::Selectable(getInterpolationType(interpolationType)->displayName, interpolationType == curve->interpolationType)) {
				setInterpolationType(interpolationType);
			}
		}
		ImGui::EndCombo();
	}
}

void ParameterSequence::sequenceTypeSelectorGui() {
	if (ImGui::BeginCombo("##SequenceTypeSelector", getSequenceType(sequenceType)->displayName)) {
		for (auto& sequenceT : curve->getCompatibleSequenceTypes()) {
			if (ImGui::Selectable(getSequenceType(sequenceT)->displayName, sequenceT == sequenceType)) {
				setSequenceType(sequenceT);
			}
		}
		ImGui::EndCombo();
	}
}


void ParameterSequence::chainPreviousTargetCheckboxGui() {
	ImGui::Checkbox("##Chain", &originIsPreviousTarget);
}


void ParameterSequence::originInputGui() {
	if (originIsPreviousTarget) BEGIN_DISABLE_IMGUI_ELEMENT
		if (sequenceType == SequenceType::Type::ANIMATED_MOVE) BEGIN_DISABLE_IMGUI_ELEMENT
			ImGui::PushID("Origin");
	origin.inputFieldGui();
	ImGui::PopID();
	if (sequenceType == SequenceType::Type::ANIMATED_MOVE) END_DISABLE_IMGUI_ELEMENT
	if (originIsPreviousTarget) END_DISABLE_IMGUI_ELEMENT
}

void ParameterSequence::targetInputGui() {
	if (sequenceType == SequenceType::Type::ANIMATED_MOVE) BEGIN_DISABLE_IMGUI_ELEMENT
		ImGui::PushID("Target");
	target.inputFieldGui();
	ImGui::PopID();
	if (sequenceType == SequenceType::Type::ANIMATED_MOVE) END_DISABLE_IMGUI_ELEMENT
}

void ParameterSequence::constraintInputGui() {
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			ImGui::InputDouble("##constraint", &timeConstraint, 0.0, 0.0, "%.3f s");
			break;
		case SequenceType::Type::VELOCITY_MOVE:
			ImGui::InputDouble("##constraint", &velocityConstraint, 0.0, 0.0, "%.3f u/s");
			break;
		default:
			break;
	}
}

void ParameterSequence::timeOffsetInputGui() {
	switch (sequenceType) {
		case SequenceType::Type::ANIMATED_MOVE:
			break;
		default:
			ImGui::InputDouble("##timeOffset", &timeOffset, 0.0, 0.0, "%.3f s");
			break;
	}
}


void ParameterSequence::rampIntInputGui() {
	switch (sequenceType) {
		case SequenceType::Type::VELOCITY_MOVE:
		case SequenceType::Type::TIMED_MOVE:
			switch (curve->interpolationType) {
				case InterpolationType::Type::BEZIER:
				case InterpolationType::Type::TRAPEZOIDAL:
					ImGui::InputDouble("##rampIn", &rampIn, 0.0, 0.0, "%.3f u");
					break;
				default:
					break;
			}
			break;
	default:
		break;
	}
}

void ParameterSequence::rampOutInputGui() {
	switch (sequenceType) {
		case SequenceType::Type::VELOCITY_MOVE:
		case SequenceType::Type::TIMED_MOVE:
			switch (curve->interpolationType) {
				case InterpolationType::Type::BEZIER:
					break;
				case InterpolationType::Type::TRAPEZOIDAL:
					if (rampsAreEqual) BEGIN_DISABLE_IMGUI_ELEMENT
					ImGui::InputDouble("##rampOut", &rampOut, 0.0, 0.0, "%.3f u");
					if (rampsAreEqual) END_DISABLE_IMGUI_ELEMENT
					break;
				default:
					break;
			}
			break;
	default:
		break;
	}
}

void ParameterSequence::equalRampsCheckboxGui() {
	switch (sequenceType) {
		case SequenceType::Type::VELOCITY_MOVE:
		case SequenceType::Type::TIMED_MOVE:
			switch (curve->interpolationType) {
				case InterpolationType::Type::BEZIER:
				case InterpolationType::Type::TRAPEZOIDAL:
					ImGui::Checkbox("##rampEqual", &rampsAreEqual);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}