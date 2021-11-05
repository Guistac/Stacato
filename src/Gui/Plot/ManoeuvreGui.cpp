#include <pch.h>

#include "Plot/Manoeuvre.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Framework/Fonts.h"
#include "Environnement/Environnement.h"
#include "Motion/Machine/Machine.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/ParameterTrack.h"
#include "Gui/Framework/Colors.h"
#include "Gui/Framework/Fonts.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Motion/Curve/Curve.h"

void Manoeuvre::listGui() {

	ImGui::BeginGroup();

	ImGui::PushFont(Fonts::robotoBold20);
	float cueNameWidth = ImGui::CalcTextSize(name).x;
	ImGui::PopFont();
	ImGui::PushFont(Fonts::robotoLight20);
	const char* manoeuvreTypeShortName = getManoeuvreType(type)->shortName;
	float typeNameWidth = ImGui::CalcTextSize(manoeuvreTypeShortName).x;
	ImGui::PopFont();

	float headerStripWidth = std::max(cueNameWidth, typeNameWidth) + ImGui::GetStyle().ItemSpacing.x;
	glm::vec2 min = ImGui::GetWindowPos();
	glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
	glm::vec4 headerStripColor;
	switch (type) {
		case ManoeuvreType::Type::KEY_POSITION:
			headerStripColor = Colors::darkYellow;
			break;
		case ManoeuvreType::Type::TIMED_MOVEMENT:
			headerStripColor = Colors::darkGray;
			break;
		case ManoeuvreType::Type::MOVEMENT_SEQUENCE:
			headerStripColor = Colors::darkRed;
			break;
	}
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
	ImGui::Text(name);
	ImGui::PopFont();

	ImGui::PushFont(Fonts::robotoLight20);
	ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 1.0, 1.0, 0.3));
	ImGui::NewLine();
	ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
	ImGui::Text(manoeuvreTypeShortName);
	ImGui::PopStyleColor();
	ImGui::PopFont();

	ImGui::EndGroup();


	ImGui::SameLine();
	ImGui::Text(description);

	if (isPriming()) {
		glm::vec2 windowPos = ImGui::GetWindowPos();
		glm::vec2 maxsize = ImGui::GetWindowSize();
		int trackCount = tracks.size();
		float trackHeight = maxsize.y / (float)trackCount;
		for (int i = 0; i < trackCount; i++) {
			glm::vec2 min(windowPos.x, windowPos.y + trackHeight * i);
			glm::vec2 max(min.x + maxsize.x * tracks[i]->getPrimingProgress(), min.y + trackHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
		}
	}
	if (isPrimed()) {
		ImGui::Text("Primed");
	}
	if (isPlaying()) {
		ImGui::SameLine();
		ImGui::Text("Playing");
	}
	if (isPlaying()) {
		glm::vec2 min = ImGui::GetWindowPos();
		glm::vec2 windowSize = ImGui::GetWindowSize();
		float progress = getPlaybackProgress();
		glm::vec2 max(min.x + windowSize.x * progress, min.y + windowSize.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
	}
}


void Manoeuvre::editGui() {

	ImGui::PushFont(Fonts::robotoBold42);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 2.0);
	ImGui::InputText("##cueName", name, 64);
	ImGui::PopFont();
	ImGui::SameLine();
	glm::vec2 descriptionFieldSize(ImGui::GetContentRegionAvail().x, ImGui::GetItemRectSize().y);
	ImGui::PushFont(Fonts::robotoRegular20);
	ImGui::InputTextMultiline("##cueDescription", description, 256, descriptionFieldSize, ImGuiInputTextFlags_CtrlEnterForNewLine);
	ImGui::PopFont();

	if (ImGui::BeginCombo("##manoeuvreTypeSelector", getManoeuvreType(type)->displayName)) {
		for (auto& manoeuvreType : getManoeuvreTypes()) {
			if (ImGui::Selectable(manoeuvreType.displayName, type == manoeuvreType.type)) {
				setType(manoeuvreType.type);
			}
		}
		ImGui::EndCombo();
	}


	bool manoeuvreIsPlaying = isPlaying();

	std::shared_ptr<ParameterTrack> removedTrack = nullptr;
	std::shared_ptr<ParameterTrack> movedUpTrack = nullptr;
	std::shared_ptr<ParameterTrack> movedDownTrack = nullptr;

	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;

	int columnCount;
	switch (type) {
	case ManoeuvreType::Type::KEY_POSITION:
		columnCount = 4;
		break;
	case ManoeuvreType::Type::TIMED_MOVEMENT:
		columnCount = 10;
		break;
	case ManoeuvreType::Type::MOVEMENT_SEQUENCE:
		columnCount = 13;
		break;
	}

	if (ImGui::BeginTable("##parameters", columnCount, tableFlags)) {

		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Machine");
		ImGui::TableSetupColumn("Parameter");
		if (type != ManoeuvreType::Type::KEY_POSITION) {
			ImGui::TableSetupColumn("Interpolation");
		}
		if (type == ManoeuvreType::Type::MOVEMENT_SEQUENCE) {
			ImGui::TableSetupColumn("Movement");
			ImGui::TableSetupColumn("Chain");
			ImGui::TableSetupColumn("Start");
		}
		ImGui::TableSetupColumn("Target");
		if (type != ManoeuvreType::Type::KEY_POSITION) {
			ImGui::TableSetupColumn("Time");
			ImGui::TableSetupColumn("Time Offset");
			ImGui::TableSetupColumn("Ramp In");
			ImGui::TableSetupColumn("=");
			ImGui::TableSetupColumn("Ramp Out");
		}

		ImGui::TableHeadersRow();

		for (auto& parameterTrack : tracks) {

			bool refreshSequence = false;

			ImGui::PushID(parameterTrack->parameter->name);
			ImGui::PushID(parameterTrack->parameter->machine->getName());

			ImGui::TableNextRow(ImGuiTableRowFlags_None);

			if (manoeuvreIsPlaying) BEGIN_DISABLE_IMGUI_ELEMENT

				ImGui::TableSetColumnIndex(0);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.1));
			if (buttonCross("##remove")) removedTrack = parameterTrack;
			ImGui::SameLine();
			bool disableMoveUp = parameterTrack == tracks.front();
			if (disableMoveUp) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::ArrowButton("##moveUp", ImGuiDir_Up)) movedUpTrack = parameterTrack;
			if (disableMoveUp) END_DISABLE_IMGUI_ELEMENT
				ImGui::SameLine();
			bool disableMoveDown = parameterTrack == tracks.back();
			if (disableMoveDown) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::ArrowButton("##moveDown", ImGuiDir_Down)) movedDownTrack = parameterTrack;
			if (disableMoveDown) END_DISABLE_IMGUI_ELEMENT
				ImGui::PopStyleVar();

			if (manoeuvreIsPlaying) END_DISABLE_IMGUI_ELEMENT

				ImGui::TableNextColumn();
			ImGui::Text(parameterTrack->parameter->machine->getName());

			ImGui::TableNextColumn();
			ImGui::Text(parameterTrack->parameter->name);

			if (manoeuvreIsPlaying) BEGIN_DISABLE_IMGUI_ELEMENT

				if (type != ManoeuvreType::Type::KEY_POSITION) {
					ImGui::TableNextColumn();
					ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
					refreshSequence |= parameterTrack->interpolationTypeSelectorGui();
				}

			if (type == ManoeuvreType::Type::MOVEMENT_SEQUENCE) {
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.5);
				refreshSequence |= parameterTrack->sequenceTypeSelectorGui();

				ImGui::TableNextColumn();
				refreshSequence |= parameterTrack->chainPreviousTargetCheckboxGui();

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
				refreshSequence |= parameterTrack->originInputGui();
			}

			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			refreshSequence |= parameterTrack->targetInputGui();

			if (type != ManoeuvreType::Type::KEY_POSITION) {
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
				refreshSequence |= parameterTrack->timeInputGui();

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
				refreshSequence |= parameterTrack->timeOffsetInputGui();

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
				refreshSequence |= parameterTrack->rampIntInputGui();

				ImGui::TableNextColumn();
				refreshSequence |= parameterTrack->equalRampsCheckboxGui();

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
				refreshSequence |= parameterTrack->rampOutInputGui();
			}

			if (manoeuvreIsPlaying) END_DISABLE_IMGUI_ELEMENT

				ImGui::PopID();
			ImGui::PopID();

			if (refreshSequence) parameterTrack->updateCurves();
		}

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		if (ImGui::Button("Add Track")) ImGui::OpenPopup("ManoeuvreTrackAdder");
		if (ImGui::BeginPopup("ManoeuvreTrackAdder")) {
			for (auto& machine : Environnement::getMachines()) {
				if (ImGui::BeginMenu(machine->getName())) {
					for (auto& parameter : machine->animatableParameters) {
						bool isSelected = hasTrack(parameter);
						if (ImGui::MenuItem(parameter->name, nullptr, isSelected)) {
							if (!isSelected) addTrack(parameter);
							else removeTrack(parameter);
						}
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndPopup();
		}

		ImGui::EndTable();
	}

	if (removedTrack) removeTrack(removedTrack->parameter);
	else if (movedUpTrack) {
		int oldIndex;
		for (int i = 0; i < tracks.size(); i++) {
			if (tracks[i] == movedUpTrack) {
				oldIndex = i;
				break;
			}
		}
		int newIndex = oldIndex - 1;
		if (newIndex >= 0) {
			tracks.erase(tracks.begin() + oldIndex);
			tracks.insert(tracks.begin() + newIndex, movedUpTrack);
		}
	}
	else if (movedDownTrack) {
		int oldIndex;
		for (int i = 0; i < tracks.size(); i++) {
			if (tracks[i] == movedDownTrack) {
				oldIndex = i;
				break;
			}
		}
		int newIndex = oldIndex + 1;
		if (newIndex < tracks.size()) {
			tracks.erase(tracks.begin() + oldIndex);
			tracks.insert(tracks.begin() + newIndex, movedDownTrack);
		}
	}

	if (type == ManoeuvreType::Type::MOVEMENT_SEQUENCE) {
		sequenceEditGui();
	}
}

void Manoeuvre::sequenceEditGui() {

	if (ImGui::Button("Center On Curves")) {
		ImPlot::FitNextPlotAxes();
	}
	ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoChild;
	if (ImPlot::BeginPlot("##SequenceCurveDisplay", 0, 0, ImGui::GetContentRegionAvail(), plotFlags)) {

		//draw manoeuvre bounds
		glm::vec2 plotBoundsMin(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().Y.Max);
		glm::vec2 plotBoundsMax(ImPlot::GetPlotLimits().X.Max, ImPlot::GetPlotLimits().Y.Min);
		double startTime = 0.0;
		double endTime = getLength_seconds();
		std::vector<glm::vec2> limits;
		limits.push_back(glm::vec2(plotBoundsMin.x, plotBoundsMin.y));
		limits.push_back(glm::vec2(startTime, plotBoundsMin.y));
		limits.push_back(glm::vec2(startTime, plotBoundsMax.y));
		limits.push_back(glm::vec2(endTime, plotBoundsMax.y));
		limits.push_back(glm::vec2(endTime, plotBoundsMin.y));
		limits.push_back(glm::vec2(plotBoundsMax.x, plotBoundsMin.y));
		if (endTime > 0.0) {
			ImPlot::SetNextFillStyle(Colors::black, 0.5);
			ImPlot::PlotShaded("##shaded", &limits.front().x, &limits.front().y, limits.size(), -INFINITY, 0, sizeof(glm::vec2));
			ImPlot::PlotVLines("##Limits1", &startTime, 1);
			ImPlot::PlotVLines("##Limits2", &endTime, 1);
		}

		for (auto& parameterTrack : tracks) {
			parameterTrack->drawCurves(startTime, endTime);
		}
		for (auto& parameterTrack : tracks) {
			ImGui::PushID(parameterTrack->parameter->machine->getName());
			ImGui::PushID(parameterTrack->parameter->name);
			parameterTrack->drawControlPoints();
			ImGui::PopID();
			ImGui::PopID();
		}

		if (isPlaying()) {
			double playbackTime = getPlaybackTime_seconds();
			ImPlot::SetNextLineStyle(Colors::white, ImGui::GetTextLineHeight() * 0.1);
			ImPlot::PlotVLines("Playhead", &playbackTime, 1);
		}

		ImPlot::EndPlot();
	}

}





void Manoeuvre::playbackControlGui() {
	float buttonHeight = ImGui::GetTextLineHeight() * 2.0;
	float availableWidth = ImGui::GetContentRegionAvail().x;
	glm::vec2 singleButtonSize(availableWidth, buttonHeight);
	glm::vec2 doubleButtonSize((availableWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, buttonHeight);
	glm::vec2 tripleButtonSize((availableWidth - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, buttonHeight);
	switch (type) {
	case ManoeuvreType::Type::KEY_POSITION:
		if (ImGui::Button("Fast Move To Key Position", singleButtonSize)) {}
		break;
	case ManoeuvreType::Type::TIMED_MOVEMENT:
		if (ImGui::Button("Timed Move", doubleButtonSize)) {}
		ImGui::SameLine();
		if (ImGui::Button("Fast Move", doubleButtonSize)) {}
		break;
	case ManoeuvreType::Type::MOVEMENT_SEQUENCE:
		if (ImGui::Button("Fast Move To Start", doubleButtonSize)) prime();
		ImGui::SameLine();
		if (ImGui::Button("Fast Move To End", doubleButtonSize)) {}

		static double targetTime = 0.0;
		ImGui::SetNextItemWidth(doubleButtonSize.x);
		ImGui::InputDouble("##targetTime", &targetTime, 0.0, 0.0, "%.1f seconds");
		if (targetTime < 0.0) targetTime = 0.0;
		else if (targetTime > getLength_seconds()) targetTime = getLength_seconds();
		ImGui::SameLine();
		if (ImGui::Button("Fast Move To Time", glm::vec2(doubleButtonSize.x, ImGui::GetItemRectSize().y))) {}

		if (ImGui::Button("Start Sequence", singleButtonSize)) startPlayback();
		break;
	}
}

float Manoeuvre::getPlaybackControlGuiHeight() {
	switch (type) {
		case ManoeuvreType::Type::KEY_POSITION:
			return ImGui::GetTextLineHeight() * 2.0 //single row of buttons
				+ ImGui::GetStyle().ItemSpacing.y;	//spacing
		case ManoeuvreType::Type::TIMED_MOVEMENT:
			return ImGui::GetTextLineHeight() * 2.0	//single row of buttons
				+ ImGui::GetStyle().ItemSpacing.y;	//spacing
		case ManoeuvreType::Type::MOVEMENT_SEQUENCE:
			return ImGui::GetTextLineHeight() * 2.0 * 2.0 //two rows of buttons
				+ ImGui::GetFrameHeight()			//one row of widgets
				+ImGui::GetStyle().ItemSpacing.y * 3.0;	//three spacings
	}
}