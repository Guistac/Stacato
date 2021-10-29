#include <pch.h>

#include "Plot/Manoeuvre.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Framework/Fonts.h"
#include "Environnement/Environnement.h"
#include "Motion/Machine/Machine.h"
#include "Motion/Machine/AnimatableParameter.h"
#include "Plot/ParameterSequence.h"

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


	if (ImGui::Button("Add Parameter")) ImGui::OpenPopup("ManoeuvreParameterAdder");
	if (ImGui::BeginPopup("ManoeuvreParameterAdder")){

		for (auto& machine : Environnement::getMachines()) {
			if (ImGui::BeginMenu(machine->getName())) {
				for (auto& parameter : machine->animatableParameters) {
					bool isSelected = hasParameter(parameter);
					if (ImGui::MenuItem(parameter->name, nullptr, isSelected)) {
						if (!isSelected) addParameter(parameter);
						else removeParameter(parameter);
					}
				}
				ImGui::EndMenu();
			}
		}

		ImGui::EndPopup();
	}

	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit;

	if (ImGui::BeginTable("##parameters", 9, tableFlags)) {
	
		ImGui::TableSetupColumn("Machine");
		ImGui::TableSetupColumn("Parameter");
		ImGui::TableSetupColumn("Sequence Type");
		ImGui::TableSetupColumn("Target");
		ImGui::TableSetupColumn("Constraint");
		ImGui::TableSetupColumn("Time Offset");
		ImGui::TableSetupColumn("Ramp In");
		ImGui::TableSetupColumn("Copy Ramp");
		ImGui::TableSetupColumn("Ramp Out");
		ImGui::TableHeadersRow();

		if (parameterSequences.empty()) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("No Parameters");
		}

		for (auto& parameterSequence : parameterSequences) {

			ImGui::PushID(parameterSequence->parameter->name);
			ImGui::PushID(parameterSequence->parameter->machine->getName());

			ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeight()*5.0);

			ImGui::TableSetColumnIndex(0);
			ImGui::Text(parameterSequence->parameter->machine->getName());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(parameterSequence->parameter->name);
			ImGui::TableSetColumnIndex(2);
			if (ImGui::BeginCombo("##SequenceTypeSelector", getSequenceType(parameterSequence->type)->displayName)) {
				for (auto& sequenceType : getSequenceTypes()) {
					if (ImGui::Selectable(sequenceType.displayName, parameterSequence->type == sequenceType.type)) {
						parameterSequence->type = sequenceType.type;
						//TODO: Switch sequence type
					}
				}
				ImGui::EndCombo();
			}
			if(parameterSequence->type == SequenceType::Type::COMPLEX_ANIMATED_MOVE) BEGIN_DISABLE_IMGUI_ELEMENT
			ImGui::TableSetColumnIndex(3);
			ImGui::InputDouble("##target", &parameterSequence->target, 0.0, 0.0, "%.3f u");
			if (parameterSequence->type != SequenceType::Type::COMPLEX_ANIMATED_MOVE) {
				ImGui::TableSetColumnIndex(4);
				switch (parameterSequence->type) {
					case SequenceType::Type::SIMPLE_TIMED_MOVE:
						ImGui::InputDouble("##constraint", &parameterSequence->constraint, 0.0, 0.0, "%.3f s");
						break;
					case SequenceType::Type::SIMPLE_VELOCITY_MOVE:
						ImGui::InputDouble("##constraint", &parameterSequence->constraint, 0.0, 0.0, "%.3f u/s");
						break;
					default:
						break;
				}
				ImGui::TableSetColumnIndex(5);
				ImGui::InputDouble("##timeOffset", &parameterSequence->offsetTime, 0.0, 0.0, "%.3f s");
				ImGui::TableSetColumnIndex(6);
				ImGui::InputDouble("##rampIn", &parameterSequence->rampIn, 0.0, 0.0, "%.3f u");
				ImGui::TableSetColumnIndex(7);
				ImGui::Checkbox("##rampEqual", &parameterSequence->rampEqual);
				ImGui::TableSetColumnIndex(8);
				if (parameterSequence->rampEqual) BEGIN_DISABLE_IMGUI_ELEMENT
					ImGui::InputDouble("##rampOut", &parameterSequence->rampOut, 0.0, 0.0, "%.3f u");
				if (parameterSequence->rampEqual) END_DISABLE_IMGUI_ELEMENT
			}
			if(parameterSequence->type == SequenceType::Type::COMPLEX_ANIMATED_MOVE) END_DISABLE_IMGUI_ELEMENT

			ImGui::PopID();
			ImGui::PopID();
			
		}


		ImGui::EndTable();
	}

	ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoChild;
	if (ImPlot::BeginPlot("##SequenceCurveDisplay", 0, 0, ImGui::GetContentRegionAvail(), plotFlags)) {

		ImPlot::EndPlot();
	}


}