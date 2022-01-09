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
			if (st.type == SequenceType::Type::CONSTANT) continue;
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


bool ParameterTrack::chainPreviousGui(float width) {
	bool edited = false;
	static char chainingButtonString[128];
	glm::vec2 buttonSize(width, ImGui::GetFrameHeight());
	if (isPreviousCrossChained()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::black);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
		if (ImGui::Button("Can't Cross-Chain##ChainPrevious", buttonSize)) {
			originIsPreviousTarget = !originIsPreviousTarget;
			edited = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(2);
	}
	else if (isPreviousChainingMasterMissing()) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::black);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
		if (ImGui::Button("Can't Chain Previous##ChainPrevious", buttonSize)) {
			originIsPreviousTarget = !originIsPreviousTarget;
			edited = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(2);
	}
	else if (previousChainedSlaveTrack) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGreen);
		sprintf(chainingButtonString, "Chained by %s##ChainPrevious", previousChainedSlaveTrack->parentManoeuvre->name);
		if (ImGui::Button(chainingButtonString, buttonSize)) {
			originIsPreviousTarget = !originIsPreviousTarget;
		}
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();
	}
	else {
		bool previousValue = originIsPreviousTarget;
		if (previousValue) {
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			sprintf(chainingButtonString, "Chaining %s##ChainPrevious", previousChainedMasterTrack->parentManoeuvre->name);
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
			sprintf(chainingButtonString, "Not Chained##ChainPrevious");
		}
		if (ImGui::Button(chainingButtonString, buttonSize)) {
			originIsPreviousTarget = !originIsPreviousTarget;
			edited = true;
		}
		if (previousValue) {
			ImGui::PopFont();
			ImGui::PopStyleColor();
		}
		else ImGui::PopStyleColor(2);
	}
	return edited;
}

bool ParameterTrack::chainNextGui(float width) {
	bool edited = false;
	static char chainingButtonString[128];
	glm::vec2 buttonSize(width, ImGui::GetFrameHeight());
	if (isNextCrossChained()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::black);
		ImGui::PushFont(Fonts::robotoBold15);
		if (ImGui::Button("Can't Cross-Chain##ChainNext", buttonSize)) {
			targetIsNextOrigin = !targetIsNextOrigin;
			edited = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(2);
	}
	else if (isNextChainingMasterMissing()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::black);
		ImGui::PushFont(Fonts::robotoBold15);
		if (ImGui::Button("Can't Chain Next##ChainNext", buttonSize)) {
			targetIsNextOrigin = !targetIsNextOrigin;
			edited = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(2);
	}
	else if (nextChainedSlaveTrack) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGreen);
		sprintf(chainingButtonString, "Chained by %s##ChainNext", nextChainedSlaveTrack->parentManoeuvre->name);
		if (ImGui::Button(chainingButtonString, buttonSize)) {
			targetIsNextOrigin = !targetIsNextOrigin;
		}
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();
	}
	else {
		bool previousValue = targetIsNextOrigin;
		if (previousValue) {
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			sprintf(chainingButtonString, "Chaining %s##ChainNext", nextChainedMasterTrack->parentManoeuvre->name);
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
			sprintf(chainingButtonString, "Not Chained##ChainNext");
		}
		if (ImGui::Button(chainingButtonString, buttonSize)) {
			targetIsNextOrigin = !targetIsNextOrigin;
			edited = true;
		}
		if (previousValue) {
			ImGui::PopStyleColor();
			ImGui::PopFont();
		}
		else {
			ImGui::PopStyleColor(2);
		}
	}
	return edited;
}


bool ParameterTrack::originInputGui(float width) {
	float captureButtonWidth = ImGui::GetTextLineHeight() * 3.5;
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
	valueChanged = origin.inputFieldGui(width - captureButtonWidth - ImGui::GetStyle().ItemSpacing.x);
	if (originValidationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
	ImGui::SameLine();
	if(ImGui::Button("Capture", glm::vec2(captureButtonWidth, ImGui::GetFrameHeight()))){
		captureCurrentValueToOrigin();
		valueChanged = true;
	}
	ImGui::PopID();
	if (disableField) END_DISABLE_IMGUI_ELEMENT
	return valueChanged;
}

bool ParameterTrack::targetInputGui(float width) {
	float captureButtonWidth = ImGui::GetTextLineHeight() * 3.5;
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
	valueChanged = target.inputFieldGui(width - captureButtonWidth - ImGui::GetStyle().ItemSpacing.x);
	if (targetValidationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
	ImGui::SameLine();
	if(ImGui::Button("Capture", glm::vec2(captureButtonWidth, ImGui::GetFrameHeight()))){
		captureCurrentValueToTarget();
		valueChanged = true;
	}
	ImGui::PopID();

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
			valueChanged = ImGui::InputDouble("##movementTime", &movementTime, 0.0, 0.0, "Movement: %.1f s");
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
					valueChanged = ImGui::InputDouble("##timeOffset", &timeOffset, 0.0, 0.0, "Time Offset: %.1f s");
					break;
			}
			break;
	}
	return valueChanged;
}



bool ParameterTrack::rampInputGui(float width) {
	bool valueChanged = false;

	static char rampInputString[128];

	switch (sequenceType) {
		case SequenceType::Type::TIMED_MOVE:
			switch (interpolationType) {
				case InterpolationType::Type::BEZIER:
				case InterpolationType::Type::TRAPEZOIDAL:
					break;
				default:
					return false;
			}
			break;
		default:
			return false;
	}

	ImGui::BeginGroup();

	bool inRampValidationError = false;
	for (auto& point : startPoints) {
		switch (point->validationError) {
		case Motion::ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO:
		case Motion::ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED:
		case Motion::ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO:
		case Motion::ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED:
			inRampValidationError = true;
			break;
		default:
			break;
		}
	}
	if (!inRampValidationError) {
		for (auto& curve : curves) {
			for (auto& interpolation : curve->interpolations) {
				switch (interpolation->validationError) {
				case Motion::ValidationError::Error::INTERPOLATION_UNDEFINED:
					inRampValidationError = true;
					break;
				default:
					break;
				}
			}
			if (inRampValidationError) break;
		}
	}

	if (inRampValidationError) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
	}
	ImGui::SetNextItemWidth(width);
	sprintf(rampInputString, "In: %.3f %s/s\xC2\xB2", rampIn, origin.shortUnitString);
	valueChanged |= ImGui::InputDouble("##rampIn", &rampIn, 0.0, 0.0, rampInputString);
	if (inRampValidationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
	
	bool outRampValidationError = false;
	for (auto& point : endPoints) {
		switch (point->validationError) {
		case Motion::ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO:
		case Motion::ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED:
		case Motion::ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO:
		case Motion::ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED:
			outRampValidationError = true;
			break;
		default:
			break;
		}
	}
	if (!outRampValidationError) {
		for (auto& curve : curves) {
			for (auto& interpolation : curve->interpolations) {
				switch (interpolation->validationError) {
				case Motion::ValidationError::Error::INTERPOLATION_UNDEFINED:
					outRampValidationError = true;
					break;
				default:
					break;
				}
			}
			if (outRampValidationError) break;
		}
	}

	if (outRampValidationError) {
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::robotoBold15);
	}
	if (rampsAreEqual) BEGIN_DISABLE_IMGUI_ELEMENT
	ImGui::SetNextItemWidth(width);
	sprintf(rampInputString, "Out: %.3f %s/s\xC2\xB2", rampOut, origin.shortUnitString);
	valueChanged |= ImGui::InputDouble("##rampOut", &rampOut, 0.0, 0.0, rampInputString);
	if (rampsAreEqual) END_DISABLE_IMGUI_ELEMENT
	if (outRampValidationError) {
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
	
	ImGui::EndGroup();

	ImGui::SameLine();

	glm::vec2 equalButtonSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight() * 2.0 + ImGui::GetStyle().ItemSpacing.y);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
	if ((rampsAreEqual && ImGui::Button("=", equalButtonSize)) || (!rampsAreEqual && ImGui::Button("!=", equalButtonSize))) {
		rampsAreEqual = !rampsAreEqual;
		valueChanged |= true;
	}
	ImGui::PopStyleColor();
	ImGui::PopFont();

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

void ParameterTrack::drawChainedCurves() {
	for (int i = 0; i < curves.size(); i++) {
		if (nextChainedMasterTrack || nextChainedSlaveTrack) {
			std::vector<Motion::CurvePoint>& points = nextChainedCurvePoints[i];
			if (!points.empty()) ImPlot::PlotLine(curves[i]->name, &points.front().time, &points.front().position, points.size(), 0, sizeof(Motion::CurvePoint));
		}
		if (previousChainedMasterTrack || previousChainedSlaveTrack) {
			std::vector<Motion::CurvePoint>& points = previousChainedCurvePoints[i];
			if (!points.empty()) ImPlot::PlotLine(curves[i]->name, &points.front().time, &points.front().position, points.size(), 0, sizeof(Motion::CurvePoint));
		}
	}
}

bool ParameterTrack::drawControlPoints() {
	bool edited = false;

	if (sequenceType == SequenceType::Type::CONSTANT) {
		for (int i = 0; i < getCurveCount(); i++) {
			edited |= ImPlot::DragLineY(curves[i]->name, &endPoints[i]->position, true, Colors::gray, 4.0);
		}
		if (edited) refreshAfterCurveEdit();
		return edited;
	}
    
    float controlPointLarge = ImGui::GetTextLineHeight() * 0.5;
    float controlPointMedium = ImGui::GetTextLineHeight() * 0.3;

	for (auto& curve : curves) {

		for (int i = 0; i < curve->getPoints().size(); i++) {
			//don't draw the first control point of a step interpolation sequence, since we can't edit it anyway
			if (sequenceType == SequenceType::Type::TIMED_MOVE && interpolationType == InterpolationType::Type::STEP && i == 0) continue;

			auto& controlPoint = curve->getPoints()[i];
			bool controlPointEdited = false;

			bool pointIsChained = (originIsPreviousTarget && i == 0) || (targetIsNextOrigin && i == curve->getPoints().size() - 1);

			if (pointIsChained) {
				if (controlPoint->b_valid) ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, controlPointMedium, Colors::black, 2.0, Colors::white);
				else ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, controlPointLarge, Colors::red, 2.0, Colors::white);
				ImPlot::PlotScatter("##ChaindPoint", &controlPoint->time, &controlPoint->position, 1);
			}
			else {
				if (controlPoint->b_valid) controlPointEdited = ImPlot::DragPoint(controlPoint->name, &controlPoint->time, &controlPoint->position, true, Colors::white, controlPointMedium);
				else controlPointEdited = ImPlot::DragPoint(controlPoint->name, &controlPoint->time, &controlPoint->position, true, Colors::red, controlPointLarge);
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
				edited = true;
			}
		}

	}
	return edited;
}
