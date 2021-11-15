#include <pch.h>

#include "Motion/Manoeuvre/ParameterTrack.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Motion/Curve/Curve.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Machine/Machine.h"
#include "Motion/Manoeuvre/Manoeuvre.h"

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
			if (st.type == SequenceType::Type::NO_MOVE) continue;
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
	if (isPreviousCrossChained()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Checkbox("##ChainPrevious", &originIsPreviousTarget);
		ImGui::SameLine();
		ImGui::Text("Cross Chained");
		ImGui::PopFont();
		ImGui::PopStyleColor();
	}
	else if (isPreviousChainingMasterMissing()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Checkbox("##ChainPrevious", &originIsPreviousTarget);
		ImGui::SameLine();
		ImGui::Text("First Track");
		ImGui::PopFont();
		ImGui::PopStyleColor();
	}
	else if (previousChainedSlaveTrack) {
		ImGui::Text("Chained by %s", previousChainedSlaveTrack->parentManoeuvre->name);
		return false;
	}
	else {
		bool previousValue = originIsPreviousTarget;
		bool edited = ImGui::Checkbox("##ChainPrevious", &originIsPreviousTarget);
		ImGui::SameLine();
		if (previousValue) {
			if (previousChainedMasterTrack) ImGui::Text("Chaining %s", previousChainedMasterTrack->parentManoeuvre->name);
		}
		else ImGui::Text("Not Chained");
		return edited;
	}
}

bool ParameterTrack::targetIsNextOriginCheckboxGui() {
	if (isNextCrossChained()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Checkbox("##ChainNext", &targetIsNextOrigin);
		ImGui::SameLine();
		ImGui::Text("Cross Chained");
		ImGui::PopFont();
		ImGui::PopStyleColor();
	}
	else if (isNextChainingMasterMissing()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Checkbox("##ChainNext", &targetIsNextOrigin);
		ImGui::SameLine();
		ImGui::Text("Last Track");
		ImGui::PopFont();
		ImGui::PopStyleColor();
	}
	else if (nextChainedSlaveTrack) {
		ImGui::Text("Chained by %s", nextChainedSlaveTrack->parentManoeuvre->name);
		return false;
	}
	else {
		bool previousValue = targetIsNextOrigin;
		bool edited = ImGui::Checkbox("##ChainNext", &targetIsNextOrigin);
		ImGui::SameLine();
		if (previousValue) {
			if (nextChainedMasterTrack) ImGui::Text("Chaining %s", nextChainedMasterTrack->parentManoeuvre->name);
		}
		else ImGui::Text("Not Chained");
		return edited;
	}
}


bool ParameterTrack::originInputGui() {
	bool originValidationError = false;
	for (auto& point : startPoints) {
		if (point->validationError == Motion::ValidationError::Error::CONTROL_POINT_POSITION_OUT_OF_RANGE) {
			originValidationError = true;
		}
	}
	bool valueChanged = false;
	bool disableField = originIsPreviousTarget;
	if (disableField) BEGIN_DISABLE_IMGUI_ELEMENT
	if (originValidationError) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
	}
	ImGui::PushID("Origin");
	valueChanged = origin.inputFieldGui(ImGui::GetTextLineHeight() * 5.0);
	ImGui::PopID();
	if (originValidationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
	if (disableField) END_DISABLE_IMGUI_ELEMENT
	return valueChanged;
}

bool ParameterTrack::targetInputGui() {
	bool targetValidationError = false;
	for (auto& point : endPoints) {
		if (point->validationError == Motion::ValidationError::Error::CONTROL_POINT_POSITION_OUT_OF_RANGE) {
			targetValidationError = true;
		}
	}
	bool valueChanged = false;
	bool disableField = targetIsNextOrigin;
	if (disableField) BEGIN_DISABLE_IMGUI_ELEMENT
	if (targetValidationError) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
	}
	ImGui::PushID("Target");
	valueChanged = target.inputFieldGui(ImGui::GetTextLineHeight() * 5.0);
	ImGui::PopID();
	if (targetValidationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
	if (disableField) END_DISABLE_IMGUI_ELEMENT
	return valueChanged;
}

bool ParameterTrack::timeInputGui() {

	bool validationError = false;
	for (auto& curve : curves) {
		for (auto& interpolation : curve->interpolations) {
			switch (interpolation->validationError) {
				case Motion::ValidationError::Error::INTERPOLATION_UNDEFINED:
				case Motion::ValidationError::Error::INTERPOLATION_VELOCITY_LIMIT_EXCEEDED:
					validationError = true;
					break;
				default:
					break;
			}
		}
		if (validationError) break;
	}

	if (validationError) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
	}

	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			valueChanged = ImGui::InputDouble("##constraint", &movementTime, 0.0, 0.0, "%.3f s");
			break;
		default:
			break;
	}

	if (validationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
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


bool ParameterTrack::rampInInputGui() {
	bool validationError = false;
	for (auto& point : startPoints) {
		switch (point->validationError) {
		case Motion::ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO:
		case Motion::ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED:
		case Motion::ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO:
		case Motion::ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED:
			validationError = true;
			break;
		default:
			break;
		}
	}
	if (!validationError) {
		for (auto& curve : curves) {
			for (auto& interpolation : curve->interpolations) {
				switch (interpolation->validationError) {
				case Motion::ValidationError::Error::INTERPOLATION_UNDEFINED:
					validationError = true;
					break;
				default:
					break;
				}
			}
			if (validationError) break;
		}
	}

	if (validationError) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
	}

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

	if (validationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}

	return valueChanged;
}

bool ParameterTrack::rampOutInputGui() {

	bool validationError = false;
	for (auto& point : endPoints) {
		switch (point->validationError) {
			case Motion::ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO:
			case Motion::ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED:
			case Motion::ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO:
			case Motion::ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED:
				validationError = true;
				break;
			default:
				break;
		}
	}
	if (!validationError) {
		for (auto& curve : curves) {
			for (auto& interpolation : curve->interpolations) {
				switch (interpolation->validationError) {
				case Motion::ValidationError::Error::INTERPOLATION_UNDEFINED:
					validationError = true;
					break;
				default:
					break;
				}
			}
			if (validationError) break;
		}
	}

	if (validationError) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
	}
	if (rampsAreEqual) BEGIN_DISABLE_IMGUI_ELEMENT

	bool valueChanged = false;
	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			switch (interpolationType) {
				case InterpolationType::Type::BEZIER:
					break;
				case InterpolationType::Type::TRAPEZOIDAL:
					valueChanged = ImGui::InputDouble("##rampOut", &rampOut, 0.0, 0.0, "%.3f u");
					break;
				default:
					break;
			}
			break;
	default:
		break;
	}

	if (rampsAreEqual) END_DISABLE_IMGUI_ELEMENT
	if (validationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
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
		auto startPoint = curve->getStart();
		if (startPoint->time > startTime) {
			std::vector<glm::vec2> headerPoints;
			headerPoints.push_back(glm::vec2(startTime, startPoint->position));
			headerPoints.push_back(glm::vec2(startPoint->time, startPoint->position));
			ImPlot::PlotLine(curve->name, &headerPoints.front().x, &headerPoints.front().y, headerPoints.size(), 0, sizeof(glm::vec2));
		}
		for (auto& interpolation : curve->interpolations) {
			if (interpolation->b_valid) {
				std::vector<Motion::CurvePoint>& points = interpolation->displayPoints;
				ImPlot::PlotLine(curve->name, &points.front().time, &points.front().position, points.size(), 0, sizeof(Motion::CurvePoint));
				std::vector<Motion::CurvePoint>& inflectionPoints = interpolation->displayInflectionPoints;
				if(!inflectionPoints.empty())
					ImPlot::PlotScatter("##inflectionPoints", &inflectionPoints.front().time, &inflectionPoints.front().position, inflectionPoints.size(), 0, sizeof(Motion::CurvePoint));
			}
			else {
				std::vector<Motion::CurvePoint> errorPoints;
				errorPoints.push_back(*interpolation->inPoint);
				errorPoints.push_back(*interpolation->outPoint);
				ImPlot::SetNextLineStyle(Colors::red, ImGui::GetTextLineHeight() * 0.2);
				ImPlot::PlotLine(curve->name, &errorPoints.front().time, &errorPoints.front().position, errorPoints.size(), 0, sizeof(Motion::CurvePoint));
				glm::vec2 averagePosition = glm::vec2(interpolation->inPoint->time, interpolation->inPoint->position) + glm::vec2(interpolation->outPoint->time, interpolation->outPoint->position);
				averagePosition /= 2.0;
				ImGui::PushFont(Fonts::robotoBold15);
				ImPlot::Annotate(averagePosition.x, averagePosition.y, glm::vec2(0, 0), glm::vec4(0.5, 0.0, 0.0, 0.5), Motion::getValidationError(interpolation->validationError)->displayName);
				
				ImGui::PopFont();
			}
		}
		auto endPoint = curve->getEnd();
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
			bool controlPointEdited = false;
			
			bool pointIsChained = (originIsPreviousTarget && i == 0) || (targetIsNextOrigin && i == curve->getPoints().size() - 1);

			if (pointIsChained) {
				if (controlPoint->b_valid) ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 10.0, Colors::black, 2.0, Colors::white);
				else ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 15.0, Colors::red, 2.0, Colors::white);
				ImPlot::PlotScatter("##ChaindPoint", &controlPoint->time, &controlPoint->position, 1);
			}
			else {
				if (controlPoint->b_valid) controlPointEdited = ImPlot::DragPoint(controlPoint->name, &controlPoint->time, &controlPoint->position, true, Colors::white, 10.0);
				else controlPointEdited = ImPlot::DragPoint(controlPoint->name, &controlPoint->time, &controlPoint->position, true, Colors::red, 15.0);
			}

			if (!controlPoint->b_valid) {
				static char errorAnnotationString[256];
				sprintf(errorAnnotationString, Motion::getValidationError(controlPoint->validationError)->displayName);
				ImPlot::Annotate(controlPoint->time, controlPoint->position, glm::vec2(30, -30), glm::vec4(0.5, 0.0, 0.0, 0.5), errorAnnotationString);
			}

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


			if (controlPointEdited) {
				refreshAfterCurveEdit();
				pointEdited = true;
			}
		}
	}
	return pointEdited;
}
