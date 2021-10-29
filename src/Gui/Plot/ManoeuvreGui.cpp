#include <pch.h>

#include "Plot/Manoeuvre.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Framework/Fonts.h"
#include "Environnement/Environnement.h"
#include "Motion/Machine/Machine.h"
#include "Motion/Machine/AnimatableParameter.h"
#include "Motion/Machine/ParameterSequence.h"

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

	if (ImGui::BeginTable("##parameters", 9, ImGuiTableFlags_Borders)) {
	
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

		for (auto& parameterSequence : parameterSequences) {
			
			ImGui::PushID(parameterSequence->parameter->name);
			ImGui::PushID(parameterSequence->parameter->machine->getName());

			ImGui::TableNextRow();

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
			ImGui::TableSetColumnIndex(3);
			double dummyTarget = 0.0;
			ImGui::InputDouble("##target", &dummyTarget, 0.0, 0.0, "%.3f u");
			ImGui::TableSetColumnIndex(4);
			double dummyConstraint = 0.0;
			ImGui::InputDouble("##constraint", &dummyConstraint, 0.0, 0.0, "%.3f u");
			ImGui::TableSetColumnIndex(5);
			double dummyTimeOffset = 0.0;
			ImGui::InputDouble("##timeOffset", &dummyTimeOffset, 0.0, 0.0, "%.3f u");
			ImGui::TableSetColumnIndex(6);
			double dummyRampIn = 0.0;
			ImGui::InputDouble("##rampIn", &dummyRampIn, 0.0, 0.0, "%.3f u");
			ImGui::TableSetColumnIndex(7);
			bool dummyRampEqual = false;
			ImGui::Checkbox("##rampEqual", &dummyRampEqual);
			ImGui::TableSetColumnIndex(8);
			double dummyRampOut = 0.0;
			ImGui::InputDouble("##rampOut", &dummyRampOut, 0.0, 0.0, "%.3f u");

			/*
			machine
			parameter
			sequencetype
			target
			constraint(vel/time)
			offsettime
			rampIn
			rampEqual
			rampOut
			*/


			ImGui::PopID();
			ImGui::PopID();
			
		}


		ImGui::EndTable();
	}




}