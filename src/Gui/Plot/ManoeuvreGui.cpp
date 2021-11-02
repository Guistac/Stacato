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

void Manoeuvre::listGui() {

	ImGui::PushFont(Fonts::robotoBold20);
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
		ImGui::TableSetupColumn("Constraint");
		ImGui::TableSetupColumn("Time Offset");
		ImGui::TableSetupColumn("Ramp In");
		ImGui::TableSetupColumn("=");
		ImGui::TableSetupColumn("Ramp Out");

		ImGui::TableHeadersRow();

		std::shared_ptr<ParameterTrack> removedSequence = nullptr;
		std::shared_ptr<ParameterTrack> movedUpSequence = nullptr;
		std::shared_ptr<ParameterTrack> movedDownSequence = nullptr;

		for (auto& parameterSequence : tracks) {

			ImGui::PushID(parameterSequence->parameter->name);
			ImGui::PushID(parameterSequence->parameter->machine->getName());

			ImGui::TableNextRow(ImGuiTableRowFlags_None);

			ImGui::TableSetColumnIndex(0);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.1));
			if (buttonCross("##remove")) removedSequence = parameterSequence;
			ImGui::SameLine();
			bool disableMoveUp = parameterSequence == tracks.front();
			if(disableMoveUp) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::ArrowButton("##moveUp", ImGuiDir_Up)) movedUpSequence = parameterSequence;
			if(disableMoveUp) END_DISABLE_IMGUI_ELEMENT
			ImGui::SameLine();
			bool disableMoveDown = parameterSequence == tracks.back();
			if(disableMoveDown) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::ArrowButton("##moveDown", ImGuiDir_Down)) movedDownSequence = parameterSequence;
			if(disableMoveDown) END_DISABLE_IMGUI_ELEMENT
			ImGui::PopStyleVar();

			ImGui::TableSetColumnIndex(1);
			ImGui::Text(parameterSequence->parameter->machine->getName());
			
			ImGui::TableSetColumnIndex(2);
			ImGui::Text(parameterSequence->parameter->name);

			ImGui::TableSetColumnIndex(3);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
			parameterSequence->interpolationTypeSelectorGui();

			ImGui::TableSetColumnIndex(4);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.5);
			parameterSequence->sequenceTypeSelectorGui();

			ImGui::TableSetColumnIndex(5);
			parameterSequence->chainPreviousTargetCheckboxGui();

			ImGui::TableSetColumnIndex(6);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			parameterSequence->originInputGui();

			ImGui::TableSetColumnIndex(7);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			parameterSequence->targetInputGui();

			ImGui::TableSetColumnIndex(8);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			parameterSequence->constraintInputGui();

			ImGui::TableSetColumnIndex(9);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			parameterSequence->timeOffsetInputGui();

			ImGui::TableSetColumnIndex(10);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			parameterSequence->rampIntInputGui();

			ImGui::TableSetColumnIndex(11);
			parameterSequence->equalRampsCheckboxGui();

			ImGui::TableSetColumnIndex(12);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			parameterSequence->rampOutInputGui();			

			ImGui::PopID();
			ImGui::PopID();
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

	ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoChild;
	if (ImPlot::BeginPlot("##SequenceCurveDisplay", 0, 0, ImGui::GetContentRegionAvail(), plotFlags)) {

		ImPlot::EndPlot();
	}


}