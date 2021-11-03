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

	ImGui::PushFont(Fonts::robotoBold20);
	float headerStripWidth = ImGui::CalcTextSize(name).x + ImGui::GetStyle().ItemSpacing.x;
	glm::vec2 min = ImGui::GetWindowPos();
	glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(Colors::darkGray), 10.0, ImDrawFlags_RoundCornersLeft);
	ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
	ImGui::Text(name);
	ImGui::PopFont();



	ImGui::SameLine();
	ImGui::Text(description);
	
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

	ImGui::SameLine();
	ImGui::PushFont(Fonts::robotoLight42);
	ImGui::Text(description);
	ImGui::PopFont();
	ImGui::Separator();

	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;

	if (ImGui::BeginTable("##parameters", 13, tableFlags)) {
		
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Machine");
		ImGui::TableSetupColumn("Parameter");
		ImGui::TableSetupColumn("Interpolation");
		ImGui::TableSetupColumn("Movement");
		ImGui::TableSetupColumn("Chain");
		ImGui::TableSetupColumn("Start");
		ImGui::TableSetupColumn("End");
		ImGui::TableSetupColumn("Time");
		ImGui::TableSetupColumn("Time Offset");
		ImGui::TableSetupColumn("Ramp In");
		ImGui::TableSetupColumn("=");
		ImGui::TableSetupColumn("Ramp Out");

		ImGui::TableHeadersRow();

		std::shared_ptr<ParameterTrack> removedSequence = nullptr;
		std::shared_ptr<ParameterTrack> movedUpSequence = nullptr;
		std::shared_ptr<ParameterTrack> movedDownSequence = nullptr;

		for (auto& parameterTrack : tracks) {

			bool refreshSequence = false;

			ImGui::PushID(parameterTrack->parameter->name);
			ImGui::PushID(parameterTrack->parameter->machine->getName());

			ImGui::TableNextRow(ImGuiTableRowFlags_None);

			ImGui::TableSetColumnIndex(0);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.1));
			if (buttonCross("##remove")) removedSequence = parameterTrack;
			ImGui::SameLine();
			bool disableMoveUp = parameterTrack == tracks.front();
			if(disableMoveUp) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::ArrowButton("##moveUp", ImGuiDir_Up)) movedUpSequence = parameterTrack;
			if(disableMoveUp) END_DISABLE_IMGUI_ELEMENT
			ImGui::SameLine();
			bool disableMoveDown = parameterTrack == tracks.back();
			if(disableMoveDown) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::ArrowButton("##moveDown", ImGuiDir_Down)) movedDownSequence = parameterTrack;
			if(disableMoveDown) END_DISABLE_IMGUI_ELEMENT
			ImGui::PopStyleVar();

			ImGui::TableSetColumnIndex(1);
			ImGui::Text(parameterTrack->parameter->machine->getName());
			
			ImGui::TableSetColumnIndex(2);
			ImGui::Text(parameterTrack->parameter->name);

			ImGui::TableSetColumnIndex(3);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
			refreshSequence |= parameterTrack->interpolationTypeSelectorGui();

			ImGui::TableSetColumnIndex(4);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.5);
			refreshSequence |= parameterTrack->sequenceTypeSelectorGui();

			ImGui::TableSetColumnIndex(5);
			refreshSequence |= parameterTrack->chainPreviousTargetCheckboxGui();

			ImGui::TableSetColumnIndex(6);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			refreshSequence |= parameterTrack->originInputGui();

			ImGui::TableSetColumnIndex(7);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			refreshSequence |= parameterTrack->targetInputGui();

			ImGui::TableSetColumnIndex(8);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			refreshSequence |= parameterTrack->timeInputGui();

			ImGui::TableSetColumnIndex(9);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			refreshSequence |= parameterTrack->timeOffsetInputGui();

			ImGui::TableSetColumnIndex(10);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			refreshSequence |= parameterTrack->rampIntInputGui();

			ImGui::TableSetColumnIndex(11);
			refreshSequence |= parameterTrack->equalRampsCheckboxGui();

			ImGui::TableSetColumnIndex(12);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			refreshSequence |= parameterTrack->rampOutInputGui();

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


		if (removedSequence) removeTrack(removedSequence->parameter);
		else if (movedUpSequence) {
			int oldIndex;
			for (int i = 0; i < tracks.size(); i++) {
				if (tracks[i] == movedUpSequence) {
					oldIndex = i;
					break;
				}
			}
			int newIndex = oldIndex - 1;
			if (newIndex >= 0) {
				tracks.erase(tracks.begin() + oldIndex);
				tracks.insert(tracks.begin() + newIndex, movedUpSequence);
			}
		}
		else if (movedDownSequence) {
			int oldIndex;
			for (int i = 0; i < tracks.size(); i++) {
				if (tracks[i] == movedDownSequence) {
					oldIndex = i;
					break;
				}
			}
			int newIndex = oldIndex + 1;
			if (newIndex < tracks.size()) {
				tracks.erase(tracks.begin() + oldIndex);
				tracks.insert(tracks.begin() + newIndex, movedDownSequence);
			}
		}

		ImGui::EndTable();
	}

	
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

		ImPlot::EndPlot();
	}
	

}