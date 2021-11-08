#include <pch.h>

#include "Motion/Manoeuvre/Manoeuvre.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Framework/Colors.h"
#include "Gui/Framework/Fonts.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Project/Environnement.h"
#include "Motion/Machine/Machine.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Motion/Curve/Curve.h"

#include "Motion/Playback.h"


void Manoeuvre::listGui(const std::shared_ptr<Manoeuvre>& manoeuvre) {

	ImGui::BeginGroup();

	ImGui::PushFont(Fonts::robotoBold20);
	float cueNameWidth = ImGui::CalcTextSize(manoeuvre->name).x;
	ImGui::PopFont();
	ImGui::PushFont(Fonts::robotoLight20);
	const char* manoeuvreTypeShortName = getManoeuvreType(manoeuvre->type)->shortName;
	float typeNameWidth = ImGui::CalcTextSize(manoeuvreTypeShortName).x;
	ImGui::PopFont();

	float headerStripWidth = std::max(cueNameWidth, typeNameWidth) + ImGui::GetStyle().ItemSpacing.x;
	glm::vec2 min = ImGui::GetWindowPos();
	glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
	glm::vec4 headerStripColor;
	switch (manoeuvre->type) {
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
	ImGui::Text(manoeuvre->name);
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
	ImGui::Text(manoeuvre->description);

	if (Playback::isInRapid(manoeuvre)) {
		glm::vec2 windowPos = ImGui::GetWindowPos();
		glm::vec2 maxsize = ImGui::GetWindowSize();
		int trackCount = manoeuvre->tracks.size();
		float trackHeight = maxsize.y / (float)trackCount;
		for (int i = 0; i < trackCount; i++) {
			glm::vec2 min(windowPos.x, windowPos.y + trackHeight * i);
			glm::vec2 max(min.x + maxsize.x * manoeuvre->tracks[i]->getRapidProgress(), min.y + trackHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
		}
	}
	if (Playback::isPrimedToStart(manoeuvre)) {
		ImGui::Text("Primed");
	}
	if (Playback::isPlaying(manoeuvre)) {
		ImGui::SameLine();
		ImGui::Text("Playing");
		glm::vec2 min = ImGui::GetWindowPos();
		glm::vec2 windowSize = ImGui::GetWindowSize();
		float progress = manoeuvre->getPlaybackProgress();
		glm::vec2 max(min.x + windowSize.x * progress, min.y + windowSize.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
	}
}


void Manoeuvre::editGui(const std::shared_ptr<Manoeuvre>& manoeuvre) {

	ImGui::PushFont(Fonts::robotoBold42);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 2.0);
	ImGui::InputText("##cueName", manoeuvre->name, 64);
	ImGui::PopFont();
	ImGui::SameLine();
	glm::vec2 descriptionFieldSize(ImGui::GetContentRegionAvail().x, ImGui::GetItemRectSize().y);
	ImGui::PushFont(Fonts::robotoRegular20);
	ImGui::InputTextMultiline("##cueDescription", manoeuvre->description, 256, descriptionFieldSize, ImGuiInputTextFlags_CtrlEnterForNewLine);
	ImGui::PopFont();

	if (ImGui::BeginCombo("##manoeuvreTypeSelector", getManoeuvreType(manoeuvre->type)->displayName)) {
		for (auto& manoeuvreType : getManoeuvreTypes()) {
			if (ImGui::Selectable(manoeuvreType.displayName, manoeuvre->type == manoeuvreType.type)) {
				manoeuvre->setType(manoeuvreType.type);
			}
		}
		ImGui::EndCombo();
	}


	bool manoeuvreIsPlaying = Playback::isPlaying(manoeuvre);

	std::shared_ptr<ParameterTrack> removedTrack = nullptr;
	std::shared_ptr<ParameterTrack> movedUpTrack = nullptr;
	std::shared_ptr<ParameterTrack> movedDownTrack = nullptr;

	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;

	int columnCount;
	switch (manoeuvre->type) {
	case ManoeuvreType::Type::KEY_POSITION:
		columnCount = 4;
		break;
	case ManoeuvreType::Type::TIMED_MOVEMENT:
		columnCount = 13;
		break;
	case ManoeuvreType::Type::MOVEMENT_SEQUENCE:
		columnCount = 14;
		break;
	}

	if (ImGui::BeginTable("##parameters", columnCount, tableFlags)) {

		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Machine");
		ImGui::TableSetupColumn("Parameter");
		if (manoeuvre->type != ManoeuvreType::Type::KEY_POSITION) {
			ImGui::TableSetupColumn("Interpolation");
		}
		if (manoeuvre->type == ManoeuvreType::Type::MOVEMENT_SEQUENCE) {
			ImGui::TableSetupColumn("Movement");
		}
		if(manoeuvre->type != ManoeuvreType::Type::KEY_POSITION){
			ImGui::TableSetupColumn("Chain");
			ImGui::TableSetupColumn("Start");
			ImGui::TableSetupColumn("Chain");
		}
		ImGui::TableSetupColumn("Target");
		if (manoeuvre->type != ManoeuvreType::Type::KEY_POSITION) {
			ImGui::TableSetupColumn("Time");
			ImGui::TableSetupColumn("Time Offset");
			ImGui::TableSetupColumn("Ramp In");
			ImGui::TableSetupColumn("=");
			ImGui::TableSetupColumn("Ramp Out");
		}

		ImGui::TableHeadersRow();

		for (auto& parameterTrack : manoeuvre->tracks) {

			bool refreshSequence = false;

			ImGui::PushID(parameterTrack->parameter->name);
			ImGui::PushID(parameterTrack->parameter->machine->getName());

			ImGui::TableNextRow(ImGuiTableRowFlags_None);

			if (manoeuvreIsPlaying) BEGIN_DISABLE_IMGUI_ELEMENT

				ImGui::TableSetColumnIndex(0);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.1));
			if (buttonCross("##remove")) removedTrack = parameterTrack;
			ImGui::SameLine();
			bool disableMoveUp = parameterTrack == manoeuvre->tracks.front();
			if (disableMoveUp) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::ArrowButton("##moveUp", ImGuiDir_Up)) movedUpTrack = parameterTrack;
			if (disableMoveUp) END_DISABLE_IMGUI_ELEMENT
				ImGui::SameLine();
			bool disableMoveDown = parameterTrack == manoeuvre->tracks.back();
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

				if (manoeuvre->type != ManoeuvreType::Type::KEY_POSITION) {
					ImGui::TableNextColumn();
					ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
					refreshSequence |= parameterTrack->interpolationTypeSelectorGui();
				}

			if (manoeuvre->type == ManoeuvreType::Type::MOVEMENT_SEQUENCE) {
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.5);
				refreshSequence |= parameterTrack->sequenceTypeSelectorGui();
			}

			if (manoeuvre->type != ManoeuvreType::Type::KEY_POSITION){

				bool disableOriginFields = manoeuvre->type == ManoeuvreType::Type::TIMED_MOVEMENT;

				if(disableOriginFields) BEGIN_DISABLE_IMGUI_ELEMENT

				ImGui::TableNextColumn();
				refreshSequence |= parameterTrack->originIsPreviousTargetCheckboxGui();

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
				refreshSequence |= parameterTrack->originInputGui();

				if(disableOriginFields) END_DISABLE_IMGUI_ELEMENT

				ImGui::TableNextColumn();
				refreshSequence |= parameterTrack->targetIsNextOriginCheckboxGui();
			}

			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			refreshSequence |= parameterTrack->targetInputGui();

			if (manoeuvre->type != ManoeuvreType::Type::KEY_POSITION) {
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

			if (refreshSequence) parameterTrack->refreshAfterParameterEdit();
		}

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		if (ImGui::Button("Add Track")) ImGui::OpenPopup("ManoeuvreTrackAdder");
		if (ImGui::BeginPopup("ManoeuvreTrackAdder")) {
			for (auto& machine : Environnement::getMachines()) {
				if (ImGui::BeginMenu(machine->getName())) {
					for (auto& parameter : machine->animatableParameters) {
						bool isSelected = manoeuvre->hasTrack(parameter);
						if (ImGui::MenuItem(parameter->name, nullptr, isSelected)) {
							if (!isSelected) manoeuvre->addTrack(parameter);
							else manoeuvre->removeTrack(parameter);
						}
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndPopup();
		}

		ImGui::EndTable();
	}

	if (removedTrack) manoeuvre->removeTrack(removedTrack->parameter);
	else if (movedUpTrack) {
		int oldIndex;
		for (int i = 0; i < manoeuvre->tracks.size(); i++) {
			if (manoeuvre->tracks[i] == movedUpTrack) {
				oldIndex = i;
				break;
			}
		}
		int newIndex = oldIndex - 1;
		if (newIndex >= 0) {
			manoeuvre->tracks.erase(manoeuvre->tracks.begin() + oldIndex);
			manoeuvre->tracks.insert(manoeuvre->tracks.begin() + newIndex, movedUpTrack);
		}
	}
	else if (movedDownTrack) {
		int oldIndex;
		for (int i = 0; i < manoeuvre->tracks.size(); i++) {
			if (manoeuvre->tracks[i] == movedDownTrack) {
				oldIndex = i;
				break;
			}
		}
		int newIndex = oldIndex + 1;
		if (newIndex < manoeuvre->tracks.size()) {
			manoeuvre->tracks.erase(manoeuvre->tracks.begin() + oldIndex);
			manoeuvre->tracks.insert(manoeuvre->tracks.begin() + newIndex, movedDownTrack);
		}
	}

	if (manoeuvre->type == ManoeuvreType::Type::MOVEMENT_SEQUENCE) {
		Manoeuvre::sequenceEditGui(manoeuvre);
	}
}

void Manoeuvre::sequenceEditGui(const std::shared_ptr<Manoeuvre>& manoeuvre) {

	if (ImGui::Button("Center On Curves")) {
		ImPlot::FitNextPlotAxes();
	}
	ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoChild;
	if (ImPlot::BeginPlot("##SequenceCurveDisplay", 0, 0, ImGui::GetContentRegionAvail(), plotFlags)) {

		//draw manoeuvre bounds
		glm::vec2 plotBoundsMin(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().Y.Max);
		glm::vec2 plotBoundsMax(ImPlot::GetPlotLimits().X.Max, ImPlot::GetPlotLimits().Y.Min);
		double startTime = 0.0;
		double endTime = manoeuvre->getLength_seconds();
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

		for (auto& parameterTrack : manoeuvre->tracks) {
			parameterTrack->drawCurves(startTime, endTime);
		}
		for (auto& parameterTrack : manoeuvre->tracks) {
			ImGui::PushID(parameterTrack->parameter->machine->getName());
			ImGui::PushID(parameterTrack->parameter->name);
			parameterTrack->drawControlPoints();
			ImGui::PopID();
			ImGui::PopID();
		}

		//if (Playback::isPlaying(manoeuvre)) {
			double playbackTime = manoeuvre->playbackPosition_seconds;
			ImPlot::SetNextLineStyle(Colors::white, ImGui::GetTextLineHeight() * 0.1);
			ImPlot::PlotVLines("Playhead", &playbackTime, 1);
		//}

		ImPlot::EndPlot();
	}

}





void Manoeuvre::playbackControlGui(const std::shared_ptr<Manoeuvre>& manoeuvre) {

	static auto fastMoveProgressOverlay = [&]() {
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		glm::vec2 max(min.x + size.x * Playback::getRapidProgress(manoeuvre), min.y + size.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	};
	static auto playbackProgressOverlay = [&]() {
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		glm::vec2 max(min.x + size.x * manoeuvre->getPlaybackProgress(), min.y + size.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
	};

	float buttonHeight = ImGui::GetTextLineHeight() * 2.0;
	float availableWidth = ImGui::GetContentRegionAvail().x;
	glm::vec2 singleButtonSize(availableWidth, buttonHeight);
	glm::vec2 doubleButtonSize((availableWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, buttonHeight);
	glm::vec2 tripleButtonSize((availableWidth - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, buttonHeight);


	switch (manoeuvre->type) {
		case ManoeuvreType::Type::KEY_POSITION:



			if (Playback::isInRapid(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Cancel Rapid", singleButtonSize)) Playback::stopRapid(manoeuvre);
				ImGui::PopStyleColor();
				fastMoveProgressOverlay();
			}
			else {
				bool isAtKeyPosition = Playback::isPrimedToEnd(manoeuvre);
				if (isAtKeyPosition) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Rapid To Key Position", singleButtonSize)) Playback::rapidToEnd(manoeuvre);
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("At Key Position", singleButtonSize)) Playback::rapidToEnd(manoeuvre);
					ImGui::PopStyleColor();
				}
			}
			break;



		case ManoeuvreType::Type::TIMED_MOVEMENT:



			if (Playback::isInRapid(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Cancel Rapid", singleButtonSize)) Playback::stopRapid(manoeuvre);
				ImGui::PopStyleColor();
				fastMoveProgressOverlay();
			}
			else {
				bool disableRapidButtons = Playback::isPlaying(manoeuvre);
				if (disableRapidButtons) BEGIN_DISABLE_IMGUI_ELEMENT
				bool primedToStart = Playback::isPrimedToStart(manoeuvre);
				if (primedToStart) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}
				if (ImGui::Button("Rapid To Start", doubleButtonSize)) Playback::rapidToStart(manoeuvre);
				if (primedToStart) {
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				ImGui::SameLine();
				bool primedToEnd = Playback::isPrimedToEnd(manoeuvre);
				if (primedToEnd) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}
				if (ImGui::Button("Rapid To End", doubleButtonSize)) Playback::rapidToEnd(manoeuvre);
				if (primedToEnd) {
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				if (disableRapidButtons) END_DISABLE_IMGUI_ELEMENT
			}
			if (Playback::isPlaying(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkRed);
				if (ImGui::Button("Stop", doubleButtonSize)) Playback::stopPlayback(manoeuvre);
				ImGui::PopStyleColor();
				ImGui::SameLine();
				if (Playback::isPaused(manoeuvre)) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Resume", doubleButtonSize)) Playback::resumePlayback(manoeuvre);
					ImGui::PopStyleColor();
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Pause", doubleButtonSize)) Playback::pausePlayback(manoeuvre);
					ImGui::PopStyleColor();
				}
				playbackProgressOverlay();
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				if (ImGui::Button("Timed Move", singleButtonSize)) Playback::startPlayback(manoeuvre);
				ImGui::PopStyleColor();
			}
			break;



		case ManoeuvreType::Type::MOVEMENT_SEQUENCE:


			if (Playback::isInRapid(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Cancel Rapid", singleButtonSize)) Playback::stopRapid(manoeuvre);
				ImGui::PopStyleColor();
				fastMoveProgressOverlay();
			}
			else {
				bool disableRapidButtons = Playback::isPlaying(manoeuvre);
				if(disableRapidButtons) BEGIN_DISABLE_IMGUI_ELEMENT
					bool primedToStart = Playback::isPrimedToStart(manoeuvre);
				if (primedToStart) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}
				if (ImGui::Button("Rapid To Start", doubleButtonSize)) Playback::rapidToStart(manoeuvre);
				if (primedToStart) {
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				ImGui::SameLine();
				bool primedToEnd = Playback::isPrimedToEnd(manoeuvre);
				if (primedToEnd) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}
				if (ImGui::Button("Rapid To End", doubleButtonSize)) Playback::rapidToEnd(manoeuvre);
				if (primedToEnd) {
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				if(disableRapidButtons) END_DISABLE_IMGUI_ELEMENT
			}

			ImGui::SetNextItemWidth(singleButtonSize.x);
			if (Playback::isPlaying(manoeuvre) && !Playback::isPaused(manoeuvre)) {
				double playhead = manoeuvre->playbackPosition_seconds;
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::InputDouble("##playbackPosition", &playhead, 0.0, 0.0, "%.3f s");
				ImGui::PopItemFlag();
			}
			else {
				ImGui::InputDouble("##targetTime", &manoeuvre->playbackPosition_seconds, 0.0, 0.0, "%.1f seconds");
				if (manoeuvre->playbackPosition_seconds < 0.0) manoeuvre->playbackPosition_seconds = 0.0;
				else if (manoeuvre->playbackPosition_seconds > manoeuvre->getLength_seconds()) manoeuvre->playbackPosition_seconds = manoeuvre->getLength_seconds();
			}
			if(manoeuvre->getPlaybackProgress() != 0.0) playbackProgressOverlay();
			if (Playback::isPlaying(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkRed);
				if (ImGui::Button("Stop", doubleButtonSize)) Playback::stopPlayback(manoeuvre);
				ImGui::PopStyleColor();
				ImGui::SameLine();
				if (Playback::isPaused(manoeuvre)) {
					if (!Playback::isPrimedToPlaybackPosition(manoeuvre)) {
						ImGui::PushStyleColor(ImGuiCol_Button, Colors::orange);
						if (ImGui::Button("Rapid To Time", doubleButtonSize)) Playback::rapidToPlaybackPosition(manoeuvre);
						ImGui::PopStyleColor();
					}
					else {
						ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
						if (ImGui::Button("Resume", doubleButtonSize)) Playback::resumePlayback(manoeuvre);
						ImGui::PopStyleColor();
					}
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Pause", doubleButtonSize)) Playback::pausePlayback(manoeuvre);
					ImGui::PopStyleColor();
				}
			}
			else {
				if (!Playback::isPrimedToPlaybackPosition(manoeuvre)) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::orange);
					if (ImGui::Button("Rapid to Playback Position", singleButtonSize)) Playback::rapidToPlaybackPosition(manoeuvre);
					ImGui::PopStyleColor();
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Start Playback", singleButtonSize)) Playback::startPlayback(manoeuvre);
					ImGui::PopStyleColor();
				}
			}



		break;
	}
}

float Manoeuvre::getPlaybackControlGuiHeight(const std::shared_ptr<Manoeuvre>& manoeuvre) {
	switch (manoeuvre->type) {
		case ManoeuvreType::Type::KEY_POSITION:
			return ImGui::GetTextLineHeight() * 2.0 //single row of buttons
				+ ImGui::GetStyle().ItemSpacing.y;	//spacing
		case ManoeuvreType::Type::TIMED_MOVEMENT:
			return ImGui::GetTextLineHeight() * 2.0 * 2.0	//single row of buttons
				+ ImGui::GetStyle().ItemSpacing.y * 2.0;	//spacing
		case ManoeuvreType::Type::MOVEMENT_SEQUENCE:
			return ImGui::GetTextLineHeight() * 2.0 * 2.0 //two rows of buttons
				+ ImGui::GetFrameHeight()			//one row of widgets
				+ImGui::GetStyle().ItemSpacing.y * 3.0;	//three spacings
	}
}